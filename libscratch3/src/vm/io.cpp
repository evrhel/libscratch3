#include "io.hpp"

#include <imgui.h>
#include <imgui_impl_sdl2.h>

#include "vm.hpp"
#include "script.hpp"
#include "sprite.hpp"
#include "../render/renderer.hpp"

bool IOHandler::GetKey(const Value &key) const
{
	return false;
}

void IOHandler::ResetTimer()
{
	_timerStart = _vm->GetTime();
}

void IOHandler::PollEvents()
{
	GLRenderer *render = _vm->GetRenderer();
	if (!render)
		return;

	SDL_Window *window = render->GetWindow();

	_lastMouseDown = _mouseDown;

	int64_t clickX, clickY;
	SDL_Scancode keyEvents[SDL_NUM_SCANCODES];
	int keyEventCount = 0;

	SDL_Event evt;
	while (SDL_PollEvent(&evt))
	{
		ImGui_ImplSDL2_ProcessEvent(&evt);

		switch (evt.type)
		{
		case SDL_QUIT:
			_vm->VMTerminate();
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				_mouseDown = true;
				render->ScreenToStage(evt.button.x, evt.button.y, &clickX, &clickY);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (evt.button.button == SDL_BUTTON_LEFT)
				_mouseDown = false;
			break;
		case SDL_MOUSEMOTION:
			render->ScreenToStage(evt.motion.x, evt.motion.y, &_mouseX, &_mouseY);
			break;
		case SDL_KEYDOWN:
			if (!_keyStates[evt.key.keysym.scancode])
				_keysPressed++;
			_keyStates[evt.key.keysym.scancode] = true;
			keyEvents[keyEventCount++] = evt.key.keysym.scancode;
			break;
		case SDL_KEYUP:
			if (!_keyStates[evt.key.keysym.scancode])
				_keysPressed--;
			_keyStates[evt.key.keysym.scancode] = false;
			break;
		case SDL_WINDOWEVENT:
			if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
				render->Resize();
			break;
		}
	}

	// Dispatch click event
	if (_mouseDown && !_lastMouseDown)
		_vm->OnClick(clickX, clickY);

	// Dispatch key press events
	for (int i = 0; i < keyEventCount; i++)
		_vm->OnKeyDown(keyEvents[i]);
}

void IOHandler::RenderIO()
{
	if (_asker)
	{
		int width, height;
		SDL_GL_GetDrawableSize(_vm->GetRenderer()->GetWindow(), &width, &height);

		float padding = ImGui::GetStyle().WindowPadding.x;

		ImGui::SetNextWindowPos(ImVec2(width / 2, height - padding), ImGuiCond_Always, ImVec2(0.5, 1.0));
		ImGui::SetNextWindowSize(ImVec2(width - 2 * padding, 0), ImGuiCond_Always);

		if (ImGui::Begin("Input", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav))
		{
			ImGui::Text("%s is asking: %s", _asker->sprite->GetName().c_str(), _question.c_str());

			ImGui::PushItemWidth(ImGui::GetWindowWidth() - 2 * padding);
			if (ImGui::InputText("##input", _inputBuf, sizeof(_inputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				SetString(_answer, _inputBuf);
				_asker->askInput = false;
				_asker->state = RUNNABLE;
				_asker = nullptr;
				memset(_inputBuf, 0, sizeof(_inputBuf));

				_question.clear();
			}
			ImGui::PopItemWidth();
		}

		ImGui::End();
	}
}

void IOHandler::Release()
{
	ReleaseValue(_answer);
	ReleaseValue(_username);
	_question.clear();
}

IOHandler::IOHandler(VirtualMachine *vm) :
	_vm(vm),
	_mouseDown(false), _lastMouseDown(false),
	_mouseX(0), _mouseY(0),
	_clickX(0), _clickY(0),
	_keysPressed(0),
	_loudness(0),
	_timerStart(0),
	_asker(nullptr)
{
	memset(_keyStates, 0, sizeof(_keyStates));
	memset(_inputBuf, 0, sizeof(_inputBuf));

	InitializeValue(_answer);
	InitializeValue(_username);

	char buf[64];
	size_t len;
	len = ls_username(buf, sizeof(buf));
	if (len != -1)
		SetString(_username, buf, len);
}

IOHandler::~IOHandler()
{
	Release();
}
