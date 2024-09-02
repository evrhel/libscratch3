#include "optimize.hpp"

#include "ast.hpp"
#include "../vm/memory.hpp"

class StaticEnvironment
{
public:
	OptionalValue &Lookup(const std::string &name)
	{
		return _variables[name];
	}

	void Clear()
	{
		_variables.clear();
	}

	StaticEnvironment()
	{

	}

	~StaticEnvironment()
	{

	}
private:
	std::unordered_map<std::string, OptionalValue> _variables;
};

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		else if (lhs.eval.IsNegativeOne())
		{
			// Replace with negation instruction
			AutoRelease<Neg> neg = new Neg();
			neg->e = &rhs;
			neg->Accept(this);

			output = neg;
		}
		else if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval = lhs.eval * rhs.eval;

		TryCollapse();
	}

	virtual void Visit(Div *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		node->eval.SetReal();
		output = node;
	}

	virtual void Visit(Greater *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		// Try to match the following patterns:
		//  x = true -> x
		//  x = false -> !x
		//  true = x -> x
		//  false = x -> !x

		/*if (rhs.eval.HasValue() && !lhs.eval.HasValue())
		{
			switch (rhs.eval.Type())
			{
			default:
				break;
			case ValueType_Integer:
			case ValueType_Real:
			case ValueType_Bool:
				if (rhs.eval.IsOne())
				{
					if (lhs.eval.Type() == ValueType_Bool)
					{
						output = &lhs;
						return;
					}
				}
				else if (rhs.eval.IsZero())
				{
					AutoRelease<LogicalNot> lnot = new LogicalNot();
					lnot->e = &lhs;
					
					output = lnot;
					lnot->Accept(this);

					return;
				}
			}
		}
		else if (lhs.eval.HasValue() && !rhs.eval.HasValue())
		{
			switch (lhs.eval.Type())
			{
			default:
				break;
			case ValueType_Integer:
			case ValueType_Real:
			case ValueType_Bool:
				if (lhs.eval.IsOne())
				{
					if (rhs.eval.Type() == ValueType_Bool)
					{
						output = &rhs;
						return;
					}
				}
				else if (lhs.eval.IsZero())
				{
					AutoRelease<LogicalNot> lnot = new LogicalNot();
					lnot->e = &rhs;

					output = lnot;
					lnot->Accept(this);

					return;
				}
			}
		}*/

		node->eval.SetBool();
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval.SetBool(Equals(lhs.eval.GetValue(), rhs.eval.GetValue()));

		TryCollapse();
	}

	virtual void Visit(LogicalAnd *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		node->e	= output.cast<Expression>();

		Expression &e = *node->e;

		node->eval.SetBool();
		output = node;

		if (e.eval.HasValue())
			node->eval.SetBool(!Truth(e.eval.GetValue()));

		TryCollapse();
	}

	virtual void Visit(Concat *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetString();
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
		{
			char c = ValueCharAt(lhs.eval.GetValue(), ToInteger(rhs.eval.GetValue()));
			
			Value tmp;
			InitializeValue(tmp);
			SetChar(tmp, c);

			node->eval.SetValue(tmp);

			ReleaseValue(tmp);
		}

		TryCollapse();
	}

	virtual void Visit(StringLength *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		Expression &e = *node->e;

		node->eval.SetInteger();
		output = node;

		if (e.eval.HasValue())
			node->eval.SetInteger(ValueLength(e.eval.GetValue()));

		TryCollapse();
	}

	virtual void Visit(StringContains *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetBool();
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
		{
			bool contains = ValueContains(lhs.eval.GetValue(), rhs.eval.GetValue());
			node->eval.SetBool(contains);
		}

		TryCollapse();
	}

	virtual void Visit(Mod *node) override
	{
		output.set(node->e1.get());
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		Expression &lhs = *node->e1;
		Expression &rhs = *node->e2;

		node->eval.SetReal();
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			Assign(tmp, lhs.eval.GetValue());

			ValueMod(tmp, rhs.eval.GetValue());

			node->eval.SetValue(tmp);

			ReleaseValue(tmp);
		}

		TryCollapse();
	}

	virtual void Visit(Round *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		Expression &e = *node->e;

		node->eval.SetInteger();
		output = node;

		if (e.eval.HasValue())
			node->eval.SetInteger((int)round(ToReal(e.eval.GetValue())));

		TryCollapse();
	}

	virtual void Visit(MathFunc *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

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
		/*OptionalValue &value = env.Lookup(node->name);
		if (value.HasValue())
		{
			Constexpr *ce = new Constexpr();
			ce->eval = value;
			output = ce;

			return;
		}

		node->eval = value;*/
		node->eval.SetUndefined();
		output = node;
	}

	virtual void Visit(BroadcastExpr *node) override
	{
		node->eval.SetString(node->id);
		output = node;
	}

	virtual void Visit(ListExpr *node) override
	{
		OptionalValue &list = env.Lookup(node->name);
		if (list.HasValue())
		{
			Constexpr *ce = new Constexpr();
			ce->eval.SetValue(list.GetValue());

			output = ce;
			return;
		}

		node->eval.SetList();
		output = node;
	}

	virtual void Visit(ListAccess *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		OptionalValue &list = env.Lookup(node->name);
		if (list.HasValue() && node->e->eval.HasValue())
		{
			Constexpr *ce = new Constexpr();

			Value tmp;
			InitializeValue(tmp);

			ListGet(tmp, list.GetValue(), ToInteger(node->e->eval.GetValue()));

			ce->eval.SetValue(tmp);

			ReleaseValue(tmp);

			output = ce;
			return;
		}

		output = node;
	}

	virtual void Visit(IndexOf *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		OptionalValue &list = env.Lookup(node->name);
		if (list.HasValue() && node->e->eval.HasValue())
		{
			Constexpr *ce = new Constexpr();

			int64_t idx = ListIndexOf(list.GetValue(), node->e->eval.GetValue());

			ce->eval.SetInteger(idx);

			output = ce;
			return;
		}

		output = node;
	}
	
	virtual void Visit(ListLength *node) override
	{
		OptionalValue &value = env.Lookup(node->name);
		if (value.HasValue())
		{
			Constexpr *ce = new Constexpr();
			int64_t len = ListGetLength(value.GetValue());

			ce->eval.SetInteger(len);

			output = ce;
			return;
		}

		node->eval.SetInteger();
		output = node;
	}

	virtual void Visit(ListContains *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		OptionalValue &list = env.Lookup(node->name);
		if (list.HasValue() && node->e->eval.HasValue());
		{
			Constexpr *ce = new Constexpr();
			bool contains = ListContainsValue(list.GetValue(), node->e->eval.GetValue());

			ce->eval.SetBool(contains);

			output = ce;
			return;
		}

		node->eval.SetBool();
		output = node;
	}

	virtual void Visit(MoveSteps *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			/* No effect */
			output = nullptr;
			return;
		}

		output = node;
	}

	virtual void Visit(TurnDegrees *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			/* No effect */
			output = nullptr;
			return;
		}

		output = node;
	}

	virtual void Visit(TurnNegDegrees *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			/* No effect */
			output = nullptr;
			return;
		}

		output = node;
	}

	virtual void Visit(Goto *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(GotoXY *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(Glide *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		env.Clear(); // Instruction causes yield, variables not preserved

		output = node;
	}

	virtual void Visit(GlideXY *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		output.set(node->e3);
		node->e3->Accept(this);
		node->e3 = output.cast<Expression>();

		env.Clear(); // Instruction causes yield, variables not preserved

		output = node;
	}

	virtual void Visit(PointDir *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(PointTowards *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(ChangeX *node) override
	{
		output = node->e.get();
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			/* No effect */
			output = nullptr;
			return;
		}

		output = node;
	}

	virtual void Visit(SetX *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(ChangeY *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			/* No effect */
			output = nullptr;
			return;
		}

		output = node;
	}

	virtual void Visit(SetY *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

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
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		env.Clear(); // Instruction causes yield, variables not preserved

		output = node;
	}

	virtual void Visit(Say *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(ThinkForSecs *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		env.Clear(); // Instruction causes yield, variables not preserved

		output = node;
	}

	virtual void Visit(Think *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(SwitchCostume *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(NextCostume *node) override
	{
		output = node;
	}

	virtual void Visit(SwitchBackdrop *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(SwitchBackdropAndWait *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(NextBackdrop *node) override
	{
		output = node;
	}

	virtual void Visit(ChangeSize *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			/* No effect */
			output = nullptr;
			return;
		}

		output = node;
	}

	virtual void Visit(SetSize *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(ChangeGraphicEffect *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(SetGraphicEffect *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

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
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			/* No effect */
			output = nullptr;
			return;
		}

		output = node;
	}

	virtual void Visit(PlaySoundUntilDone *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		env.Clear(); // Instruction causes yield, variables not preserved

		output = node;
	}

	virtual void Visit(StartSound *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(StopAllSounds *node) override
	{
		output = node;
	}

	virtual void Visit(ChangeSoundEffect *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(SetSoundEffect *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(ClearSoundEffects *node) override
	{
		output = node;
	}

	virtual void Visit(ChangeVolume *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			/* No effect */
			output = nullptr;
			return;
		}

		output = node;
	}

	virtual void Visit(SetVolume *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

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
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		env.Clear(); // Instruction may cause yield, variables not preserved

		output = node;
	}

	virtual void Visit(BroadcastAndWait *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		env.Clear(); // Instruction may cause yield, variables not preserved

		output = node;
	}

	virtual void Visit(WaitSecs *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		env.Clear(); // Instruction causes yield, variables not preserved

		output = node;
	}

	virtual void Visit(Repeat *node) override
	{
		env.Clear(); // Variables not preserved between iterations

		if (!node->e)
		{
			/* No count, repeat 0 times */
			output = nullptr;
			return;
		}

		if (!node->sl)
		{
			// TODO: may cause inconsistencies
			output = nullptr;
			return;
		}

		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output.set(node->sl);
		node->sl->Accept(this);
		node->sl = output.cast<StatementList>();

		output = node;
	}

	virtual void Visit(Forever *node) override
	{
		env.Clear(); // Variables not preserved between iterations

		if (!node->sl)
		{
			/* Must have a body */
			node->sl = new StatementList();
			output = node;
			return;
		}

		output.set(node->sl);
		node->sl->Accept(this);
		node->sl = output.cast<StatementList>();

		output = node;
	}

	virtual void Visit(If *node) override
	{
		if (!node->sl)
		{
			/* No branch */
			output = nullptr;
			return;
		}

		if (!node->e)
		{
			/* Condition always false */
			output = nullptr;
			return;
		}

		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.HasValue())
		{
			/* Evaluate condition */
			if (Truth(node->e->eval.GetValue()))
			{
				output.set(node->sl);
				node->sl->Accept(this);
			}
			else
				output = nullptr;

			return;
		}

		output.set(node->sl);
		node->sl->Accept(this);
		node->sl = output.cast<StatementList>();

		output = node;
	}

	virtual void Visit(IfElse *node) override
	{
		if (!node->sl1 && !node->sl2)
		{
			/* No branches */
			output = nullptr;
			return;
		}

		if (!node->e)
			goto only_else;

		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.HasValue())
		{
			/* Evaluate condition */
			if (Truth(node->e->eval.GetValue()))
			{
				if (node->sl1)
				{
					output.set(node->sl1);
					node->sl1->Accept(this);
				}
				else
					output = nullptr;
			}
			else
			{
			only_else:
				if (node->sl2)
				{
					output.set(node->sl2);
					node->sl2->Accept(this);
				}
				else
					output = nullptr;
			}

			return;
		}

		if (!node->sl1)
		{
			/* Invert condition and swap branches */
			AutoRelease<LogicalNot> notNode = new LogicalNot();
			notNode->e = node->e;

			AutoRelease<If> ifNode = new If();
			ifNode->e = notNode.get();
			ifNode->sl = node->sl2;

			output.set(ifNode);
			ifNode->Accept(this);

			return;
		}

		if (!node->sl2)
		{
			/* Remove else branch */
			AutoRelease<If> ifNode = new If();
			ifNode->e = node->e;
			ifNode->sl = node->sl1;

			output.set(ifNode);
			ifNode->Accept(this);

			return;
		}

		output.set(node->sl1);
		node->sl1->Accept(this);
		node->sl1 = output.cast<StatementList>();

		output.set(node->sl2);
		node->sl2->Accept(this);
		node->sl2 = output.cast<StatementList>();

		output = node;
	}

	virtual void Visit(WaitUntil *node) override
	{
		env.Clear(); // Instruction causes yield, variables not preserved

		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(RepeatUntil *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		env.Clear(); // Instruction causes yield, variables not preserved

		output.set(node->sl);
		node->sl->Accept(this);
		node->sl = output.cast<Expression>();

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
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		output = node;
	}

	virtual void Visit(DeleteClone *node) override
	{
		output = node;
	}

	virtual void Visit(AskAndWait *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		env.Clear(); // Instruction causes yield, variables not preserved

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
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		OptionalValue &value = env.Lookup(node->name);
		value = node->e->eval;

		output = node;
	}

	virtual void Visit(ChangeVariable *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		OptionalValue &value = env.Lookup(node->name);
		if (value.HasValue() && node->e->eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			Assign(tmp, value.GetValue());

			ValueAdd(tmp, node->e->eval.GetValue());

			value.SetValue(tmp);

			ReleaseValue(tmp);
		}
		else
			value.SetUndefined(); // TODO: make more specific

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
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		OptionalValue &list = env.Lookup(node->name);
		if (list.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			Assign(tmp, list.GetValue());

			ListAppend(tmp, node->e->eval.GetValue());

			list.SetValue(tmp);

			ReleaseValue(tmp);
		}
		else
			list.SetList();

		output = node;
	}

	virtual void Visit(DeleteFromList *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		OptionalValue &list = env.Lookup(node->name);
		if (list.HasValue() && node->e->eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			Assign(tmp, list.GetValue());

			ListDelete(tmp, node->e->eval.GetValue());

			list.SetValue(tmp);

			ReleaseValue(tmp);
		}
		else
			list.SetList();

		output = node;
	}

	virtual void Visit(DeleteAllList *node) override
	{
		/* Create a new empty list */
		OptionalValue &value = env.Lookup(node->name);

		Value tmp;
		InitializeValue(tmp);
		AllocList(tmp, 0);

		value.SetValue(tmp);

		ReleaseValue(tmp);

		output = node;
	}

	virtual void Visit(InsertInList *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output = node->e2.get();
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		OptionalValue &list = env.Lookup(node->name);
		if (list.HasValue() && node->e1->eval.HasValue() && node->e2->eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			Assign(tmp, list.GetValue());

			ListInsert(tmp, ToInteger(node->e2->eval.GetValue()), node->e1->eval.GetValue());

			list.SetValue(tmp);

			ReleaseValue(tmp);
		}
		else
			list.SetList();

		output = node;
	}

	virtual void Visit(ReplaceInList *node) override
	{
		output.set(node->e1);
		node->e1->Accept(this);
		node->e1 = output.cast<Expression>();

		output.set(node->e2);
		node->e2->Accept(this);
		node->e2 = output.cast<Expression>();

		OptionalValue &list = env.Lookup(node->name);
		list.SetList();

		// TODO: implement

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
			output.set(p.second);
			p.second->Accept(this);
			p.second = output.cast<Expression>();
		}

		output = node;
	}

	virtual void Visit(StatementList *node) override
	{
		for (AutoRelease<Statement> &s : node->sl)
		{
			output.set(s);
			s->Accept(this);
			s = output.cast<Statement>();
		}

		output = node;
	}

	virtual void Visit(StatementListList *node) override
	{
		for (AutoRelease<StatementList> &sl : node->sll)
		{
			env.Clear(); // Variables not set yet

			output = sl.get();
			sl->Accept(this);
			sl = output.cast<StatementList>();
		}

		output = node;
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
	StaticEnvironment env;
	int level;

	OptimizeVisitor(int level) : level(level) {}
};

void Optimize(Program *prog, int level)
{
	OptimizeVisitor visitor(level);
	prog->Accept(&visitor);
}
