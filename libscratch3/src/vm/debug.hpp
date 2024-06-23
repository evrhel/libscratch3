#pragma once

#define AUDIO_HISTOGRAM_SIZE 32
#define FPS_HISTOGRAM_SIZE 64

class VirtualMachine;

//! \brief Controls the in app debugger
class Debugger final
{
public:
	//! \brief Render the debug window
	void Render();

	Debugger &operator=(const Debugger &) = delete;
	Debugger &operator=(Debugger &&) = delete;

	Debugger(VirtualMachine *vm);
	Debugger(const Debugger &) = delete;
	Debugger(Debugger &&) = delete;
	~Debugger();
private:
	VirtualMachine *_vm;

	double _nextSampleTime;

	float _audioHistogramTimes[AUDIO_HISTOGRAM_SIZE];

	float _audioHistogramL[AUDIO_HISTOGRAM_SIZE];
	float _audioHistogramLMax, _audioHistogramLMin;
	float _audioHistogramR[AUDIO_HISTOGRAM_SIZE];
	float _audioHistogramRMax, _audioHistogramRMin;

	float _fpsHistogramTimes[FPS_HISTOGRAM_SIZE];
	float _fpsHistogram[FPS_HISTOGRAM_SIZE];
};
