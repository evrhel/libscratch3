#include "script.hpp"

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

void ScriptInit(Script &script, const ScriptInfo *info)
{
	if (script.stack)
		abort();

	script.stack = EMBRYO;
	script.sprite = nullptr;
	script.fiber = nullptr;
	script.sleepUntil = 0.0;
	script.waitInput = false;
	script.askInput = false;
	script.ticks = 0;
	script.entry = info->loc;
	script.pc = script.entry;
	script.stack = (Value *)malloc(STACK_SIZE * sizeof(Value));

	// fill stack with garbage
	if (script.stack)
	{
		script.sp = script.stack + STACK_SIZE;
		memset(script.stack, 0xab, STACK_SIZE * sizeof(Value));
	}
	else
		script.sp = nullptr; // out of memory
}

void ScriptDestroy(Script &script)
{
	free(script.stack);
	memset(&script, 0, sizeof(Script));
}

void ScriptReset(Script &script)
{
	script.state = EMBRYO;
	script.sleepUntil = 0.0;
	script.waitInput = false;
	script.askInput = false;
	script.ticks = 0;
	script.pc = script.entry;

	// release stack
	Value *const stackEnd = script.stack + STACK_SIZE;
	while (script.sp < stackEnd)
	{
		ReleaseValue(*script.sp);
		memset(script.sp, 0xab, sizeof(Value));
		script.sp++;
	}
}

void ScriptStart(Script &script)
{
	ScriptReset(script);
	script.state = RUNNABLE;
}

int ScriptMain(void *scriptPtr)
{
	Script &script = *(Script *)scriptPtr;
	VirtualMachine &vm = *script.vm;
	Sprite &sprite = *script.sprite;

	ProgramHeader *header = (ProgramHeader *)vm.GetBytecode();
	uint8_t *textBegin = vm.GetBytecode() + header->text;
	uint8_t *textEnd = textBegin + header->text_size;

	bool b, b2;
	uint64_t ui64;
	int64_t i64;
	double real;
	String *str;

	for (;;)
	{
		script.ticks++;

		Opcode opcode = (Opcode)*script.pc;
		script.pc++;

		switch (opcode)
		{
		default:
			vm.Raise(VMError, "Invalid opcode");
		case Op_noop:
			break;
		case Op_int:
			vm.Raise(VMError, "Software interrupt");
		case Op_varset:
			vm.Raise(NotImplemented, "varset");
		case Op_varadd:
			vm.Raise(NotImplemented, "varadd");
		case Op_varget:
			vm.Raise(NotImplemented, "varget");
		case Op_listcreate:
			i64 = *(int64_t *)script.pc;
			script.pc += sizeof(int64_t);
			AllocList(vm.Push(), i64);
			break;
		case Op_jmp:
			ui64 = *(uint64_t *)script.pc;
			script.pc = textBegin + ui64;
			break;
		case Op_jz:
			ui64 = *(uint64_t *)script.pc;
			b = Truth(vm.StackAt(0));
			vm.Pop();

			if (!b)
				script.pc = textBegin + ui64;
			else
				script.pc += sizeof(uint64_t);
			break;
		case Op_jnz:
			ui64 = *(uint64_t *)script.pc;
			b = Truth(vm.StackAt(0));
			vm.Pop();

			if (b)
				script.pc = textBegin + ui64;
			else
				script.pc += sizeof(uint64_t);
			break;
		case Op_yield:
			vm.Sched();
			break;
		case Op_pop:
			vm.Pop();
			break;
		case Op_pushnone:
			vm.Push();
			break;
		case Op_pushint:
			i64 = *(int64_t *)script.pc;
			script.pc += sizeof(int64_t);
			SetInteger(vm.Push(), i64);
			break;
		case Op_pushreal:
			real = *(double *)script.pc;
			script.pc += sizeof(double);
			SetReal(vm.Push(), real);
			break;
		case Op_pushtrue:
			SetBool(vm.Push(), true);
			break;
		case Op_pushfalse:
			SetBool(vm.Push(), false);
			break;
		case Op_pushstring:
			ui64 = *(uint64_t *)script.pc;
			str = (String *)(vm.GetBytecode() + ui64);
			script.pc += sizeof(uint64_t);
			SetStaticString(vm.Push(), str);
			break;
		case Op_dup:
			vm.Push();
			Assign(vm.StackAt(0), vm.StackAt(1));
			break;
		case Op_eq:
			SetBool(vm.StackAt(1), Equals(vm.StackAt(0), vm.StackAt(1)));
			vm.Pop();
			break;
		case Op_neq:
			SetBool(vm.StackAt(1), !Equals(vm.StackAt(0), vm.StackAt(1)));
			vm.Pop();
			break;
		case Op_gt:
			SetBool(vm.StackAt(1), ToReal(vm.StackAt(0)) > ToReal(vm.StackAt(1)));
			vm.Pop();
			break;
		case Op_ge:
			SetBool(vm.StackAt(1), ToReal(vm.StackAt(0)) >= ToReal(vm.StackAt(1)));
			vm.Pop();
			break;
		case Op_lt:
			SetBool(vm.StackAt(1), ToReal(vm.StackAt(0)) < ToReal(vm.StackAt(1)));
			vm.Pop();
			break;
		case Op_le:
			SetBool(vm.StackAt(1), ToReal(vm.StackAt(0)) <= ToReal(vm.StackAt(1)));
			vm.Pop();
			break;
		case Op_land:
			SetBool(vm.StackAt(1), Truth(vm.StackAt(0)) && Truth(vm.StackAt(1)));
			vm.Pop();
			break;
		case Op_lor:
			SetBool(vm.StackAt(1), Truth(vm.StackAt(0)) || Truth(vm.StackAt(1)));
			vm.Pop();
			break;
		case Op_lnot:
			SetBool(vm.StackAt(0), !Truth(vm.StackAt(0)));
			break;
		case Op_add:
			SetReal(vm.StackAt(0), ToReal(vm.StackAt(0)) + ToReal(vm.StackAt(1)));
			break;
		case Op_sub:
			SetReal(vm.StackAt(0), ToReal(vm.StackAt(0)) - ToReal(vm.StackAt(1)));
			break;
		case Op_mul:
			SetReal(vm.StackAt(0), ToReal(vm.StackAt(0)) * ToReal(vm.StackAt(1)));
			break;
		case Op_div:
			vm.Raise(NotImplemented, "div");
		case Op_mod:
			SetReal(vm.StackAt(0), fmod(ToReal(vm.StackAt(0)), ToReal(vm.StackAt(1))));
			break;
		case Op_neg:
			SetReal(vm.StackAt(0), -ToReal(vm.StackAt(0)));
			break;
		case Op_round:
			SetReal(vm.StackAt(0), round(ToReal(vm.StackAt(0))));
			break;
		case Op_abs:
			SetReal(vm.StackAt(0), fabs(ToReal(vm.StackAt(0))));
			break;
		case Op_floor:
			SetReal(vm.StackAt(0), floor(ToReal(vm.StackAt(0))));
			break;
		case Op_ceil:
			SetReal(vm.StackAt(0), ceil(ToReal(vm.StackAt(0))));
			break;
		case Op_sqrt:
			SetReal(vm.StackAt(0), sqrt(ToReal(vm.StackAt(0))));
			break;
		case Op_sin:
			SetReal(vm.StackAt(0), sin(ToReal(vm.StackAt(0))));
			break;
		case Op_cos:
			SetReal(vm.StackAt(0), cos(ToReal(vm.StackAt(0))));
			break;
		case Op_tan:
			SetReal(vm.StackAt(0), tan(ToReal(vm.StackAt(0))));
			break;
		case Op_asin:
			SetReal(vm.StackAt(0), asin(ToReal(vm.StackAt(0))));
			break;
		case Op_acos:
			SetReal(vm.StackAt(0), acos(ToReal(vm.StackAt(0))));
			break;
		case Op_atan:
			SetReal(vm.StackAt(0), atan(ToReal(vm.StackAt(0))));
			break;
		case Op_ln:
			SetReal(vm.StackAt(0), log(ToReal(vm.StackAt(0))));
			break;
		case Op_log10:
			SetReal(vm.StackAt(0), log10(ToReal(vm.StackAt(0))));
			break;
		case Op_exp:
			SetReal(vm.StackAt(0), exp(ToReal(vm.StackAt(0))));
			break;
		case Op_exp10:
			SetReal(vm.StackAt(0), pow(10.0, ToReal(vm.StackAt(0))));
			break;
		case Op_strcat:
			ConcatValue(vm.StackAt(1), vm.StackAt(0));
			vm.Pop();
			break;
		case Op_charat:
			SetChar(vm.StackAt(1), ValueCharAt(vm.StackAt(0), ToInteger(vm.StackAt(1))));
			vm.Pop();
			break;
		case Op_strlen:
			SetInteger(vm.StackAt(0), ValueLength(vm.StackAt(0)));
			break;
		case Op_strstr:
			break;
		case Op_inc:
			vm.Raise(NotImplemented, "inc");
		case Op_dec:
			vm.Raise(NotImplemented, "dec");
		case Op_movesteps:
			break;
		case Op_turndegrees:
			sprite.SetDirection(ToReal(vm.StackAt(0)) + sprite.GetDirection());
			vm.Pop();
			break;
		case Op_goto:
			vm.Raise(NotImplemented, "goto");
		case Op_gotoxy:
			sprite.SetXY(ToReal(vm.StackAt(1)), ToReal(vm.StackAt(0)));
			vm.Pop();
			vm.Pop();
			break;
		case Op_glide:
			vm.Raise(NotImplemented, "glide");
		case Op_glidexy:
			vm.Raise(NotImplemented, "glidexy");
		case Op_setdir:
			sprite.SetDirection(ToReal(vm.StackAt(0)));
			vm.Pop();
			break;
		case Op_lookat:
			vm.Raise(NotImplemented, "lookat");
		case Op_addx:
			sprite.SetX(ToReal(vm.StackAt(0)) + sprite.GetX());
			vm.Pop();
			break;
		case Op_setx:
			sprite.SetX(ToReal(vm.StackAt(0)));
			vm.Pop();
			break;
		case Op_addy:
			sprite.SetY(ToReal(vm.StackAt(0)) + sprite.GetY());
			vm.Pop();
			break;
		case Op_sety:
			sprite.SetY(ToReal(vm.StackAt(0)));
			vm.Pop();
			break;
		case Op_bounceonedge:
			vm.Raise(NotImplemented, "bounceonedge");
		case Op_setrotationstyle:
			sprite.SetRotationStyle((RotationStyle)*(uint8_t *)script.pc);
			script.pc++;
			break;
		case Op_getx:
			SetReal(vm.Push(), sprite.GetX());
			break;
		case Op_gety:
			SetReal(vm.Push(), sprite.GetY());
			break;
		case Op_getdir:
			SetReal(vm.Push(), sprite.GetDirection());
			break;
		case Op_say:
			vm.Raise(NotImplemented, "say");
		case Op_think:
			vm.Raise(NotImplemented, "think");
		case Op_setcostume:
			vm.Raise(NotImplemented, "setcostume");
		case Op_findcostume:
			vm.Raise(NotImplemented, "findcostume");
		case Op_nextcostume:
			vm.Raise(NotImplemented, "nextcostume");
		case Op_setbackdrop:
			vm.Raise(NotImplemented, "setbackdrop");
		case Op_findbackdrop:
			vm.Raise(NotImplemented, "findbackdrop");
		case Op_nextbackdrop:
			vm.Raise(NotImplemented, "nextbackdrop");
		case Op_addsize:
			sprite.SetSize(sprite.GetSize() + ToReal(vm.StackAt(0)));
			vm.Pop();
			break;
		case Op_setsize:
			sprite.SetSize(ToReal(vm.StackAt(0)));
			vm.Pop();
			break;
		case Op_addgraphiceffect:
			vm.Raise(NotImplemented, "addgraphiceffect");
		case Op_setgraphiceffect:
			vm.Raise(NotImplemented, "setgraphiceffect");
		case Op_cleargraphiceffects:
			vm.Raise(NotImplemented, "cleargraphiceffects");
		case Op_show:
			sprite.SetShown(true);
			break;
		case Op_hide:
			sprite.SetShown(false);
			break;
		case Op_gotolayer:
			vm.Raise(NotImplemented, "gotolayer");
		case Op_movelayer:
			vm.Raise(NotImplemented, "movelayer");
		case Op_getcostume:
			SetInteger(vm.Push(), sprite.GetCostume());
			break;
		case Op_getcostumename:
			SetString(vm.Push(), sprite.GetCostumeName());
			break;
		case Op_getbackdrop:
			vm.Raise(NotImplemented, "getbackdrop");
		case Op_getsize:
			SetReal(vm.Push(), sprite.GetSize());
			break;
		case Op_playsoundandwait:
			vm.Raise(NotImplemented, "playsoundandwait");
		case Op_playsound:
			vm.Raise(NotImplemented, "playsound");
		case Op_findsound:
			vm.Raise(NotImplemented, "findsound");
		case Op_stopsound:
			vm.Raise(NotImplemented, "stopsound");
		case Op_addsoundeffect:
			vm.Raise(NotImplemented, "addsoundeffect");
		case Op_setsoundeffect:
			vm.Raise(NotImplemented, "setsoundeffect");
		case Op_clearsoundeffects:
			vm.Raise(NotImplemented, "clearsoundeffects");
		case Op_addvolume:
			sprite.SetVolume(sprite.GetVolume() + ToReal(vm.StackAt(0)));
			break;
		case Op_setvolume:
			sprite.SetVolume(ToReal(vm.StackAt(0)));
			vm.Pop();
			break;
		case Op_getvolume:
			SetReal(vm.Push(), sprite.GetVolume());
			break;
		case Op_onflag:
			// do nothing
			break;
		case Op_onkey:
			script.pc += sizeof(uint16_t); // skip key
			break;
		case Op_onclick:
			// do nothing
			break;
		case Op_onbackdropswitch:
			// do nothing
			break;
		case Op_ongt:
			vm.Raise(NotImplemented, "ongt");
		case Op_onevent:
			script.pc += sizeof(uint64_t); // skip event
			break;
		case Op_send:
			vm.Raise(NotImplemented, "send");
		case Op_sendandwait:
			vm.Raise(NotImplemented, "sendandwait");
		case Op_findevent:
			vm.Raise(NotImplemented, "findevent");
		case Op_waitsecs:
			vm.Sleep(ToReal(vm.StackAt(0)));
			vm.Pop();
			break;
		case Op_stopall:
			vm.VMTerminate();
			break;
		case Op_stopself:
			vm.Terminate();
			break;
		case Op_stopother:
			vm.Raise(NotImplemented, "stopother");
		case Op_onclone:
			// do nothing
			break;
		case Op_clone:
			vm.Raise(NotImplemented, "clone");
		case Op_deleteclone:
			vm.Raise(NotImplemented, "deleteclone");
		case Op_touching:
			vm.Raise(NotImplemented, "touching");
		case Op_touchingcolor:
			vm.Raise(NotImplemented, "touchingcolor");
		case Op_colortouching:
			vm.Raise(NotImplemented, "colortouching");
		case Op_distanceto:
			vm.Raise(NotImplemented, "distanceto");
		case Op_ask:
			vm.Raise(NotImplemented, "ask");
		case Op_getanswer:
			Assign(vm.Push(), vm.GetIO().GetAnswer());
			break;
		case Op_keypressed:
			vm.Raise(NotImplemented, "keypressed");
		case Op_mousedown:
			SetBool(vm.Push(), vm.GetIO().IsMouseDown());
			break;
		case Op_mousex:
			SetReal(vm.Push(), vm.GetIO().GetMouseX());
			break;
		case Op_mousey:
			SetReal(vm.Push(), vm.GetIO().GetMouseY());
			break;
		case Op_setdragmode:
			vm.Raise(NotImplemented, "setdragmode");
		case Op_getloudness:
			vm.Raise(NotImplemented, "getloudness");
		case Op_gettimer:
			SetReal(vm.Push(), vm.GetTimer());
			break;
		case Op_resettimer:
			vm.ResetTimer();
			break;
		case Op_propertyof:
			vm.Raise(NotImplemented, "propertyof");
		case Op_gettime:
			vm.Raise(NotImplemented, "gettime");
		case Op_getdayssince2000:
			vm.Raise(NotImplemented, "getdayssince2000");
		case Op_getusername:
			Assign(vm.Push(), vm.GetIO().GetUsername());
			break;
		case Op_rand:
			vm.Raise(NotImplemented, "rand");
		case Op_varshow:
			vm.Raise(NotImplemented, "varshow");
		case Op_varhide:
			vm.Raise(NotImplemented, "varhide");
		case Op_listadd:
			ListAppend(vm.StackAt(0), vm.StackAt(1));
			vm.Pop();
			vm.Pop();
			break;
		case Op_listremove:
			vm.Raise(NotImplemented, "listremove");
		case Op_listclear:
			ListClear(vm.StackAt(0));
			vm.Pop();
			break;
		case Op_listinsert:
			vm.Raise(NotImplemented, "listinsert");
		case Op_listreplace:
			vm.Raise(NotImplemented, "listreplace");
		case Op_listat:
			ListGet(vm.StackAt(1), vm.StackAt(0), ToInteger(vm.StackAt(1)));
			break;
		case Op_listfind:
			vm.Raise(NotImplemented, "listfind");
		case Op_listlen:
			SetInteger(vm.StackAt(0), ListGetLength(vm.StackAt(0)));
			break;
		case Op_listcontains:
			vm.Raise(NotImplemented, "listcontains");
		case Op_invoke:
			vm.Raise(NotImplemented, "invoke");
		case Op_ext:
			vm.Raise(VMError, "Extensions are not supported");
		}
	}

	return 0;
}
