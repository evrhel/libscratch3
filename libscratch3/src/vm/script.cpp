#include "script.hpp"

#include "vm.hpp"

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
	script.sp = script.stack ? script.stack + STACK_SIZE : 0;
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

	for (;;)
	{
		script.ticks++;

		uint8_t opcode = *script.pc;
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
			break;
		case Op_varadd:
			break;
		case Op_varget:
			break;
		case Op_listcreate:
			break;
		case Op_jmp:
			break;
		case Op_jz:
			break;
		case Op_jnz:
			break;
		case Op_yield:
			vm.Sched();
			break;
		case Op_pop:
			break;
		case Op_pushnone:
			break;
		case Op_pushint:
			break;
		case Op_pushreal:
			break;
		case Op_pushtrue:
			break;
		case Op_pushfalse:
			break;
		case Op_pushstring:
			break;
		case Op_dup:
			break;
		case Op_eq:
			break;
		case Op_neq:
			break;
		case Op_gt:
			break;
		case Op_ge:
			break;
		case Op_lt:
			break;
		case Op_le:
			break;
		case Op_land:
			break;
		case Op_lor:
			break;
		case Op_lnot:
			break;
		case Op_add:
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
			break;
		case Op_charat:
			break;
		case Op_strlen:
			break;
		case Op_strstr:
			break;
		case Op_inc:
			break;
		case Op_dec:
			break;
		case Op_movesteps:
			break;
		case Op_turndegrees:
			break;
		case Op_goto:
			break;
		case Op_gotoxy:
			break;
		case Op_glide:
			break;
		case Op_glidexy:
			break;
		case Op_setdir:
			break;
		case Op_lookat:
			break;
		case Op_addx:
			break;
		case Op_setx:
			break;
		case Op_addy:
			break;
		case Op_sety:
			break;
		case Op_bounceonedge:
			break;
		case Op_setrotationstyle:
			break;
		case Op_getx:
			break;
		case Op_gety:
			break;
		case Op_getdir:
			break;
		case Op_say:
			break;
		case Op_think:
			break;
		case Op_setcostume:
			break;
		case Op_findcostume:
			break;
		case Op_nextcostume:
			break;
		case Op_setbackdrop:
			break;
		case Op_findbackdrop:
			break;
		case Op_nextbackdrop:
			break;
		case Op_addsize:
			break;
		case Op_setsize:
			break;
		case Op_addgraphiceffect:
			break;
		case Op_setgraphiceffect:
			break;
		case Op_cleargraphiceffects:
			break;
		case Op_show:
			break;
		case Op_hide:
			break;
		case Op_gotolayer:
			break;
		case Op_movelayer:
			break;
		case Op_getcostume:
			break;
		case Op_getcostumename:
			break;
		case Op_getbackdrop:
			break;
		case Op_getsize:
			break;
		case Op_playsoundandwait:
			break;
		case Op_playsound:
			break;
		case Op_findsound:
			break;
		case Op_stopsound:
			break;
		case Op_addsoundeffect:
			break;
		case Op_setsoundeffect:
			break;
		case Op_clearsoundeffects:
			break;
		case Op_addvolume:
			break;
		case Op_setvolume:
			break;
		case Op_getvolume:
			break;
		case Op_onflag:
			break;
		case Op_onkey:
			break;
		case Op_onclick:
			break;
		case Op_onbackdropswitch:
			break;
		case Op_ongt:
			break;
		case Op_onevent:
			break;
		case Op_send:
			break;
		case Op_sendandwait:
			break;
		case Op_findevent:
			break;
		case Op_waitsecs:
			break;
		case Op_stopall:
			break;
		case Op_stopself:
			break;
		case Op_stopother:
			break;
		case Op_onclone:
			break;
		case Op_clone:
			break;
		case Op_deleteclone:
			break;
		case Op_touching:
			break;
		case Op_touchingcolor:
			break;
		case Op_colortouching:
			break;
		case Op_distanceto:
			break;
		case Op_ask:
			break;
		case Op_getanswer:
			break;
		case Op_keypressed:
			break;
		case Op_mousedown:
			break;
		case Op_mousex:
			break;
		case Op_mousey:
			break;
		case Op_setdragmode:
			break;
		case Op_getloudness:
			break;
		case Op_gettimer:
			break;
		case Op_resettimer:
			break;
		case Op_propertyof:
			break;
		case Op_gettime:
			break;
		case Op_getdayssince2000:
			break;
		case Op_getusername:
			break;
		case Op_rand:
			break;
		case Op_varshow:
			break;
		case Op_varhide:
			break;
		case Op_listadd:
			break;
		case Op_listremove:
			break;
		case Op_listclear:
			break;
		case Op_listinsert:
			break;
		case Op_listreplace:
			break;
		case Op_listat:
			break;
		case Op_listfind:
			break;
		case Op_listlen:
			break;
		case Op_listcontains:
			break;
		case Op_invoke:
			break;
		case Op_ext:
			vm.Raise(VMError, "Extensions are not supported");
		}
	}

	return 0;
}
