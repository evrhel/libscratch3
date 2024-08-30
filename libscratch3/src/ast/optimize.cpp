#include "optimize.hpp"

#include "ast.hpp"
#include "../vm/memory.hpp"

class OptimizeVisitor : public Visitor
{
public:
	inline void TryCollapse()
	{
		if (output->Is(Ast_Expression))
		{
			Expression &e = (Expression &)*output;
			if (e.eval.HasValue())
			{
				Constexpr *ce = new Constexpr();
				ce->value = ToString(e.eval.GetValue());
				ce->eval = e.eval;
				output = ce;
			}
		}
	}

	virtual void Visit(XPos *node) override
	{
		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(YPos *node) override
	{
		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(Direction *node) override
	{
		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(CurrentCostume *node) override
	{
		switch (node->type)
		{
		default:
			node->eval.SetUndefined();
			break;
		case PropGetType_Number:
			node->eval.SetReal();
			break;
		case PropGetType_Name:
			node->eval.SetString();
			break;
		}

		output = node;
	}

	virtual void Visit(CurrentBackdrop *node) override
	{
		switch (node->type)
		{
		default:
			node->eval.SetUndefined();
			break;
		case PropGetType_Number:
			node->eval.SetReal();
			break;
		case PropGetType_Name:
			node->eval.SetString();
			break;
		}

		output = node;
	}

	virtual void Visit(Size *node) override
	{
		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(Volume *node) override
	{
		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(Touching *node) override
	{
		node->eval.SetBool();
		output = node;
	}

	virtual void Visit(TouchingColor *node) override
	{
		node->eval.SetBool();
		output = node;
	}

	virtual void Visit(ColorTouching *node) override
	{
		node->eval.SetBool();
		output = node;
	}

	virtual void Visit(DistanceTo *node) override
	{
		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(Answer *node) override
	{
		node->eval.SetUndefined();
		output = node;
	}

	virtual void Visit(KeyPressed *node) override
	{
		node->eval.SetBool();
		output = node;
	}

	virtual void Visit(MouseDown *node) override
	{
		node->eval.SetBool();
		output = node;
	}

	virtual void Visit(MouseX *node) override
	{
		node->eval.SetInteger();
		output = node;
	}

	virtual void Visit(MouseY *node) override
	{
		node->eval.SetInteger();
		output = node;
	}

	virtual void Visit(Loudness *node) override
	{
		node->eval.SetInteger();
		output = node;
	}

	virtual void Visit(TimerValue *node) override
	{
		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(PropertyOf *node) override
	{
		switch (node->target)
		{
		default:
			node->eval.SetUndefined();
			break;
		case PropertyTarget_BackdropNumber:
			node->eval.SetInteger();
			break;
		case PropertyTarget_BackdropName:
			node->eval.SetInteger();
			break;
		case PropertyTarget_XPosition:
			node->eval.SetReal();
			break;
		case PropertyTarget_YPosition:
			node->eval.SetReal();
			break;
		case PropertyTarget_Direction:
			node->eval.SetReal();
			break;
		case PropertyTarget_CostumeNumber:
			node->eval.SetInteger();
			break;
		case PropertyTarget_CostumeName:
			node->eval.SetInteger();
			break;
		case PropertyTarget_Size:
			node->eval.SetReal();
			break;
		case PropertyTarget_Volume:
			node->eval.SetReal();
			break;
		}

		output = node;
	}

	virtual void Visit(CurrentDate *node) override
	{
		node->eval.SetInteger();
		output = node;
	}

	virtual void Visit(DaysSince2000 *node) override
	{
		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(Add *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetReal();
		output = node;

		if (lhs.eval.IsZeroLike())
			output = &rhs; // Remove addition by zero
		else if (rhs.eval.IsZeroLike())
			output = &lhs; // Remove addition by zero
		else if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval = lhs.eval + rhs.eval;

		TryCollapse();
	}

	virtual void Visit(Sub *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetReal();
		output = node;
	
		if (lhs.eval.IsZeroLike())
		{
			// Replace with negation instruction
			Neg *neg = new Neg();
			neg->e = &rhs;
			neg->Accept(this);

			output = neg;
		}
		else if (rhs.eval.IsZeroLike())
			output = &lhs; // Remove subtraction by zero
		else if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval = lhs.eval - rhs.eval;

		TryCollapse();
	}

	virtual void Visit(Mul *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetReal();
		output = node;

		if (lhs.eval.IsZeroLike() || rhs.eval.IsZeroLike())
			node->eval.SetInteger(0);
		else if (lhs.eval.IsOne())
			output = &rhs; // Remove multiplication by one
		else if (rhs.eval.IsOne())
			output = &lhs; // Remove multiplication by one
		else if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval = lhs.eval * rhs.eval;

		TryCollapse();
	}

	virtual void Visit(Div *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetReal();
		output = node;

		if (rhs.eval.IsZeroLike())
		{
			if (lhs.eval.HasValue())
			{
				if (lhs.eval.IsZeroLike())
					node->eval.SetReal(NAN); // 0 / 0
				else if (lhs.eval.IsPositive())
					node->eval.SetReal(INFINITY); // x / 0
				else
					node->eval.SetReal(-INFINITY); // -x / 0
			}
			else
				node->eval.SetReal(); // result depends on sign of lhs
		}
		else if (lhs.eval.IsZeroLike())
			node->eval.SetInteger(0);
		else if (rhs.eval.IsOne())
			output = &lhs; // Remove division by one
		else if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval = lhs.eval / rhs.eval;

		TryCollapse();
	}

	virtual void Visit(Neg *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		Expression &e = *node->e;

		node->eval.SetReal();
		output = node;

		if (e.eval.IsZeroLike())
			node->eval.SetInteger(0);
		else if (e.eval.HasValue())
			node->eval = -e.eval;

		TryCollapse();
	}

	virtual void Visit(Random *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(Greater *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetBool();
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval.SetBool(ToReal(lhs.eval.GetValue()) > ToReal(rhs.eval.GetValue()));

		TryCollapse();
	}

	virtual void Visit(Less *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetBool();
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval.SetBool(ToReal(lhs.eval.GetValue()) < ToReal(rhs.eval.GetValue()));

		TryCollapse();
	}

	virtual void Visit(Equal *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetBool();
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval.SetBool(Equals(lhs.eval.GetValue(), rhs.eval.GetValue()));

		TryCollapse();
	}

	virtual void Visit(LogicalAnd *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetBool();
		output = node;

		if (lhs.eval.HasValue())
		{
			if (Truth(lhs.eval.GetValue()))
			{
				if (rhs.eval.HasValue())
				{
					if (Truth(rhs.eval.GetValue()))
						node->eval.SetBool(true);
					else
						node->eval.SetBool(false);
				}
				else
					output = &rhs;
			}
			else
				node->eval.SetBool(false);
		}
		else if (rhs.eval.HasValue())
		{
			if (Truth(rhs.eval.GetValue()))
				output = &lhs;
			else
				node->eval.SetBool(false);
		}

		TryCollapse();
	}

	virtual void Visit(LogicalOr *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetBool();
		output = node;

		if (lhs.eval.HasValue())
		{
			if (rhs.eval.HasValue())
				node->eval.SetBool(Truth(lhs.eval.GetValue()) || Truth(rhs.eval.GetValue()));
			else
				output = &rhs;
		}
		else if (rhs.eval.HasValue())
		{
			if (Truth(rhs.eval.GetValue()))
				node->eval.SetBool(true);
			else
				output = &lhs;
		}

		TryCollapse();
	}

	virtual void Visit(LogicalNot *node) override
	{
		output = node;
		node->e->Accept(this);
		node->e	= (Expression *)output.get();

		Expression &e = *node->e;

		node->eval.SetBool();
		output = node;

		if (e.eval.HasValue())
			node->eval.SetBool(!Truth(e.eval.GetValue()));

		TryCollapse();
	}

	virtual void Visit(Concat *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetUndefined(); // concatenation could result in any type
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			Assign(tmp, lhs.eval.GetValue());
			ConcatValue(tmp, rhs.eval.GetValue());

			node->eval.SetValue(tmp);

			ReleaseValue(tmp);
		}
		else if (lhs.eval.IsNone())
			output = &rhs;
		else if (rhs.eval.IsNone())
			output = &lhs;

		TryCollapse();
	}

	virtual void Visit(CharAt *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetString();
		output = node;

		// TODO: Implement

		TryCollapse();
	}

	virtual void Visit(StringLength *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		Expression &e = *node->e;

		node->eval.SetInteger();
		output = node;

		TryCollapse();
	}

	virtual void Visit(StringContains *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetBool();
		output = node;

		// TODO: Implement

		TryCollapse();
	}

	virtual void Visit(Mod *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetReal();
		output = node;

		// TODO: Implement

		TryCollapse();
	}

	virtual void Visit(Round *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		Expression &e = *node->e;

		node->eval.SetInteger();
		output = node;

		if (e.eval.HasValue())
			node->eval.SetInteger((int)round(ToReal(e.eval.GetValue())));

		TryCollapse();
	}

	virtual void Visit(MathFunc *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		Expression &e = *node->e;

		node->eval.SetReal();
		output = node;

		switch (node->func)
		{
		default:
			node->eval.SetEmpty();
			break;
		case MathFuncType_Abs:
			if (e.eval.HasValue())
				node->eval.SetReal(fabs(ToReal(e.eval.GetValue())));
			break;
		case MathFuncType_Floor:
			if (e.eval.HasValue())
				node->eval.SetReal(floor(ToReal(e.eval.GetValue())));
			break;
		case MathFuncType_Ceil:
			if (e.eval.HasValue())
				node->eval.SetReal(ceil(ToReal(e.eval.GetValue())));
			break;
		case MathFuncType_Sqrt:
			if (e.eval.HasValue())
				node->eval.SetReal(sqrt(ToReal(e.eval.GetValue())));
			break;
		case MathFuncType_Sin:
			break;
		case MathFuncType_Cos:
			break;
		case MathFuncType_Tan:
			break;
		case MathFuncType_Asin:
			break;
		case MathFuncType_Acos:
			break;
		case MathFuncType_Atan:
			break;
		case MathFuncType_Ln:
			if (e.eval.HasValue())
				node->eval.SetReal(log(ToReal(e.eval.GetValue())));
			break;
		case MathFuncType_Log:
			if (e.eval.HasValue())
				node->eval.SetReal(log10(ToReal(e.eval.GetValue())));
			break;
		case MathFuncType_Exp:
			if (e.eval.HasValue())
				node->eval.SetReal(exp(ToReal(e.eval.GetValue())));
			break;
		case MathFuncType_Exp10:
			if (e.eval.HasValue())
				node->eval.SetReal(pow(10, ToReal(e.eval.GetValue())));
			break;
		}

		TryCollapse();
	}

	virtual void Visit(VariableExpr *node) override
	{
		output = node;
	}

	virtual void Visit(BroadcastExpr *node) override
	{
		output = node;
	}

	virtual void Visit(ListExpr *node) override
	{
		node->eval.SetList();
		output = node;
	}

	virtual void Visit(ListAccess *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(IndexOf *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}
	
	virtual void Visit(ListLength *node) override
	{
		node->eval.SetInteger();
		output = node;
	}

	virtual void Visit(ListContains *node) override
	{
		node->eval.SetBool();
		output = node;
	}

	virtual void Visit(MoveSteps *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(TurnDegrees *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(TurnNegDegrees *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(Goto *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(GotoXY *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(Glide *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(GlideXY *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		output = node->e3.get();
		node->e3->Accept(this);
		node->e3 = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(PointDir *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(PointTowards *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ChangeX *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SetX *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ChangeY *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SetY *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(BounceIfOnEdge *node) override
	{
		output = node;
	}

	virtual void Visit(SetRotationStyle *node) override
	{
		output = node;
	}

	virtual void Visit(SayForSecs *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(Say *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ThinkForSecs *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(Think *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SwitchCostume *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(NextCostume *node) override
	{
		output = node;
	}

	virtual void Visit(SwitchBackdrop *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SwitchBackdropAndWait *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(NextBackdrop *node) override
	{
		output = node;
	}

	virtual void Visit(ChangeSize *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SetSize *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ChangeGraphicEffect *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SetGraphicEffect *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ClearGraphicEffects *node) override
	{
		output = node;
	}

	virtual void Visit(ShowSprite *node) override
	{
		output = node;
	}

	virtual void Visit(HideSprite *node) override
	{
		output = node;
	}

	virtual void Visit(GotoLayer *node) override
	{
		output = node;
	}

	virtual void Visit(MoveLayer *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(PlaySoundUntilDone *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(StartSound *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(StopAllSounds *node) override
	{
		output = node;
	}

	virtual void Visit(ChangeSoundEffect *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SetSoundEffect *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ClearSoundEffects *node) override
	{
		output = node;
	}

	virtual void Visit(ChangeVolume *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SetVolume *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(OnFlagClicked *node) override
	{
		output = node;
	}

	virtual void Visit(OnKeyPressed *node) override
	{
		output = node;
	}

	virtual void Visit(OnSpriteClicked *node) override
	{
		output = node;
	}

	virtual void Visit(OnBackdropSwitch *node) override
	{
		output = node;
	}

	virtual void Visit(OnGreaterThan *node) override
	{
		output = node;
	}

	virtual void Visit(OnEvent *node) override
	{
		output = node;
	}

	virtual void Visit(Broadcast *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(BroadcastAndWait *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(WaitSecs *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(Repeat *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node->sl.get();
		node->sl->Accept(this);
		node->sl = (StatementList *)output.get();

		output = node;
	}

	virtual void Visit(Forever *node) override
	{
		output = node->sl.get();
		node->sl->Accept(this);
		node->sl = (StatementList *)output.get();

		output = node;
	}

	virtual void Visit(If *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node->sl.get();
		node->sl->Accept(this);
		node->sl = (StatementList *)output.get();

		output = node;
	}

	virtual void Visit(IfElse *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node->sl1.get();
		node->sl1->Accept(this);
		node->sl1 = (StatementList *)output.get();

		output = node->sl2.get();
		node->sl2->Accept(this);
		node->sl2 = (StatementList *)output.get();

		output = node;
	}

	virtual void Visit(WaitUntil *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(RepeatUntil *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node->sl.get();
		node->sl->Accept(this);
		node->sl = (StatementList *)output.get();

		output = node;
	}

	virtual void Visit(Stop *node) override
	{
		output = node;
	}

	virtual void Visit(CloneStart *node) override
	{
		output = node;
	}

	virtual void Visit(CreateClone *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(DeleteClone *node) override
	{
		output = node;
	}

	virtual void Visit(AskAndWait *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(SetDragMode *node) override
	{
		output = node;
	}

	virtual void Visit(ResetTimer *node) override
	{
		output = node;
	}

	virtual void Visit(SetVariable *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ChangeVariable *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ShowVariable *node) override
	{
		output = node;
	}

	virtual void Visit(HideVariable *node) override
	{
		output = node;
	}

	virtual void Visit(AppendToList *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(DeleteFromList *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(DeleteAllList *node) override
	{
		output = node;
	}

	virtual void Visit(InsertInList *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ReplaceInList *node) override
	{
		output = node->e1.get();
		node->e1->Accept(this);
		node->e1 = (Expression *)output.get();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = (Expression *)output.get();

		output = node;
	}

	virtual void Visit(ShowList *node) override
	{
		output = node;
	}

	virtual void Visit(HideList *node) override
	{
		output = node;
	}

	virtual void Visit(ProcProto *node) override
	{
		output = node;
	}

	virtual void Visit(DefineProc *node) override
	{
		output = node;
	}

	virtual void Visit(Call *node) override
	{
		for (std::pair<const std::string, AutoRelease<Expression>> &p : node->args)
		{
			output = p.second.get();
			p.second->Accept(this);
			p.second = (Expression *)output.get();
		}
	}

	virtual void Visit(StatementList *node) override
	{
		for (AutoRelease<Statement> &s : node->sl)
		{
			output = s.get();
			s->Accept(this);
			s = (Statement *)output.get();
		}

		output = node;
	}

	virtual void Visit(StatementListList *node) override
	{
		for (AutoRelease<StatementList> &sl : node->sll)
			sl->Accept(this);
	}

	virtual void Visit(SpriteDef *node) override
	{
		node->scripts->Accept(this);
	}

	virtual void Visit(SpriteDefList *node) override
	{
		for (AutoRelease<SpriteDef> &sd : node->sprites)
			sd->Accept(this);
	}

	virtual void Visit(Program *node) override
	{
		node->sprites->Accept(this);
	}

	AutoRelease<ASTNode> output;
	int level;

	OptimizeVisitor(int level) : level(level) {}
};

void Optimize(Program *prog, int level)
{
	OptimizeVisitor visitor(level);
	prog->Accept(&visitor);
}
