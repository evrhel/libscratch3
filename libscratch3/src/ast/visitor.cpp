#include "visitor.hpp"

#include <cstdarg>

#include "ast.hpp"

class DumpVisitor : public Visitor
{
public:
	virtual void Visit(ExpressionList *node)
	{
		Printf("ExpressionList\n");
		_indent++;
		for (auto expr : node->expressions)
			expr->Accept(this);
		_indent--;
	}

	virtual void Visit(Number *node)
	{
		Printf("\033[1mNumber\033[0m \033[32m%s\033[0m\n", node->value.c_str());
	}

	virtual void Visit(PositiveNumber *node)
	{
		Printf("PositiveNumber \033[32m%s\033[0m\n", node->value.c_str());
	}

	virtual void Visit(PositiveInt *node)
	{
		Printf("PositiveInt \033[32m%s\033[0m\n", node->value.c_str());
	}

	virtual void Visit(Int *node)
	{
		Printf("Int \033[32m%s\033[0m\n", node->value.c_str());
	}

	virtual void Visit(Angle *node)
	{
		Printf("Angle \033[32m%s\033[0m deg\n", node->value.c_str());
	}

	virtual void Visit(Color *node)
	{
		Printf("Color %s\n", node->value.c_str());
	}

	virtual void Visit(String *node)
	{
		Printf("\033[1mString\033[0m \033[33;1m\"%s\"\033[0m\n", node->value.c_str());
	}

	virtual void Visit(True *node)
	{
		Printf("\033[1mTrue\033[0m\n");
	}

	virtual void Visit(False *node)
	{
		Printf("\033[1mFalse\033[0m\n");
	}

	virtual void Visit(Null *node)
	{
		Printf("\033[1mNull\033[0m\n");
	}

	virtual void Visit(XPos *node) {}
	virtual void Visit(YPos *node) {}
	virtual void Visit(Direction *node) {}
	virtual void Visit(CurrentCostume *node) {}
	virtual void Visit(CurrentBackdrop *node) {}
	virtual void Visit(Size *node) {}
	virtual void Visit(Volume *node) {}
	virtual void Visit(Touching *node) {}
	virtual void Visit(TouchingColor *node) {}
	virtual void Visit(ColorTouching *node) {}
	virtual void Visit(DistanceTo *node) {}
	virtual void Visit(Answer *node) {}
	virtual void Visit(KeyPressed *node) {}
	virtual void Visit(MouseDown *node) {}
	virtual void Visit(MouseX *node) {}
	virtual void Visit(MouseY *node) {}
	virtual void Visit(Loudness *node) {}
	virtual void Visit(TimerValue *node) {}
	virtual void Visit(PropertyOf *node) {}

	virtual void Visit(CurrentDate *node)
	{
		Printf("\033[36;1mCurrentDate\033[0m %s\n", node->format.c_str());
	}

	virtual void Visit(DaysSince2000 *node) {}
	virtual void Visit(Username *node) {}

	virtual void Visit(Add *node)
	{
		Printf("\033[32;1mAdd\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Sub *node) {}

	virtual void Visit(Mul *node)
	{
		Printf("\033[32;1mMul\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Div *node) {}

	virtual void Visit(Random *node)
	{
		Printf("\033[32;1mRandom\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Greater *node) {}
	virtual void Visit(Less *node) {}
	virtual void Visit(Equal *node) {}
	virtual void Visit(LogicalAnd *node) {}
	virtual void Visit(LogicalOr *node) {}
	virtual void Visit(LogicalNot *node) {}
	virtual void Visit(Concat *node) {}
	virtual void Visit(CharAt *node) {}
	virtual void Visit(StringLength *node) {}
	virtual void Visit(StringContains *node) {}
	virtual void Visit(Mod *node) {}
	virtual void Visit(Round *node) {}
	virtual void Visit(MathFunc *node) {}

	virtual void Visit(VariableExpr *node)
	{
		Printf("\033[31;1mVariableExpr\033[0m %s\n", node->name.c_str());
	}

	virtual void Visit(ListExpr *node) {}
	virtual void Visit(ListAccess *node) {}
	virtual void Visit(IndexOf *node) {}
	virtual void Visit(ListLength *node) {}
	virtual void Visit(ListContains *node) {}

	virtual void Visit(StatementList *node)
	{
		Printf("\033[1mStatementList\033[0m\n");
		_indent++;
		for (auto stmt : node->sl)
			stmt->Accept(this);
		_indent--;
	}

	virtual void Visit(MoveSteps *node) {}

	virtual void Visit(TurnDegrees *node)
	{
		Printf("\033[34;1mTurnDegrees\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
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
		Printf("\033[34;1mSetX\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ChangeY *node) {}
	virtual void Visit(SetY *node) {}
	virtual void Visit(BounceIfOnEdge *node) {}
	virtual void Visit(SetRotationStyle *node) {}

	virtual void Visit(SayForSecs *node)
	{
		Printf("\033[35mSayForSecs\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Say *node) {}
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
		Printf("\033[35mSetGraphicEffect\033[0m %s\n", GraphicEffectToString(node->effect));
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ClearGraphicEffects *node) {}
	virtual void Visit(ShowSprite *node) {}
	virtual void Visit(HideSprite *node) {}
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

	virtual void Visit(OnFlagClicked *node)
	{
		Printf("\033[33mOnFlagClicked\033[0m\n");
	}

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
		Printf("\033[33mWaitSecs\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(Repeat *node) {}
	virtual void Visit(Forever *node) {}
	virtual void Visit(If *node) {}
	virtual void Visit(IfElse *node) {}
	virtual void Visit(WaitUntil *node) {}
	virtual void Visit(RepeatUntil *node) {}
	virtual void Visit(Stop *node) {}
	virtual void Visit(CloneStart *node) {}
	virtual void Visit(CreateClone *node) {}
	virtual void Visit(DeleteClone *node) {}
	virtual void Visit(AskAndWait *node) {}
	virtual void Visit(SetDragMode *node) {}
	virtual void Visit(ResetTimer *node) {}

	virtual void Visit(SetVariable *node)
	{
		Printf("\033[31;1mSetVariable\033[0m %s\n", node->name.c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
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
	virtual void Visit(DefineProc *node) {}
	virtual void Visit(Call *node) {}

	virtual void Visit(VariableDef *node)
	{
		Printf("\033[31;1mVariableDef\033[0m %s\n", node->name.c_str());
		_indent++;
		node->value->Accept(this);
		_indent--;
	}

	virtual void Visit(VariableDefList *node)
	{
		Printf("\033[1mVariableDefList\033[0m\n");
		_indent++;
		for (auto v : node->variables)
			v.second->Accept(this);
		_indent--;
	}

	virtual void Visit(ListDef *node)
	{
		Printf("\033[1mListDef\033[0m %s\n", node->name.c_str());
		_indent++;
		for (auto v : node->value)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(ListDefList *node)
	{
		Printf("\033[1mListDefList\033[0m\n");
		_indent++;
		for (auto v : node->lists)
			v.second->Accept(this);
		_indent--;
	}

	virtual void Visit(StatementListList *node)
	{
		Printf("\033[1mStatementListList\033[0m\n");
		_indent++;
		for (auto v : node->sll)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(SpriteDef *node)
	{
		Printf("\033[1mSpriteDef\033[0m %s\n", node->name.c_str());
		_indent++;
		node->variables->Accept(this);
		node->lists->Accept(this);
		node->scripts->Accept(this);
		_indent--;
	}

	virtual void Visit(SpriteDefList *node)
	{
		Printf("\033[1mSpriteDefList\033[0m\n");
		_indent++;
		for (auto v : node->sprites)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(StageDef *node)
	{
		Printf("\033[1mStageDef\033[0m\n");
		_indent++;
		node->variables->Accept(this);
		node->lists->Accept(this);
		node->scripts->Accept(this);
		_indent--;
	}

	virtual void Visit(ValMonitor *node) {}

	virtual void Visit(ValMonitorList *node)
	{
		Printf("\033[1mValMonitorList\033[0m\n");
		_indent++;
		for (auto v : node->monitors)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(Program *node)
	{
		Printf("\033[1mProgram\033[0m\n");
		_indent++;
		if (node->stage)
			node->stage->Accept(this);

		if (node->sprites)
			node->sprites->Accept(this);

		if (node->monitors)
			node->monitors->Accept(this);

		_indent--;
	}
private:
	void Printf(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		for (int i = 0; i < _indent; i++)
			printf("  ");
		vprintf(fmt, args);
		va_end(args);
	}

	int _indent = 0;
};

Visitor *CreateDumpVisitor() { return new DumpVisitor(); }
