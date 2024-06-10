#include "compiler.hpp"

#include <cstdio>

#include "opcode.hpp"

#include <SDL.h>

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
		// TODO: Implement
		printf("Warning: PropertyOf will not return a value\n");
		cp.WriteOpcode(Op_pushnone);
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
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
	}

	virtual void Visit(BroadcastExpr *node)
	{
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
	}

	virtual void Visit(ListExpr *node)
	{
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
	}

	virtual void Visit(ListAccess *node)
	{
		node->e->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
		cp.WriteOpcode(Op_listat);
	}

	virtual void Visit(IndexOf *node)
	{
		node->e->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
		cp.WriteOpcode(Op_listfind);
	}

	virtual void Visit(ListLength *node)
	{
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
		cp.WriteOpcode(Op_listlen);
	}

	virtual void Visit(ListContains *node)
	{
		node->e->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
		cp.WriteOpcode(Op_listcontains);
	}

	virtual void Visit(StatementList *node)
	{
		for (AutoRelease<Statement> &stmt : node->sl)
			stmt->Accept(this);
		cp.WriteOpcode(Op_stopself); // implicit stop
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
		cp.WriteOpcode(Op_glide);
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
		cp.WriteText<int8_t>(node->layer);
	}

	virtual void Visit(MoveLayer *node)
	{
		node->e->Accept(this);
		cp.WriteOpcode(Op_movelayer);
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
		uint16_t scancode = SDL_GetScancodeFromName(node->key.c_str());
		
		cp.WriteOpcode(Op_onkey);
		cp.WriteText(scancode);
	}

	virtual void Visit(OnSpriteClicked *node)
	{
		cp.WriteOpcode(Op_onclick);
	}

	virtual void Visit(OnStageClicked *node)
	{
		// TODO: Implement
	}

	virtual void Visit(OnBackdropSwitch *node)
	{
		// TODO: Implement
	}

	virtual void Visit(OnGreaterThan *node)
	{
		// TODO: Implement
	}

	virtual void Visit(OnEvent *node)
	{
		cp.WriteOpcode(Op_onevent);
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
		node->e->Accept(this);
	}

	virtual void Visit(Forever *node)
	{
		int64_t start = cp._text.size();

		node->sl->Accept(this);

		int64_t diff = cp._text.size() - start;

		cp.WriteOpcode(Op_jmp);
		cp.WriteText<int64_t>(-diff); // jump back to start
	}

	virtual void Visit(If *node)
	{
		int64_t top, jz;

		node->e->Accept(this);

		top = cp._text.size();
		cp.WriteOpcode(Op_jz);
		jz = cp._text.size();
		cp.WriteText<int64_t>(0); // placeholder

		node->sl->Accept(this);

		(int64_t &)cp._text[jz] = cp._text.size() - top;
	}

	virtual void Visit(IfElse *node)
	{
		int64_t top, jz;
		int64_t trueJmp;
		int64_t elseTop;

		node->e->Accept(this);

		// conditional jump to else block
		top = cp._text.size();
		cp.WriteOpcode(Op_jz);
		jz = cp._text.size();
		cp.WriteText<int64_t>(0); // placeholder

		node->sl1->Accept(this);

		// unconditional jump to end
		cp.WriteOpcode(Op_jmp);
		trueJmp = cp._text.size();
		cp.WriteText<int64_t>(0); // placeholder

		// set jump destination for else block
		elseTop = cp._text.size();
		(int64_t &)cp._text[jz] = elseTop - top;

		node->sl2->Accept(this);

		// set jump destination for end
		(int64_t &)cp._text[trueJmp] = cp._text.size() - trueJmp;
	}

	virtual void Visit(WaitUntil *node)
	{
		int64_t top, jnz;

		top = cp._text.size();
		node->e->Accept(this);

		cp.WriteOpcode(Op_jnz);
		jnz = cp._text.size();
		cp.WriteText<int64_t>(0); // placeholder

		cp.WriteOpcode(Op_yield);

		// jump back to top
		cp.WriteOpcode(Op_jmp);
		cp.WriteText<int64_t>(top - cp._text.size());

		(int64_t &)cp._text[jnz] = cp._text.size() - top;
	}

	virtual void Visit(RepeatUntil *node)
	{
		int64_t top, jnz;

		top = cp._text.size();
		node->e->Accept(this);

		cp.WriteOpcode(Op_jnz);
		jnz = cp._text.size();
		cp.WriteText<int64_t>(0); // placeholder
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
		// TODO: Implement
	}

	virtual void Visit(CreateClone *node)
	{
		// TODO: Implement
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
		node->e->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varset);
	}

	virtual void Visit(ChangeVariable *node)
	{
		node->e->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varadd);
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
		node->e->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
		cp.WriteOpcode(Op_listadd);
	}

	virtual void Visit(DeleteFromList *node)
	{
		node->e->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
		cp.WriteOpcode(Op_listremove);
	}

	virtual void Visit(DeleteAllList *node)
	{
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
		cp.WriteOpcode(Op_listclear);
	}

	virtual void Visit(InsertInList *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
		cp.WriteOpcode(Op_listinsert);
	}

	virtual void Visit(ReplaceInList *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varget);
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
	virtual void Visit(DefineProc *node) {}
	virtual void Visit(Call *node) {}

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
		cp.PushString(node->value);
	}

	virtual void Visit(ArgReporterBoolean *node)
	{
		cp.PushString(node->value);
	}

	virtual void Visit(VariableDef *node)
	{
		Value v;
		InitializeValue(v);
		SetParsedString(v, node->value->value);

		cp.PushValue(v);
		cp.PushString(node->id);
		cp.WriteOpcode(Op_varset);

		ReleaseValue(v);
	}

	virtual void Visit(VariableDefList *node)
	{
		for (AutoRelease<VariableDef> &vd : node->variables)
			vd->Accept(this);
	}

	virtual void Visit(ListDef *node)
	{
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

		cp.PushString(node->id);
		cp.WriteOpcode(Op_varset);

		ReleaseValue(v);
	}

	virtual void Visit(ListDefList *node)
	{
		for (AutoRelease<ListDef> &ld : node->lists)
			ld->Accept(this);
	}

	virtual void Visit(StatementListList *node)
	{
		cp.WriteStable<int64_t>(node->sll.size());
		for (AutoRelease<StatementList> &sl : node->sll)
		{
			cp.WriteStable<int64_t>(cp._text.size()); // offset
			sl->Accept(this);
		}
	}

	virtual void Visit(CostumeDef *node)
	{
		Resource *rsrc = loader.Find(node->md5ext);
		const uint8_t *data = rsrc->Data();
		size_t size = rsrc->Size();

		cp.WriteString(Segment_stable, node->name);
		cp.WriteString(Segment_stable, node->dataFormat);
		cp.WriteRdata<int32_t>(node->bitmapResolution);
		cp.WriteRdata<double>(node->rotationCenterX);
		cp.WriteRdata<double>(node->rotationCenterY);
		cp.WriteRdata<int64_t>(size);

		cp.WriteRdata(data, size);
	}

	virtual void Visit(CostumeDefList *node)
	{
		cp.WriteStable<int64_t>(node->costumes.size());
		for (AutoRelease<CostumeDef> &costume : node->costumes)
			costume->Accept(this);
	}

	virtual void Visit(SpriteDef *node)
	{
		cp.WriteString(Segment_stable, node->name);
		cp.WriteStable<double>(node->x);
		cp.WriteStable<double>(node->y);
		cp.WriteStable<double>(node->size);
		cp.WriteStable<double>(node->direction);
		cp.WriteStable<int64_t>(node->currentCostume);
		cp.WriteStable<int64_t>(node->layer);

		cp.WriteStable<uint8_t>(node->visible);
		cp.WriteStable<uint8_t>(node->isStage);
		cp.WriteStable<uint8_t>(node->draggable);
		cp.WriteStable<uint8_t>(RotationStyleFromString(node->rotationStyle));

		// Reference to initializer
		cp.CreateReference(Segment_stable, Segment_text, cp._text.size());
		cp.WriteStable<uint64_t>(0); // placeholder

		// Write initializer to text segment
		node->variables->Accept(this);
		node->lists->Accept(this);
		cp.WriteOpcode(Op_stopself);

		node->scripts->Accept(this);

		node->costumes->Accept(this);
	}

	CompiledProgram &cp;
	Loader &loader;

	Compiler(CompiledProgram *cp, Loader *loader) : cp(*cp), loader(*loader) {}
};

void CompiledProgram::WriteText(const void *data, size_t size)
{
	_text.resize(_text.size() + size);
	memcpy(_text.data() + _text.size() - size, data, size);
}

void CompiledProgram::WriteString(SegmentType seg, const std::string &str)
{
	uint64_t off;
	switch (seg)
	{
	default:
		off = 0;
		break;
	case Segment_stable:
		off = _stable.size();
		off += sizeof(uint64_t); // placeholder
		break;
	case Segment_text:
		off = _text.size();
		off += sizeof(uint64_t); // placeholder
		break;
	case Segment_data:
		off = _data.size();
		off += sizeof(uint64_t); // placeholder
		break;
	case Segment_rdata:
		abort(); // cannot write strings to rdata
		break;
	}

	auto it = _strings.find(str);
	if (it != _strings.end())
	{
		_references.emplace_back(DataReference{ seg, off }, it->second);
		return;
	}

	uint64_t rdoff = _rdata.size();
	WriteRdata(str.c_str(), str.size() + 1);

	_strings[str] = DataReference{ Segment_rdata, rdoff };
	WriteString(seg, str); // recurse
}

void CompiledProgram::PushString(const std::string &str)
{
	WriteOpcode(Op_pushstring);

	auto it = _strings.find(str);
	if (it != _strings.end())
	{
		DataReference &dr = it->second;
		CreateReference(Segment_text, dr.seg, dr.off);

		WriteText<uint64_t>(0); // placeholder
		return;
	}

	uint64_t off = _data.size();

	size_t size = offsetof(String, str) + str.size() + 1;
	AllocRdata(size);

	String *s = (String *)(_rdata.data() + _rdata.size() - size);
	s->ref.count = 1;
	s->ref.flags = 0;
	s->len = str.size();
	s->hash = HashString(str.c_str());
	memcpy(s->str, str.c_str(), str.size() + 1);

	CreateReference(Segment_text, Segment_rdata, off);
	WriteText<uint64_t>(0); // placeholder

	_strings[str] = DataReference{ Segment_rdata, off };
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
		WriteData(value.u.integer);
		break;
	case ValueType_Real:
		WriteOpcode(Op_pushreal);
		WriteData(value.u.real);
		break;
	case ValueType_Bool:
		WriteOpcode(value.u.boolean ? Op_pushtrue : Op_pushfalse);
		break;
	case ValueType_String:
		PushString(value.u.string->str);
		break;
	}
}

void CompiledProgram::WriteStable(const void *data, size_t size)
{
	_stable.resize(_stable.size() + size);
	memcpy(_stable.data() + _stable.size() - size, data, size);
}

void CompiledProgram::WriteData(const void *data, size_t size)
{
	_data.resize(_data.size() + size);
	memcpy(_data.data() + _data.size() - size, data, size);
}

void CompiledProgram::AllocRdata(size_t size)
{
	_rdata.resize(_rdata.size() + size);
}

void CompiledProgram::WriteRdata(const void *data, size_t size)
{
	_rdata.resize(_rdata.size() + size);
	memcpy(_rdata.data() + _rdata.size() - size, data, size);
}

void CompiledProgram::CreateReference(SegmentType src, SegmentType dst, uint64_t dstoff)
{
	DataReference from;
	from.seg = src;

	switch (src)
	{
	default:
		return;
	case Segment_text:
		from.off = _text.size();
		break;
	case Segment_data:
		from.off = _data.size();
		break;
	case Segment_rdata:
		from.off = _rdata.size();
		break;
	}

	DataReference to;
	to.seg = dst;
	to.off = dstoff;

	_references.emplace_back(from, to);
}

CompiledProgram *Compile(Program *p, Loader *loader)
{
	CompiledProgram *cp = new CompiledProgram();
	Compiler compiler(cp, loader);
	p->Accept(&compiler);
	return cp;
}
