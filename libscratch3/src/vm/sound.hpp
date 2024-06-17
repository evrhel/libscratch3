#pragma once

#include <portaudio.h>
#include <sndfile.h>

#include "preload.hpp"
#include "memory.hpp"

struct SoundMemoryFile
{
	const char *data;
	sf_count_t size;
	sf_count_t pos;
};

class Sound final
{
public:
	constexpr const String *GetName() const { return _name.u.string; }

	void Init(const SoundInfo *info);

	void Load();

	// use VirtualMachine::PlaySound
	void Play();
	void Stop();
	bool IsPlaying() const;

	Sound &operator=(const Sound &) = delete;
	Sound &operator=(Sound &&) = delete;

	Sound();
	Sound(const Sound &) = delete;
	Sound(Sound &&) = delete;
	~Sound();
private:
	Value _name;

	PaStream *_stream;

	// data
	std::string _dataFormat;
	unsigned int _rate;
	unsigned int _sampleCount;
	SoundMemoryFile _data;

	SNDFILE *_file;
	SF_INFO _info;

	void Cleanup();

	static int paCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo *timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);
};
