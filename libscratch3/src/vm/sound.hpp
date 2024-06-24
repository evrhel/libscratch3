#pragma once

#include <list>

#include <portaudio.h>

#include "preload.hpp"
#include "memory.hpp"

#include "../codegen/util.hpp"

#define BUFFER_LENGTH 512

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

typedef float MONO_SAMPLE;
struct STEREO_SAMPLE
{
	float L, R;
};

class Voice;

//! \brief Represents a sound.
class AbstractSound final
{
public:
	constexpr const String *GetName() const { return _name.u.string; }
	constexpr const char *GetNameString() const { return _name.u.string->str; }

	//! \brief Initialize the sound.
	//!
	//! \param bytecode The program bytecode.
	//! \param bytecodeSize The size of the bytecode.
	//! \param info The sound information, as loaded from the bytecode.
	//! \param streamed Whether the sound is streamed.
	//! 
	//! \return Whether the sound was successfully initialized.
	bool Init(uint8_t *bytecode, size_t bytecodeSize, const bc::Sound *info, bool streamed);

	//! \brief Load the sound data.
	//! 
	//! \return Whether the sound was successfully loaded.
	bool Load();

	Voice *CreateVoice();

	constexpr size_t GetStreamSize() const { return _streamSize; }
	constexpr const float *GetAudioStream() const { return _audioStream; }
	constexpr unsigned long GetFrameCount() const { return _frameCount; }
	constexpr double GetDuration() const { return static_cast<double>(_frameCount) / _sampleRate; }
	constexpr int GetChannelCount() const { return _nChannels; }
	constexpr int GetSampleRate() const { return _sampleRate; }

	constexpr size_t GetVoiceCount() const { return _voiceCount; }

	AbstractSound &operator=(const AbstractSound &) = delete;
	AbstractSound &operator=(AbstractSound &&) = delete;

	AbstractSound();
	AbstractSound(const AbstractSound &) = delete;
	AbstractSound(AbstractSound &&) = delete;
	~AbstractSound();
private:
	Value _name;

	bool _streamed; // true if the sound is streamed

	// data
	uint8_t *_data; // full audio data
	uint64_t _dataSize; // size of the data

	// stream
	size_t _streamSize; // size of the stream buffer (in bytes)
	float *_audioStream; // full audio stream
	unsigned long _frameCount; // number of frames
	int _nChannels; // number of channels
	int _sampleRate; // sample rate

	size_t _voiceCount; // number of voices playing this sound

	//! \brief Release resources.
	void Cleanup();
};

class Voice final
{
public:
	constexpr AbstractSound *GetSound() const { return _sound; }
	constexpr DSPController *GetDSP() { return &_dsp; }
	constexpr unsigned long GetStreamPos() const { return _streamPos; }

	void Play();
	void Stop();

	constexpr const STEREO_SAMPLE &GetSample() const { return _sample; }

	Voice &operator=(const Voice &) = delete;
	Voice &operator=(Voice &&) = delete;

	Voice();
	Voice(const Voice &) = delete;
	Voice(Voice &&) = delete;
	~Voice();
private:
	AbstractSound *_sound;
	DSPController _dsp;

	PaStream *_stream;
	bool _isPlaying;

	unsigned long _streamPos;

	STEREO_SAMPLE _sample;

	//! \brief Read samples from the audio stream.
	//! 
	//! \param buffer The buffer to read into.
	//! \param frames The number of frames to read.
	//! 
	//! \return The number of frames read.
	inline unsigned long ReadFrames(void *buffer, unsigned long frames)
	{
		const float *const stream = _sound->GetAudioStream();
		const unsigned long nFrames = _sound->GetFrameCount();
		const int nChannels = _sound->GetChannelCount();

		unsigned long framesToRead = frames;
		if (_streamPos + framesToRead > nFrames)
			framesToRead = nFrames - _streamPos;

		if (framesToRead > 0)
		{
			memcpy(buffer, stream + _streamPos * nChannels, framesToRead * nChannels * sizeof(float));
			_streamPos += framesToRead;
		}

		return framesToRead;
	}

	void Cleanup();

	static int paMonoCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo *timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);

	static int paStereoCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo *timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);
};
