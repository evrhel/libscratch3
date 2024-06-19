#include "sound.hpp"

#include <cassert>

#include <mutil/mutil.h>

using namespace mutil;

static sf_count_t mem_get_filelen(void *user_data)
{
	SoundMemoryFile *file = static_cast<SoundMemoryFile *>(user_data);
	return file->size;
}

static sf_count_t mem_seek(sf_count_t offset, int whence, void *user_data)
{
	SoundMemoryFile *file = static_cast<SoundMemoryFile *>(user_data);
	switch (whence)
	{
	case SEEK_SET:
		file->pos = offset;
		break;
	case SEEK_CUR:
		file->pos += offset;
		break;
	case SEEK_END:
		file->pos = file->size + offset;
		break;
	default:
		return -1;
	}
	return file->pos;
}

static sf_count_t mem_read(void *ptr, sf_count_t count, void *user_data)
{
	SoundMemoryFile *file = static_cast<SoundMemoryFile *>(user_data);
	if (file->pos + count > file->size)
		count = file->size - file->pos;
	memcpy(ptr, file->data + file->pos, count);
	file->pos += count;
	return count;
}

static sf_count_t mem_tell(void *user_data)
{
	SoundMemoryFile *file = static_cast<SoundMemoryFile *>(user_data);
	return file->pos;
}

static SF_VIRTUAL_IO _sfVirtualIo = {
	mem_get_filelen,
	mem_seek,
	mem_read,
	nullptr,
	mem_tell
};

void DSPController::SetPitch(double pitch)
{
	// 2^(1/12)
	constexpr double kSemitone = 1.0594630943592952645618252949463;

	if (pitch < -360.0)
		pitch = -360.0;
	else if (pitch > 360.0)
		pitch = 360.0;

	_pitch = pitch;
	_resampleRatio = static_cast<float>(pow(kSemitone, pitch / 10)); // 10 units = 1 semitone
}

void Sound::Init(const SoundInfo *info, DSPController *dsp)
{
	SetString(_name, info->name);
	_dataFormat = info->dataFormat;
	_rate = info->rate;
	_sampleCount = info->sampleCount;

	_data.data = info->data;
	_data.size = info->dataSize;
	_data.pos = 0;

	_dsp = dsp;
}

void Sound::Load()
{
	_file = sf_open_virtual(&_sfVirtualIo, SFM_READ, &_info, &_data);
	if (!_file)
	{
		printf("Sound::Load: sf_open_virtual failed\n");
		Cleanup();
		return;
	}

	// defer stream creation until Play is called
}

void Sound::Play()
{
	if (!_stream)
	{
		PaError err;

		err = Pa_OpenDefaultStream(
			&_stream,
			0,
			_info.channels,
			paFloat32,
			_info.samplerate,
			BUFFER_LENGTH,
			paCallback,
			this);
		if (err != paNoError)
		{
			printf("Sound::Load: Pa_OpenDefaultStream failed: %s\n", Pa_GetErrorText(err));
			Cleanup();
			return;
		}
	}

	if (_isPlaying)
		(void)Pa_StopStream(_stream);

	_data.pos = 0;
	_isPlaying = true;

	PaError err = Pa_StartStream(_stream);
	if (err != paNoError)
		printf("Sound::Play: Pa_StartStream failed: %s\n", Pa_GetErrorText(err));
}

void Sound::Stop()
{
	if (!_stream)
		return;

	PaError err = Pa_StopStream(_stream);
	if (err != paNoError)
		printf("Sound::Stop: Pa_StopStream failed: %s\n", Pa_GetErrorText(err));

	_data.pos = 0;
	_isPlaying = false;
}

Sound::Sound() :
	_stream(nullptr),
	_isPlaying(false),
	_rate(0),
	_sampleCount(0),
	_data({}),
	_file(nullptr),
	_info({}),
	_dsp(nullptr)
{
	InitializeValue(_name);
}

Sound::~Sound()
{
	Cleanup();
}

void Sound::Cleanup()
{
	if (_stream)
		Pa_CloseStream(_stream), _stream = nullptr;

	if (_file)
		sf_close(_file), _file = nullptr;

	ReleaseValue(_name);
}

int Sound::paCallback(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo *timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	Sound *sound = static_cast<Sound *>(userData);
	float *out = static_cast<float *>(outputBuffer);
	DSPController *dsp = sound->_dsp;
	const float volume = dsp->GetVolumeMultiplier();
	const float resampleRatio = dsp->GetResampleRatio();
	const float panFactor = dsp->GetPanFactor();
	sf_count_t read;
	float tmp[BUFFER_LENGTH];

	(void)panFactor; // TODO: implement pan

	assert(framesPerBuffer == BUFFER_LENGTH);

	// we assume mono for now

	if (resampleRatio == 1.0f)
	{
		// no resampling, just copy
		read = sf_readf_float(sound->_file, out, BUFFER_LENGTH);
		if (read == 0)
		{
			sound->_isPlaying = false;
			return paComplete;
		}
	}
	else if (resampleRatio > 1.0f)
	{
		float srcPos = 0.0f;
		sf_count_t offset = 0;
		sf_count_t outIdx = 0;

		// total number of samples needed
		sf_count_t needed = static_cast<sf_count_t>(mutil::ceil(framesPerBuffer * resampleRatio));

		for (;;)
		{
			// clamp remaining samples to read to the buffer size
			sf_count_t toRead = needed - offset;
			if (toRead > BUFFER_LENGTH)
				toRead = BUFFER_LENGTH;

			read = sf_readf_float(sound->_file, tmp, toRead);
			if (read == 0)
			{
				sound->_isPlaying = false;
				return paComplete;
			}

			while (outIdx < BUFFER_LENGTH)
			{
				float pos = srcPos - offset;

				sf_count_t a = static_cast<sf_count_t>(pos);
				if (a >= read)
					break;

				sf_count_t b = a + 1;
				float frac = mutil::fract(pos); // position between a and b

				if (b >= read)
				{
					out[outIdx] = tmp[a]; // last sample
					outIdx = BUFFER_LENGTH; // break out of the loop
					break;
				}
				else // interpolate
					out[outIdx++] = mutil::lerp(tmp[a], tmp[b], frac);

				srcPos += resampleRatio; // increement source position
			}

			if (outIdx >= BUFFER_LENGTH)
				break;

			offset += read;
		}
	}
	else // < 1.0f
	{
		// number of samples needed
		sf_count_t floatsNeeded = static_cast<sf_count_t>(mutil::round(framesPerBuffer * resampleRatio));

		read = sf_readf_float(sound->_file, tmp, floatsNeeded);
		if (read == 0)
		{
			sound->_isPlaying = false;
			return paComplete;
		}

		// interpolate
		for (sf_count_t i = 0; i < BUFFER_LENGTH; i++)
		{
			float fpos = i * resampleRatio;
			float frac = mutil::fract(fpos);
			sf_count_t pos = static_cast<sf_count_t>(fpos);
			if (pos + 1 >= read)
			{
				out[i] = tmp[pos]; // last sample
				break;
			}
			else
				out[i] = mutil::lerp(tmp[pos], tmp[pos + 1], frac);
		}
	}

	return paContinue;
}
