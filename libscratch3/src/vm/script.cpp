#include "script.hpp"

#include <cstdio>

#include "vm.hpp"
#include "sprite.hpp"
#include "../codegen/compiler.hpp"

const char *GetStateName(int state)
{
	switch (state)
	{
	default:
		return "<unknown>";
	case EMBRYO:
		return "EMBRYO";
	case RUNNABLE:
		return "RUNNABLE";
	case RUNNING:
		return "RUNNING";
	case WAITING:
		return "WAITING";
	case SUSPENDED:
		return "SUSPENDED";
	case TERMINATED:
		return "TERMINATED";
	}
}

void Script::Init(const ScriptInfo *info)
{
	if (stack)
		abort();

	state = EMBRYO;
	sprite = nullptr;
	fiber = nullptr;
	sleepUntil = 0.0;
	waitInput = false;
	askInput = false;
	ticks = 0;
	entry = info->loc;
	pc = entry;
	stack = (Value *)malloc(STACK_SIZE * sizeof(Value));

	// fill stack with garbage
	if (stack)
	{
		sp = stack + STACK_SIZE;
		memset(stack, 0xab, STACK_SIZE * sizeof(Value));
	}
	else
		sp = nullptr; // out of memory

	except = Exception_None;
	exceptMessage = nullptr;
}

void Script::Destroy()
{
	free(stack);
	memset(this, 0, sizeof(Script));
}

void Script::Reset()
{
	state = EMBRYO;
	sleepUntil = 0.0;
	waitInput = false;
	askInput = false;
	ticks = 0;
	pc = entry;

	// release stack
	Value *const stackEnd = stack + STACK_SIZE;
	while (sp < stackEnd)
	{
		ReleaseValue(*sp);
		memset(sp, 0xab, sizeof(Value));
		sp++;
	}
}

void Script::Start()
{
	Reset();
	state = RUNNABLE;
}

void Script::Main()
{
	uint8_t *bytecode = vm->GetBytecode();
	ProgramHeader *header = (ProgramHeader *)bytecode;
	uint8_t *textBegin = bytecode + header->text;
	uint8_t *textEnd = textBegin + header->text_size;

	bool b, b2;
	uint64_t ui64;
	int64_t i64;
	double real;
	String *str;
	Value *lhs, *rhs;

	for (;;)
	{
		ticks++;

		Opcode opcode = (Opcode)*pc;
		pc++;

		switch (opcode)
		{
		default:
			Raise(VMError, "Invalid opcode");
		case Op_noop:
			// do nothing
			break;
		case Op_int:
			Raise(VMError, "Software interrupt");
		case Op_varset:
			Assign(vm->GetVariableRef(StackAt(0)), StackAt(1));
			Pop();
			Pop();
			break;
		case Op_varadd: {
			Value &v = vm->GetVariableRef(StackAt(0));
			SetReal(v, ToReal(v) + ToReal(StackAt(1)));
			Pop();
			Pop();
			break;
		}
		case Op_varget:
			Assign(StackAt(0), vm->GetVariableRef(StackAt(0)));
			break;
		case Op_listcreate:
			i64 = *(int64_t *)pc;
			pc += sizeof(int64_t);
			AllocList(Push(), i64);
			break;
		case Op_jmp:
			ui64 = *(uint64_t *)pc;
			pc = textBegin + ui64;
			break;
		case Op_jz:
			ui64 = *(uint64_t *)pc;
			b = Truth(StackAt(0));
			Pop();

			if (!b)
				pc = textBegin + ui64;
			else
				pc += sizeof(uint64_t);
			break;
		case Op_jnz:
			ui64 = *(uint64_t *)pc;
			b = Truth(StackAt(0));
			Pop();

			if (b)
				pc = textBegin + ui64;
			else
				pc += sizeof(uint64_t);
			break;
		case Op_yield:
			Sched();
			break;
		case Op_pop:
			Pop();
			break;
		case Op_pushnone:
			Push();
			break;
		case Op_pushint:
			i64 = *(int64_t *)pc;
			pc += sizeof(int64_t);
			SetInteger(Push(), i64);
			break;
		case Op_pushreal:
			real = *(double *)pc;
			pc += sizeof(double);
			SetReal(Push(), real);
			break;
		case Op_pushtrue:
			SetBool(Push(), true);
			break;
		case Op_pushfalse:
			SetBool(Push(), false);
			break;
		case Op_pushstring:
			ui64 = *(uint64_t *)pc;
			str = (String *)(bytecode + ui64);
			pc += sizeof(uint64_t);
			SetStaticString(Push(), str);
			break;
		case Op_dup:
			Push();
			Assign(StackAt(0), StackAt(1));
			break;
		case Op_eq:
			SetBool(StackAt(1), Equals(StackAt(0), StackAt(1)));
			Pop();
			break;
		case Op_neq:
			SetBool(StackAt(1), !Equals(StackAt(0), StackAt(1)));
			Pop();
			break;
		case Op_gt:
			SetBool(StackAt(1), ToReal(StackAt(0)) > ToReal(StackAt(1)));
			Pop();
			break;
		case Op_ge:
			SetBool(StackAt(1), ToReal(StackAt(0)) >= ToReal(StackAt(1)));
			Pop();
			break;
		case Op_lt:
			SetBool(StackAt(1), ToReal(StackAt(0)) < ToReal(StackAt(1)));
			Pop();
			break;
		case Op_le:
			SetBool(StackAt(1), ToReal(StackAt(0)) <= ToReal(StackAt(1)));
			Pop();
			break;
		case Op_land:
			SetBool(StackAt(1), Truth(StackAt(0)) && Truth(StackAt(1)));
			Pop();
			break;
		case Op_lor:
			SetBool(StackAt(1), Truth(StackAt(0)) || Truth(StackAt(1)));
			Pop();
			break;
		case Op_lnot:
			SetBool(StackAt(0), !Truth(StackAt(0)));
			break;
		case Op_add:
			lhs = &StackAt(1);
			rhs = &StackAt(0);
			real = ToReal(*lhs) + ToReal(*rhs);
			Pop();
			SetReal(*lhs, real);
			break;
		case Op_sub:
			break;
		case Op_mul:
			break;
		case Op_div:
			break;
		case Op_mod:
			break;
		case Op_neg:
			break;
		case Op_round:
			break;
		case Op_abs:
			break;
		case Op_floor:
			break;
		case Op_ceil:
			break;
		case Op_sqrt:
			break;
		case Op_sin:
			break;
		case Op_cos:
			break;
		case Op_tan:
			break;
		case Op_asin:
			break;
		case Op_acos:
			break;
		case Op_atan:
			break;
		case Op_ln:
			break;
		case Op_log10:
			break;
		case Op_exp:
			break;
		case Op_exp10:
			break;
		case Op_strcat:
			Raise(NotImplemented, "strcat");
		case Op_charat:
			Raise(NotImplemented, "charat");
		case Op_strlen:
			break;
		case Op_strstr:
			break;
		case Op_inc:
			Raise(NotImplemented, "inc");
		case Op_dec:
			Raise(NotImplemented, "dec");
		case Op_movesteps:
			break;
		case Op_turndegrees:
			sprite->SetDirection(ToReal(StackAt(0)) + sprite->GetDirection());
			Pop();
			break;
		case Op_goto:
			Raise(NotImplemented, "goto");
		case Op_gotoxy:
			sprite->SetXY(ToReal(StackAt(1)), ToReal(StackAt(0)));
			Pop();
			Pop();
			break;
		case Op_glide:
			Raise(NotImplemented, "glide");
		case Op_glidexy:
			Raise(NotImplemented, "glidexy");
		case Op_setdir:
			sprite->SetDirection(ToReal(StackAt(0)));
			Pop();
			break;
		case Op_lookat:
			Raise(NotImplemented, "lookat");
		case Op_addx:
			sprite->SetX(ToReal(StackAt(0)) + sprite->GetX());
			Pop();
			break;
		case Op_setx:
			sprite->SetX(ToReal(StackAt(0)));
			Pop();
			break;
		case Op_addy:
			sprite->SetY(ToReal(StackAt(0)) + sprite->GetY());
			Pop();
			break;
		case Op_sety:
			sprite->SetY(ToReal(StackAt(0)));
			Pop();
			break;
		case Op_bounceonedge:
			Raise(NotImplemented, "bounceonedge");
		case Op_setrotationstyle:
			sprite->SetRotationStyle((RotationStyle)*(uint8_t *)pc);
			pc++;
			break;
		case Op_getx:
			SetReal(Push(), sprite->GetX());
			break;
		case Op_gety:
			SetReal(Push(), sprite->GetY());
			break;
		case Op_getdir:
			SetReal(Push(), sprite->GetDirection());
			break;
		case Op_say:
			Raise(NotImplemented, "say");
		case Op_think:
			Raise(NotImplemented, "think");
		case Op_setcostume:
			Raise(NotImplemented, "setcostume");
		case Op_nextcostume:
			Raise(NotImplemented, "nextcostume");
		case Op_setbackdrop:
			Raise(NotImplemented, "setbackdrop");
		case Op_nextbackdrop:
			Raise(NotImplemented, "nextbackdrop");
		case Op_addsize:
			sprite->SetSize(sprite->GetSize() + ToReal(StackAt(0)));
			Pop();
			break;
		case Op_setsize:
			sprite->SetSize(ToReal(StackAt(0)));
			Pop();
			break;
		case Op_addgraphiceffect:
			Raise(NotImplemented, "addgraphiceffect");
		case Op_setgraphiceffect:
			Raise(NotImplemented, "setgraphiceffect");
		case Op_cleargraphiceffects:
			Raise(NotImplemented, "cleargraphiceffects");
		case Op_show:
			sprite->SetShown(true);
			break;
		case Op_hide:
			sprite->SetShown(false);
			break;
		case Op_gotolayer:
			Raise(NotImplemented, "gotolayer");
		case Op_movelayer:
			Raise(NotImplemented, "movelayer");
		case Op_getcostume:
			SetInteger(Push(), sprite->GetCostume());
			break;
		case Op_getcostumename:
			SetString(Push(), sprite->GetCostumeName());
			break;
		case Op_getbackdrop:
			Raise(NotImplemented, "getbackdrop");
		case Op_getsize:
			SetReal(Push(), sprite->GetSize());
			break;
		case Op_playsoundandwait:
			Raise(NotImplemented, "playsoundandwait");
		case Op_playsound:
			Raise(NotImplemented, "playsound");
		case Op_findsound:
			Raise(NotImplemented, "findsound");
		case Op_stopsound:
			Raise(NotImplemented, "stopsound");
		case Op_addsoundeffect:
			Raise(NotImplemented, "addsoundeffect");
		case Op_setsoundeffect:
			Raise(NotImplemented, "setsoundeffect");
		case Op_clearsoundeffects:
			Raise(NotImplemented, "clearsoundeffects");
		case Op_addvolume:
			sprite->SetVolume(sprite->GetVolume() + ToReal(StackAt(0)));
			Pop();
			break;
		case Op_setvolume:
			sprite->SetVolume(ToReal(StackAt(0)));
			Pop();
			break;
		case Op_getvolume:
			SetReal(Push(), sprite->GetVolume());
			break;
		case Op_onflag:
			// do nothing
			break;
		case Op_onkey:
			pc += sizeof(uint16_t); // skip key
			break;
		case Op_onclick:
			// do nothing
			break;
		case Op_onbackdropswitch:
			// do nothing
			break;
		case Op_ongt:
			Raise(NotImplemented, "ongt");
		case Op_onevent:
			pc += sizeof(uint64_t); // skip event
			break;
		case Op_send:
			Raise(NotImplemented, "send");
		case Op_sendandwait:
			Raise(NotImplemented, "sendandwait");
		case Op_findevent:
			Raise(NotImplemented, "findevent");
		case Op_waitsecs:
			Sleep(ToReal(StackAt(0)));
			Pop();
			break;
		case Op_stopall:
			Raise(NotImplemented, "stopall");
		case Op_stopself:
			Terminate();
			break;
		case Op_stopother:
			Raise(NotImplemented, "stopother");
		case Op_onclone:
			// do nothing
			break;
		case Op_clone:
			Raise(NotImplemented, "clone");
		case Op_deleteclone:
			Raise(NotImplemented, "deleteclone");
		case Op_touching:
			Raise(NotImplemented, "touching");
		case Op_touchingcolor:
			Raise(NotImplemented, "touchingcolor");
		case Op_colortouching:
			Raise(NotImplemented, "colortouching");
		case Op_distanceto:
			Raise(NotImplemented, "distanceto");
		case Op_ask:
			Raise(NotImplemented, "ask");
		case Op_getanswer:
			Assign(Push(), vm->GetIO().GetAnswer());
			break;
		case Op_keypressed:
			Raise(NotImplemented, "keypressed");
		case Op_mousedown:
			SetBool(Push(), vm->GetIO().IsMouseDown());
			break;
		case Op_mousex:
			SetReal(Push(), vm->GetIO().GetMouseX());
			break;
		case Op_mousey:
			SetReal(Push(), vm->GetIO().GetMouseY());
			break;
		case Op_setdragmode:
			Raise(NotImplemented, "setdragmode");
		case Op_getloudness:
			Raise(NotImplemented, "getloudness");
		case Op_gettimer:
			SetReal(Push(), vm->GetTimer());
			break;
		case Op_resettimer:
			vm->ResetTimer();
			break;
		case Op_propertyof:
			Raise(NotImplemented, "propertyof");
		case Op_gettime:
			Raise(NotImplemented, "gettime");
		case Op_getdayssince2000:
			Raise(NotImplemented, "getdayssince2000");
		case Op_getusername:
			Assign(Push(), vm->GetIO().GetUsername());
			break;
		case Op_rand:
			Raise(NotImplemented, "rand");
		case Op_varshow:
			Raise(NotImplemented, "varshow");
		case Op_varhide:
			Raise(NotImplemented, "varhide");
		case Op_listadd:
			ListAppend(StackAt(0), StackAt(1));
			Pop();
			Pop();
			break;
		case Op_listremove:
			ListDelete(StackAt(0), StackAt(1));
			Pop();
			Pop();
			break;
		case Op_listclear:
			ListClear(StackAt(0));
			Pop();
			break;
		case Op_listinsert:
			ListInsert(StackAt(0), ToInteger(StackAt(1)), StackAt(2));
			Pop();
			Pop();
			Pop();
			break;
		case Op_listreplace:
			ListSet(StackAt(0), ToInteger(StackAt(1)), StackAt(2));
			Pop();
			Pop();
			Pop();
			break;
		case Op_listat:
			ListGet(StackAt(1), StackAt(0), ToInteger(StackAt(1)));
			Pop();
			break;
		case Op_listfind:
			SetInteger(StackAt(1), ListIndexOf(StackAt(0), StackAt(1)));
			Pop();
			break;
		case Op_listlen:
			SetInteger(StackAt(0), ListGetLength(StackAt(0)));
			break;
		case Op_listcontains:
			SetBool(StackAt(1), ListContainsValue(StackAt(0), StackAt(1)));
			Pop();
			break;
		case Op_invoke:
			Raise(NotImplemented, "invoke");
		case Op_ext:
			Raise(VMError, "Extensions are not supported");
		}
	}
}

Value &Script::Push()
{
	if (sp <= stack)
		Raise(StackOverflow, "Stack overflow");
	sp--;
	InitializeValue(*sp);
	return *sp;
}

void Script::Pop()
{
	if (sp > stack + STACK_SIZE)
		Raise(StackUnderflow, "Stack underflow");
	ReleaseValue(*sp);
	sp++;
}

Value &Script::StackAt(size_t i)
{
	if (sp + i >= stack + STACK_SIZE)
		Raise(StackUnderflow, "Stack underflow");
	return sp[i];
}

void Script::Sched()
{
	ticks = 0;
	ls_fiber_sched();
}

void Script::Terminate()
{
	state = TERMINATED;
	Sched();

	if (state != RUNNABLE)
		vm->Panic("Terminated script was rescheduled");

	// Script was restarted, jump back to the beginning
	longjmp(scriptMain, 1);
}

void LS_NORETURN Script::Raise(ExceptionType type, const char *message)
{
	except = type;
	exceptMessage = message;
	Terminate();
}

void Script::Sleep(double seconds)
{
	sleepUntil = vm->GetTime() + seconds;
	state = WAITING;
	Sched();
}

void Script::Glide(double x, double y, double t)
{
	if (t <= 0.0)
	{
		sprite->SetXY(x, y);
		Sched();
		return;
	}

	GlideInfo &glide = *sprite->GetGlide();

	glide.x0 = sprite->GetX();
	glide.y0 = sprite->GetY();
	glide.x1 = x;
	glide.y1 = y;
	glide.start = vm->GetTime();
	glide.end = glide.start + t;

	state = WAITING;

	Sched();
}

void Script::AskAndWait(const std::string &question)
{
	askInput = true;
	state = WAITING;
	vm->EnqueueAsk(this, question);
	Sched();
}

void Script::Dump()
{
	printf("Script %p\n", this);

	if (state >= EMBRYO && state <= TERMINATED)
		printf("    state = %s\n", GetStateName(state));
	else
		printf("    state = Unknown\n");

	printf("    sprite = %s\n", sprite ? sprite->GetName().c_str() : "(null)");

	printf("    sleepUntil = %g\n", sleepUntil);
	printf("    waitInput = %s\n", waitInput ? "true" : "false");
	printf("    stack = %p\n", stack);
	printf("    sp = %p\n", sp);
	printf("    pc = %p\n", pc); // TODO: display disassembly
}
