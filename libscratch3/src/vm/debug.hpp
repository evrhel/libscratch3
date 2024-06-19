#pragma once

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
};
