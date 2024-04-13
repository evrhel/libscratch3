#include "vm.hpp"

#include <cassert>
#include <cstdio>

static const char TRUE[4] = {
	't', 'r', 'u', 'e'
};

static const char FALSE[5] = {
	'f', 'a', 'l', 's', 'e'
};

class Executor : public Visitor
{
public:

	//
	/////////////////////////////////////////////////////////////////
	// Expressions
	//

	virtual void Visit(Constexpr *node)
	{
		vm->SetParsedString(vm->Push(), node->value);
	}

	virtual void Visit(XPos *node)
	{
		vm->SetReal(vm->Push(), script->sprite->x);
	}

	virtual void Visit(YPos *node)
	{
		vm->SetReal(vm->Push(), script->sprite->y);
	}

	virtual void Visit(Direction *node)
	{
		vm->SetReal(vm->Push(), script->sprite->direction);
	}

	virtual void Visit(CurrentCostume *node)
	{
		Value &val = vm->Push();

		switch (node->type)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case PropGetType_Number:
			vm->SetInteger(val, script->sprite->costume);
			break;
		case PropGetType_Name:
			// TODO: implement
			vm->SetString(val, "costume1"); // always costume1
			break;
		}
	}

	virtual void Visit(CurrentBackdrop *node) {}

	virtual void Visit(Size *node)
	{
		vm->SetReal(vm->Push(), script->sprite->size);
	}

	virtual void Visit(Volume *node)
	{
		vm->SetReal(vm->Push(), script->sprite->volume);
	}

	virtual void Visit(Touching *node)
	{
		// TODO: implement
		vm->SetBool(vm->Push(), false);
	}

	virtual void Visit(TouchingColor *node)
	{
		// TODO: implement
		vm->SetBool(vm->Push(), false);
	}

	virtual void Visit(ColorTouching *node)
	{
		// TODO: implement
		vm->SetBool(vm->Push(), false);
	}

	virtual void Visit(DistanceTo *node)
	{
		// TODO: implement
		vm->SetReal(vm->Push(), INFINITY);
	}

	virtual void Visit(Answer *node)
	{
		vm->Assign(vm->Push(), vm->GetAnswer());
	}

	virtual void Visit(KeyPressed *node)
	{
		// TODO: implement
		vm->SetBool(vm->Push(), false);
	}

	virtual void Visit(MouseDown *node)
	{
		vm->SetBool(vm->Push(), vm->GetMouseDown());
	}

	virtual void Visit(MouseX *node)
	{
		vm->SetInteger(vm->Push(), vm->GetMouseX());
	}

	virtual void Visit(MouseY *node)
	{
		vm->SetInteger(vm->Push(), vm->GetMouseY());
	}

	virtual void Visit(Loudness *node)
	{
		vm->SetReal(vm->Push(), vm->GetLoudness());
	}

	virtual void Visit(TimerValue *node)
	{
		vm->SetReal(vm->Push(), vm->GetTimer());
	}

	virtual void Visit(PropertyOf *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(CurrentDate *node)
	{
		Value &val = vm->Push();

		struct ls_timespec ts;
		ls_get_local_time(&ts);

		switch (node->format)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case DateFormat_Year:
			vm->SetInteger(val, ts.year);
			break;
		case DateFormat_Month:
			vm->SetInteger(val, ts.month);
			break;
		case DateFormat_Date:
			vm->SetInteger(val, ts.day);
			break;
		case DateFormat_DayOfWeek:
			// TODO: implement
			vm->SetInteger(val, 4); // always Thursday
			break;
		case DateFormat_Hour:
			vm->SetInteger(val, ts.hour);
			break;
		case DateFormat_Minute:
			vm->SetInteger(val, ts.minute);
			break;
		case DateFormat_Second:
			vm->SetInteger(val, ts.second);
			break;
		}
	}

	virtual void Visit(DaysSince2000 *node)
	{
		// TODO: implement
		vm->SetReal(vm->Push(), 0.0);
	}

	virtual void Visit(Username *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(Add *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = vm->ToReal(lhs) + vm->ToReal(rhs);

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(Sub *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = vm->ToReal(lhs) - vm->ToReal(rhs);

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(Mul *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = vm->ToReal(lhs) * vm->ToReal(rhs);

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(Div *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = vm->ToReal(lhs) / vm->ToReal(rhs);

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(Random *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &from = vm->StackAt(1);
		Value &to = vm->StackAt(0);

		if (from.type == ValueType_Real || to.type == ValueType_Real)
		{
			double f = vm->ToReal(from);
			double t = vm->ToReal(to);

			if (t < f) // swap
			{
				double temp = f;
				f = t;
				t = temp;
			}

			vm->Pop();
			vm->Pop();

			double r = f + (t - f) * (rand() / (double)RAND_MAX);

			vm->SetReal(vm->Push(), r);
		}
		else
		{
			int64_t f = vm->ToInteger(from);
			int64_t t = vm->ToInteger(to);

			if (t < f) // swap
			{
				int64_t temp = f;
				f = t;
				t = temp;
			}

			vm->Pop();
			vm->Pop();

			int64_t r = f + (rand() % (t - f + 1));

			vm->SetInteger(vm->Push(), r);
		}
	}

	virtual void Visit(Greater *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		double lhs = vm->ToReal(vm->StackAt(1));
		double rhs = vm->ToReal(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetBool(vm->Push(), lhs > rhs);
	}

	virtual void Visit(Less *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);

		double lhs = vm->ToReal(vm->StackAt(1));
		double rhs = vm->ToReal(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetBool(vm->Push(), lhs < rhs);
	}

	virtual void Visit(Equal *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
	
		bool equal = vm->Equals(vm->StackAt(1), vm->StackAt(0));

		vm->Pop();
		vm->Pop();
		
		vm->SetBool(vm->Push(), equal);
	}

	virtual void Visit(LogicalAnd *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		bool lhs = vm->Truth(vm->StackAt(1));
		bool rhs = vm->Truth(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetBool(vm->Push(), lhs && rhs);
	}

	virtual void Visit(LogicalOr *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);

		bool lhs = vm->Truth(vm->StackAt(1));
		bool rhs = vm->Truth(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetBool(vm->Push(), lhs || rhs);
	}

	virtual void Visit(LogicalNot *node)
	{
		node->e->Accept(this);

		bool truth = vm->Truth(vm->StackAt(0));

		vm->Pop();

		vm->SetBool(vm->Push(), !truth);
	}

	virtual void Visit(Concat *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(CharAt *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(StringLength *node)
	{
		// TODO: implement
		vm->SetInteger(vm->Push(), 0);
	}

	virtual void Visit(StringContains *node)
	{
		// TODO: implement
		vm->SetBool(vm->Push(), false);
	}

	virtual void Visit(Mod *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);

		double lhs = vm->ToReal(vm->StackAt(1));
		double rhs = vm->ToReal(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), fmod(lhs, rhs));
	}

	virtual void Visit(Round *node)
	{
		node->e->Accept(this);

		double val = vm->ToReal(vm->StackAt(0));

		vm->Pop();

		vm->SetInteger(vm->Push(), static_cast<int64_t>(round(val)));
	}

	virtual void Visit(MathFunc *node)
	{
		// TODO: implement
		vm->SetReal(vm->Push(), 0.0);
	}

	virtual void Visit(VariableExpr *node)
	{
		// TODO: replace with id
		Value &var = vm->FindVariable(node->name);
		vm->Assign(vm->Push(), var);
	}

	virtual void Visit(BroadcastExpr *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(ListExpr *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(ListAccess *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(IndexOf *node)
	{
		// TODO: implement
		vm->SetInteger(vm->Push(), 0);
	}

	virtual void Visit(ListLength *node)
	{
		// TODO: implement
		vm->SetInteger(vm->Push(), 0);
	}

	virtual void Visit(ListContains *node)
	{
		// TODO: implement
		vm->SetBool(vm->Push(), false);
	}

	//
	/////////////////////////////////////////////////////////////////
	// Statements
	//

	virtual void Visit(StatementList *node) {}
	virtual void Visit(MoveSteps *node) {}

	virtual void Visit(TurnDegrees *node)
	{
		node->e->Accept(this);
		script->sprite->direction += vm->ToReal(vm->StackAt(0));
		printf("%s.direction = %g\n", script->sprite->name.c_str(), script->sprite->direction);
		vm->Pop();
	}

	virtual void Visit(TurnNegDegrees *node) {}
	virtual void Visit(Goto *node) {}
	virtual void Visit(GotoXY *node) {}
	virtual void Visit(Glide *node) {}
	virtual void Visit(GlideXY *node) {}
	virtual void Visit(PointDir *node) {}
	virtual void Visit(PointTowards *node) {}
	virtual void Visit(ChangeX *node) {}

	virtual void Visit(SetX *node)
	{
		node->e->Accept(this);
		script->sprite->x = vm->ToReal(vm->StackAt(0));
		printf("%s.x = %g\n", script->sprite->name.c_str(), script->sprite->x);
		vm->Pop();
	}

	virtual void Visit(ChangeY *node) {}
	virtual void Visit(SetY *node) {}
	virtual void Visit(BounceIfOnEdge *node) {}
	virtual void Visit(SetRotationStyle *node) {}

	virtual void Visit(SayForSecs *node)
	{
		node->e1->Accept(this); // message
		node->e2->Accept(this); // duration

		Value &message = vm->StackAt(1);
		Value &duration = vm->StackAt(0);

		std::string mstr = vm->ToString(message);
		double secs = vm->ToReal(duration);

		vm->Pop();
		vm->Pop();

		printf("%s saying \"%s\" for %g secs\n",
			script->sprite->name.c_str(),
			mstr.c_str(),
			secs);

		if (mstr.size())
			script->sprite->messageState = MESSAGE_STATE_SAY;
		else
			script->sprite->messageState = MESSAGE_STATE_NONE;

		vm->Sleep(secs);
	}

	virtual void Visit(Say *node)
	{
		node->e->Accept(this); // message

		Value &message = vm->StackAt(0);

		std::string mstr = vm->ToString(message);

		vm->Pop();

		printf("%s saying \"%s\"\n",
			script->sprite->name.c_str(),
			mstr.c_str());

		if (mstr.size())
			script->sprite->messageState = MESSAGE_STATE_SAY;
		else
			script->sprite->messageState = MESSAGE_STATE_NONE;
	}

	virtual void Visit(ThinkForSecs *node) {}
	virtual void Visit(Think *node) {}
	virtual void Visit(SwitchCostume *node) {}
	virtual void Visit(NextCostume *node) {}
	virtual void Visit(SwitchBackdrop *node) {}
	virtual void Visit(SwitchBackdropAndWait *node) {}
	virtual void Visit(NextBackdrop *node) {}
	virtual void Visit(ChangeSize *node) {}
	virtual void Visit(SetSize *node) {}
	virtual void Visit(ChangeGraphicEffect *node) {}

	virtual void Visit(SetGraphicEffect *node)
	{
		node->e->Accept(this);

		double val = vm->ToReal(vm->StackAt(0));

		vm->Pop();

		auto &g = script->sprite->graphics;
		switch (node->effect)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case GraphicEffect_Color:
			g.color = val;
			break;
		case GraphicEffect_Fisheye:
			g.fisheye = val;
			break;
		case GraphicEffect_Whirl:
			g.whirl = val;
			break;
		case GraphicEffect_Pixelate:
			g.pixelate = val;
			break;
		case GraphicEffect_Mosaic:
			g.mosaic = val;
			break;
		case GraphicEffect_Brightness:
			g.brightness = val;
			break;
		case GraphicEffect_Ghost:
			g.ghost = val;
			break;
		}
	}

	virtual void Visit(ClearGraphicEffects *node)
	{
		memset(&script->sprite->graphics, 0, sizeof(script->sprite->graphics));
	}

	virtual void Visit(ShowSprite *node)
	{
		script->sprite->visible = true;
	}

	virtual void Visit(HideSprite *node)
	{
		script->sprite->visible	= false;
	}

	virtual void Visit(GotoLayer *node) {}
	virtual void Visit(MoveLayer *node) {}
	virtual void Visit(PlaySoundUntilDone *node) {}
	virtual void Visit(StartSound *node) {}
	virtual void Visit(StopAllSounds *node) {}
	virtual void Visit(ChangeSoundEffect *node) {}
	virtual void Visit(SetSoundEffect *node) {}
	virtual void Visit(ClearSoundEffects *node) {}
	virtual void Visit(ChangeVolume *node) {}
	virtual void Visit(SetVolume *node) {}
	virtual void Visit(OnFlagClicked *node) {}
	virtual void Visit(OnKeyPressed *node) {}
	virtual void Visit(OnSpriteClicked *node) {}
	virtual void Visit(OnStageClicked *node) {}
	virtual void Visit(OnBackdropSwitch *node) {}
	virtual void Visit(OnGreaterThan *node) {}
	virtual void Visit(OnEvent *node) {}
	virtual void Visit(Broadcast *node) {}
	virtual void Visit(BroadcastAndWait *node) {}

	virtual void Visit(WaitSecs *node)
	{
		node->e->Accept(this);
		vm->Sleep(vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(Repeat *node)
	{
		node->e->Accept(this);
		int64_t count = vm->ToInteger(vm->StackAt(0));
		vm->Pop();
		vm->PushFrame(*node->sl, count, 0);
	}

	virtual void Visit(Forever *node)
	{
		vm->PushFrame(*node->sl, 0, FRAME_EXEC_FOREVER);
	}

	virtual void Visit(If *node)
	{
		node->e->Accept(this);
		bool truth = vm->Truth(vm->StackAt(0));
		vm->Pop();

		if (truth)
			vm->PushFrame(*node->sl, 1, 0);
	}

	virtual void Visit(IfElse *node)
	{
		node->e->Accept(this);
		bool truth = vm->Truth(vm->StackAt(0));
		vm->Pop();

		if (truth)
			vm->PushFrame(*node->sl1, 1, 0);
		else
			vm->PushFrame(*node->sl2, 1, 0);
	}

	virtual void Visit(WaitUntil *node)
	{
		// NOTE: don't evaluate the expression here
		vm->WaitUntil(*node->e);
	}

	virtual void Visit(RepeatUntil *node)
	{
		node->e->Accept(this);

		bool truth = vm->Truth(vm->StackAt(0));
		vm->Pop();

		if (!truth)
			vm->PushFrame(*node->sl, 1, FRAME_EXEC_AGAIN);
	}

	virtual void Visit(Stop *node)
	{
		switch (node->mode)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case StopMode_All:
			vm->VMTerminate();
			break;
		case StopMode_ThisScript:
			vm->Terminate();
			break;
		case StopMode_OtherScriptsInSprite:
			// TODO: implement
			break;
		}
	}

	virtual void Visit(CloneStart *node) {}
	virtual void Visit(CreateClone *node) {}
	virtual void Visit(DeleteClone *node) {}

	virtual void Visit(AskAndWait *node)
	{
		vm->AskAndWait();
	}

	virtual void Visit(SetDragMode *node) {}

	virtual void Visit(ResetTimer *node)
	{}

	virtual void Visit(SetVariable *node)
	{
		// TODO: replace with id
		Value &var = vm->FindVariable(node->name);
		node->e->Accept(this);
		vm->Assign(var, vm->StackAt(0));

		printf("%s = %s\n",
			node->name.c_str(),
			vm->ToString(vm->StackAt(0)).c_str());

		vm->Pop();
	}

	virtual void Visit(ChangeVariable *node) {}
	virtual void Visit(ShowVariable *node) {}
	virtual void Visit(HideVariable *node) {}
	virtual void Visit(AppendToList *node) {}
	virtual void Visit(DeleteFromList *node) {}
	virtual void Visit(DeleteAllList *node) {}
	virtual void Visit(InsertInList *node) {}
	virtual void Visit(ReplaceInList *node) {}
	virtual void Visit(ShowList *node) {}
	virtual void Visit(HideList *node) {}
	virtual void Visit(ProcProto *node) {}
	virtual void Visit(DefineProc *node) {}
	virtual void Visit(Call *node) {}

	//
	/////////////////////////////////////////////////////////////////
	// Reporters
	//

	virtual void Visit(GotoReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(GlideReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(PointTowardsReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(CostumeReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(BackdropReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(SoundReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(BroadcastReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(CloneReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(TouchingReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(DistanceReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(KeyReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(PropertyOfReporter *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(ArgReporterStringNumber *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(ArgReporterBoolean *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	VirtualMachine *vm = nullptr;
	Script *script = nullptr;
};

static std::string trim(const std::string &str, const std::string &ws = " \t\n\r")
{
	size_t start = str.find_first_not_of(ws);
	if (start == std::string::npos)
		return "";

	size_t end = str.find_last_not_of(ws);
	return str.substr(start, end - start + 1);
}

int VirtualMachine::Load(Program *prog)
{
	if (_prog || !prog)
		return -1;

	std::vector<AutoRelease<SpriteDef>> &defs = prog->sprites->sprites;
	
	for (AutoRelease<SpriteDef> &def : defs)
	{
		auto it = _sprites.find(def->name);
		if (it != _sprites.end())
		{
			// duplicate sprite name
			Cleanup();
			return -1;
		}

		Sprite &sprite = _sprites[def->name];
		sprite.name = def->name;
		sprite.x = def->x;
		sprite.y = def->y;
		sprite.direction = def->direction;
		sprite.costume = def->currentCostume;
		sprite.size = def->size;
		sprite.volume = def->volume;

		if (def->variables)
		{
			for (AutoRelease<VariableDef> &vdef : def->variables->variables)
			{
				auto it = _variables.find(vdef->name);
				if (it != _variables.end())
				{
					// duplicate variable name
					Cleanup();
					return -1;
				}

				Value &v = _variables[vdef->name] = { 0 };
				SetParsedString(v, vdef->value->value);

				if (v.type == ValueType_Exception)
				{
					Cleanup();
					return -1;
				}
			}
		}

		if (def->scripts)
		{
			for (AutoRelease<StatementList> &sl : def->scripts->sll)
			{
				Script script = { 0 };

				script.state = EMBRYO;
				script.sprite = &sprite;
				script.entry = *sl;
				//script.pc = 1;

				script.lock = ls_lock_create();
				if (!script.lock)
				{
					Cleanup();
					return -1;
				}

				script.sleepUntil = 0.0;
				script.waitExpr = nullptr;

				script.stack = (Value *)malloc(sizeof(Value) * STACK_SIZE);
				if (!script.stack)
				{
					Cleanup();
					return -1;
				}

				// fill stack with garbage
				memset(script.stack, 0xab, sizeof(Value) * STACK_SIZE);

				script.sp = script.stack + STACK_SIZE;

				_scripts.push_back(script);
			}
		}
	}

	_prog = Retain(prog);
	return 0;
}

int VirtualMachine::VMStart()
{
	ls_lock(_lock);
	if (_running)
	{
		ls_unlock(_lock);
		return -1;
	}

	_shouldStop = false;

	ls_handle thread = ls_thread_create(&ThreadProc, this);
	if (!thread)
	{
		ls_unlock(_lock);
		return -1;
	}

	SendFlagClicked();

	while (!_running)
		ls_cond_wait(_cond, _lock);
	ls_unlock(_lock);

	return 0;
}

void VirtualMachine::VMTerminate()
{
	ls_lock(_lock);
	_shouldStop = true;
	ls_unlock(_lock);
}

int VirtualMachine::VMWait(unsigned long ms)
{
	int rc = 0;

	ls_lock(_lock);
	_waitCount++;
	while (_running)
	{
		rc = ls_cond_timedwait(_cond, _lock, ms);
		if (rc == -1)
			break;
	}
	_waitCount--;
	ls_unlock(_lock);

	return rc;
}

void VirtualMachine::SendFlagClicked()
{
	for (Script &script : _scripts)
	{
		ls_lock(script.lock);

		AutoRelease<Statement> &s = script.entry->sl[0];
		OnFlagClicked *fc = s->As<OnFlagClicked>();
		if (fc)
		{
			script.state = RUNNABLE;
			script.sleepUntil = 0.0;
			script.waitExpr = nullptr;

			script.frames[0].sl = script.entry;
			script.frames[0].pc = 1;
			script.frames[0].count = 0;
			script.frames[0].flags = 0;
			script.fp = 0;
		}

		ls_unlock(script.lock);
	}
}

void VirtualMachine::Sleep(double seconds)
{
	_current->sleepUntil = _time + seconds;
	_current->state = WAITING;
}

void VirtualMachine::WaitUntil(Expression *expr)
{
	_current->waitExpr = expr;
	_current->state = WAITING;

	printf("%p in %s is waiting\n",
		_current,
		_current->sprite->name.c_str());
}

void VirtualMachine::AskAndWait()
{
	// TODO: implement
}

void VirtualMachine::Terminate()
{
	_current->state = TERMINATED;

	printf("%p in %s terminated\n",
		_current,
		_current->sprite->name.c_str());
}

void VirtualMachine::TerminateScript(unsigned long id)
{
	// TODO: implement
}

Value &VirtualMachine::Raise(ExceptionType type)
{
	assert(_exception.type == ValueType_Exception);
	if (_exception.u.exception == Exception_None)
		_exception.u.exception = type;
	return _exception;
}

Value &VirtualMachine::Push()
{
	if (_exception.u.exception != Exception_None)
		return _exception;

	if (_current->sp < _current->sp)		
		return Raise(StackOverflow);
	_current->sp--;
	*_current->sp = { 0 }; // zero out the value
	return *_current->sp;
}

bool VirtualMachine::Pop()
{
	if (_exception.u.exception != Exception_None)
		return false;

	if (_current->sp >= _current->stack + STACK_SIZE)
	{
		Raise(StackUnderflow);
		return false;
	}

	ReleaseValue(*_current->sp);

	// fill with garbage
	memset(_current->sp, 0xab, sizeof(Value));

	_current->sp++;

	return true;
}

Value &VirtualMachine::StackAt(size_t i)
{
	if (_exception.u.exception != Exception_None)
		return _exception;

	if (_current->sp + i >= _current->stack + STACK_SIZE)
		return Raise(StackUnderflow);

	return _current->sp[i];
}

void VirtualMachine::PushFrame(StatementList *sl, int64_t count, uint32_t flags)
{
	if (_exception.u.exception != Exception_None && count > 0)
		return;

	if (_current->fp >= SCRIPT_DEPTH - 1)
	{
		Raise(StackOverflow);
		return;
	}

	Frame &f = _current->frames[++_current->fp];
	f.sl = sl;
	f.pc = 0;
	f.count = count;
	f.flags = flags;
}

bool VirtualMachine::Truth(const Value &val)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return false;

	if (val.type == ValueType_Exception)
		return false;

	if (val.type == ValueType_Bool)
		return val.u.boolean;

	if (val.type == ValueType_String)
	{

		if (val.u.string->len != 4)
			return false;

		for (size_t i = 0; i < 4; i++)
		{
			if (tolower(val.u.string->str[i]) != TRUE[i])
				return false;
		}

		return true;
	}

	return false;
}

bool VirtualMachine::Equals(const Value &lhs, const Value &rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return false;

	if (lhs.type == ValueType_Exception || rhs.type == ValueType_Exception)
		return false;

	switch (lhs.type)
	{
	case ValueType_Integer: {
		if (rhs.type == ValueType_Integer)
			return lhs.u.integer == rhs.u.integer;
		if (rhs.type == ValueType_Real)
			return lhs.u.integer == rhs.u.real;
		return false;
	}
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return lhs.u.real == rhs.u.real;
		if (rhs.type == ValueType_Integer)
			return lhs.u.real == rhs.u.integer;
		return false;
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return lhs.u.boolean == rhs.u.boolean;
		return false;
	case ValueType_String: {
		// String comparisions are case-insensitive and ignore
		// leading and trailing whitespace

		String *lstr = lhs.u.string;
		String *rstr = rhs.u.string;

		if (lstr == rstr)
			return true;

		char *lstart = lstr->str;
		while (isspace(*lstart))
			lstart++;

		char *rend = lstart;
		while (!isspace(*rend))
			rend++;

		char *rstart = rstr->str;
		while (isspace(*rstart))
			rstart++;

		char *lend = rend;
		while (!isspace(*lend))
			lend++;

		if (rend - lstart != lend - rstart)
			return false;

		size_t len = rend - lstart;
		for (size_t i = 0; i < len; i++)
		{
			if (tolower(lstart[i]) != tolower(rstart[i]))
				return false;
		}

		return true;
	}
	default:
		return false;
	}
}

Value &VirtualMachine::Assign(Value &lhs, const Value &rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return _exception;

	if (lhs.type == ValueType_Exception)
		return lhs;

	if (&lhs == &rhs)
		return lhs;

	if (lhs.u.ref == rhs.u.ref)
		return RetainValue(lhs);

	ReleaseValue(lhs);
	return RetainValue(lhs = rhs);
}


Value &VirtualMachine::SetInteger(Value &lhs, int64_t val)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return _exception;

	if (lhs.type == ValueType_Exception)
		return lhs;

	ReleaseValue(lhs);
	lhs.type = ValueType_Integer;
	lhs.u.integer = val;
	return lhs;
}

Value &VirtualMachine::SetReal(Value &lhs, double rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return _exception;

	if (lhs.type == ValueType_Exception)
		return lhs;

	ReleaseValue(lhs);
	lhs.type = ValueType_Real;
	lhs.u.real = rhs;
	return lhs;
}

Value &VirtualMachine::SetBool(Value &lhs, bool rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return _exception;

	if (lhs.type == ValueType_Exception)
		return lhs;

	ReleaseValue(lhs);
	lhs.type = ValueType_Bool;
	lhs.u.boolean = rhs;
	return lhs;
}

Value &VirtualMachine::SetString(Value &lhs, const std::string &rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return _exception;

	if (lhs.type == ValueType_Exception)
		return lhs;

	ReleaseValue(lhs);

	if (rhs.size() == 0)
		return Assign(lhs, _emptyString);

	size_t cb;

	lhs.type = ValueType_String;

	cb = offsetof(String, str) + rhs.size() + 1;
	lhs.u.string = (String *)calloc(1, cb);
	if (!lhs.u.string)
		return Raise(OutOfMemory);

	memcpy(lhs.u.string->str, rhs.data(), rhs.size() + 1);

	lhs.u.string->len = rhs.size();
	lhs.u.string->ref.count = 1;

	return lhs;
}

Value &VirtualMachine::SetParsedString(Value &lhs, const std::string &rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return _exception;

	if (lhs.type == ValueType_Exception)
		return lhs;

	std::string str = trim(rhs);

	char *end;

	int64_t integer = strtoll(str.c_str(), &end, 10);
	if (*end == 0)
		return SetInteger(lhs, integer);

	double real = strtod(str.c_str(), &end);
	if (*end == 0)
		return SetReal(lhs, real);

	if (str.size() == 4)
	{
		size_t i;
		for (i = 0; i < 4; i++)
			if (tolower(str[i]) != TRUE[i])
				break;

		if (i == 4)
			return SetBool(lhs, true);
	}

	if (str.size() == 5)
	{
		size_t i;
		for (i = 0; i < 5; i++)
			if (tolower(str[i]) != FALSE[i])
				break;

		if (i == 5)
			return SetBool(lhs, false);
	}

	return SetString(lhs, str);
}

std::string VirtualMachine::ToString(const Value &val)
{
	if (_exception.u.exception != Exception_None)
		return std::string();

	switch (val.type)
	{
	default:
	case ValueType_Exception:
	case ValueType_None:
		return std::string();
	case ValueType_Integer:
		return std::to_string(val.u.integer);
	case ValueType_Real: {
		char buf[64];
		int cch = snprintf(buf, sizeof(buf), "%.8g", val.u.real);
		return std::string(buf, cch);
	}
	case ValueType_Bool:
		return val.u.boolean ? "true" : "false";
	case ValueType_String:
		return std::string(val.u.string->str, val.u.string->len);
	}
}

int64_t VirtualMachine::ToInteger(const Value &val)
{
	if (_exception.u.exception != Exception_None)
		return 0;

	switch (val.type)
	{
	default:
	case ValueType_Exception:
	case ValueType_None:
	case ValueType_Bool:
	case ValueType_String:
		return 0;
	case ValueType_Real:
		return static_cast<int64_t>(round(val.u.real));
	case ValueType_Integer:
		return val.u.integer;
	}
}

double VirtualMachine::ToReal(const Value &val)
{
	if (_exception.u.exception != Exception_None)
		return 0.0;

	switch (val.type)
	{
	default:
	case ValueType_Exception:
	case ValueType_None:
	case ValueType_Bool:
	case ValueType_String:
		return 0.0;
	case ValueType_Real:
		return val.u.real;
	case ValueType_Integer:
		return static_cast<double>(val.u.integer);
	}
}

Value &VirtualMachine::RetainValue(Value &val)
{
	if (val.type == ValueType_String)
	{
		assert(val.u.string->ref.count > 0);
		val.u.ref->count++;
	}
	return val;
}

void VirtualMachine::ReleaseValue(Value &val)
{
	if (val.type == ValueType_String)
	{
		assert(val.u.string->ref.count > 0);

		if (--val.u.ref->count == 0)
			FreeValue(val);
	}
}

void VirtualMachine::FreeValue(Value &val)
{
	if (val.type == ValueType_String)
	{
		assert(val.u.string->ref.count == 0);
		free(val.u.string);

		// for safety
		val.u.ref = nullptr;
	}
}

Value &VirtualMachine::FindVariable(const std::string &id)
{
	auto it = _variables.find(id);
	if (it == _variables.end())
		return Raise(VariableNotFound);
	return it->second;
}

void VirtualMachine::ResetTimer()
{
	_timerStart = _time;
}

VirtualMachine::VirtualMachine()
{
	_prog = nullptr;

	_emptyString.type = ValueType_String;
	_emptyString.u.string = (String *)calloc(1, sizeof(String));
	if (!_emptyString.u.string)
		throw std::bad_alloc();
	_emptyString.u.ref->count = 1;

	_answer = { 0 };
	_mouseDown = false;
	_mouseX = 0;
	_mouseY = 0;
	_loudness = 0.0;
	_timer = 0.0;
	_username = { 0 };

	_timerStart = false;

	_shouldStop = false;
	_waitCount = 0;

	_running = false;
	_activeScripts = 0;
	_exception = { 0 };

	_current = nullptr;
	_time = 0.0;

	_lock = ls_lock_create();
	_cond = ls_cond_create();
}

VirtualMachine::~VirtualMachine()
{
	Cleanup();

	ReleaseValue(_username);
	ReleaseValue(_answer);

	ReleaseValue(_emptyString);

	Release(_prog);

	ls_close(_cond);
	ls_close(_lock);
}

void VirtualMachine::Cleanup()
{
	for (auto &p : _variables)
		ReleaseValue(p.second);

	for (Script &script : _scripts)
	{
		ls_close(script.lock);
		free(script.stack);
	}
}

void VirtualMachine::Scheduler()
{
	Executor executor;
	double nextFrame; // time at which to render the next frame

	executor.vm = this;

	ls_lock(_lock);
	_running = true;
	ls_cond_signal(_cond);

	nextFrame = ls_time64();
	for (;;)
	{
		if (_exception.u.exception != Exception_None)
		{
			printf("<EXCEPTION> %u\n", _exception.u.exception);
			break;
		}

		if (_shouldStop)
			break;

		// Nothing running and threads are waiting
		if (_activeScripts == 0 && _waitCount != 0)
			break;

		ls_unlock(_lock);

		_time = ls_time64();
		_timer = _time - _timerStart;

		size_t activeScripts = 0;
		for (Script &script : _scripts)
		{
			ls_lock(script.lock);

			_current = &script;

			// Handle waiting scripts
			if (script.state == WAITING)
			{
				if (script.waitExpr)
				{
					// Execute the wait expression
					executor.script = &script;
					script.waitExpr->Accept(&executor);

					// If true, awaken the script
					if (Truth(script.sp[0]))
						script.state = RUNNABLE;
				}
				else if (script.sleepUntil <= _time)
				{
					// Wake up the script
					script.state = RUNNABLE;
				}
			}

			// Don't execute scripts that are not runnable
			if (script.state != RUNNABLE)
			{
				if (script.state == WAITING)
					activeScripts++;

				_current = nullptr;
				ls_unlock(script.lock);
				continue;
			}

			// Pop frames until we find one that can execute
			Frame *f = &script.frames[script.fp];		
			while (f->pc >= f->sl->sl.size())
			{
				// top of the stack, script is done
				if (script.fp == 0)
				{
					script.state = TERMINATED;
					break;
				}

				// this script should execute forever, so reset the program counter
				if (f->flags & FRAME_EXEC_FOREVER)
				{
					f->pc = 0;
					break;
				}

				// decrement the repeat count, if it's zero, pop the frame
				f->count--;
				if (f->count > 0)
				{
					f->pc = 0;
					break;
				}

				bool again = (f->flags & FRAME_EXEC_AGAIN) != 0;

				// Pop the frame
				script.fp--;
				f = &script.frames[script.fp];

				// Counteracts the increment from the last iteration
				if (again)
					f->pc--;
			}

			// Execute the script
			if (script.state == RUNNABLE)
			{
				activeScripts++;

				executor.script = &script;

				// Run the statement
				Statement *stmt = *f->sl->sl[f->pc];
				stmt->Accept(&executor);

				// Exception handling
				if (_exception.u.exception == Exception_None)
				{
					if (script.sp != script.stack + STACK_SIZE)
						Raise(VMError); // stack should be empty
					else if (script.fp >= SCRIPT_DEPTH)
						Raise(VMError); // invalid frame pointer
				}

				f->pc++;
			}

			_current = nullptr;

			ls_unlock(script.lock);
		}

		if (_time >= nextFrame)
		{
			// TODO: do something with rendering

			nextFrame = _time + 1.0 / FRAMERATE;
		}

		ls_lock(_lock);

		_activeScripts = activeScripts;
	}

	// Lock is held here

	_activeScripts = 0;
	_running = false;

	ls_cond_broadcast(_cond); // notify waiting threads
	ls_unlock(_lock);
}

int VirtualMachine::ThreadProc(void *data)
{
	VirtualMachine *vm = (VirtualMachine *)data;
	vm->Scheduler();
	return 0;
}
