#include "sound.hpp"

#include <cassert>

#include <sndfile.h>
#include <mutil/mutil.h>

using namespace mutil;

struct SoundMemoryFile
{
	const uint8_t *data; // Pointer to the data
	sf_count_t size; // Size of the data
	sf_count_t pos; // Current position
};

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

bool AbstractSound::Init(uint8_t *bytecode, size_t bytecodeSize, const bc::Sound *info, bool stream)
{
	if (_data)
	{
		printf("Sound::Init: already initialized\n");
		return false;
	}

	if (SetString(_name, (char *)(bytecode + info->name)).type != ValueType_String)
	{
		printf("Sound::Init: SetString failed\n");
		return false;
	}

	_data = bytecode + info->data;
	_dataSize = info->dataSize;

	_streamed = stream;

	return true;
}

bool AbstractSound::Load()
{
	if (_audioStream)
		return false; // already loaded, not an error

	if (_dataSize == 0)
	{
		_streamSize = 0;
		_frameCount = 0;
		_nChannels = 0;
		_sampleRate = 44100; // arbitrary, to prevent division by zero

		printf("Sound::Load: warning: empty sound (%s)\n", _name.u.string->str);
		return true;
	}

	SoundMemoryFile fileData;
	fileData.data = _data;
	fileData.size = _dataSize;
	fileData.pos = 0;

	SF_INFO info = {};
	SNDFILE *file = sf_open_virtual(&_sfVirtualIo, SFM_READ, &info, &fileData);
	if (!file)
	{
		printf("Sound::Load: sf_open_virtual failed (%s)\n", _name.u.string->str);
		return false;
	}

	_nChannels = info.channels;
	if (_nChannels > 2)
	{
		printf("Sound::Load: max 2 channels per audio stream (%s)\n", _name.u.string->str);
		sf_close(file);
		return false;
	}

	_frameCount = info.frames;
	_sampleRate = info.samplerate;

	_streamSize = _frameCount * _nChannels;
	_audioStream = new float[_streamSize];
	sf_count_t read = sf_readf_float(file, _audioStream, _frameCount);
	if (read != _frameCount)
	{
		printf("Sound::Load: sf_readf_float failed\n");
		delete[] _audioStream, _audioStream = nullptr;
		sf_close(file);
		return false;
	}

	sf_close(file);

	return true;
}

Voice *AbstractSound::CreateVoice()
{
	// TODO: implement
	return nullptr;
}

AbstractSound::AbstractSound() :
	_streamed(false),
	_data(nullptr), _dataSize(0),
	_streamSize(0), _audioStream(nullptr),
	_frameCount(0), _nChannels(0), _sampleRate(0),
	_voiceCount(0)
{
	InitializeValue(_name);
}

AbstractSound::~AbstractSound()
{
	Cleanup();
}

void AbstractSound::Cleanup()
{
	if (_voiceCount != 0)
	{
		printf("Sound::Cleanup: FATAL: voices still playing\n");
		abort();
	}

	if (_audioStream)
		delete[] _audioStream, _audioStream = nullptr;

	_streamed = false;

	ReleaseValue(_name);
}

void Voice::Play()
{
	if (!_sound->GetAudioStream())
	{
		printf("Voice::Play: sound not loaded\n");
		return;
	}

	PaError err;

	if (!_stream)
	{
		err = Pa_OpenDefaultStream(
			&_stream,
			0,
			_sound->GetChannelCount(),
			paFloat32,
			_sound->GetSampleRate(),
			BUFFER_LENGTH,
			_sound->GetChannelCount() == 1 ? paMonoCallback : paStereoCallback,
			this);
		if (err != paNoError)
		{
			printf("Voice::Play: Pa_OpenDefaultStream failed: %s\n", Pa_GetErrorText(err));
			return;
		}
	}

	if (_isPlaying)
	{
		_streamPos = 0;
		_sample = STEREO_SAMPLE{ 0.0f, 0.0f };
		return;
	}

	_streamPos = 0;
	_isPlaying = true;
	_sample = STEREO_SAMPLE{ 0.0f, 0.0f };

	err = Pa_StartStream(_stream);
	if (err != paNoError)
		printf("Voice::Play: Pa_StartStream failed: %s\n", Pa_GetErrorText(err));
}

void Voice::Stop()
{
	if (!_stream || !_isPlaying)
		return;

	Pa_StopStream(_stream);

	_streamPos = 0;
	_isPlaying = false;
	_sample = STEREO_SAMPLE{ 0.0f, 0.0f };
}

Voice::Voice() :
	_sound(nullptr),
	_stream(nullptr),
	_isPlaying(false),
	_streamPos(0),
	_sample({ 0.0f, 0.0f })
{

}

Voice::~Voice()
{
	Cleanup();
}

void Voice::Cleanup()
{
	if (_stream)
		Pa_CloseStream(_stream), _stream = nullptr;
}

int Voice::paMonoCallback(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo *timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	Voice *const voice = static_cast<Voice *>(userData);
	float *const out = static_cast<float *>(outputBuffer);

	const DSPController &dsp = voice->_dsp;
	const float volume = dsp.GetVolumeMultiplier(); // [0.0, 1.0]
	const float resampleRatio = dsp.GetResampleRatio(); // [0.0, inf)
	const float panFactor = dsp.GetPanFactor(); // [-1.0, 1.0]

	unsigned long read;
	float tmp[BUFFER_LENGTH];

	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;

	assert(framesPerBuffer == BUFFER_LENGTH);
	assert(voice->GetSound()->GetChannelCount() == 1);

	// TODO: write to a stereo buffer

	if (resampleRatio == 1.0f)
	{
		// no resampling, just copy
		read = voice->ReadFrames(out, BUFFER_LENGTH);
		if (read == 0)
		{
			voice->_isPlaying = false;
			return paComplete;
		}
	}
	else if (resampleRatio > 1.0f)
	{
		float srcPos = 0.0f;
		unsigned long offset = 0;
		unsigned long outIdx = 0;

		// total number of samples needed
		unsigned long needed = static_cast<unsigned long>(mutil::ceil(framesPerBuffer * resampleRatio));

		for (;;)
		{
			// clamp remaining samples to read to the buffer size
			sf_count_t toRead = needed - offset;
			if (toRead > BUFFER_LENGTH)
				toRead = BUFFER_LENGTH;

			read = voice->ReadFrames(tmp, toRead);
			if (read == 0)
			{
				voice->_isPlaying = false;
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

		read = voice->ReadFrames(tmp, floatsNeeded);
		if (read == 0)
		{
			voice->_isPlaying = false;
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

	float max = 0.0f;

	// apply volume
	for (unsigned long i = 0; i < BUFFER_LENGTH; i++)
	{
		out[i] *= volume;
		
		if (mutil::abs(out[i]) > mutil::abs(max))
			max = out[i]; // sample with largest amplitude
	}

	voice->_sample = STEREO_SAMPLE{ max, max };

	return paContinue;
}

int Voice::paStereoCallback(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo *timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	Voice *voice = static_cast<Voice *>(userData);
	STEREO_SAMPLE *stereoOut = static_cast<STEREO_SAMPLE *>(outputBuffer);

	const DSPController &dsp = voice->_dsp;
	const float volume = dsp.GetVolumeMultiplier(); // [0.0, 1.0]
	const float resampleRatio = dsp.GetResampleRatio(); // [0.0, inf)
	const float panFactor = dsp.GetPanFactor(); // [-1.0, 1.0]

	unsigned long read;
	float tmp[BUFFER_LENGTH];

	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;

	assert(framesPerBuffer == BUFFER_LENGTH);
	assert(voice->GetSound()->GetChannelCount() == 2);

	if (resampleRatio == 1.0f)
	{
		// no resampling, just copy
		read = voice->ReadFrames(stereoOut, BUFFER_LENGTH);
		if (read == 0)
		{
			voice->_isPlaying = false;
			return paComplete;
		}
	}

	// apply panning
	if (panFactor != 0)
	{
		for (unsigned long i = 0; i < BUFFER_LENGTH; i++)
		{
			STEREO_SAMPLE &sample = stereoOut[i];

			if (panFactor < 1)
			{
				sample.L *= panFactor;
				sample.R *= 1.0f - panFactor;
			}
			else if (panFactor > 1)
			{
				sample.L *= 1.0f - (panFactor - 1.0f);
				sample.R *= panFactor - 1.0f;
			}
		}
	}

	STEREO_SAMPLE max{ 0.0f, 0.0f };

	// apply volume
	for (unsigned long i = 0; i < BUFFER_LENGTH; i++)
	{
		STEREO_SAMPLE &sample = stereoOut[i];

		sample.L *= volume;
		sample.R *= volume;

		if (mutil::abs(sample.L) > mutil::abs(max.L))
			max.L = sample.L;
	
		if (mutil::abs(sample.R) > mutil::abs(max.R))
			max.R = sample.R;
	}

	voice->_sample = max;

	return paContinue;
}
