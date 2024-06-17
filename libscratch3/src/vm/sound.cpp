#include "sound.hpp"

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

void Sound::Init(const SoundInfo *info)
{
	SetString(_name, info->name);
	_dataFormat = info->dataFormat;
	_rate = info->rate;
	_sampleCount = info->sampleCount;

	_data.data = (const char *)info->data;
	_data.size = info->dataSize;
	_data.pos = 0;
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

	PaError err;

	err = Pa_OpenDefaultStream(
		&_stream,
		0,
		_info.channels,
		paFloat32,
		_info.samplerate,
		paFramesPerBufferUnspecified,
		paCallback,
		this);
	if (err != paNoError)
	{
		printf("Sound::Load: Pa_OpenDefaultStream failed: %s\n", Pa_GetErrorText(err));
		Cleanup();
		return;
	}
}

void Sound::Play()
{
	if (!_stream)
		return;

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
}

bool Sound::IsPlaying() const
{
	if (!_stream)
		return false;
	return Pa_IsStreamActive(_stream) == 1;
}

Sound::Sound() :
	_stream(nullptr),
	_rate(0),
	_sampleCount(0),
	_data({}),
	_file(nullptr),
	_info({})
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
	sf_count_t read;

	read = sf_readf_float(sound->_file, static_cast<float *>(outputBuffer), framesPerBuffer);
	if (read < framesPerBuffer)
		return paComplete;

	return paContinue;
}
