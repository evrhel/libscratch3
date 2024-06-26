#include "debug.hpp"

#include <imgui.h>
#include <implot.h>

#include "vm.hpp"
#include "io.hpp"
#include "sprite.hpp"

#include "../render/renderer.hpp"

void Debugger::Render()
{
	if (ImGui::Begin("Debug"))
	{
		if (ImGui::BeginTabBar("DebugTabs"))
		{
			GLRenderer *render = _vm->GetRenderer();
			auto &io = _vm->GetIO();

			SDL_Window *window = render->GetWindow();
			int width, height;
			SDL_GL_GetDrawableSize(window, &width, &height);

			if (ImGui::BeginTabItem("System"))
			{
				struct ls_meminfo mi;
				struct ls_cpuinfo ci;

				ls_get_meminfo(&mi);
				ls_get_cpuinfo(&ci);

				const char *archString;
				switch (ci.arch)
				{
				default:
					archString = "unknown";
					break;
				case LS_ARCH_AMD64:
					archString = "x86_64";
					break;
				case LS_ARCH_ARM:
					archString = "arm";
					break;
				case LS_ARCH_ARM64:
					archString = "arm64";
					break;
				case LS_ARCH_X86:
					archString = "x86";
					break;
				case LS_ARCH_IA64:
					archString = "ia64";
					break;
				}

				ImGui::SeparatorText("Host");
				ImGui::LabelText("Name", LS_OS);
				ImGui::LabelText("Architecture", "%s", archString);
				ImGui::LabelText("Processor Count", "%d", ci.num_cores);
				ImGui::LabelText("Total Physical", "%llu MiB", mi.total / 1024 / 1024);

				ImGui::SeparatorText("Target");
				ImGui::LabelText("Compiler", LS_COMPILER);
				ImGui::LabelText("Target Architecture", LS_ARCH);
				ImGui::LabelText("Build Date", __TIMESTAMP__);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Graphics"))
			{
				int left = render->GetLogicalLeft();
				int right = render->GetLogicalRight();
				int top = render->GetLogicalTop();
				int bottom = render->GetLogicalBottom();

				ImGui::SeparatorText("Performance");
				ImGui::LabelText("Framerate", "%.2f (%.0f ms)", render->GetFramerate(), render->GetDeltaTime() * 1000);

				if (ImPlot::BeginPlot("Framerate"))
				{
					for (int i = 0; i < FPS_HISTOGRAM_SIZE - 1; i++)
						_fpsHistogram[i] = _fpsHistogram[i + 1];
					_fpsHistogram[FPS_HISTOGRAM_SIZE - 1] = render->GetFramerate();

					ImPlot::SetupAxisLimits(ImAxis_X1, -FPS_HISTOGRAM_SIZE, 0, ImGuiCond_Always);
					ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 360, ImGuiCond_Always);

					ImPlot::SetupAxisTicks(ImAxis_X1, -FPS_HISTOGRAM_SIZE, 0, 9);
					ImPlot::SetupAxisTicks(ImAxis_Y1, 0, 360, 7);

					ImPlot::PlotBars("##fps", _fpsHistogramTimes, _fpsHistogram, FPS_HISTOGRAM_SIZE, 1.0);

					ImPlot::EndPlot();
				}

				ImGui::LabelText("Frame", "%d", render->GetFrame());
				ImGui::LabelText("Resolution", "%dx%d", width, height);
				ImGui::LabelText("Viewport Size", "%dx%d", right - left, top - bottom);
				ImGui::LabelText("Objects Drawn", "%d", render->GetObjectsDrawn());

				ImGui::SeparatorText("Device");
				ImGui::LabelText("OpenGL", "%s", glGetString(GL_VERSION));
				ImGui::LabelText("OpenGL Vendor", "%s", glGetString(GL_VENDOR));
				ImGui::LabelText("OpenGL Renderer", "%s", glGetString(GL_RENDERER));
				ImGui::LabelText("OpenGL Shading Language", "%s", glGetString(GL_SHADING_LANGUAGE_VERSION));
				ImGui::LabelText("Window Driver", "%s", SDL_GetVideoDriver(0));

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("I/O"))
			{
				ImGui::SeparatorText("Mouse");
				ImGui::LabelText("Mouse Down", "%s", io.IsMouseDown() ? "true" : "false");
				ImGui::LabelText("Mouse", "%lld, %lld", io.GetMouseX(), io.GetMouseY());

				ImGui::SeparatorText("Keyboard");
				ImGui::LabelText("Keys Pressed", "%d", io.GetKeysPressed());

				std::string keys;
				for (int i = 0; i < SDL_NUM_SCANCODES; i++)
				{
					if (io.GetKey(i))
					{
						if (keys.size() > 0)
							keys += ", ";
						keys += SDL_GetScancodeName((SDL_Scancode)i);
					}
				}
				ImGui::LabelText("Keys", "%s", keys.c_str());

				struct ls_timespec ts;
				ls_get_time(&ts);

				ImGui::SeparatorText("Timers");
				ImGui::LabelText("Timer", "%.2f", _vm->GetTimer());
				ImGui::LabelText("Year", "%d", ts.year);
				ImGui::LabelText("Month", "%d", ts.month);
				ImGui::LabelText("Date", "%d", ts.day);
				ImGui::LabelText("Day of Week", "%d", 4); // TODO: implement
				ImGui::LabelText("Hour", "%d", ts.hour);
				ImGui::LabelText("Minute", "%d", ts.minute);
				ImGui::LabelText("Second", "%d", ts.second);
				ImGui::LabelText("Days Since 2000", "%d", 0); // TODO: implement

				ImGui::SeparatorText("Sound");
				ImGui::LabelText("Loudness", "%.2f", io.GetLoudness());

				ImGui::SeparatorText("Other");
				ImGui::LabelText("Username", "%s", ToString(io.GetUsername()));
				ImGui::LabelText("Answer", "%s", ToString(io.GetAnswer()));

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Virtual Machine"))
			{
				int framerate = _vm->GetOptions().framerate;

				ImGui::SeparatorText("Information");
				ImGui::LabelText("Program Name", "%s", _vm->GetProgramName().c_str());
				ImGui::LabelText("Framerate", "%d Hz", framerate);

				constexpr double kNsToSec = 1.0 / 1000000000;

				ImGui::SeparatorText("Performance");
				ImGui::LabelText("Interpreter Time", "%.2f ms", (_vm->_interpreterTime * kNsToSec * 1000));
				ImGui::LabelText("Execution Rate", "%.0f kHz", 1 / (1000 * kNsToSec * _vm->_interpreterTime));
				ImGui::LabelText("Quota", "%.2f%%", (_vm->_interpreterTime * kNsToSec * framerate) * 100.0);
				ImGui::LabelText("Idle", "%.2f%%", (1.0 - _vm->_interpreterTime * kNsToSec * _vm->_render->GetFramerate()) * 100.0);

				ImGui::SeparatorText("Scheduler");
				ImGui::LabelText("Suspended", "%s", _vm->IsSuspended() ? "true" : "false");
				ImGui::LabelText("Time", "%.2f", _vm->GetTime());
				ImGui::LabelText("Script Count", "%zu", _vm->_scripts.size());
				ImGui::LabelText("Running", "%d", _vm->_activeScripts);
				ImGui::LabelText("Waiting", "%d", _vm->_waitingScripts);

				ImGui::SeparatorText("Global Variables");
				uint8_t *bytecode = _vm->GetBytecode();
				bc::Header *header = (bc::Header *)bytecode;
				bc::uint64 count = *(bc::uint64 *)(bytecode + header->rdata);
				Value *vars = (Value *)(bytecode + header->data);

				for (bc::uint64 i = 0; i < count; i++)
				{
					Value &v = vars[i];

					char name[32];
					snprintf(name, sizeof(name), "[%llu]", i);
					
					switch (v.type)
					{
					default:
						ImGui::LabelText(name, "<unknown>");
						break;
					case ValueType_None:
						ImGui::LabelText(name, "None");
						break;
					case ValueType_Integer:
						ImGui::LabelText(name, "%lld", v.u.integer);
						break;
					case ValueType_Real:
						ImGui::LabelText(name, "%g", v.u.real);
						break;
					case ValueType_Bool:
						ImGui::LabelText(name, "%s", v.u.boolean ? "true" : "false");
						break;
					case ValueType_String:
						ImGui::LabelText(name, "\"%s\"", v.u.string->str);
						break;
					case ValueType_List:
						ImGui::LabelText(name, "<list> (length: %lld)", v.u.list->len);
						break;
					case ValueType_IntPtr:
						ImGui::LabelText(name, "<intptr>: 0x%X", v.u.intptr);
						break;
					}
				}

				ImGui::SeparatorText("Control");

				if (ImGui::Button("Send Flag Clicked"))
					_vm->SendFlagClicked();

				if (_vm->IsSuspended())
				{
					if (ImGui::Button("Resume"))
						_vm->VMResume();
				}
				else
				{
					if (ImGui::Button("Suspend"))
						_vm->VMSuspend();
				}

				if (ImGui::Button("Terminate"))
					_vm->VMTerminate();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Sprites"))
			{
				ImGui::SeparatorText("Information");
				ImGui::LabelText("Sprite Count", "%d", (int)(_vm->_spritesEnd - _vm->_sprites - 1));

				ImGui::SeparatorText("Sprites");
				for (Sprite *s = _vm->_sprites; s < _vm->_spritesEnd; s++)
				{
					if (ImGui::CollapsingHeader(s->GetNameString()))
						s->DebugUI();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Scripts"))
			{
				static bool showRunning = true;
				static bool showWaiting = true;
				static bool showWaitingForScreen = true;
				static bool showSuspended = false;
				static bool showTerminated = false;
				static bool showEmbryo = false;

				ImGui::Checkbox("Running", &showRunning);
				
				ImGui::SameLine();
				ImGui::Checkbox("Waiting", &showWaiting);

				ImGui::SameLine();
				ImGui::Checkbox("Suspended", &showSuspended);

				ImGui::SameLine();
				ImGui::Checkbox("Terminated", &showTerminated);

				ImGui::SameLine();
				ImGui::Checkbox("Embryo", &showEmbryo);

				for (Script &script : _vm->_scripts)
				{
					switch (script.state)
					{
					default:
						abort();
						break;
					case EMBRYO:
						if (!showEmbryo)
							continue;
						break;
					case RUNNABLE:
					case RUNNING:
						if (!showRunning)
							continue;
						break;
					case WAITING:
						if (!showWaiting)
							continue;
						break;
					case SUSPENDED:
						if (!showSuspended)
							continue;
						break;
					case TERMINATED:
						if (!showTerminated)
							continue;
						break;
					}


					char name[128];
					snprintf(name, sizeof(name), "%p (%s)", &script, script.sprite->GetNameString());

					if (ImGui::CollapsingHeader(name))
					{
						double wakeup = script.sleepUntil ? script.sleepUntil - _vm->GetTime() : 0.0;
						if (wakeup < 0.0)
							wakeup = 0.0;

						ImGui::LabelText("State", "%s", GetStateName(script.state));
						ImGui::LabelText("Sprite", "%s", script.sprite->GetNameString());
						ImGui::LabelText("Wakeup", "%.2f", wakeup);
						ImGui::LabelText("Wait Input", script.waitInput ? "true" : "false");
						ImGui::LabelText("Ask Input", script.askInput ? "true" : "false");
						ImGui::LabelText("Sound Wait", script.waitSound ? script.waitSound->GetNameString() : "(none)");
					}
				}

				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem("Audio"))
			{
				static bool showPlaying = true;
				static bool showStopped = false;
				static bool showUnloaded = false;

				ImGui::SeparatorText("Information");

				ImGui::LabelText("Host Supports Audio", "%s", _vm->HasAudio() ? "true" : "false");
				ImGui::LabelText("Buffer Length", "%d", BUFFER_LENGTH);

				if (_vm->HasAudio())
				{
					const PaDeviceInfo *info = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());
					ImGui::LabelText("Output Device", "%s", info->name);
				}
				else
				{
					ImGui::LabelText("Output Device", "(none)");
				}

				STEREO_SAMPLE sample{ 0.0f, 0.0f };
				int loaded = 0;
				for (Sound *sound : _vm->GetSounds())
				{
					if (sound->IsLoaded())
						loaded++;
					sample.L += sound->GetCurrentSample().L;
					sample.R += sound->GetCurrentSample().R;
				}

				ImGui::LabelText("Sounds Loaded", "%d/%zu", loaded, _vm->GetSounds().size());

				float oldLMax = _audioHistogramLMax, oldRMax = _audioHistogramRMax;
				float oldLMin = _audioHistogramLMin, oldRMin = _audioHistogramRMin;

				// shift histogram
				if (_nextSampleTime < _vm->GetTime())
				{
					_nextSampleTime = _vm->GetTime() + (1.0 / 60.0);

					_audioHistogramLMax = -INFINITY, _audioHistogramRMax = -INFINITY;
					_audioHistogramLMin = INFINITY, _audioHistogramRMin = INFINITY;
					for (int i = 0; i < AUDIO_HISTOGRAM_SIZE - 1; i++)
					{
						float tmp = _audioHistogramL[i] = _audioHistogramL[i + 1];
						if (tmp > _audioHistogramLMax)
							_audioHistogramLMax = tmp;
						else if (tmp < _audioHistogramLMin)
							_audioHistogramLMin = tmp;

						tmp = _audioHistogramR[i] = _audioHistogramR[i + 1];
						if (tmp > _audioHistogramRMax)
							_audioHistogramRMax = tmp;
						else if (tmp < _audioHistogramRMin)
							_audioHistogramRMin = tmp;
					}

					_audioHistogramL[AUDIO_HISTOGRAM_SIZE - 1] = sample.L;
					if (sample.L > _audioHistogramLMax)
						_audioHistogramLMax = sample.L;
					else if (sample.L < _audioHistogramLMin)
						_audioHistogramLMin = sample.L;

					_audioHistogramR[AUDIO_HISTOGRAM_SIZE - 1] = sample.R;
					if (sample.R > _audioHistogramRMax)
						_audioHistogramRMax = sample.R;
					else if (sample.R < _audioHistogramRMin)
						_audioHistogramRMin = sample.R;
				}
				
				if (_audioHistogramLMax < oldLMax)
					_audioHistogramLMax = oldLMax * 0.99f;

				if (_audioHistogramLMin > oldLMin)
					_audioHistogramLMin = oldLMin * 0.99f;

				if (_audioHistogramRMax < oldRMax)
					_audioHistogramRMax = oldRMax * 0.99f;

				if (_audioHistogramRMin > oldRMin)
					_audioHistogramRMin = oldRMin * 0.99f;

				if (ImPlot::BeginPlot("Stream (L)", ImVec2(-1, 0), ImPlotFlags_None))
				{
					ImPlot::SetupAxisLimits(ImAxis_X1, -AUDIO_HISTOGRAM_SIZE, 0, ImGuiCond_Always);
					ImPlot::SetupAxisLimits(ImAxis_Y1, -1.0f, 1.0f, ImGuiCond_Always);

					ImPlot::SetupAxisTicks(ImAxis_X1, -AUDIO_HISTOGRAM_SIZE, 0, 9);

					ImPlot::SetupFinish();

					ImPlot::PlotLine("##histogramL", _audioHistogramTimes, _audioHistogramL, AUDIO_HISTOGRAM_SIZE);

					const float xs[] = { -AUDIO_HISTOGRAM_SIZE, 0.0f };
					const float hi[] = { _audioHistogramLMax, _audioHistogramLMax };
					const float lo[] = { _audioHistogramLMin, _audioHistogramLMin };

					ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(255, 0, 0, 255));
					ImPlot::PlotLine("##max", xs, hi, 2);
					ImPlot::PopStyleColor();

					ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(0, 255, 0, 255));
					ImPlot::PlotLine("##min", xs, lo, 2);
					ImPlot::PopStyleColor();

					ImPlot::EndPlot();
				}

				if (ImPlot::BeginPlot("Stream (R)", ImVec2(-1, 0), ImPlotFlags_None))
				{
					ImPlot::SetupAxisLimits(ImAxis_X1, -AUDIO_HISTOGRAM_SIZE, 0, ImGuiCond_Always);
					ImPlot::SetupAxisLimits(ImAxis_Y1, -1.0f, 1.0f, ImGuiCond_Always);

					ImPlot::SetupAxisTicks(ImAxis_X1, -AUDIO_HISTOGRAM_SIZE, 0, 9);

					ImPlot::SetupFinish();

					ImPlot::PlotLine("##histogramR", _audioHistogramTimes, _audioHistogramR, AUDIO_HISTOGRAM_SIZE);

					const float xs[] = { -AUDIO_HISTOGRAM_SIZE, 0.0f };
					const float hi[] = { _audioHistogramRMax, _audioHistogramRMax };
					const float lo[] = { _audioHistogramRMin, _audioHistogramRMin };

					ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(255, 0, 0, 255));
					ImPlot::PlotLine("##max", xs, hi, 2);
					ImPlot::PopStyleColor();

					ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(0, 255, 0, 255));
					ImPlot::PlotLine("##min", xs, lo, 2);
					ImPlot::PopStyleColor();

					ImPlot::EndPlot();
				}

				ImGui::SeparatorText("Sounds");

				ImGui::Checkbox("Playing", &showPlaying);
				ImGui::SameLine();

				ImGui::Checkbox("Stopped", &showStopped);
				ImGui::SameLine();

				ImGui::Checkbox("Unloaded", &showUnloaded);

				for (Sound *s : _vm->GetSounds())
				{
					if (s->IsLoaded())
					{
						if (s->IsPlaying() && !showPlaying)
							continue;

						if (!s->IsPlaying() && !showStopped)
							continue;
					}
					else if (!showUnloaded)
						continue;

					char name[128];
					snprintf(name, sizeof(name), "%p (%s)", s, s->GetNameString());

					if (ImGui::CollapsingHeader(name))
					{
						unsigned long pos = s->GetStreamPos();
						unsigned long size = s->GetSampleCount();
						int rate = s->GetSampleRate();

						double duration = s->GetDuration();
						double location = duration * pos / size;

						ImGui::LabelText("Name", "%s", s->GetNameString());

						ImGui::LabelText("Rate", "%d Hz", rate);

						int min = static_cast<int>(duration / 60);
						int sec = duration - min * 60;
						ImGui::LabelText("Duration", "%d:%02d (%.2f sec)", min, sec, duration);

						min = static_cast<int>(location / 60);
						sec = location - min * 60;
						ImGui::LabelText("Position", "%d:%02d (%.2f sec)", min, sec, location);

						ImGui::LabelText("Channels", "%d", s->GetChannelCount());

						ImGui::LabelText("Loaded", s->IsLoaded() ? "true" : "false");
						if (s->IsLoaded())
						{
							ImGui::LabelText("Playing", s->IsPlaying() ? "true" : "false");
							ImGui::LabelText("CPU", "%.2f%%", Pa_GetStreamCpuLoad(s->GetStream()) * 100.0);
						}
					}
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

Debugger::Debugger(VirtualMachine *vm) :
	_vm(vm)
{
	memset(_audioHistogramTimes, 0, sizeof(_audioHistogramTimes));
	for (int i = 0; i < AUDIO_HISTOGRAM_SIZE; i++)
		_audioHistogramTimes[i] = i - AUDIO_HISTOGRAM_SIZE + 1;

	memset(_audioHistogramL, 0, sizeof(_audioHistogramL));
	_audioHistogramLMax = 0.0f;
	_audioHistogramLMin = 0.0f;

	memset(_audioHistogramR, 0, sizeof(_audioHistogramR));
	_audioHistogramRMax = 0.0f;
	_audioHistogramRMin = 0.0f;

	memset(_fpsHistogramTimes, 0, sizeof(_fpsHistogramTimes));
	for (int i = 0; i < FPS_HISTOGRAM_SIZE; i++)
		_fpsHistogramTimes[i] = i - FPS_HISTOGRAM_SIZE + 1;

	memset(_fpsHistogram, 0, sizeof(_fpsHistogram));

	_nextSampleTime = 0.0;
}

Debugger::~Debugger()
{

}
