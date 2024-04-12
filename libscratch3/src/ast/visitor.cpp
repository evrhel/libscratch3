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
		for (auto &expr : node->expressions)
			expr->Accept(this);
		_indent--;
	}

	virtual void Visit(Constexpr *node)
	{
		switch (node->syminfo.type)
		{
		default:
		case SymbolType_String:
			Printf("\033[33;1m\"%s\"\033[0m",
				node->value.c_str());
			break;
		case SymbolType_Bool:
			Printf("\033[35m%s\033[0m",
				node->value.c_str());
			break;
		case SymbolType_Number:
			Printf("\033[32m%s\033[0m",
				node->value.c_str());
			break;
		case SymbolType_Int:
			Printf("\033[32m%s\033[0m",
				node->value.c_str());
			break;
		}

		printf(" -> %s\n", node->syminfo.ToString().c_str());
	}

	virtual void Visit(XPos *node)
	{
		Printf("\033[34;1mXPos\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(YPos *node)
	{
		Printf("\033[34;1mYPos\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(Direction *node)
	{
		Printf("\033[34;1mDirection\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(CurrentCostume *node)
	{
		Printf("\033[35mCurrentCostume\033[0m %s -> \033[1m%s\033[0m\n",
			PropGetTypeStrings[node->type],
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(CurrentBackdrop *node)
	{
		Printf("\033[35mCurrentBackdrop\033[0m %s -> \033[1m%s\033[0m\n",
			PropGetTypeStrings[node->type],
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(Size *node)
	{
		Printf("\033[35Size\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(Volume *node)
	{
		Printf("\033[35;1mVolume\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(Touching *node)
	{
		Printf("\033[36;1mTouching\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(TouchingColor *node)
	{
		Printf("\033[36;1mTouchingColor\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ColorTouching *node)
	{
		Printf("\033[36;1mColorTouching\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(DistanceTo *node)
	{
		Printf("\033[36;1mDistanceTo\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(Answer *node)
	{
		Printf("\033[36;1mAnswer\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(KeyPressed *node)
	{
		Printf("\033[36;1mKeyPressed\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(MouseDown *node)
	{
		Printf("\033[36;1mMouseDown\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(MouseX *node)
	{
		Printf("\033[36;1mMouseX\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(MouseY *node)
	{
		Printf("\033[36;1mMouseY\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(Loudness *node)
	{
		Printf("\033[36;1mLoudness\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(TimerValue *node)
	{
		Printf("\033[36;1mTimerValue\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(PropertyOf *node)
	{
		if (node->target == PropertyTarget_Variable)
		{
			Printf("\033[36;1mPropertyOf\033[0m %s \033[31;1m%s\033[0m -> \033[1m%s\033[0m\n",
				PropertyTargetStrings[node->target],
				node->name.c_str(),
				node->syminfo.ToString().c_str());
		}
		else
		{
			Printf("\033[36;1mPropertyOf\033[0m %s -> \033[1m%s\033[0m\n",
				PropertyTargetStrings[node->target],
				node->syminfo.ToString().c_str());
		}
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(CurrentDate *node)
	{
		Printf("\033[36;1mCurrentDate\033[0m %s -> \033[1m%s\033[0m\n",
			DateFormatStrings[node->format],
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(DaysSince2000 *node)
	{
		Printf("\033[36;1mDaysSince2000\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(Username *node)
	{
		Printf("\033[36;1mUsername\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());
	}

	virtual void Visit(Add *node)
	{
		Printf("\033[32;1mAdd\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Sub *node)
	{
		Printf("\033[32;1mSub\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Mul *node)
	{
		Printf("\033[32;1mMul\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Div *node)
	{
		Printf("\033[32;1mDiv\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Random *node)
	{
		Printf("\033[32;1mRandom\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Greater *node)
	{
		Printf("\033[32;1mGreater\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Less *node)
	{
		Printf("\033[32;1mLess\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Equal *node)
	{
		Printf("\033[32;1mEqual\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(LogicalAnd *node)
	{
		Printf("\033[32;1mLogicalAnd\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(LogicalOr *node)
	{
		Printf("\033[32;1mLogicalOr\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(LogicalNot *node)
	{
		Printf("\033[32;1mLogicalNot\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(Concat *node)
	{
		Printf("\033[32;1mConcat\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(CharAt *node)
	{
		Printf("\033[32;1mCharAt\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(StringLength *node)
	{
		Printf("\033[32;1mStringLength\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(StringContains *node)
	{
		Printf("\033[32;1mStringContains\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Mod *node)
	{
		Printf("\033[32;1mMod\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Round *node)
	{
		Printf("\033[32;1mRound\033[0m -> \033[1m%s\033[0m\n",
			node->syminfo.ToString().c_str());

		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(MathFunc *node)
	{
		Printf("\033[32;1mMathFunc\033[0m %s -> \033[1m%s\033[0m\n",
			MathFuncStrings[node->func],
			node->syminfo.ToString().c_str());

		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(VariableExpr *node)
	{
		Printf("\033[31;1m%s\033[0m\n",
			node->name.c_str(), node->syminfo.ToString().c_str());
	}

	virtual void Visit(ListExpr *node)
	{
		Printf("\033[31m%s\033[0m\n",
			node->name.c_str(), node->syminfo.ToString().c_str());
	}

	virtual void Visit(ListAccess *node)
	{
		Printf("\033[31mListAccess\033[0m %s -> %s\n",
			node->name.c_str(), node->syminfo.ToString().c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(IndexOf *node)
	{
		Printf("\033[31mIndexOf\033[0m %s -> %s\n",
			node->name.c_str(), node->syminfo.ToString().c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ListLength *node)
	{
		Printf("\033[31mListLength\033[0m %s -> %s\n",
			node->name.c_str(), node->syminfo.ToString().c_str());
	}

	virtual void Visit(ListContains *node)
	{
		Printf("\033[31mListContains\033[0m %s -> %s\n",
			node->name.c_str(), node->syminfo.ToString().c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(StatementList *node)
	{
		Printf("\033[1mStatementList\033[0m\n");
		_indent++;
		for (auto &stmt : node->sl)
			stmt->Accept(this);
		_indent--;
	}

	virtual void Visit(MoveSteps *node)
	{
		Printf("\033[34;1mMoveSteps\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(TurnDegrees *node)
	{
		Printf("\033[34;1mTurnDegrees\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(TurnNegDegrees *node)
	{
		Printf("\033[34;1mTurnNegDegrees\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(Goto *node)
	{
		Printf("\033[34;1mGoto\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(GotoXY *node)
	{
		Printf("\033[34;1mGotoXY\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Glide *node)
	{
		Printf("\033[34;1mGlide\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(GlideXY *node)
	{
		Printf("\033[34;1mGlideXY\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		node->e3->Accept(this);
		_indent--;
	}

	virtual void Visit(PointDir *node)
	{
		Printf("\033[34;1mPointDir\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(PointTowards *node)
	{
		Printf("\033[34;1mPointDir\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ChangeX *node)
	{
		Printf("\033[34;1mChangeX\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SetX *node)
	{
		Printf("\033[34;1mSetX\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ChangeY *node)
	{
		Printf("\033[34;1mChangeY\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SetY *node)
	{
		Printf("\033[34;1mSetY\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(BounceIfOnEdge *node)
	{
		Printf("\033[34;1mBounceIfOnEdge\033[0m\n");
	}

	virtual void Visit(SetRotationStyle *node)
	{
		Printf("\033[34;1mSetRotationStyle\033[0m %s\n",
			RotationStyleStrings[node->style]);
	}

	virtual void Visit(SayForSecs *node)
	{
		Printf("\033[35mSayForSecs\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Say *node)
	{
		Printf("\033[35mSay\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ThinkForSecs *node)
	{
		Printf("\033[35mThinkForSecs\033[0m\n");
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(Think *node)
	{
		Printf("\033[35mThink\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SwitchCostume *node)
	{
		Printf("\033[35mSwitchCostume\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(NextCostume *node)
	{
		Printf("\033[35mNextCostume\033[0m\n");
	}

	virtual void Visit(SwitchBackdrop *node)
	{
		Printf("\033[35mSwitchBackdrop\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SwitchBackdropAndWait *node)
	{
		Printf("\033[35mSwitchBackdropAndWait\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(NextBackdrop *node)
	{
		Printf("\033[35mNextBackdrop\033[0m\n");
	}

	virtual void Visit(ChangeSize *node)
	{
		Printf("\033[35mChangeSize\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SetSize *node)
	{
		Printf("\033[35mSetSize\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ChangeGraphicEffect *node)
	{
		Printf("\033[35mChangeGraphicEffect\033[0m %s\n", GraphicEffectStrings[node->effect]);
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SetGraphicEffect *node)
	{
		Printf("\033[35mSetGraphicEffect\033[0m %s\n", GraphicEffectStrings[node->effect]);
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ClearGraphicEffects *node)
	{
		Printf("\033[35mClearGraphicEffects\033[0m\n");
	}

	virtual void Visit(ShowSprite *node)
	{
		Printf("\033[35mShowSprite\033[0m\n");
	}

	virtual void Visit(HideSprite *node)
	{
		Printf("\033[35mHideSprite\033[0m\n");
	}

	virtual void Visit(GotoLayer *node)
	{
		Printf("\033[35mGotoLayer\033[0m %s\n", LayerTypeStrings[node->layer]);
	}

	virtual void Visit(MoveLayer *node)
	{
		Printf("\033[35mMoveLayer\033[0m %s\n", LayerDirStrings[node->direction]);
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(PlaySoundUntilDone *node)
	{
		Printf("\033[35;1mPlaySoundUntilDone\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(StartSound *node)
	{
		Printf("\033[35;1mPlaySound\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(StopAllSounds *node)
	{
		Printf("\033[35;1mStopAllSounds\033[0m\n");
	}

	virtual void Visit(ChangeSoundEffect *node)
	{
		Printf("\033[35;1mChangeSoundEffect\033[0m %s\n",
			SoundEffectStrings[node->effect]);
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SetSoundEffect *node)
	{
		Printf("\033[35;1mSetSoundEffect\033[0m %s\n",
			SoundEffectStrings[node->effect]);
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ClearSoundEffects *node)
	{
		Printf("\033[35;1mClearSoundEffects\033[0m\n");
	}

	virtual void Visit(ChangeVolume *node)
	{
		Printf("\033[35;1mChangeVolume\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SetVolume *node)
	{
		Printf("\033[35;1mSetVolume\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(OnFlagClicked *node)
	{
		Printf("\033[33mOnFlagClicked\033[0m\n");
	}

	virtual void Visit(OnKeyPressed *node)
	{
		Printf("\033[33mOnKeyPressed\033[0m %s\n",
			GetKeyName(node->key));
	}

	virtual void Visit(OnSpriteClicked *node)
	{
		Printf("\033[33mOnSpriteClicked\033[0m\n");
	}

	virtual void Visit(OnStageClicked *node)
	{
		Printf("\033[33mOnStageClicked\033[0m\n");
	}

	virtual void Visit(OnBackdropSwitch *node)
	{
		Printf("\033[33mOnSpriteClicked\033[0m %s\n",
			node->backdrop.c_str());
	}

	virtual void Visit(OnGreaterThan *node)
	{
		Printf("\033[33mOnGreaterThan\033[0m %s\n",
			ListenValueTypeStrings[node->value]);
	}

	virtual void Visit(OnEvent *node)
	{
		Printf("\033[33mOnEvent\033[0m %s\n", node->message.c_str());
	}

	virtual void Visit(Broadcast *node)
	{
		Printf("\033[33mBroadcast\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(BroadcastAndWait *node)
	{
		Printf("\033[33mBroadcastAndWait\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(WaitSecs *node)
	{
		Printf("\033[33mWaitSecs\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(Repeat *node)
	{
		Printf("\033[33mRepeat\033[0m\n");
		_indent++;
		node->e->Accept(this);
		if (node->sl) node->sl->Accept(this);
		else Printf("(empty)\n");
		_indent--;
	}

	virtual void Visit(Forever *node)
	{
		Printf("\033[33mForever\033[0m\n");
		_indent++;
		if (node->sl) node->sl->Accept(this);
		_indent--;
	}

	virtual void Visit(If *node)
	{
		Printf("\033[33mIf\033[0m\n");
		_indent++;
		if (node->e) node->e->Accept(this);
		if (node->sl) node->sl->Accept(this);
		_indent--;
	}

	virtual void Visit(IfElse *node)
	{
		Printf("\033[33mIfElse\033[0m\n");
		_indent++;
		if (node->e) node->e->Accept(this);
		if (node->sl1) node->sl1->Accept(this);
		if (node->sl2) node->sl2->Accept(this);
		_indent--;
	}

	virtual void Visit(WaitUntil *node)
	{
		Printf("\033[33mWaitUntil\033[0m\n");
		_indent++;
		if (node->e) node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(RepeatUntil *node)
	{
		Printf("\033[33mRepeatUntil\033[0m\n");
		_indent++;
		if (node->e) node->e->Accept(this);
		if (node->sl) node->sl->Accept(this);
		_indent--;
	}

	virtual void Visit(Stop *node)
	{
		Printf("\033[33mStop %s\033[0m\n",
			StopModeStrings[node->mode]);
	}

	virtual void Visit(CloneStart *node)
	{
		Printf("\033[33mCloneStart\033[0m\n");
	}

	virtual void Visit(CreateClone *node)
	{
		Printf("\033[33mCreateClone\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(DeleteClone *node)
	{
		Printf("\033[33mDeleteClone\033[0m\n");
	}

	virtual void Visit(AskAndWait *node)
	{
		Printf("\033[36;1mAskAndWait\033[0m\n");
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(SetDragMode *node)
	{
		Printf("\033[36;1mSetDragMode\033[0m %s\n",
			DragModeStrings[node->mode]);
	}

	virtual void Visit(ResetTimer *node)
	{
		Printf("\033[36;1mResetTimer\033[0m\n");
	}

	virtual void Visit(SetVariable *node)
	{
		Printf("\033[31;1mSetVariable\033[0m %s\n", node->name.c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ChangeVariable *node)
	{
		Printf("\033[31;1mChangeVariable\033[0m %s\n", node->name.c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(ShowVariable *node)
	{
		Printf("\033[31;1mShowVariable\033[0m %s\n", node->name.c_str());
	}

	virtual void Visit(HideVariable *node)
	{
		Printf("\033[31;1mHideVariable\033[0m %s\n", node->name.c_str());
	}

	virtual void Visit(AppendToList *node)
	{
		Printf("\033[31mAppendToList\033[0m %s\n",
			node->name.c_str());
		_indent++;
		node->e->Accept(this);
		_indent--;
	}

	virtual void Visit(DeleteAllList *node)
	{
		Printf("\033[31mDeleteAllList\033[0m %s\n",
			node->name.c_str());
	}

	virtual void Visit(InsertInList *node)
	{
		Printf("\033[31mInsertInList\033[0m %s\n",
			node->name.c_str());
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(ReplaceInList *node)
	{
		Printf("\033[31mInsertInList\033[0m %s\n",
			node->name.c_str());
		_indent++;
		node->e1->Accept(this);
		node->e2->Accept(this);
		_indent--;
	}

	virtual void Visit(ShowList *node)
	{
		Printf("\033[31mShowList\033[0m %s\n",
			node->name.c_str());
	}

	virtual void Visit(HideList *node)
	{
		Printf("\033[31mHideList\033[0m %s\n",
			node->name.c_str());
	}

	virtual void Visit(ProcProto *node)
	{
		Printf("\033[1mProcProto\033[0m %s\n", node->proccode.c_str());
		for (auto &name : node->argumentNames)
			Printf("  %s\n", name.c_str());
	}

	virtual void Visit(DefineProc *node)
	{
		Printf("\033[1mDefineProc\033[0m\n");
		_indent++;
		node->proto->Accept(this);
		_indent--;
	}

	virtual void Visit(Call *node)
	{
		Printf("\033[1mCall\033[0m %s\n", node->proccode.c_str());
		_indent++;
		for (auto &p : node->args)
		{
			Printf("\033[31;1m%s\033[0m\n", p.first.c_str());
			_indent++;
			p.second->Accept(this);
			_indent--;
		}
		_indent--;
	}

	virtual void Visit(GotoReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(GlideReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(PointTowardsReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(CostumeReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(BackdropReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(SoundReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(BroadcastReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(CloneReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(TouchingReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(DistanceReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(KeyReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(PropertyOfReporter *node)
	{
		Printf("\033[33;1m`%s`\033[0m\n", node->value.c_str());
	}

	virtual void Visit(VariableDef *node)
	{
		Printf("\033[31;1m%s\033[0m = ", node->name.c_str());

		int indent = _indent;
		_indent = 0;
		node->value->Accept(this);
		_indent = indent;
	}

	virtual void Visit(VariableDefList *node)
	{
		Printf("\033[1mVariableDefList\033[0m\n");
		_indent++;
		for (auto &v : node->variables)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(ListDef *node)
	{
		Printf("\033[31m%s\033[0m = {\n", node->name.c_str());
		_indent++;
		for (auto &v : node->value)
			v->Accept(this);
		_indent--;
		Printf("}\n");
	}

	virtual void Visit(ListDefList *node)
	{
		Printf("\033[1mListDefList\033[0m\n");
		_indent++;
		for (auto &v : node->lists)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(StatementListList *node)
	{
		Printf("\033[1mStatementListList\033[0m\n");
		_indent++;
		for (auto &v : node->sll)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(SpriteDef *node)
	{
		Printf("\033[1mSpriteDef\033[0m \033[33;1m%s\033[0m\n", node->name.c_str());
		_indent++;
		if (node->variables) node->variables->Accept(this);
		if (node->lists) node->lists->Accept(this);
		if (node->scripts) node->scripts->Accept(this);
		_indent--;
	}

	virtual void Visit(SpriteDefList *node)
	{
		Printf("\033[1mSpriteDefList\033[0m\n");
		_indent++;
		for (auto &v : node->sprites)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(StageDef *node)
	{
		Printf("\033[1mStageDef\033[0m\n");
		_indent++;
		if (node->variables) node->variables->Accept(this);
		if (node->lists) node->lists->Accept(this);
		if (node->scripts) node->scripts->Accept(this);
		_indent--;
	}

	virtual void Visit(ValMonitor *node) {}

	virtual void Visit(ValMonitorList *node)
	{
		Printf("\033[1mValMonitorList\033[0m\n");
		_indent++;
		for (auto &v : node->monitors)
			v->Accept(this);
		_indent--;
	}

	virtual void Visit(Program *node)
	{
		Printf("\033[1mProgram\033[0m\n");
		_indent++;
		if (node->stage) node->stage->Accept(this);
		if (node->sprites) node->sprites->Accept(this);
		if (node->monitors) node->monitors->Accept(this);
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

		fflush(stdout);
	}

	int _indent = 0;
};

Visitor *CreateDumpVisitor() { return new DumpVisitor(); }
