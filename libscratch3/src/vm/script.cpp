#include "script.hpp"

#include <cstdio>
#include <cassert>

#include "vm.hpp"
#include "sprite.hpp"
#include "../codegen/compiler.hpp"
#include "../codegen/util.hpp"

#define DEG2RAD (0.017453292519943295769236907684886)
#define RAD2DEG (57.295779513082320876798154814105)

void Script::Dump()
{
	printf("Script %p\n", this);

	if (state >= EMBRYO && state <= TERMINATED)
		printf("    state = %s\n", GetStateName(state));
	else
		printf("    state = Unknown\n");

	printf("    sprite = %s\n", sprite ? sprite->GetBase()->GetNameString() : "(null)");

	printf("    sleepUntil = %g\n", sleepUntil);
	printf("    waitInput = %s\n", waitInput ? "true" : "false");
	printf("    stack = %p\n", stack);
	printf("    sp = %p\n", sp);
	printf("    pc = %p\n", pc); // TODO: display disassembly
}

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
	case WAITING:
		return "WAITING";
	case SUSPENDED:
		return "SUSPENDED";
	case TERMINATED:
		return "TERMINATED";
	}
}

int ScriptMain()
{
	uint8_t *bytecode = VM->GetBytecode();
	bc::Header *header = (bc::Header *)bytecode;

	bool b;
	uint64_t ui64;
	int64_t i64;
	double real;
	String *str;
	Value *lhs, *rhs;

	Script *self = VM->GetCurrentScript();
	Sprite *sprite = self->sprite;
	for (;;)
	{
		Opcode opcode = (Opcode)*self->pc;
		self->pc++;

		switch (opcode)
		{
		default:
			Raise(VMError, "Invalid opcode");
		case Op_noop:
			// do nothing
			break;
		case Op_int:
			Raise(VMError, "Software interrupt");
		case Op_setstatic: {
			bc::VarId *id = (bc::VarId *)self->pc;
			self->pc += sizeof(bc::VarId);

			Assign(VM->GetStaticVariable(id->ToInt()), StackAt(-1));
			Pop();
			break;
		}
		case Op_getstatic: {
			bc::VarId *id = (bc::VarId *)self->pc;
			self->pc += sizeof(bc::VarId);
			Assign(Push(), VM->GetStaticVariable(id->ToInt()));
			break;
		}
		case Op_addstatic: {
			bc::VarId *id = (bc::VarId *)self->pc;
			self->pc += sizeof(bc::VarId);
			Value &v = VM->GetStaticVariable(id->ToInt());
			SetReal(v, ToReal(v) + ToReal(StackAt(-1)));
			Pop();
			break;
		}
		case Op_listcreate:
			i64 = *(int64_t *)self->pc;
			self->pc += sizeof(int64_t);
			AllocList(Push(), i64);
			break;
		case Op_jmp:
			ui64 = *(uint64_t *)self->pc;
			self->pc = bytecode + ui64;
			break;
		case Op_jz:
			ui64 = *(uint64_t *)self->pc;
			b = Truth(StackAt(-1));
			Pop();

			if (!b)
				self->pc = bytecode + ui64;
			else
				self->pc += sizeof(uint64_t);
			break;
		case Op_jnz:
			ui64 = *(uint64_t *)self->pc;
			b = Truth(StackAt(-1));
			Pop();

			if (b)
				self->pc = bytecode + ui64;
			else
				self->pc += sizeof(uint64_t);
			break;
		case Op_call: {
			bc::_bool warp = *self->pc;
			self->pc += sizeof(bc::_bool);

			int argc = static_cast<int>(*(bc::uint16 *)self->pc);
			self->pc += sizeof(bc::uint16);

			uint8_t *proc = bytecode + *(bc::uint64 *)self->pc;
			self->pc += sizeof(bc::uint64);

			// space for return address and base pointer
			Push(), Push();

			// move arguments to new stack frame
			for (int i = 1; i <= argc; i++)
				Assign(StackAt(-i), StackAt(-i - 2));

			// store base pointer
			SetIntPtr(StackAt(-argc - 1), (intptr_t)self->bp);

			// store return address
			SetIntPtr(StackAt(-argc - 2), (intptr_t)self->pc);

			// set new base pointer
			self->bp = self->sp + argc;

			// jump to procedure
			self->pc = proc;
			break;
		}
		case Op_ret: {
			if (self->bp == self->stack + STACK_SIZE)
				Raise(StackUnderflow, "Stack underflow");

			if (self->bp->type != ValueType_IntPtr)
				Raise(VMError, "Corrupt stack frame");

			// release stack
			while (self->sp < self->bp)
			{
				ReleaseValue(*self->sp);
				memset(self->sp, 0xab, sizeof(Value));
				self->sp++;
			}
			
			// restore old base pointer
			self->bp = (Value *)self->bp->u.intptr;
			Pop();

			// pop return address and jump
			Value &raddr = StackAt(-1);
			if (raddr.type != ValueType_IntPtr)
				Raise(VMError, "Corrupt stack frame");
			self->pc = (uint8_t *)raddr.u.intptr;
			Pop();
			break;
		}
		case Op_enter:
			// Do nothing
			break;
		case Op_leave:
			// Do nothing
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
			i64 = *(int64_t *)self->pc;
			self->pc += sizeof(int64_t);
			SetInteger(Push(), i64);
			break;
		case Op_pushreal:
			real = *(double *)self->pc;
			self->pc += sizeof(double);
			SetReal(Push(), real);
			break;
		case Op_pushtrue:
			SetBool(Push(), true);
			break;
		case Op_pushfalse:
			SetBool(Push(), false);
			break;
		case Op_pushstring:
			ui64 = *(uint64_t *)self->pc;
			str = (String *)(bytecode + ui64);
			self->pc += sizeof(uint64_t);
			SetStaticString(Push(), str);
			break;
		case Op_push: {
			int16_t index = *(int16_t *)self->pc;
			self->pc += sizeof(int16_t);
			Value &v = StackAt(index);
			Assign(Push(), v);
			break;
		}
		case Op_eq:
			SetBool(StackAt(-2), Equals(StackAt(-2), StackAt(-1)));
			Pop();
			break;
		case Op_neq:
			SetBool(StackAt(-2), !Equals(StackAt(-2), StackAt(-1)));
			Pop();
			break;
		case Op_gt:
			SetBool(StackAt(-2), ToReal(StackAt(-2)) > ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_ge:
			SetBool(StackAt(-2), ToReal(StackAt(-2)) >= ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_lt:
			SetBool(StackAt(-2), ToReal(StackAt(-2)) < ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_le:
			SetBool(StackAt(-2), ToReal(StackAt(-2)) <= ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_land:
			SetBool(StackAt(-2), Truth(StackAt(-2)) && Truth(StackAt(-1)));
			Pop();
			break;
		case Op_lor:
			SetBool(StackAt(-2), Truth(StackAt(-2)) || Truth(StackAt(-1)));
			Pop();
			break;
		case Op_lnot:
			SetBool(StackAt(-1), !Truth(StackAt(-1)));
			break;
		case Op_add:
			lhs = &StackAt(-2);
			rhs = &StackAt(-1);
			real = ToReal(*lhs) + ToReal(*rhs);
			Pop();
			SetReal(*lhs, real);
			break;
		case Op_sub:
			lhs = &StackAt(-2);
			rhs = &StackAt(-1);
			real = ToReal(*lhs) - ToReal(*rhs);
			Pop();
			SetReal(*lhs, real);
			break;
		case Op_mul:
			lhs = &StackAt(-2);
			rhs = &StackAt(-1);
			real = ToReal(*lhs) * ToReal(*rhs);
			Pop();
			SetReal(*lhs, real);
			break;
		case Op_div:
			lhs = &StackAt(-2);
			rhs = &StackAt(-1);
			real = ToReal(*lhs) / ToReal(*rhs);
			Pop();
			SetReal(*lhs, real);
			break;
		case Op_mod:
			lhs = &StackAt(-2);
			rhs = &StackAt(-1);
			real = fmod(ToReal(*lhs), ToReal(*rhs));
			Pop();
			SetReal(*lhs, real);
			break;
		case Op_neg:
			SetReal(StackAt(-1), -ToReal(StackAt(-1)));
			break;
		case Op_round:
			SetReal(StackAt(-1), round(ToReal(StackAt(-1))));
			break;
		case Op_abs:
			SetReal(StackAt(-1), fabs(ToReal(StackAt(-1))));
			break;
		case Op_floor:
			SetReal(StackAt(-1), floor(ToReal(StackAt(-1))));
			break;
		case Op_ceil:
			SetReal(StackAt(-1), ceil(ToReal(StackAt(-1))));
			break;
		case Op_sqrt:
			SetReal(StackAt(-1), sqrt(ToReal(StackAt(-1))));
			break;
		case Op_sin:
			SetReal(StackAt(-1), sin(ToReal(StackAt(-1))));
			break;
		case Op_cos:
			SetReal(StackAt(-1), cos(ToReal(StackAt(-1))));
			break;
		case Op_tan:
			SetReal(StackAt(-1), tan(ToReal(StackAt(-1))));
			break;
		case Op_asin:
			SetReal(StackAt(-1), asin(ToReal(StackAt(-1))));
			break;
		case Op_acos:
			SetReal(StackAt(-1), acos(ToReal(StackAt(-1))));
			break;
		case Op_atan:
			SetReal(StackAt(-1), atan(ToReal(StackAt(-1))));
			break;
		case Op_ln:
			SetReal(StackAt(-1), log(ToReal(StackAt(-1))));
			break;
		case Op_log10:
			SetReal(StackAt(-1), log10(ToReal(StackAt(-1))));
			break;
		case Op_exp:
			SetReal(StackAt(0), exp(ToReal(StackAt(-1))));
			break;
		case Op_exp10:
			SetReal(StackAt(-1), pow(10.0, ToReal(StackAt(-1))));
			break;
		case Op_strcat:
			lhs = &StackAt(-2);
			rhs = &StackAt(-1);
			ConcatValue(*lhs, *rhs);
			Pop();
			break;
		case Op_charat:
			lhs = &StackAt(-2);
			i64 = ToInteger(StackAt(-1));
			Pop();
			SetChar(*lhs, ValueCharAt(*lhs, i64));
			break;
		case Op_strlen:
			SetInteger(StackAt(-1), ValueLength(StackAt(-1)));
			break;
		case Op_strstr:
			SetBool(StackAt(-2), ValueContains(StackAt(-1), StackAt(-2)));
			Pop();
			break;
		case Op_inc:
			SetReal(StackAt(-1), ToReal(StackAt(-1)) + 1.0);
			break;
		case Op_dec:
			SetReal(StackAt(-1), ToReal(StackAt(-1)) - 1.0);
			break;
		case Op_movesteps: {
			double steps = ToReal(StackAt(-1));
			Pop();

			double dir = (sprite->GetDirection() - 90.0) * DEG2RAD;
			double dx = steps * cos(dir);
			double dy = steps * sin(-dir); // y-axis is flipped

			sprite->SetXY(sprite->GetX() + dx, sprite->GetY() + dy);

			break;
		}
		case Op_turndegrees:
			sprite->SetDirection(ToReal(StackAt(-1)) + sprite->GetDirection());
			Pop();
			break;
		case Op_goto:
			Raise(NotImplemented, "goto");
		case Op_gotoxy:
			sprite->SetXY(ToReal(StackAt(-2)), ToReal(StackAt(-1)));
			Pop();
			Pop();
			break;
		case Op_glide:
			Raise(NotImplemented, "glide");
		case Op_glidexy:
			GlideTo(ToReal(StackAt(-2)), ToReal(StackAt(-1)), ToReal(StackAt(-3)));
			break;
		case Op_setdir:
			sprite->SetDirection(ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_lookat:
			Raise(NotImplemented, "lookat");
		case Op_addx:
			sprite->SetX(ToReal(StackAt(-1)) + sprite->GetX());
			Pop();
			break;
		case Op_setx:
			sprite->SetX(ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_addy:
			sprite->SetY(ToReal(StackAt(-1)) + sprite->GetY());
			Pop();
			break;
		case Op_sety:
			sprite->SetY(ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_bounceonedge:
			Raise(NotImplemented, "bounceonedge");
		case Op_setrotationstyle:
			sprite->SetRotationStyle((RotationStyle)*(uint8_t *)self->pc);
			self->pc++;
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
			sprite->SetMessage(StackAt(-1), false);
			Pop();
			break;
		case Op_think:
			sprite->SetMessage(StackAt(-1), true);
			Pop();
			break;
		case Op_setcostume: {
			Value &v = StackAt(-1);

			switch (v.type)
			{
			case ValueType_Integer:
				sprite->SetCostume(v.u.integer);
				break;
			case ValueType_Real:
				sprite->SetCostume(static_cast<int64_t>(round(v.u.real)));
				break;
			case ValueType_String:
				sprite->SetCostume(sprite->GetBase()->FindCostume(v.u.string));
				break;
			default:
				break; // do nothing
			}

			Pop();
			break;
		}
		case Op_nextcostume:
			sprite->SetCostume(sprite->GetCostume() + 1);
			break;
		case Op_setbackdrop: {
			Value &v = StackAt(-1);
			Sprite *stage = VM->GetStage();

			switch (v.type)
			{
			default:
				break;
			case ValueType_Integer:
				stage->SetCostume(v.u.integer);
				break;
			case ValueType_Real:
				stage->SetCostume(static_cast<int64_t>(round(v.u.real)));
				break;
			case ValueType_String:
				stage->SetCostume(stage->GetBase()->FindCostume(v.u.string));
				break;
			}

			Pop();
			break;
		}
		case Op_nextbackdrop: {
			Sprite *stage = VM->GetStage();
			stage->SetCostume(stage->GetCostume() + 1);
			break;
		}
		case Op_addsize:
			sprite->SetSize(sprite->GetSize() + ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_setsize:
			sprite->SetSize(ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_addgraphiceffect: {
			GraphicEffect effect = (GraphicEffect)*(uint8_t *)self->pc;
			self->pc++;

			double val = ToReal(StackAt(-1));
			Pop();

			GraphicEffectController &gec = sprite->GetGraphicEffects();
			switch (effect)
			{
			default:
				Raise(InvalidArgument, "Invalid graphic effect");
			case GraphicEffect_Color:
				gec.AddColorEffect(val);
				break;
			case GraphicEffect_Fisheye:
				gec.AddFisheyeEffect(val);
				break;
			case GraphicEffect_Whirl:
				gec.AddWhirlEffect(val);
				break;
			case GraphicEffect_Pixelate:
				gec.AddPixelateEffect(val);
				break;
			case GraphicEffect_Mosaic:
				gec.AddMosaicEffect(val);
				break;
			case GraphicEffect_Brightness:
				gec.AddBrightnessEffect(val);
				break;
			case GraphicEffect_Ghost:
				gec.AddGhostEffect(val);
				break;
			}

			break;
		}
		case Op_setgraphiceffect: {
			GraphicEffect effect = (GraphicEffect)*(uint8_t *)self->pc;
			self->pc++;

			double val = ToReal(StackAt(-1));
			Pop();

			GraphicEffectController &gec = sprite->GetGraphicEffects();

			switch (effect)
			{
			default:
				Raise(InvalidArgument, "Invalid graphic effect");
			case GraphicEffect_Color:
				gec.SetColorEffect(val);
				break;
			case GraphicEffect_Fisheye:
				gec.SetFisheyeEffect(val);
				break;
			case GraphicEffect_Whirl:
				gec.SetWhirlEffect(val);
				break;
			case GraphicEffect_Pixelate:
				gec.SetPixelateEffect(val);
				break;
			case GraphicEffect_Mosaic:
				gec.SetMosaicEffect(val);
				break;
			case GraphicEffect_Brightness:
				gec.SetBrightnessEffect(val);
				break;
			case GraphicEffect_Ghost:
				gec.SetGhostEffect(val);
				break;
			}

			break;
		}
		case Op_cleargraphiceffects:
			sprite->GetGraphicEffects().ClearEffects();
			break;
		case Op_show:
			sprite->SetVisible(true);
			break;
		case Op_hide:
			sprite->SetVisible(false);
			break;
		case Op_gotolayer: {
			Sprite *stage = VM->GetStage();
			if (sprite == stage)
				break;

			SpriteList *sprites = VM->GetSpriteList();

			switch (*(uint8_t *)self->pc)
			{
			default:
				Raise(InvalidArgument, "Invalid layer");
			case LayerType_Front:
				sprites->Insert(stage, sprite);
				break;
			case LayerType_Back:
				sprites->Insert(sprites->Tail(), sprite);
				break;
			}

			self->pc++;
			break;
		}
		case Op_movelayer: {
			Sprite *stage = VM->GetStage();
			if (sprite == VM->GetStage())
				break;

			int64_t amount = ToInteger(StackAt(-1));
			Pop();

			SpriteList *sprites = VM->GetSpriteList();

			switch (*(uint8_t *)self->pc)
			{
			default:
				Raise(InvalidArgument, "Invalid direction");
			case LayerDir_Forward:
				sprites->Move(sprite, amount);
				break;
			case LayerDir_Backward:
				sprites->Move(sprite, -amount);
				break;
			}

			self->pc++;
			break;
		}
		case Op_getcostume:
			SetInteger(Push(), sprite->GetCostume());
			break;
		case Op_getcostumename: {
			int64_t costume = sprite->GetCostume();
			Costume *c = sprite->GetBase()->GetCostume(costume);
			Assign(Push(), c->GetNameValue());
			break;
		}
		case Op_getbackdrop:
			Raise(NotImplemented, "getbackdrop");
		case Op_getsize:
			SetReal(Push(), sprite->GetSize());
			break;
		case Op_playsoundandwait: {
			Value &v = CvtString(StackAt(-1));
			if (v.type != ValueType_String)
			{
				Pop();
				break;
			}

			int64_t sid = sprite->GetBase()->FindSound(v.u.string);
			Voice *voice = sprite->GetVoiceFor(sid);
			if (voice == nullptr)
			{
				Pop();
				break;
			}
						
			Pop();

			// play sound
			VM->StartVoice(voice);
			WaitForVoice(voice);
			break;
		}
		case Op_playsound: {
			Value &v = CvtString(StackAt(-1));
			if (v.type != ValueType_String)
			{
				Pop();
				break;
			}

			int64_t sid = sprite->GetBase()->FindSound(v.u.string);
			Voice *voice = sprite->GetVoiceFor(sid);
			if (voice == nullptr)
			{
				Pop();
				break;
			}

			Pop();

			// play sound
			VM->StartVoice(voice);
			break;
		}
		case Op_stopsound:
			VM->StopAllSounds();
			break;
		case Op_addsoundeffect: {
			SoundEffect effect = (SoundEffect)*(uint8_t *)self->pc;
			self->pc++;

			DSPController &dsp = sprite->GetDSP();
			switch (effect)
			{
			default:
				Raise(InvalidArgument, "Invalid sound effect");
			case SoundEffect_Pitch:
				dsp.AddPitch(ToReal(StackAt(-1)));
				break;
			case SoundEffect_Pan:
				dsp.AddPan(ToReal(StackAt(-1)));
				break;
			}

			Pop();
			break;
		}
		case Op_setsoundeffect: {
			SoundEffect effect = (SoundEffect)*(uint8_t *)self->pc;
			self->pc++;

			DSPController &dsp = sprite->GetDSP();
			switch (effect)
			{
			default:
				Raise(InvalidArgument, "Invalid sound effect");
			case SoundEffect_Pitch:
				dsp.SetPitch(ToReal(StackAt(-1)));
				break;
			case SoundEffect_Pan:
				dsp.SetPan(ToReal(StackAt(-1)));
				break;
			}

			Pop();
			break;
		}
		case Op_clearsoundeffects: {
			sprite->GetDSP().ClearEffects();
			break;
		}
		case Op_addvolume: {
			DSPController &dsp = sprite->GetDSP();
			dsp.AddVolume(ToReal(StackAt(-1)));
			Pop();
			break;
		}
		case Op_setvolume: {
			DSPController &dsp = sprite->GetDSP();
			dsp.SetVolume(ToReal(StackAt(-1)));
			Pop();
			break;
		}
		case Op_getvolume:
			SetReal(Push(), sprite->GetDSP().GetVolume());
			break;
		case Op_onflag:
			// do nothing
			break;
		case Op_onkey:
			self->pc += sizeof(uint16_t); // skip key
			break;
		case Op_onclick:
			// do nothing
			break;
		case Op_onbackdropswitch: {
			Sprite *stage = VM->GetStage();
			char *targetName = (char *)(bytecode + *(uint64_t *)self->pc);
			self->pc += sizeof(uint64_t);

			int64_t lastBackdrop = stage->GetCostume();
			for (;;)
			{
				// check if backdrop has changed
				int64_t currentBackdrop = stage->GetCostume();
				if (lastBackdrop == currentBackdrop)
				{
					Sched();
					continue;
				}

				lastBackdrop = currentBackdrop;

				// if matching target, break
				const Value &name = stage->GetCostumeName();
				if (!strcmp(name.u.string->str, targetName))
					break;
			}
			break;
		}
		case Op_ongt:
			// do nothing (handled in bytecode, see compiler.cpp)
			break;
		case Op_onevent:
			self->pc += sizeof(uint64_t); // skip event
			break;
		case Op_send: {
			int64_t len;
			const char *message = ToString(StackAt(-1), &len);
			VM->Send(std::string(message, len));
			Pop();
			break;
		}
		case Op_sendandwait: {
			int64_t len;
			const char *message = ToString(StackAt(-1), &len);
			VM->SendAndWait(std::string(message, len));
			Pop();
			break;
		}
		case Op_findevent:
			Raise(NotImplemented, "findevent");
		case Op_waitsecs:
			Sleep(ToReal(StackAt(-1)));
			Pop();
			break;
		case Op_stopall: {
			for (unsigned long sid = 0; sid < MAX_SCRIPTS; sid++)
			{
				Script *script = VM->OpenScript(sid);
				if (script && script != self)
					VM->TerminateScript(script);
			}

			Terminate();
		}
		case Op_stopself:
			Terminate();
		case Op_stopother: {
			for (unsigned long sid = 0; sid < MAX_SCRIPTS; sid++)
			{
				Script *script = VM->OpenScript(sid);
				if (script && script->sprite == sprite && script != self)
					VM->TerminateScript(script);
			}

			break;
		}
		case Op_onclone:
			// do nothing
			break;
		case Op_clone: {
			Value &targetName = CvtString(StackAt(-1));
			if (targetName.type != ValueType_String)
			{
				Pop();
				break;
			}

			if (StringEquals(targetName.u.string->str, "_myself_"))
				self->sprite->Clone();
			else
			{
				Sprite *target = VM->FindSprite(targetName);
				if (target)
					target->Clone();
			}

			Pop();
			break;
		}
		case Op_deleteclone:
			// Only destroy clones, not the original sprite
			if (self->sprite->GetInstanceId() != BASE_INSTANCE_ID)
				self->sprite->Destroy();
			break;
		case Op_touching: {
			Value &v = CvtString(StackAt(-1)); // same as stack[0] = CvtString(stack[0]);
			if (v.type != ValueType_String)
			{
				SetBool(v, false);
				break;
			}

			if (!strcmp(v.u.string->str, "_mouse_"))
			{
				auto &io = VM->GetIO();
				SetBool(v, sprite->TouchingPoint(Vector2(io.GetMouseX(), io.GetMouseY())));
				break;
			}

			Sprite *s = VM->FindSprite(v);
			if (!s)
			{
				SetBool(v, false);
				break;
			}

			SetBool(v, sprite->TouchingSprite(s));
			break;
		}
		case Op_touchingcolor:
			Raise(NotImplemented, "touchingcolor");
		case Op_colortouching:
			Raise(NotImplemented, "colortouching");
		case Op_distanceto:
			Raise(NotImplemented, "distanceto");
		case Op_ask:
			Raise(NotImplemented, "ask");
		case Op_getanswer:
			Assign(Push(), VM->GetIO().GetAnswer());
			break;
		case Op_keypressed: {
			Value &v = CvtString(StackAt(-1));
			if (v.type != ValueType_String)
			{
				SetBool(v, false);
				break;
			}

			String *s = v.u.string;
			int scancode;
			if (s->len == 1)
			{
				char c = tolower(s->str[0]);
				if (c >= 'a' && c <= 'z')
					scancode = SDL_SCANCODE_A + (c - 'a');
				else if (c >= '0' && c <= '9')
					scancode = SDL_SCANCODE_0 + (c - '0');
				else
				{
					SetBool(v, false);
					break;
				}
			}
			else if (StringEqualsRaw(s->str, "space"))
				scancode = SDL_SCANCODE_SPACE;
			else if (StringEqualsRaw(s->str, "up arrow"))
				scancode = SDL_SCANCODE_UP;
			else if (StringEqualsRaw(s->str, "down arrow"))
				scancode = SDL_SCANCODE_DOWN;
			else if (StringEqualsRaw(s->str, "right arrow"))
				scancode = SDL_SCANCODE_RIGHT;
			else if (StringEqualsRaw(s->str, "left arrow"))
				scancode = SDL_SCANCODE_LEFT;
			else if (StringEqualsRaw(s->str, "any"))
				scancode = -1;
			else
			{
				SetBool(v, false);
				break;
			}

			SetBool(v, VM->GetIO().GetKey(scancode));
			break;
		}
		case Op_mousedown:
			SetBool(Push(), VM->GetIO().IsMouseDown());
			break;
		case Op_mousex:
			SetReal(Push(), VM->GetIO().GetMouseX());
			break;
		case Op_mousey:
			SetReal(Push(), VM->GetIO().GetMouseY());
			break;
		case Op_setdragmode:
			Raise(NotImplemented, "setdragmode");
		case Op_getloudness:
			Raise(NotImplemented, "getloudness");
		case Op_gettimer:
			SetReal(Push(), VM->GetTimer());
			break;
		case Op_resettimer:
			VM->ResetTimer();
			break;
		case Op_propertyof: {
			PropertyTarget target = (PropertyTarget)*(uint8_t *)self->pc;
			self->pc++;

			Sprite *s = VM->FindSprite(CvtString(StackAt(-1)));
			Pop(); // pop name

			if (!s)
			{
				if (target == PropertyTarget_Variable)
					Pop(); // variable name
				Push(); // none
				break;
			}

			switch (target)
			{
			default:
				Push(); // none
				break;
			case PropertyTarget_BackdropNumber:
				SetInteger(Push(), VM->GetStage()->GetCostume());
				break;
			case PropertyTarget_BackdropName:
				Assign(Push(), VM->GetStage()->GetCostumeName());
				break;
			case PropertyTarget_XPosition:
				SetReal(Push(), s->GetX());
				break;
			case PropertyTarget_YPosition:
				SetReal(Push(), s->GetY());
				break;
			case PropertyTarget_CostumeNumber:
				SetInteger(Push(), s->GetCostume());
				break;
			case PropertyTarget_CostumeName:
				Assign(Push(), s->GetCostumeName());
				break;
			case PropertyTarget_Size:
				SetReal(Push(), s->GetSize());
				break;
			case PropertyTarget_Volume:
				SetReal(Push(), s->GetDSP().GetVolume());
				break;
			case PropertyTarget_Variable:
				Pop();
				Push(); // TODO: get variable value
				break;
			}

			break;
		}
		case Op_gettime:
			Raise(NotImplemented, "gettime");
		case Op_getdayssince2000:
			Raise(NotImplemented, "getdayssince2000");
		case Op_getusername:
			Assign(Push(), VM->GetIO().GetUsername());
			break;
		case Op_rand: {
			Value &a = StackAt(-2);
			Value &b = StackAt(-1);

			if (a.type == ValueType_Real || b.type == ValueType_Real)
			{
				double min = ToReal(a);
				double max = ToReal(b);

				if (max < min)
				{
					double temp = min;
					min = max;
					max = temp;
				}
			
				SetReal(a, min + (ls_rand_double() * (max - min)));
				Pop();
			}
			else
			{
				int64_t min = ToInteger(a);
				int64_t max = ToInteger(b);

				if (max < min)
				{
					int64_t temp = min;
					min = max;
					max = temp;
				}

				SetInteger(a, min + (ls_rand_uint64() % (max - min + 1)));
				Pop();
			}

			break;
		}
		case Op_varshow:
			Pop();
			break;
		case Op_varhide:
			Pop();
			break;
		case Op_listadd:
			ListAppend(StackAt(-1), StackAt(-2));
			Pop();
			Pop();
			break;
		case Op_listremove:
			ListDelete(StackAt(-1), StackAt(-2));
			Pop();
			Pop();
			break;
		case Op_listclear:
			ListClear(StackAt(-1));
			Pop();
			break;
		case Op_listinsert:
			ListInsert(StackAt(-1), ToInteger(StackAt(-2)), StackAt(-3));
			Pop();
			Pop();
			Pop();
			break;
		case Op_listreplace:
			ListSet(StackAt(-1), ToInteger(StackAt(-2)), StackAt(-3));
			Pop();
			Pop();
			Pop();
			break;
		case Op_listat:
			ListGet(StackAt(-2), StackAt(-1), ToInteger(StackAt(-2)));
			Pop();
			break;
		case Op_listfind:
			SetInteger(StackAt(-2), ListIndexOf(StackAt(-1), StackAt(-2)));
			Pop();
			break;
		case Op_listlen:
			SetInteger(StackAt(-1), ListGetLength(StackAt(-1)));
			break;
		case Op_listcontains:
			SetBool(StackAt(-2), ListContainsValue(StackAt(-1), StackAt(-2)));
			Pop();
			break;
		case Op_ext:
			Raise(VMError, "Extensions are not supported");
		}
	}

	return 0;
}

Value &Push()
{
	Script *self = VM->GetCurrentScript();
	if (self->sp <= self->stack)
		Raise(StackOverflow, "Stack overflow");
	self->sp--;
	InitializeValue(*self->sp);
	return *self->sp;
}

void Pop()
{
	Script *self = VM->GetCurrentScript();
	if (self->sp >= self->stack + STACK_SIZE)
		Raise(StackUnderflow, "Stack underflow");
	ReleaseValue(*self->sp);
#if _DEBUG
	memset(self->sp, 0xab, sizeof(Value)); // fill with garbage
#endif // _DEBUG
	self->sp++;
}

Value &StackAt(int i)
{
	Script *self = VM->GetCurrentScript();

	Value *val;
	if (i < 0)
	{
		val = self->sp - i - 1;
		if (val >= self->bp)
			Raise(AccessViolation, "Stack index out of bounds");
	}
	else
	{
		val = self->bp - i - 1;
		if (val < self->sp)
			Raise(AccessViolation, "Stack index out of bounds");
	}

	return *val;
}

void Sched()
{
	ls_fiber_sched();

	Script *self = VM->GetCurrentScript();
	if (self->restart)
	{
		assert(self->sp == self->stack + STACK_SIZE);
		longjmp(self->entryJmp, 1);
	}
}

void LS_NORETURN Terminate()
{
	VM->TerminateScript(VM->GetCurrentScript());
	VM->Panic("Terminated script rescheduled");
}

void LS_NORETURN Raise(ExceptionType type, const char *message)
{
	Script *self = VM->GetCurrentScript();
	self->except = type;
	self->exceptMessage = message;
	Terminate();
}

void Sleep(double seconds)
{
	Script *self = VM->GetCurrentScript();
	self->sleepUntil = VM->GetTime() + seconds;
	self->state = WAITING;
	Sched();
}

void WaitForVoice(Voice *voice)
{
	Script *self = VM->GetCurrentScript();
	self->waitVoice = voice;
	self->state = WAITING;
	Sched();
}

void GlideTo(double x, double y, double t)
{
	Script *self = VM->GetCurrentScript();
	Sprite *sprite = self->sprite;

	if (t <= 0.0)
	{
		sprite->SetXY(x, y);
		Sched();
		return;
	}

	GlideInfo &glide = sprite->GetGlideInfo();

	glide.x0 = sprite->GetX();
	glide.y0 = sprite->GetY();
	glide.x1 = x;
	glide.y1 = y;
	glide.start = VM->GetTime();
	glide.end = glide.start + t;

	self->state = WAITING;

	Sched();
}

void AskWait(const std::string &question)
{
	Script *self = VM->GetCurrentScript();
	self->askInput = true;
	self->state = WAITING;
	VM->EnqueueAsk(self, question);
	Sched();
}
