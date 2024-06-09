#include "compiler.hpp"

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
	}

	virtual void Visit(BroadcastExpr *node)
	{

	}

	virtual void Visit(ListExpr *node)
	{
	}

	virtual void Visit(ListAccess *node)
	{
	}

	virtual void Visit(IndexOf *node)
	{
	}

	virtual void Visit(ListLength *node)
	{
	}

	virtual void Visit(ListContains *node)
	{
	}

	virtual void Visit(StatementList *node)
	{
		for (AutoRelease<Statement> &stmt : node->sl)
			stmt->Accept(this);
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
	}

	virtual void Visit(SetX *node)
	{
	}

	virtual void Visit(ChangeY *node)
	{
	}

	virtual void Visit(SetY *node)
	{
	}

	virtual void Visit(BounceIfOnEdge *node)
	{
	}

	virtual void Visit(SetRotationStyle *node)
	{
	}

	virtual void Visit(SayForSecs *node)
	{
	}

	virtual void Visit(Say *node)
	{
	}

	virtual void Visit(ThinkForSecs *node)
	{
	}

	virtual void Visit(Think *node)
	{
	}

	virtual void Visit(SwitchCostume *node)
	{
	}

	virtual void Visit(NextCostume *node)
	{
	}

	virtual void Visit(SwitchBackdrop *node)
	{
	}

	virtual void Visit(SwitchBackdropAndWait *node)
	{
	}

	virtual void Visit(NextBackdrop *node)
	{
	}

	virtual void Visit(ChangeSize *node)
	{
	}

	virtual void Visit(SetSize *node)
	{
	}

	virtual void Visit(ChangeGraphicEffect *node)
	{
	}

	virtual void Visit(SetGraphicEffect *node)
	{
	}

	virtual void Visit(ClearGraphicEffects *node)
	{
	}

	virtual void Visit(ShowSprite *node)
	{
	}

	virtual void Visit(HideSprite *node)
	{
	}

	virtual void Visit(GotoLayer *node)
	{
	}

	virtual void Visit(MoveLayer *node)
	{
	}

	virtual void Visit(PlaySoundUntilDone *node) {}
	virtual void Visit(StartSound *node) {}

	virtual void Visit(StopAllSounds *node) {}
	virtual void Visit(ChangeSoundEffect *node) {}
	virtual void Visit(SetSoundEffect *node) {}
	virtual void Visit(ClearSoundEffects *node) {}

	virtual void Visit(ChangeVolume *node)
	{
	}

	virtual void Visit(SetVolume *node)
	{
	}

	virtual void Visit(OnFlagClicked *node)
	{
		cp.WriteOpcode(Op_onflag);
		cp.Align(Segment_text);
	}

	virtual void Visit(OnKeyPressed *node)
	{
		uint16_t scancode = SDL_GetScancodeFromName(node->key.c_str());
		
		cp.WriteOpcode(Op_onkey);
		cp.WriteText(scancode);
		cp.Align(Segment_text);
	}

	virtual void Visit(OnSpriteClicked *node)
	{
		cp.WriteOpcode(Op_onclick);
		cp.Align(Segment_text);
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
		if (node->e->Is(Ast_BroadcastReporter))
		{
			BroadcastReporter &br = static_cast<BroadcastReporter &>(*node->e);
			cp.PushEventSender(br.value);
			cp.WriteOpcode(Op_pushsym);
		}
		else
			cp.WriteOpcode(Op_findevent);

		cp.WriteOpcode(Op_send);
	}

	virtual void Visit(BroadcastAndWait *node)
	{
		if (node->e->Is(Ast_BroadcastReporter))
		{
			BroadcastReporter &br = static_cast<BroadcastReporter &>(*node->e);
			cp.PushEventSender(br.value);
			cp.WriteOpcode(Op_pushsym);
		}
		else
			cp.WriteOpcode(Op_findevent);

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
	}

	virtual void Visit(CreateClone *node)
	{
	}

	virtual void Visit(DeleteClone *node)
	{
	}

	virtual void Visit(AskAndWait *node)
	{

	}

	virtual void Visit(SetDragMode *node)
	{

	}

	virtual void Visit(ResetTimer *node)
	{

	}

	virtual void Visit(SetVariable *node)
	{

	}

	virtual void Visit(ChangeVariable *node)
	{

	}

	virtual void Visit(ShowVariable *node)
	{
	}

	virtual void Visit(HideVariable *node)
	{
	}

	virtual void Visit(AppendToList *node)
	{

	}

	virtual void Visit(DeleteFromList *node)
	{

	}

	virtual void Visit(DeleteAllList *node)
	{

	}

	virtual void Visit(InsertInList *node)
	{

	}

	virtual void Visit(ReplaceInList *node)
	{

	}

	virtual void Visit(ShowList *node)
	{
	}

	virtual void Visit(HideList *node)
	{
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
	}

	virtual void Visit(GlideReporter *node)
	{
	}

	virtual void Visit(PointTowardsReporter *node)
	{
	}

	virtual void Visit(CostumeReporter *node)
	{
	}

	virtual void Visit(BackdropReporter *node)
	{
	}

	virtual void Visit(SoundReporter *node)
	{
	}

	virtual void Visit(BroadcastReporter *node)
	{
	}

	virtual void Visit(CloneReporter *node)
	{
	}

	virtual void Visit(TouchingReporter *node)
	{
	}

	virtual void Visit(DistanceReporter *node)
	{
	}

	virtual void Visit(KeyReporter *node)
	{
	}

	virtual void Visit(PropertyOfReporter *node)
	{
	}

	virtual void Visit(ArgReporterStringNumber *node)
	{
	}

	virtual void Visit(ArgReporterBoolean *node)
	{
	}

	virtual void Visit(VariableDef *node)
	{

	}

	virtual void Visit(VariableDefList *node)
	{

	}

	virtual void Visit(ListDef *node)
	{

	}

	virtual void Visit(ListDefList *node)
	{

	}

	virtual void Visit(StatementListList *node)
	{

	}

	virtual void Visit(CostumeDef *node)
	{
		Resource *rsrc = loader.Find(node->md5ext);
		const uint8_t *data = rsrc->Data();
		size_t size = rsrc->Size();
	}

	virtual void Visit(CostumeDefList *node)
	{
		for (AutoRelease<CostumeDef> &costume : node->costumes)
			costume->Accept(this);
	}

	virtual void Visit(SpriteDef *node)
	{
		cp.WriteData<double>(node->x);
		cp.WriteData<double>(node->y);
		cp.WriteData<double>(node->size);
		cp.WriteData<double>(node->direction);
		cp.WriteData<int64_t>(node->costumes);
		cp.WriteData<int64_t>(node->layer);

		cp.WriteData<uint8_t>(node->visible);
		cp.WriteData<uint8_t>(node->isStage);
		cp.WriteData<uint8_t>(node->draggable);
		cp.WriteData<uint8_t>(RotationStyleFromString(node->rotationStyle));

		cp.Align(Segment_data);

		node->costumes->Accept(this);
	}

	CompiledProgram &cp;
	Loader &loader;

	Compiler(CompiledProgram *cp, Loader *loader) : cp(*cp), loader(*loader) {}
};

void CompiledProgram::PushString(const std::string &str)
{

}

void CompiledProgram::PushValue(const Value &value)
{

}

void CompiledProgram::WriteData(const void *data, size_t size)
{
	_data.resize(_data.size() + size);
	memcpy(_data.data() + _data.size() - size, data, size);
}

void CompiledProgram::WriteRodata(const void *data, size_t size)
{
	_rodata.resize(_rodata.size() + size);
	memcpy(_rodata.data() + _rodata.size() - size, data, size);
}

void CompiledProgram::WriteBss(size_t size)
{
	_bss.resize(_bss.size() + size);
	memset(_bss.data() + _bss.size() - size, 0, size);
}

CompiledProgram *Compile(Program *p, Loader *loader)
{
	CompiledProgram *cp = new CompiledProgram();
	Compiler compiler(cp, loader);
	p->Accept(&compiler);
	return cp;
}
