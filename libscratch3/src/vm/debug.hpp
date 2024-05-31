#pragma once

class VirtualMachine;

class Debugger final
{
public:
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
