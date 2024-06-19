#pragma once

#include <list>

#include <portaudio.h>
#include <sndfile.h>

#include "preload.hpp"
#include "memory.hpp"

#define BUFFER_LENGTH 512

struct SoundMemoryFile
{
	const uint8_t *data;
	sf_count_t size;
	sf_count_t pos;
};

class DSPController final
{
public:
	constexpr void SetVolume(double volume)
	{
		if (volume < 0.0)
			volume = 0.0;
		else if (volume > 100.0)
			volume = 100.0;

		_volume = volume;
		_volumeMultiplier = static_cast<float>(volume / 100.0);
	}

	constexpr double GetVolume() const { return _volume; }
	constexpr float GetVolumeMultiplier() const { return _volumeMultiplier; }

	void SetPitch(double pitch);
	constexpr double GetPitch() const { return _pitch; }
	constexpr float GetResampleRatio() const { return _resampleRatio; }

	constexpr void SetPan(double pan)
	{
		if (pan < -100.0)
			pan = -100.0;
		else if (pan > 100.0)
			pan = 100.0;

		_pan = pan;
		_panFactor = static_cast<float>(pan / 100.0);
	}

	constexpr double GetPan() const { return _pan; }
	constexpr float GetPanFactor() const { return _panFactor; }

	DSPController() = default;
	~DSPController() = default;
private:
	double _volume = 100.0;
	float _volumeMultiplier = 1.0f;

	double _pitch = 0.0;
	float _resampleRatio = 1.0f;

	double _pan = 0.0;
	float _panFactor = 0.0f;
};

class Sound final
{
public:
	constexpr const String *GetName() const { return _name.u.string; }
	constexpr const char *GetNameString() const { return _name.u.string->str; }

	void Init(const SoundInfo *info, DSPController *dsp);

	void Load();

	constexpr bool IsLoaded() const { return _stream != nullptr; }

	// use VirtualMachine::PlaySound
	void Play();
	void Stop();
	constexpr bool IsPlaying() const { return _isPlaying; }

	// used by debuggers
	constexpr PaStream *GetStream() const { return _stream; }

	constexpr const SoundMemoryFile &GetData() const { return _data; }

	constexpr const std::string &GetFormat() const { return _dataFormat; }

	constexpr double GetRate() const { return _rate; }
	constexpr unsigned int GetSampleCount() const { return _sampleCount; }
	constexpr double GetDuration() const { return static_cast<double>(_sampleCount) / _rate; }

	Sound &operator=(const Sound &) = delete;
	Sound &operator=(Sound &&) = delete;

	Sound();
	Sound(const Sound &) = delete;
	Sound(Sound &&) = delete;
	~Sound();
private:
	Value _name;

	PaStream *_stream;
	bool _isPlaying;

	// data
	std::string _dataFormat;
	double _rate;
	unsigned int _sampleCount;
	SoundMemoryFile _data;

	SNDFILE *_file;
	SF_INFO _info;

	DSPController *_dsp;

	void Cleanup();

	static int paCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo *timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);
};
