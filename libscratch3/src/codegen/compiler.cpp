#include "compiler.hpp"

#include <cstdio>

#include "opcode.hpp"
#include "util.hpp"

#include <SDL.h>

struct ProcInfo
{
	ProcProto *proto;

	bool FindArgument(const std::string &name, int16_t *arg) const
	{
		// TODO: check if out of uint16_t range

		for (size_t i = 0; i < proto->arguments.size(); ++i)
		{
			if (proto->arguments[i].second == name)
			{
				*arg = i;
				return true;
			}
		}

		return false;
	}
};

class Compiler : public Visitor
{
public:
	virtual void Visit(Constexpr *node)
	{
		Value v;
		InitializeValue(v);
		SetParsedString(v, node->value);

		cp.PushValue(v);

		ReleaseValue(v);
	}

	virtual void Visit(XPos *node)
	{
		cp.WriteOpcode(Op_getx);
	}

	virtual void Visit(YPos *node)
	{
		cp.WriteOpcode(Op_gety);
	}

	virtual void Visit(Direction *node)
	{
		cp.WriteOpcode(Op_getdir);
	}

	virtual void Visit(CurrentCostume *node)
	{
		cp.WriteOpcode(Op_getcostume);
	}

	virtual void Visit(CurrentBackdrop *node)
	{
		cp.WriteOpcode(Op_getbackdrop);
	}

	virtual void Visit(Size *node)
	{
		cp.WriteOpcode(Op_getsize);
	}

	virtual void Visit(Volume *node)
	{
		cp.WriteOpcode(Op_getvolume);
	}

	virtual void Visit(Touching *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_touching);
	}

	virtual void Visit(TouchingColor *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_touchingcolor);
	}

	virtual void Visit(ColorTouching *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_colortouching);
	}

	virtual void Visit(DistanceTo *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_distanceto);
	}

	virtual void Visit(Answer *node)
	{
		cp.WriteOpcode(Op_getanswer);
	}

	virtual void Visit(KeyPressed *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_keypressed);
	}

	virtual void Visit(MouseDown *node)
	{
		cp.WriteOpcode(Op_mousedown);
	}

	virtual void Visit(MouseX *node)
	{
		cp.WriteOpcode(Op_mousex);
	}

	virtual void Visit(MouseY *node)
	{
		cp.WriteOpcode(Op_mousey);
	}

	virtual void Visit(Loudness *node)
	{
		cp.WriteOpcode(Op_getloudness);
	}

	virtual void Visit(TimerValue *node)
	{
		cp.WriteOpcode(Op_gettimer);
	}

	virtual void Visit(PropertyOf *node)
	{
		if (node->target == PropertyTarget_Variable)
			cp.PushString(node->id);

		node->e->Accept(this);

		cp.WriteOpcode(Op_propertyof);
		cp.WriteText<uint8_t>(node->target);
	}

	virtual void Visit(CurrentDate *node)
	{
		cp.WriteOpcode(Op_gettime);
		cp.WriteText<uint8_t>(node->format);
	}

	virtual void Visit(DaysSince2000 *node)
	{
		cp.WriteOpcode(Op_getdayssince2000);
	}

	virtual void Visit(Username *node)
	{
		cp.WriteOpcode(Op_getusername);
	}

	virtual void Visit(Add *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_add);
	}

	virtual void Visit(Sub *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_sub);
	}

	virtual void Visit(Mul *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_mul);
	}

	virtual void Visit(Div *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_div);
	}

	virtual void Visit(Random *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_rand);
	}

	virtual void Visit(Greater *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_gt);
	}

	virtual void Visit(Less *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_lt);
	}

	virtual void Visit(Equal *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_eq);
	}

	virtual void Visit(LogicalAnd *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_land);
	}

	virtual void Visit(LogicalOr *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_lor);
	}

	virtual void Visit(LogicalNot *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_lnot);
	}

	virtual void Visit(Concat *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_strcat);
	}

	virtual void Visit(CharAt *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_charat);
	}

	virtual void Visit(StringLength *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_strlen);
	}

	virtual void Visit(StringContains *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_strstr);
	}

	virtual void Visit(Mod *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_mod);
	}

	virtual void Visit(Round *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_round);
	}

	virtual void Visit(MathFunc *node)
	{
		node->e->Accept(this);
		switch (node->func)
		{
		default:
			cp.WriteOpcode(Op_pop);
			// TODO: push null or throw error
			break;
		case MathFuncType_Abs:
			cp.WriteOpcode(Op_abs);
			break;
		case MathFuncType_Floor:
			cp.WriteOpcode(Op_floor);
			break;
		case MathFuncType_Ceil:
			cp.WriteOpcode(Op_ceil);
			break;
		case MathFuncType_Sqrt:
			cp.WriteOpcode(Op_sqrt);
			break;
		case MathFuncType_Sin:
			cp.WriteOpcode(Op_sin);
			break;
		case MathFuncType_Cos:
			cp.WriteOpcode(Op_cos);
			break;
		case MathFuncType_Tan:
			cp.WriteOpcode(Op_tan);
			break;
		case MathFuncType_Asin:
			cp.WriteOpcode(Op_asin);
			break;
		case MathFuncType_Acos:
			cp.WriteOpcode(Op_acos);
			break;
		case MathFuncType_Atan:
			cp.WriteOpcode(Op_atan);
			break;
		case MathFuncType_Ln:
			cp.WriteOpcode(Op_ln);
			break;
		case MathFuncType_Log:
			cp.WriteOpcode(Op_log10);
			break;
		case MathFuncType_Exp:
			cp.WriteOpcode(Op_exp);
			break;
		case MathFuncType_Exp10:
			cp.WriteOpcode(Op_exp10);
			break;
		}
	}

	virtual void Visit(VariableExpr *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined variable %s\n", node->id.c_str());
			abort();
		}

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);
	}

	virtual void Visit(BroadcastExpr *node)
	{
		cp.PushString(node->id);
	}

	virtual void Visit(ListExpr *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);
	}

	virtual void Visit(ListAccess *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		node->e->Accept(this);

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listat);
	}

	virtual void Visit(IndexOf *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		node->e->Accept(this);

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listfind);
	}

	virtual void Visit(ListLength *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listlen);
	}

	virtual void Visit(ListContains *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		node->e->Accept(this);

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listcontains);
	}

	virtual void Visit(StatementList *node)
	{
		bool oldTopLevel = topLevel;
		topLevel = false;

		for (AutoRelease<Statement> &stmt : node->sl)
			stmt->Accept(this);

		topLevel = oldTopLevel;

		if (topLevel)
		{
			// implicit stop
			cp.WriteOpcode(Op_stopself);

			cp.WriteOpcode(Op_int);
			cp.AlignText();
		}
	}

	virtual void Visit(MoveSteps *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_movesteps);
	}

	virtual void Visit(TurnDegrees *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_turndegrees);
	}

	virtual void Visit(TurnNegDegrees *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_neg);
		cp.WriteOpcode(Op_turndegrees);
	}

	virtual void Visit(Goto *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_goto);
	}

	virtual void Visit(GotoXY *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_gotoxy);
	}

	virtual void Visit(Glide *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.WriteOpcode(Op_glide);
	}

	virtual void Visit(GlideXY *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		node->e3->Accept(this);
		cp.WriteOpcode(Op_glidexy);
	}

	virtual void Visit(PointDir *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_setdir);
	}

	virtual void Visit(PointTowards *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_lookat);
	}

	virtual void Visit(ChangeX *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_addx);
	}

	virtual void Visit(SetX *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_setx);
	}

	virtual void Visit(ChangeY *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_addy);
	}

	virtual void Visit(SetY *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_sety);
	}

	virtual void Visit(BounceIfOnEdge *node)
	{
		cp.WriteOpcode(Op_bounceonedge);
	}

	virtual void Visit(SetRotationStyle *node)
	{
		cp.WriteOpcode(Op_setrotationstyle);
		cp.WriteText<uint8_t>(node->style);
	}

	virtual void Visit(SayForSecs *node)
	{
		node->e1->Accept(this);
		cp.WriteOpcode(Op_say);

		node->e2->Accept(this);
		cp.WriteOpcode(Op_waitsecs);
		
		// clear speech bubble
		cp.WriteOpcode(Op_pushnone);
		cp.WriteOpcode(Op_say);
	}

	virtual void Visit(Say *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_say);
	}

	virtual void Visit(ThinkForSecs *node)
	{
		node->e1->Accept(this);
		cp.WriteOpcode(Op_think);

		node->e2->Accept(this);
		cp.WriteOpcode(Op_waitsecs);

		// clear thought bubble
		cp.WriteOpcode(Op_pushnone);
		cp.WriteOpcode(Op_think);
	}

	virtual void Visit(Think *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_think);
	}

	virtual void Visit(SwitchCostume *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_setcostume);
	}

	virtual void Visit(NextCostume *node)
	{
		cp.WriteOpcode(Op_nextcostume);
	}

	virtual void Visit(SwitchBackdrop *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_setbackdrop);
	}

	virtual void Visit(SwitchBackdropAndWait *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_setbackdrop);
		// TODO: Implement wait
		
		printf("Warning: SwitchBackdropAndWait will not wait\n");
	}

	virtual void Visit(NextBackdrop *node)
	{
		cp.WriteOpcode(Op_nextbackdrop);
	}

	virtual void Visit(ChangeSize *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_addsize);
	}

	virtual void Visit(SetSize *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_setsize);
	}

	virtual void Visit(ChangeGraphicEffect *node)
	{
		node->e->Accept(this);

		cp.WriteOpcode(Op_addgraphiceffect);
		cp.WriteText<uint8_t>(node->effect);
	}

	virtual void Visit(SetGraphicEffect *node)
	{
		node->e->Accept(this);

		cp.WriteOpcode(Op_setgraphiceffect);
		cp.WriteText<uint8_t>(node->effect);
	}

	virtual void Visit(ClearGraphicEffects *node)
	{
		cp.WriteOpcode(Op_cleargraphiceffects);
	}

	virtual void Visit(ShowSprite *node)
	{
		cp.WriteOpcode(Op_show);
	}

	virtual void Visit(HideSprite *node)
	{
		cp.WriteOpcode(Op_hide);
	}

	virtual void Visit(GotoLayer *node)
	{
		cp.WriteOpcode(Op_gotolayer);
		cp.WriteText<uint8_t>(node->layer);
	}

	virtual void Visit(MoveLayer *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_movelayer);
		cp.WriteText<uint8_t>(node->direction);
	}

	virtual void Visit(PlaySoundUntilDone *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_playsoundandwait);
	}

	virtual void Visit(StartSound *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_playsound);
	}

	virtual void Visit(StopAllSounds *node)
	{
		cp.WriteOpcode(Op_stopsound);
	}

	virtual void Visit(ChangeSoundEffect *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_addsoundeffect);
		cp.WriteText<uint8_t>(node->effect);
	}

	virtual void Visit(SetSoundEffect *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_setsoundeffect);
		cp.WriteText<uint8_t>(node->effect);
	}

	virtual void Visit(ClearSoundEffects *node)
	{
		cp.WriteOpcode(Op_clearsoundeffects);
	}

	virtual void Visit(ChangeVolume *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_addvolume);
	}

	virtual void Visit(SetVolume *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_setvolume);
	}

	virtual void Visit(OnFlagClicked *node)
	{
		cp.WriteOpcode(Op_onflag);
	}

	virtual void Visit(OnKeyPressed *node)
	{
		uint16_t scancode = 0;
		if (node->key.size() == 1)
		{
			char c = node->key[0];
			if (c >= 'a' && c <= 'z')
				scancode = SDL_SCANCODE_A + c - 'a';
			else if (c >= 'A' && c <= 'Z')
				scancode = SDL_SCANCODE_A + c - 'A';
			else if (c >= '0' && c <= '9')
				scancode = SDL_SCANCODE_0 + c - '0';
		}
		else if (StringEqualsRaw(node->key.c_str(), "space"))
			scancode = SDL_SCANCODE_SPACE;
		else if (StringEqualsRaw(node->key.c_str(), "left arrow"))
			scancode = SDL_SCANCODE_LEFT;
		else if (StringEqualsRaw(node->key.c_str(), "right arrow"))
			scancode = SDL_SCANCODE_RIGHT;
		else if (StringEqualsRaw(node->key.c_str(), "up arrow"))
			scancode = SDL_SCANCODE_UP;
		else if (StringEqualsRaw(node->key.c_str(), "down arrow"))
			scancode = SDL_SCANCODE_DOWN;
		else if (StringEqualsRaw(node->key.c_str(), "any"))
			scancode = -1;

		cp.WriteOpcode(Op_onkey);
		cp.WriteText(scancode);
	}

	virtual void Visit(OnSpriteClicked *node)
	{
		cp.WriteOpcode(Op_onclick);
	}

	virtual void Visit(OnStageClicked *node)
	{
		cp.WriteOpcode(Op_onclick);
	}

	virtual void Visit(OnBackdropSwitch *node)
	{
		cp.WriteOpcode(Op_onbackdropswitch);
		cp.WriteString(Segment_text, node->backdrop);
	}

	virtual void Visit(OnGreaterThan *node)
	{
		// semantics:
		//  (entry)
		//  wait until condition is false
		//  wait until condition is true

		cp.WriteOpcode(Op_ongt);

		uint64_t waitFalseTop = cp._text.size();
		switch (node->value)
		{
		default:
			abort();
			break;
		case ListenValueType_Loudness:
			cp.WriteOpcode(Op_getloudness);
			break;
		case ListenValueType_Timer:
			cp.WriteOpcode(Op_gettimer);
			break;
		}

		node->e->Accept(this);
		cp.WriteOpcode(Op_gt);

		cp.WriteOpcode(Op_yield);

		// loop until condition is false
		cp.WriteOpcode(Op_jnz);
		cp.WriteReference(Segment_text, Segment_text, waitFalseTop);

		uint64_t waitTrueTop = cp._text.size();
		switch (node->value)
		{
		default:
			abort();
			break;
		case ListenValueType_Loudness:
			cp.WriteOpcode(Op_getloudness);
			break;
		case ListenValueType_Timer:
			cp.WriteOpcode(Op_gettimer);
			break;
		}

		node->e->Accept(this);
		cp.WriteOpcode(Op_gt);

		cp.WriteOpcode(Op_yield);

		// loop until condition is true
		cp.WriteOpcode(Op_jz);
		cp.WriteReference(Segment_text, Segment_text, waitTrueTop);

		// done
	}

	virtual void Visit(OnEvent *node)
	{
		cp.WriteOpcode(Op_onevent);
		cp.WriteString(Segment_text, node->message);
	}

	virtual void Visit(Broadcast *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_send);
	}

	virtual void Visit(BroadcastAndWait *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_sendandwait);
	}

	virtual void Visit(WaitSecs *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_waitsecs);
	}

	virtual void Visit(Repeat *node)
	{
		Value zero;
		InitializeValue(zero);
		SetInteger(zero, 0);

		// push counter
		node->e->Accept(this);
		cp.WriteOpcode(Op_round);

		uint64_t top = cp._text.size();

		// check counter
		cp.WriteOpcode(Op_push);
		cp.WriteText<int16_t>(-1); // top of stack

		cp.PushValue(zero);
		cp.WriteOpcode(Op_gt);
		cp.WriteOpcode(Op_jz);
		size_t jz = cp.WriteReference(Segment_text, Segment_text);

		if (node->sl)
			node->sl->Accept(this);

		cp.WriteOpcode(Op_yield);

		// decrement counter
		cp.WriteOpcode(Op_dec);

		// jump back to top
		cp.WriteAbsoluteJump(Op_jmp, top);

		cp.SetReference(jz, Segment_text, cp._text.size()); // set jump destination

		// pop counter
		cp.WriteOpcode(Op_pop);

		ReleaseValue(zero);
	}

	virtual void Visit(Forever *node)
	{
		int64_t start = cp._text.size();

		if (node->sl)
			node->sl->Accept(this);

		cp.WriteOpcode(Op_yield);
		cp.WriteAbsoluteJump(Op_jmp, start);
	}

	virtual void Visit(If *node)
	{
		if (!node->sl)
			return; // empty if substack, discard

		node->e->Accept(this);

		cp.WriteOpcode(Op_jz);
		size_t jz = cp.WriteReference(Segment_text, Segment_text);

		node->sl->Accept(this);

		cp.SetReference(jz, Segment_text, cp._text.size()); // set jump destination
	}

	virtual void Visit(IfElse *node)
	{
		if (!node->sl1 && !node->sl2)
			return; // both if and else substacks are empty, discard

		if (!node->sl1)
		{
			// no true substack
			// functionally equivalent to If with inverted condition

			node->e->Accept(this);

			cp.WriteOpcode(Op_jnz);
			size_t jnz = cp.WriteReference(Segment_text, Segment_text);

			node->sl2->Accept(this);

			cp.SetReference(jnz, Segment_text, cp._text.size()); // set jump destination

			return;
		}
		else if (!node->sl2)
		{
			// no false substack
			// functionally equivalent to If

			AutoRelease<If> ifNode = new If();
			ifNode->e = node->e;
			ifNode->sl = node->sl1;

			Visit(ifNode.get());

			return;
		}

		node->e->Accept(this);

		// conditional jump to else block
		cp.WriteOpcode(Op_jz);
		size_t jz = cp.WriteReference(Segment_text, Segment_text);

		node->sl1->Accept(this);

		// unconditional jump to end
		cp.WriteOpcode(Op_jmp);
		size_t trueJmp = cp.WriteReference(Segment_text, Segment_text);

		// set jump destination for else block
		cp.SetReference(jz, Segment_text, cp._text.size());

		node->sl2->Accept(this);

		// set jump destination for end
		cp.SetReference(trueJmp, Segment_text, cp._text.size());
	}

	virtual void Visit(WaitUntil *node)
	{
		uint64_t top;

		top = cp._text.size();
		node->e->Accept(this);

		cp.WriteOpcode(Op_jnz);
		size_t jnz = cp.WriteReference(Segment_text, Segment_text);
		cp.WriteText<int64_t>(0); // placeholder

		cp.WriteOpcode(Op_yield);
		cp.WriteAbsoluteJump(Op_jmp, top);

		// set jump destination for condition
		cp.SetReference(jnz, Segment_text, cp._text.size());
	}

	virtual void Visit(RepeatUntil *node)
	{
		int64_t top;

		top = cp._text.size();
		node->e->Accept(this);

		cp.WriteOpcode(Op_jnz);
		size_t jnz = cp.WriteReference(Segment_text, Segment_text);

		if (node->sl)
			node->sl->Accept(this);

		cp.WriteOpcode(Op_yield);
		cp.WriteAbsoluteJump(Op_jmp, top);

		// set jump destination for condition
		cp.SetReference(jnz, Segment_text, cp._text.size());
	}

	virtual void Visit(Stop *node)
	{
		switch (node->mode)
		{
		default:
		case StopMode_All:
			cp.WriteOpcode(Op_stopall);
			break;
		case StopMode_ThisScript:
			cp.WriteOpcode(Op_stopself);
			break;
		case StopMode_OtherScriptsInSprite:
			cp.WriteOpcode(Op_stopother);
			break;
		}
	}

	virtual void Visit(CloneStart *node)
	{
		cp.WriteOpcode(Op_onclone);
	}

	virtual void Visit(CreateClone *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_clone);
	}

	virtual void Visit(DeleteClone *node)
	{
		cp.WriteOpcode(Op_deleteclone);
	}

	virtual void Visit(AskAndWait *node)
	{
		cp.WriteOpcode(Op_ask);
	}

	virtual void Visit(SetDragMode *node)
	{
		cp.WriteOpcode(Op_setdragmode);
		cp.WriteText<uint8_t>(node->mode);
	}

	virtual void Visit(ResetTimer *node)
	{
		cp.WriteOpcode(Op_resettimer);
	}

	virtual void Visit(SetVariable *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Undefined variable %s\n", node->id.c_str());
			abort();
		}

		node->e->Accept(this);

		cp.WriteOpcode(Op_setstatic);
		cp.WriteText<bc::VarId>(it->second);
	}

	virtual void Visit(ChangeVariable *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Undefined variable %s\n", node->id.c_str());
			abort();
		}

		node->e->Accept(this);

		cp.WriteOpcode(Op_addstatic);
		cp.WriteText<bc::VarId>(it->second);
	}

	virtual void Visit(ShowVariable *node)
	{
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varshow);
	}

	virtual void Visit(HideVariable *node)
	{
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varhide);
	}

	virtual void Visit(AppendToList *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		node->e->Accept(this);

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listadd);
	}

	virtual void Visit(DeleteFromList *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		node->e->Accept(this);

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listremove);
	}

	virtual void Visit(DeleteAllList *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		cp.PushString(node->id);

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listclear);
	}

	virtual void Visit(InsertInList *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		node->e1->Accept(this);
		node->e2->Accept(this);

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listinsert);
	}

	virtual void Visit(ReplaceInList *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		// flip to be more consistent with other list operations
		node->e2->Accept(this);
		node->e1->Accept(this);

		cp.WriteOpcode(Op_getstatic);
		cp.WriteText<bc::VarId>(it->second);

		cp.WriteOpcode(Op_listreplace);
	}

	virtual void Visit(ShowList *node)
	{
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varshow);
	}

	virtual void Visit(HideList *node)
	{
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varhide);
	}

	virtual void Visit(ProcProto *node) {}

	virtual void Visit(DefineProc *node)
	{
		std::string proccode = currentSpriteName + node->proto->proccode;
		auto it = procedureTable.find(proccode);
		if (it != procedureTable.end())
		{
			printf("Error: Duplicate procedure definition\n");
			abort();
		}

		procedureTable[proccode] = ProcInfo{ node->proto.get() };
	}

	virtual void Visit(Call *node)
	{
		// lookup procedure
		auto it = procedureTable.find(currentSpriteName + node->proccode);
		if (it == procedureTable.end())
		{
			printf("Error: Undefined procedure %s\n", node->proccode.c_str());
			abort();
		}

		// argument count check
		size_t formalargc = it->second.proto->arguments.size();
		size_t actualargc = node->args.size();
		if (formalargc != actualargc)
		{
			printf("Error: Argument count mismatch in procedure call %s\n", node->proccode.c_str());
			abort();
		}

		// push arguments
		for (auto &arg : it->second.proto->arguments)
		{
			auto it = node->args.find(arg.first);
			if (it == node->args.end())
			{
				printf("Error: Argument %s (%s) missing in procedure call %s\n",
					arg.first.c_str(), arg.second.c_str(), node->proccode.c_str());
				abort();
			}

			it->second->Accept(this);
		}

		cp.WriteOpcode(Op_call);
		cp.WriteText<bc::_bool>(it->second.proto->warp);
		cp.WriteText<bc::uint16>(actualargc);

		// import symbol
		uint64_t offset = cp._text.size();
		cp._importSymbols.emplace_back(offset, currentSpriteName + it->second.proto->proccode);
		cp.WriteText<bc::uint64>(0);
	}

	//
	/////////////////////////////////////////////////////////////////
	// Reporters
	//

	virtual void Visit(GotoReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(GlideReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(PointTowardsReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(CostumeReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(BackdropReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(SoundReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(BroadcastReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(CloneReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(TouchingReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(DistanceReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(KeyReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(PropertyOfReporter *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(ArgReporterStringNumber *node)
	{
		if (currentProc == nullptr)
		{
			// not in a procedure, push null
			Value v;
			InitializeValue(v);
			cp.PushValue(v);
			ReleaseValue(v);
			return;
		}

		int16_t arg;
		if (!currentProc->FindArgument(node->value, &arg))
		{
			printf("Error: Undefined argument %s\n", node->value.c_str());
			abort();
		}

		// push argument
		cp.WriteOpcode(Op_push);
		cp.WriteText<int16_t>(arg);
	}

	virtual void Visit(ArgReporterBoolean *node)
	{
		if (currentProc == nullptr)
		{
			// not in a procedure, push null
			Value v;
			InitializeValue(v);
			cp.PushValue(v);
			ReleaseValue(v);
			return;
		}

		int16_t arg;
		if (!currentProc->FindArgument(node->value, &arg))
		{
			printf("Error: Undefined argument %s\n", node->value.c_str());
			abort();
		}

		// push argument
		cp.WriteOpcode(Op_push);
		cp.WriteText<int16_t>(arg);
	}

	virtual void Visit(VariableDef *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined variable %s\n", node->id.c_str());
			abort();
		}

		Value v;
		InitializeValue(v);
		SetParsedString(v, node->value->value);

		cp.PushValue(v);

		cp.WriteOpcode(Op_setstatic);
		cp.WriteText<bc::VarId>(it->second);

		ReleaseValue(v);
	}

	virtual void Visit(VariableDefList *node)
	{
		uint64_t count = node->variables.size();
		cp._text.reserve(cp._text.capacity() + count * 16); // ~16 bytes per initializer instruction 

		for (AutoRelease<VariableDef> &vd : node->variables)
			vd->Accept(this);
	}

	virtual void Visit(ListDef *node)
	{
		auto it = staticVariables.find(node->id);
		if (it == staticVariables.end())
		{
			printf("Error: Undefined list %s\n", node->id.c_str());
			abort();
		}

		Value v;
		InitializeValue(v);

		int64_t size = static_cast<int64_t>(node->value.size());

		for (int64_t i = size - 1; i >= 0; --i)
		{
			SetParsedString(v, node->value[i]->value);
			cp.PushValue(v);
		}
		
		cp.WriteOpcode(Op_listcreate);
		cp.WriteText<int64_t>(size);

		cp.WriteOpcode(Op_setstatic);
		cp.WriteText<bc::VarId>(it->second);

		ReleaseValue(v);
	}

	virtual void Visit(ListDefList *node)
	{
		uint64_t count = node->lists.size();
		cp._text.reserve(cp._text.capacity() + count * 64); // ~64 bytes per initializer instruction

		for (AutoRelease<ListDef> &ld : node->lists)
			ld->Accept(this);
	}

	virtual void Visit(StatementListList *node)
	{
		uint64_t count = node->sll.size();

		// Define procedures
		for (AutoRelease<StatementList> &sl : node->sll)
		{
			DefineProc *proc = sl->sl[0]->As<DefineProc>();
			if (proc != nullptr)
			{
				count--;
				topLevel = true;
				proc->Accept(this);
			}
		}

		// Generate code for procedures
		for (AutoRelease<StatementList> &sl : node->sll)
		{
			DefineProc *proc = sl->sl[0]->As<DefineProc>();
			if (proc != nullptr)
			{
				topLevel = true;

				std::string name = currentSpriteName + proc->proto->proccode;

				auto it = cp._exportSymbols.find(name);
				if (it != cp._exportSymbols.end())
				{
					printf("Error: Duplicate procedure definition\n");
					abort();
				}

				// export symbol
				cp._exportSymbols[name] = cp._text.size();

				currentProc = &procedureTable[name];

				cp.WriteOpcode(Op_enter);

				auto &statements = sl->sl;
				for (size_t i = 1; i < statements.size(); i++)
					statements[i]->Accept(this);

				cp.WriteOpcode(Op_leave);
				cp.WriteOpcode(Op_ret);

				cp.WriteOpcode(Op_int);
				cp.AlignText();

				currentProc = nullptr;
			}
		}

		currentSprite->numScripts = count;
		if (count == 0)
		{
			// dont write empty script array
			currentSprite->scripts = 0;
			return;
		}

		uint64_t arrayStart = cp._rdata.size();

		// Generate code for top-level statements
		for (AutoRelease<StatementList> &sl : node->sll)
		{
			DefineProc *proc = sl->sl[0]->As<DefineProc>();
			if (proc == nullptr)
			{
				topLevel = true;

				bc::Script *script = (bc::Script *)cp.AllocRdata(sizeof(bc::Script));
				cp.CreateReference(&script->offset, Segment_text);

				sl->Accept(this);
			}
		}

		// Create reference to script array
		cp.CreateReference(&currentSprite->scripts, Segment_rdata, arrayStart);
	}

	virtual void Visit(CostumeDef *node) {}

	virtual void Visit(CostumeDefList *node)
	{
		currentSprite->numCostumes = node->costumes.size();
		if (currentSprite->numCostumes == 0)
		{
			// dont write empty costume array
			currentSprite->costumes = 0;
			return;
		}

		cp.CreateReference(&currentSprite->costumes, Segment_rdata);

		uint64_t arrayStart = cp._rdata.size();

		// Write array of costume definitions
		for (AutoRelease<CostumeDef> &cd : node->costumes)
		{
			bc::Costume *costume = (bc::Costume *)cp.AllocRdata(sizeof(bc::Costume));
			cp.CreateString(&costume->name, cd->name);
			cp.CreateString(&costume->format, cd->dataFormat);
			costume->bitmapResolution = cd->bitmapResolution;
			costume->reserved = 0;
			costume->rotationCenterX = cd->rotationCenterX;
			costume->rotationCenterY = cd->rotationCenterY;

			// Filled below
			costume->dataSize = 0;
			costume->data = 0;
		}

		// Write costume data
		uint64_t i = 0;
		for (AutoRelease<CostumeDef> &cd : node->costumes)
		{
			// do this every time, as the array might have been reallocated
			bc::Costume *costumes = (bc::Costume *)(cp._rdata.data() + arrayStart);

			Resource *rsrc = loader.Find(cd->md5ext);
			if (!rsrc)
			{
				printf("Error: Missing resource %s\n", cd->md5ext.c_str());
				abort();
			}

			const uint8_t *data = rsrc->Data();
			size_t size = rsrc->Size();

			// Write data and store reference in definition
			bc::Costume &costume = costumes[i];
			costume.dataSize = size;
			cp.CreateReference(&costume.data, Segment_rdata);
			cp.WriteRdata(data, size);

			i++;
		}
	}

	virtual void Visit(SoundDef *node) {}

	virtual void Visit(SoundDefList *node)
	{
		currentSprite->numSounds = node->sounds.size();
		if (currentSprite->numSounds == 0)
		{
			// dont write empty sound array
			currentSprite->sounds = 0;
			return;
		}

		cp.CreateReference(&currentSprite->sounds, Segment_rdata);

		uint64_t arrayStart = cp._rdata.size();

		// Write array of sound definitions
		for (AutoRelease<SoundDef> &sd : node->sounds)
		{
			bc::Sound *sound = (bc::Sound *)cp.AllocRdata(sizeof(bc::Sound));
			cp.CreateString(&sound->name, sd->name);
			cp.CreateString(&sound->format, sd->dataFormat);
			sound->rate = sd->rate;
			sound->sampleCount = sd->sampleCount;

			// Filled below
			sound->dataSize = 0;
			sound->data = 0;
		}

		// Write sound data
		uint64_t i = 0;
		for (AutoRelease<SoundDef> &sd : node->sounds)
		{
			// do this every time, as the array might have been reallocated
			bc::Sound *sounds = (bc::Sound *)(cp._rdata.data() + arrayStart);

			Resource *rsrc = loader.Find(sd->md5ext);
			if (!rsrc)
			{
				printf("Error: Missing resource %s\n", sd->md5ext.c_str());
				abort();
			}

			const uint8_t *data = rsrc->Data();
			size_t size = rsrc->Size();

			// Write data and store reference in definition
			bc::Sound &sound = sounds[i];
			sound.dataSize = size;
			cp.CreateReference(&sound.data, Segment_rdata);
			cp.WriteRdata(data, size);

			i++;
		}
	}

	virtual void Visit(SpriteDef *node)
	{
		bc::Sprite *sprite = (bc::Sprite *)cp.AllocStable(sizeof(bc::Sprite));

		currentSpriteName = node->name;
		currentSprite = sprite;

		// Basic data
		cp.CreateString(&sprite->name, node->name);
		sprite->x = node->x;
		sprite->y = node->y;
		sprite->direction = node->direction;
		sprite->size = node->size;
		sprite->currentCostume = node->currentCostume;
		sprite->layer = node->layer;
		sprite->visible = node->visible;
		sprite->isStage = node->isStage;
		sprite->draggable = node->draggable;
		sprite->rotationStyle = RotationStyleFromString(node->rotationStyle);

		if (node->variables->variables.size() + node->lists->lists.size() > 0)
		{
			// Reference to initializer
			cp.CreateReference(&sprite->initializer.offset, Segment_text);

			// Write initializer to text segment
			node->variables->Accept(this);
			node->lists->Accept(this);
			cp.WriteOpcode(Op_stopself);
			cp.AlignText();
		}
		else
		{
			// No initializer needed
			sprite->initializer.offset = 0;
		}

		// Write scripts
		node->scripts->Accept(this);

		// Write costumes and sounds
		node->costumes->Accept(this);
		node->sounds->Accept(this);

		currentSprite = nullptr;
		currentSpriteName.clear();
	}

	virtual void Visit(SpriteDefList *node)
	{
		uint64_t count = node->sprites.size();
		cp._stable.reserve(cp._stable.capacity() + count * 512); // ~512 bytes per sprite table entry

		cp.WriteStable<uint64_t>(count);
		for (AutoRelease<SpriteDef> &sd : node->sprites)
			sd->Accept(this);
	}

	virtual void Visit(StageDef *node)
	{

	}

	virtual void Visit(ValMonitorList *node)
	{

	}

	void MapStaticVariables(Program *node)
	{
		SpriteDef *stage = nullptr;
		for (AutoRelease<SpriteDef> &sprite : node->sprites->sprites)
		{
			if (sprite->isStage)
			{
				stage = sprite.get();
				break;
			}
		}

		if (!stage)
			return;

		uint64_t offset = cp._data.size();

		auto &vars = stage->variables->variables;
		auto &lists = stage->lists->lists;

		size_t numVars = vars.size() + lists.size();

		cp.WriteRdata<bc::uint64>(numVars);

		size_t id = 0;
		for (AutoRelease<VariableDef> &vd : vars)
		{
			printf("%s -> %zu\n", vd->id.c_str(), id);
			staticVariables[vd->id] = id++;

			Value v;
			InitializeValue(v);

			cp.WriteData(v);

			ReleaseValue(v);
		}

		for (AutoRelease<ListDef> &ld : lists)
		{
			printf("%s -> %zu\n", ld->id.c_str(), id);
			staticVariables[ld->id] = id++;

			Value v;
			InitializeValue(v);

			cp.WriteData(v);

			ReleaseValue(v);
		}
	}

	virtual void Visit(Program *node)
	{
		MapStaticVariables(node);

		node->sprites->Accept(this);

		cp.FlushStringPool();
		cp.Link();
	}

	CompiledProgram &cp;
	Loader &loader;
	const Scratch3CompilerOptions &options;
	bool topLevel = false;

	std::string currentSpriteName;
	bc::Sprite *currentSprite = nullptr;
	ProcInfo *currentProc = nullptr;

	std::unordered_map<std::string, bc::VarId> staticVariables; // name -> VarId

	std::unordered_map<std::string, ProcInfo> procedureTable; // name -> ProcInfo

	Compiler(CompiledProgram *cp, Loader *loader, const Scratch3CompilerOptions *options) :
		cp(*cp), loader(*loader), options(*options) {}
};

uint8_t *CompiledProgram::Export(size_t *outSize) const
{
	size_t size = sizeof(bc::Header) + _stable.size() + _text.size() + _data.size() + _rdata.size() + _debug.size();
	uint8_t *data = new uint8_t[size];

	// write header
	bc::Header *header = (bc::Header *)data;
	header->magic = PROGRAM_MAGIC;
	header->version = PROGRAM_VERSION;
	header->text = sizeof(bc::Header);
	header->text_size = _text.size();
	header->stable = header->text + header->text_size;
	header->stable_size = _stable.size();
	header->data = header->stable + header->stable_size;
	header->data_size = _data.size();
	header->rdata = header->data + header->data_size;
	header->rdata_size = _rdata.size();
	header->debug = header->rdata + header->rdata_size;
	header->debug_size = _debug.size();

	// write text segment
	memcpy(data + header->text, _text.data(), _text.size());

	// write stable segment
	memcpy(data + header->stable, _stable.data(), _stable.size());

	// write data segment
	memcpy(data + header->data, _data.data(), _data.size());

	// write rdata segment
	memcpy(data + header->rdata, _rdata.data(), _rdata.size());
	
	// write debug segment
	memcpy(data + header->debug, _debug.data(), _debug.size());

	// resolve references
	for (auto &p : _references)
	{
		const DataReference &from = p.first;
		const DataReference &to = p.second;

		uint64_t fromOff, toOff;

		switch (from.seg)
		{
		default:
			continue;
		case Segment_text:
			fromOff = header->text + from.off;
			break;
		case Segment_stable:
			fromOff = header->stable + from.off;
			break;
		case Segment_data:
			fromOff = header->data + from.off;
			break;
		case Segment_rdata:
			fromOff = header->rdata + from.off;
			break;
		case Segment_debug:
			fromOff = header->debug + from.off;
			break;
		}

		switch (to.seg)
		{
		default:
			continue;
		case Segment_text:
			toOff = header->text + to.off;
			break;
		case Segment_stable:
			toOff = header->stable + to.off;
			break;
		case Segment_data:
			toOff = header->data + to.off;
			break;
		case Segment_rdata:
			toOff = header->rdata + to.off;
			break;
		case Segment_debug:
			toOff = header->debug + to.off;
			break;
		}

		// write reference
		*(uint64_t *)(data + fromOff) = toOff;
	}

	*outSize = size;
	return data;
}

void CompiledProgram::Write(SegmentType seg, const void *data, size_t size)
{
	switch (seg)
	{
	case Segment_text:
		WriteText(data, size);
		break;
	case Segment_stable:
		WriteStable(data, size);
		break;
	case Segment_data:
		WriteData(data, size);
		break;
	case Segment_rdata:
		WriteRdata(data, size);
		break;
	default:
		printf("Internal error: Invalid segment type\n");
		abort();
	}
}

void *CompiledProgram::AllocText(size_t size)
{
	_text.resize(_text.size() + size);
	return _text.data() + _text.size() - size;
}

void CompiledProgram::WriteText(const void *data, size_t size)
{
	_text.resize(_text.size() + size);
	memcpy(_text.data() + _text.size() - size, data, size);
}

void CompiledProgram::WriteString(SegmentType seg, const std::string &str)
{
	void *ptr;

	switch (seg)
	{
	case Segment_text:
		ptr = AllocText(sizeof(bc::ptr<bc::string>));
		break;
	case Segment_stable:
		ptr = AllocStable(sizeof(bc::ptr<bc::string>));
		break;
	case Segment_data:
		ptr = AllocData(sizeof(bc::ptr<bc::string>));
		break;
	case Segment_rdata:
		ptr = AllocRdata(sizeof(bc::ptr<bc::string>));
		break;
	case Segment_debug:
		ptr = AllocDebug(sizeof(bc::ptr<bc::string>));
		break;
	default:
		printf("Internal error: Invalid segment type\n");
		abort();
	}

	CreateString(ptr, str);
}

void CompiledProgram::CreateString(void *dst, const std::string &str)
{
	uint8_t *ptr = (uint8_t *)dst;

	SegmentType seg;
	uint64_t off;

	ResolvePointer(ptr, &seg, &off);

	// fill with garbage
	memset(ptr, 0xbb, sizeof(bc::ptr<bc::string>));

	_plainStrings[str].push_back(DataReference{ seg, off });
}

void CompiledProgram::FlushStringPool()
{
	for (auto &p : _managedStrings)
	{
		const std::string &str = p.first;
		const auto &refs = p.second;

		bc::uint64 off = _rdata.size();

		DataReference to{ Segment_rdata, off };

		// write the string
		size_t size = offsetof(String, str) + str.size() + 1;
		String *s = (String *)AllocRdata(size);

		s->ref.count = 1;
		s->ref.flags = VALUE_STATIC;
		s->len = str.size();
		s->hash = HashString(str.c_str());
		memcpy(s->str, str.c_str(), str.size() + 1);

		// create references
		for (const DataReference &ref : refs)
			_references.emplace_back(ref, to);
	}
	_managedStrings.clear();

	for (auto &p : _plainStrings)
	{
		const std::string &str = p.first;
		const auto &refs = p.second;

		bc::uint64 off = _rdata.size();

		DataReference to{ Segment_rdata, off };

		// write the string
		WriteRdata(str.c_str(), str.size() + 1);

		// create references
		for (const DataReference &ref : refs)
			_references.emplace_back(ref, to);
	}
	_plainStrings.clear();
}

void CompiledProgram::WriteAbsoluteJump(uint8_t opcode, uint64_t off)
{
	WriteOpcode(opcode);
	WriteReference(Segment_text, Segment_text, off);
}

void CompiledProgram::WriteRelativeJump(uint8_t opcode, int64_t off)
{
	uint64_t target = static_cast<uint64_t>(static_cast<int64_t>(_text.size()) + off);
	WriteAbsoluteJump(opcode, target);
}

void CompiledProgram::PushString(const std::string &str)
{
	WriteOpcode(Op_pushstring);
	uint8_t *ptr = (uint8_t *)AllocText(sizeof(bc::ptr<String>));

	// fill with garbage
	memset(ptr, 0xdd, sizeof(bc::ptr<String>));

	_managedStrings[str].push_back(DataReference{ Segment_text, (uint64_t)(ptr - _text.data()) });
}

void CompiledProgram::PushValue(const Value &value)
{
	switch (value.type)
	{
	default:
	case ValueType_None:
		WriteOpcode(Op_pushnone);
		break;
	case ValueType_Integer:
		WriteOpcode(Op_pushint);
		WriteText<bc::int64>(value.u.integer);
		break;
	case ValueType_Real:
		WriteOpcode(Op_pushreal);
		WriteText<bc::float64>(value.u.real);
		break;
	case ValueType_Bool:
		WriteOpcode(value.u.boolean ? Op_pushtrue : Op_pushfalse);
		break;
	case ValueType_String:
		PushString(value.u.string->str);
		break;
	}
}

void CompiledProgram::AlignText()
{
	size_t off = _text.size();

	// round up to nearest multiple of 8
	off = (off + 7) & ~7;

	// fill with interrupts
	while (_text.size() < off)
		WriteOpcode(Op_int);
}

void *CompiledProgram::AllocStable(size_t size)
{
	size_t off = _stable.size();
	_stable.resize(off + size);
	return _stable.data() + off;
}

void CompiledProgram::WriteStable(const void *data, size_t size)
{
	_stable.resize(_stable.size() + size);
	memcpy(_stable.data() + _stable.size() - size, data, size);
}

void *CompiledProgram::AllocData(size_t size)
{
	_data.resize(_data.size() + size);
	return _data.data() + _data.size() - size;
}

void CompiledProgram::WriteData(const void *data, size_t size)
{
	_data.resize(_data.size() + size);
	memcpy(_data.data() + _data.size() - size, data, size);
}

void *CompiledProgram::AllocRdata(size_t size)
{
	_rdata.resize(_rdata.size() + size);
	return _rdata.data() + _rdata.size() - size;
}

void CompiledProgram::WriteRdata(const void *data, size_t size)
{
	_rdata.resize(_rdata.size() + size);
	memcpy(_rdata.data() + _rdata.size() - size, data, size);
}

void *CompiledProgram::AllocDebug(size_t size)
{
	_debug.resize(_debug.size() + size);
	return _debug.data() + _debug.size() - size;
}

void CompiledProgram::WriteDebug(const void *data, size_t size)
{
	_debug.resize(_debug.size() + size);
	memcpy(_debug.data() + _debug.size() - size, data, size);
}

size_t CompiledProgram::WriteReference(SegmentType from, SegmentType to, uint64_t off)
{
	uint8_t *ptr;

	switch (from)
	{
	case Segment_text:
		ptr = (uint8_t *)AllocText(sizeof(bc::ptr<void>));
		break;
	case Segment_stable:
		ptr = (uint8_t *)AllocStable(sizeof(bc::ptr<void>));
		break;
	case Segment_data:
		ptr = (uint8_t *)AllocData(sizeof(bc::ptr<void>));
		break;
	case Segment_rdata:
		ptr = (uint8_t *)AllocRdata(sizeof(bc::ptr<void>));
		break;
	case Segment_debug:
		ptr = (uint8_t *)AllocDebug(sizeof(bc::ptr<void>));
		break;
	default:
		printf("Internal error: Invalid segment type\n");
		abort();
	}

	return CreateReference(ptr, to, off);
}

size_t CompiledProgram::CreateReference(void *dst, SegmentType seg, uint64_t off)
{
	SegmentType fromSeg;
	uint64_t fromOff;

	switch (seg)
	{
	case Segment_text:
		if (off == -1)
			off = _text.size();
		break;
	case Segment_stable:
		if (off == -1)
			off = _stable.size();
		break;
	case Segment_data:
		if (off == -1)
			off = _data.size();
		break;
	case Segment_rdata:
		if (off == -1)
			off = _rdata.size();
		break;
	case Segment_debug:
		if (off == -1)
			off = _debug.size();
		break;
	default:
		printf("Internal error: Invalid segment type\n");
		abort();
	}

	ResolvePointer(dst, &fromSeg, &fromOff);

	// fill with garbage
	memset(dst, 0xef, sizeof(bc::ptr<void>));

	_references.emplace_back(DataReference{ fromSeg, fromOff }, DataReference{ seg, off });

	return _references.size() - 1;
}

size_t CompiledProgram::SetReference(size_t refId, SegmentType seg, uint64_t newOff)
{
	auto &p = _references[refId];
	p.second.seg = seg;
	p.second.off = newOff;
	return refId;
}

void CompiledProgram::ResolvePointer(void *dst, SegmentType *seg, uint64_t *off)
{
	uint8_t *ptr = (uint8_t *)dst;

	if (ptr >= _stable.data() && ptr + sizeof(bc::ptr<bc::string>) <= _stable.data() + _stable.size())
	{
		*seg = Segment_stable;
		*off = ptr - _stable.data();
	}
	else if (ptr >= _text.data() && ptr + sizeof(bc::ptr<bc::string>) <= _text.data() + _text.size())
	{
		*seg = Segment_text;
		*off = ptr - _text.data();
	}
	else if (ptr >= _rdata.data() && ptr + sizeof(bc::ptr<bc::string>) <= _rdata.data() + _rdata.size())
	{
		*seg = Segment_rdata;
		*off = ptr - _rdata.data();
	}
	else if (ptr >= _data.data() && ptr + sizeof(bc::ptr<bc::string>) <= _data.data() + _data.size())
	{
		*seg = Segment_data;
		*off = ptr - _data.data();
	}
	else if (ptr >= _debug.data() && ptr + sizeof(bc::ptr<bc::string>) <= _debug.data() + _debug.size())
	{
		*seg = Segment_debug;
		*off = ptr - _debug.data();
	}
	else
	{
		printf("Internal error: Invalid pointer\n");
		abort();
	}
}

void CompiledProgram::Link()
{
	for (auto &p : _importSymbols)
	{
		uint64_t off = p.first;
		const std::string &name = p.second;

		auto it = _exportSymbols.find(name);
		if (it == _exportSymbols.end())
		{
			printf("Link error: Unresolved symbol %s\n", name.c_str());
			abort();
		}

		// create reference
		_references.emplace_back(DataReference{ Segment_text, off }, DataReference{ Segment_text, it->second });
	}
}

CompiledProgram *CompileProgram(Program *p, Loader *loader, const Scratch3CompilerOptions *options)
{
	CompiledProgram *cp = new CompiledProgram();
	Compiler compiler(cp, loader, options);
	p->Accept(&compiler);
	return cp;
}
