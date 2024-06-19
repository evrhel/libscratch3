#pragma once

#include <list>

#include <portaudio.h>
#include <sndfile.h>

#include "preload.hpp"
#include "memory.hpp"

#define BUFFER_LENGTH 512

struct SoundMemoryFile
{
	const uint8_t *data; // Pointer to the data
	sf_count_t size; // Size of the data
	sf_count_t pos; // Current position
};

//! \brief Handles DSP effects.
//!
//! Used by sprites to adjust audio playback properties such as volume,
//! pitch, and panning.
class DSPController final
{
public:
	//! \brief Set the volume.
	//!
	//! \param volume The volume, clamped to the range [0.0, 100.0].
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

	//! \brief Set the pitch.
	//!
	//! 10 units of pitch corresponds to one semitone, meaning a
	//! pitch of 120 corresponds to an octave higher.
	//!
	//! \param pitch The pitch, clamped to the range [-360.0, 360.0].
	void SetPitch(double pitch);

	constexpr double GetPitch() const { return _pitch; }

	//! \brief Return the resampling ratio.
	//!
	//! The resampling ratio is used to adjust the playback speed of
	//! the sound to match the pitch. The ratio is calculated as
	//! (2^(1/12))^(pitch/10). Values greater than 1.0 speed up the
	//! sound, while values less than 1.0 slow it down.
	constexpr float GetResampleRatio() const { return _resampleRatio; }

	//! \brief Set the pan amount.
	//!
	//! Panning is used to adjust the stereo balance of the playback. A
	//! factor of -100.0 corresponds to full left, 0.0 to center, and
	//! 100.0 to full right.
	//!
	//! \param pan The amount of panning, clamped to the range [-100.0, 100.0].
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

//! \brief Represents a sound.
class Sound final
{
public:
	constexpr const String *GetName() const { return _name.u.string; }
	constexpr const char *GetNameString() const { return _name.u.string->str; }

	//! \brief Initialize the sound.
	//!
	//! \param info The sound information, as loaded from the bytecode.
	//! \param dsp The DSP controller.
	void Init(const SoundInfo *info, DSPController *dsp);

	//! \brief Load the sound data.
	//!
	//! Audio stream creation is deferred until Play() is called for
	//! the first time. This will merely set up the data for playback.
	void Load();

	constexpr bool IsLoaded() const { return _stream != nullptr; }

	// use VirtualMachine::PlaySound
	void Play();

	//! \brief Stop audio playback.
	//!
	//! Stops the audio playback and resets the stream to the
	//! beginning. The function may briefly block until the stream has
	//! fully finished playing its buffer.
	void Stop();
	
	//! \brief Check if the sound is currently playing.
	//!
	//! \return true if the sound is playing, false otherwise.
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

	PaStream *_stream; // PortAudio stream
	bool _isPlaying; // true if the sound is currently playing

	// data
	std::string _dataFormat; // "wav", "mp3", etc.
	double _rate; // sample rate, in Hz
	unsigned int _sampleCount; // number of samples (duration = sampleCount / rate)
	SoundMemoryFile _data; // audio data

	// for libsndfile
	SNDFILE *_file;
	SF_INFO _info;

	DSPController *_dsp;

	//! \brief Release resources.
	void Cleanup();

	//! \brief PortAudio callback function.
	static int paCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo *timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);
};
