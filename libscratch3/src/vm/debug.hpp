#pragma once

#define HISTOGRAM_SIZE 32

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

	float _histogramTimes[HISTOGRAM_SIZE];
	float _audioHistogram[HISTOGRAM_SIZE];
	float _audioHistogramMax, _audioHistogramMin;
};
