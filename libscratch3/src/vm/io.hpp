#pragma once

#include <cstdint>
#include <string>

#include <SDL.h>

#include "memory.hpp"

class VirtualMachine;
struct Script;

//! \brief Manages various input/output operations
class IOHandler final
{
public:
	constexpr const Value &GetAnswer() const { return _answer; }
	constexpr bool IsMouseDown() const { return _mouseDown; }
	constexpr int64_t GetMouseX() const { return _mouseX; }
	constexpr int64_t GetMouseY() const { return _mouseY; }

	//! \brief Get the state of a key
	//!
	//! \param key The key to check, as an SDL scancode
	//!
	//! \return true if the key is pressed, false otherwise
	constexpr bool GetKey(int key) const { return _keyStates[key]; }

	//! \brief Get the state of a key
	//!
	//! \param key The key to check, as a string
	//!
	//! \return true if the key is pressed, false otherwise
	bool GetKey(const Value &key) const;

	constexpr int GetKeysPressed() const { return _keysPressed; }
	constexpr double GetLoudness() const { return _loudness; }
	constexpr const Value &GetUsername() const { return _username; }

	constexpr Script *GetAsker() const { return _asker; }
	constexpr void SetAsker(Script *asker) { _asker = asker; }
	inline void SetQuestion(const std::string &question) { _question = question; }

	//! \brief Reset the Scratch timer
	void ResetTimer();

	//! \brief Poll I/O events
	void PollEvents();

	//! \brief Render debug information
	void RenderIO();

	//! \brief Release the IO handler
	//!
	//! This may be called multiple times.
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

	double _timerStart; // Scratch timer epoch

	std::string _question;
	Script *_asker;
	char _inputBuf[512];
};
