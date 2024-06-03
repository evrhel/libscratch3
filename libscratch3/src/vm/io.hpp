#pragma once

#include <cstdint>
#include <string>

#include <SDL.h>

#include "memory.hpp"

class VirtualMachine;
struct Script;

class IOHandler final
{
public:
	constexpr const Value &GetAnswer() const { return _answer; }
	constexpr bool IsMouseDown() const { return _mouseDown; }
	constexpr int64_t GetMouseX() const { return _mouseX; }
	constexpr int64_t GetMouseY() const { return _mouseY; }
	constexpr bool GetKey(int key) const { return _keyStates[key]; }
	bool GetKey(const Value &key) const;
	constexpr int GetKeysPressed() const { return _keysPressed; }
	constexpr double GetLoudness() const { return _loudness; }
	constexpr const Value &GetUsername() const { return _username; }

	constexpr Script *GetAsker() const { return _asker; }
	constexpr void SetAsker(Script *asker) { _asker = asker; }
	inline void SetQuestion(const std::string &question) { _question = question; }

	void ResetTimer();

	void PollEvents();

	void RenderIO();

	void Release();

	IOHandler &operator=(const IOHandler &) = delete;
	IOHandler &operator=(IOHandler &&) = delete;

	IOHandler(VirtualMachine *vm);
	IOHandler(const IOHandler &) = delete;
	IOHandler(IOHandler &&) = delete;
	~IOHandler();
private:
	VirtualMachine *_vm;

	Value _answer;
	bool _mouseDown, _lastMouseDown;
	int64_t _mouseX, _mouseY;
	int64_t _clickX, _clickY;
	bool _keyStates[SDL_NUM_SCANCODES];
	int _keysPressed;
	double _loudness;
	Value _username;

	double _timerStart;

	std::string _question;
	Script *_asker;
	char _inputBuf[512];
};
