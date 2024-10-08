#include "optimize.hpp"

#include <list>

#include "ast.hpp"
#include "../vm/memory.hpp"
#include "../codegen/util.hpp"

class StaticEnvironment
{
public:
	inline OptionalValue &Lookup(const std::string &name)
	{
		return _variables[name];
	}

	inline void Clear()
	{
		_variables.clear();
	}

	void Merge(const StaticEnvironment &from)
	{
		for (auto &p : from._variables)
		{
			OptionalValue &oldval = _variables[p.first];
			const OptionalValue &newval = p.second;

			if (oldval.HasValue() && newval.HasValue())
			{
				if (!Equals(oldval.GetValue(), newval.GetValue()))
				{
					if (oldval.Type() == newval.Type())
						oldval.SetType(newval.Type()); // Removes value, keeps type
					else
						oldval.SetUndefined();
				}
			}
			else if (oldval.Type() == newval.Type())
				oldval.SetType(newval.Type()); // Removes value, keeps type
			else
				oldval.SetUndefined();
		}
	}

	StaticEnvironment &operator=(const StaticEnvironment &env)
	{
		if (this == &env)
			return *this;

		Value tmp;
		InitializeValue(tmp);

		for (auto &p : env._variables)
		{
			if (p.second.HasValue())
			{
				ValueDeepCopy(tmp, p.second.GetValue());
				_variables[p.first].SetValue(tmp);
			}
			else
				_variables[p.first].SetType(p.second.Type());
		}

		ReleaseValue(tmp);

		return *this;
	}

	StaticEnvironment &operator=(StaticEnvironment &&env) noexcept
	{
		if (this == &env)
			return *this;

		_variables = std::move(env._variables);

		return *this;
	}

	inline StaticEnvironment() {}
	inline StaticEnvironment(const StaticEnvironment &env) { operator=(env); }
	inline StaticEnvironment(StaticEnvironment &&env) noexcept { _variables = std::move(env._variables); }
	inline ~StaticEnvironment() {}
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
			node->eval.SetInteger();
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
			node->eval.SetInteger();
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
		else if (lhs.eval.IsOne())
		{
			AutoRelease<Inc> inc = new Inc();
			inc->e = &rhs;

			output = inc;
			inc->Accept(this);
		}
		else if (lhs.eval.IsNegativeOne())
		{
			AutoRelease<Dec> dec = new Dec();
			dec->e = &rhs;

			output = dec;
			dec->Accept(this);
		}
		else if (rhs.eval.IsOne())
		{
			AutoRelease<Inc> inc = new Inc();
			inc->e = &lhs;

			output = inc;
			inc->Accept(this);
		}
		else if (rhs.eval.IsNegativeOne())
		{
			AutoRelease<Dec> dec = new Dec();
			dec->e = &lhs;

			output = dec;
			dec->Accept(this);
		}

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
		else if (rhs.eval.IsOne())
		{
			AutoRelease<Dec> dec = new Dec();
			dec->e = &lhs;

			output = dec;
			dec->Accept(this);
		}
		else if (rhs.eval.IsNegativeOne())
		{
			AutoRelease<Inc> inc = new Inc();
			inc->e = &lhs;

			output = inc;
			inc->Accept(this);
		}

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
		else if (rhs.eval.IsNegativeOne())
		{
			// Replace with negation instruction
			AutoRelease<Neg> neg = new Neg();
			neg->e = &lhs;
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

	virtual void Visit(Inc *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		Expression &e = *node->e;

		node->eval.SetReal();
		output = node;

		if (e.eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);

			SetInteger(tmp, 1);

			ValueAdd(tmp, e.eval.GetValue());

			node->eval.SetValue(tmp);

			ReleaseValue(tmp);
		}

		TryCollapse();
	}

	virtual void Visit(Dec *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		Expression &e = *node->e;

		node->eval.SetReal();
		output = node;

		if (e.eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);

			Assign(tmp, e.eval.GetValue());

			e.eval.SetInteger(1);

			ValueSub(tmp, e.eval.GetValue());

			node->eval.SetValue(tmp);

			ReleaseValue(tmp);
		}

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

		node->eval.SetBool();
		output = node;

		if (lhs.eval.HasValue() && rhs.eval.HasValue())
			node->eval.SetBool(Equals(lhs.eval.GetValue(), rhs.eval.GetValue()));
		else if (rhs.eval.IsZero())
		{
			AutoRelease<LogicalNot> not = new LogicalNot();
			not->e = &lhs;

			output = not;
			not->Accept(this);
		}
		else if (lhs.eval.IsZero())
		{
			AutoRelease<LogicalNot> not = new LogicalNot();
			not->e = &rhs;

			output = not;
			not->Accept(this);
		}
		else if (rhs.eval.IsOne())
		{
			if (lhs.eval.Type() == ValueType_Bool)
				output = &lhs; // Optimize boolean equality
			else
			{
				AutoRelease<Constexpr> ce = new Constexpr();
				ce->eval.SetBool(false);
				node->e2 = ce;

				AutoRelease<LogicalNot> not = new LogicalNot();
				not->e = node;

				output = not;
				not->Accept(this);
			}
		}
		else if (lhs.eval.IsOne())
		{
			if (rhs.eval.Type() == ValueType_Bool)
				output = &rhs; // Optimize boolean equality
			else
			{
				AutoRelease<Constexpr> ce = new Constexpr();
				ce->eval.SetBool(false);
				node->e1 = ce;

				AutoRelease<LogicalNot> not = new LogicalNot();
				not->e = node;

				output = not;
				not->Accept(this);
			}
		}

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
		if (!node->e)
		{
			AutoRelease<Constexpr> ce = new Constexpr();
			ce->eval.SetBool(true);

			output = ce;
			return;
		}

		output = node->e;
		node->e->Accept(this);
		node->e	= output.cast<Expression>();

		if (node->e->Is(Ast_LogicalNot))
		{
			AutoRelease<LogicalNot> other = node->e.cast<LogicalNot>();
			if (other->e->eval.Type() == ValueType_Bool) // can only be performed on a boolean
			{
				// double negation
				output = other->e;

				TryCollapse();
				return;
			}
		}

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
		else if (e.eval.Type() == ValueType_Integer || e.eval.Type() == ValueType_Bool)
			output.set(&e); // no need to round an integral type

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
				node->eval.SetInteger(static_cast<int64_t>(floor(ToReal(e.eval.GetValue()))));
			else if (e.eval.Type() == ValueType_Integer || e.eval.Type() == ValueType_Bool)
				output.set(&e); // no need to floor an integral type
			break;
		case MathFuncType_Ceil:
			if (e.eval.HasValue())
				node->eval.SetInteger(static_cast<int64_t>(ceil(ToReal(e.eval.GetValue()))));
			else if (e.eval.Type() == ValueType_Integer || e.eval.Type() == ValueType_Bool)
				output.set(&e); // no need to ceil an integral type
			break;
		case MathFuncType_Sqrt:
			if (e.eval.HasValue())
				node->eval.SetReal(sqrt(ToReal(e.eval.GetValue())));
			break;
		case MathFuncType_Sin:
			if (e.eval.HasValue())
				node->eval.SetReal(sin(ToReal(e.eval.GetValue()) * DEG2RAD));
			break;
		case MathFuncType_Cos:
			if (e.eval.HasValue())
				node->eval.SetReal(cos(ToReal(e.eval.GetValue()) * DEG2RAD));
			break;
		case MathFuncType_Tan:
			if (e.eval.HasValue())
				node->eval.SetReal(tan(ToReal(e.eval.GetValue()) * DEG2RAD));
			break;
		case MathFuncType_Asin:
			if (e.eval.HasValue())
				node->eval.SetReal(asin(ToReal(e.eval.GetValue())) * RAD2DEG);
			break;
		case MathFuncType_Acos:
			if (e.eval.HasValue())
				node->eval.SetReal(acos(ToReal(e.eval.GetValue())) * RAD2DEG);
			break;
		case MathFuncType_Atan:
			if (e.eval.HasValue())
				node->eval.SetReal(atan(ToReal(e.eval.GetValue())) * RAD2DEG);
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
		OptionalValue &value = GetEnv().Lookup(node->id);
		if (value.HasValue())
		{
			Constexpr *ce = new Constexpr();
			ce->eval = value;
			output = ce;

			return;
		}

		node->eval = value;
		output = node;
	}

	virtual void Visit(BroadcastExpr *node) override
	{
		node->eval.SetString(node->id);
		output = node;
	}

	virtual void Visit(ListExpr *node) override
	{
		OptionalValue &list = GetEnv().Lookup(node->id);
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

		OptionalValue &list = GetEnv().Lookup(node->id);
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

		OptionalValue &list = GetEnv().Lookup(node->id);
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
		OptionalValue &value = GetEnv().Lookup(node->id);
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

		OptionalValue &list = GetEnv().Lookup(node->id);
		if (list.HasValue() && node->e->eval.HasValue())
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

	virtual void Visit(PenMenuColorProperty *node) override
	{
		node->eval.SetString();
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

		AutoRelease<Neg> neg = new Neg();
		neg->e = node->e;

		AutoRelease<TurnDegrees> turn = new TurnDegrees();
		turn->e = neg;

		output.set(turn);
		turn->Accept(this);
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

		GetEnv().Clear(); // Instruction causes yield, variables not preserved

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

		GetEnv().Clear(); // Instruction causes yield, variables not preserved

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

		GetEnv().Clear(); // Instruction causes yield, variables not preserved

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

		GetEnv().Clear(); // Instruction causes yield, variables not preserved

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

		GetEnv().Clear(); // Instruction causes yield, variables not preserved

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

		GetEnv().Clear(); // Instruction may cause yield, variables not preserved

		output = node;
	}

	virtual void Visit(BroadcastAndWait *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		GetEnv().Clear(); // Instruction may cause yield, variables not preserved

		output = node;
	}

	virtual void Visit(WaitSecs *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		// NOTE: cannot optimize even if e is <= 0, waiting zero seconds still has an effect

		GetEnv().Clear(); // Instruction causes yield, variables not preserved

		output = node;
	}

	virtual void Visit(Repeat *node) override
	{
		if (!node->e)
		{
			// No effect
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

		if (!node->e->eval.IsPositiveOrZero())
		{
			// No effect
			output = nullptr;
			return;
		}

		if (node->e->eval.IsOne())
		{
			// Single iteration
			output = node->sl;
			node->sl->Accept(this);

			return;
		}
			
		GetEnv().Clear(); // Variables not preserved between iterations

		output.set(node->sl);
		node->sl->Accept(this);
		node->sl = output.cast<StatementList>();

		output = node;
	}

	virtual void Visit(Forever *node) override
	{
		GetEnv().Clear(); // Variables not preserved between iterations

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

		PushEnv();

		output.set(node->sl);
		node->sl->Accept(this);
		node->sl = output.cast<StatementList>();

		PopEnv();

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

		if (node->e->Is(Ast_LogicalNot))
		{
			// swap branches and invert condition
			AutoRelease<StatementList> tmp = node->sl1;
			node->sl1 = node->sl2;
			node->sl2 = tmp;

			AutoRelease<LogicalNot> not = node->e.cast<LogicalNot>();
			node->e = not->e;
		}

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
		}
		else if (!node->sl2)
		{
			/* Remove else branch */
			AutoRelease<If> ifNode = new If();
			ifNode->e = node->e;
			ifNode->sl = node->sl1;

			output.set(ifNode);
			ifNode->Accept(this);
		}
		else
		{
			PushEnv();

			output.set(node->sl1);
			node->sl1->Accept(this);
			node->sl1 = output.cast<StatementList>();

			PushEnv(1); // Separate environment for else branch

			output.set(node->sl2);
			node->sl2->Accept(this);
			node->sl2 = output.cast<StatementList>();

			PopEnv(); // merge if and else environments
			OverwriteEnv(); // overwrite the current environment with the merged one

			output = node;
		}
	}

	virtual void Visit(WaitUntil *node) override
	{
		GetEnv().Clear(); // Instruction causes yield, variables not preserved

		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
			printf("Warning: wait until will never terminate\n");

		output = node;
	}

	virtual void Visit(RepeatUntil *node) override
	{
		GetEnv().Clear(); // Instruction causes yield, variables not preserved

		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		if (node->e->eval.IsZero())
		{
			printf("Warning: repeat until will never terminate\n");

			AutoRelease<Forever> forever = new Forever();
			forever->sl = node->sl;

			output.set(forever);
			forever->Accept(this);

			return;
		}

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

		GetEnv().Clear(); // Instruction causes yield, variables not preserved

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

		OptionalValue &value = GetEnv().Lookup(node->id);

		if (value.HasValue() && node->e->eval.HasValue())
		{
			if (Equals(value.GetValue(), node->e->eval.GetValue()))
			{
				// No change
				output = nullptr;
				return;
			}
		}

		value = node->e->eval;

		output = node;
	}

	virtual void Visit(ChangeVariable *node) override
	{
		output.set(node->e);
		node->e->Accept(this);
		node->e = output.cast<Expression>();

		OptionalValue &value = GetEnv().Lookup(node->id);
		OptionalValue &dx = node->e->eval;

		if (value.HasValue() && dx.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			Assign(tmp, value.GetValue());

			ValueAdd(tmp, dx.GetValue());

			value.SetValue(tmp);

			ReleaseValue(tmp);
		}
		else if (dx.IsZeroLike())
		{
			switch (value.Type())
			{
			default:
			case ValueType_Undefined:
			case ValueType_String:
			case ValueType_List:
				value.SetReal();
				break;
			case ValueType_None: {
				AutoRelease<SetVariable> sv = new SetVariable();
				sv->id = node->id;
				sv->name = node->name;

				AutoRelease<Constexpr> ce = new Constexpr();
				ce->eval.SetInteger(0);

				sv->e = ce.get();
				
				break;
			}
			case ValueType_Integer:
			case ValueType_Real:
				output = nullptr;
				break;
			case ValueType_Bool:
				if (value.HasValue())
					value.SetInteger(Truth(value.GetValue()));
				else
					value.SetInteger();
				break;
			}
		}
		else if (value.IsZeroLike())
		{
			// Adding to zero is same as assignment which is more efficient

			AutoRelease<SetVariable> sv = new SetVariable();
			sv->id = node->id;
			sv->name = node->name;

			sv->e = node->e;

			output.set(sv);

			return;
		}

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

		OptionalValue &list = GetEnv().Lookup(node->id);
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

		OptionalValue &list = GetEnv().Lookup(node->id);
		if (list.HasValue() && node->e->eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			ValueDeepCopy(tmp, list.GetValue());

			ListDelete(tmp, node->e->eval.GetValue());

			list.SetValue(tmp);

			ReleaseValue(tmp);
		}
		else if (node->e->eval.HasValue() && node->e->eval.Type() == ValueType_String && !strcmp(node->e->eval.GetValue().u.string->str, "all"))
		{
			Value tmp;
			InitializeValue(tmp);
			AllocList(tmp, 0);

			list.SetValue(tmp);

			ReleaseValue(tmp);

			AutoRelease<DeleteAllList> dal = new DeleteAllList();
			dal->id = node->id;
			dal->name = node->name;

			output.set(dal);
			dal->Accept(this);

			return;
		}
		else
			list.SetList();

		output = node;
	}

	virtual void Visit(DeleteAllList *node) override
	{
		/* Create a new empty list */
		OptionalValue &value = GetEnv().Lookup(node->id);

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

		OptionalValue &list = GetEnv().Lookup(node->name);
		list.SetList();

		if (list.HasValue() && node->e1->eval.HasValue() && node->e2->eval.HasValue())
		{
			Value tmp;
			InitializeValue(tmp);
			ValueDeepCopy(tmp, list.GetValue());

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

		OptionalValue &list = GetEnv().Lookup(node->name);
		if (list.Type() != ValueType_List)
			list.SetList();

		if (node->e1->eval.IsNegativeOrZero())
		{
			// No effect
			output = nullptr;
			return;
		}

		if (list.HasValue())
		{
			if (node->e1->eval.HasValue())
			{
				int64_t idx;
				if (node->e1->eval.Type() == ValueType_Integer || node->e1->eval.Type() == ValueType_Bool)
					idx = ToInteger(node->e1->eval.GetValue());
				else if (node->e1->eval.Type() == ValueType_Real)
					idx = static_cast<int64_t>(round(ToReal(node->e1->eval.GetValue())));
				else
				{
					// Invalid index
					output = nullptr;
					return;
				}

				if (node->e2->eval.HasValue())
				{
					Value tmp;
					InitializeValue(tmp);
					
					ListGet(tmp, list.GetValue(), idx);

					if (Equals(tmp, node->e2->eval.GetValue()))
					{
						// No effect
						output = nullptr;
						return;
					}
					
					ValueDeepCopy(tmp, list.GetValue());

					ListSet(tmp, idx, node->e2->eval.GetValue());

					list.SetValue(tmp);

					ReleaseValue(tmp);
				}
				else
					list.SetList(); // Varying state
			}
			else
				list.SetList(); // Varying state
		}

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

	virtual void Visit(PenClear *node) override
	{
		output = node;
	}

	virtual void Visit(PenStamp *node) override
	{
		output = node;
	}

	virtual void Visit(PenDown *node) override
	{
		output = node;
	}

	virtual void Visit(PenUp *node) override
	{
		output = node;
	}

	virtual void Visit(SetPenColor *node) override
	{
		output = node;
	}

	virtual void Visit(ChangePenProperty *node) override
	{
		output = node;
	}

	virtual void Visit(SetPenProperty *node) override
	{
		output = node;
	}

	virtual void Visit(ChangePenSize *node) override
	{
		output = node;
	}

	virtual void Visit(SetPenSize *node) override
	{
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
			PushEnv();

			output = sl.get();
			sl->Accept(this);
			sl = output.cast<StatementList>();

			PopEnv();
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
	std::list<StaticEnvironment> envs;
	int level;

	OptimizeVisitor(int level) : level(level) {}
private:

	inline StaticEnvironment &GetEnv()
	{
		return envs.front();
	}

	inline void PushEnv()
	{
		if (envs.size() == 0)
			envs.emplace_front();
		else
			envs.push_front(GetEnv());
	}

	inline void PushEnv(int idx)
	{
		if (envs.size() == 0)
			envs.emplace_front();
		else
		{
			auto it = envs.begin();
			while (idx-- > 0)
				it++;

			envs.push_front(*it);
		}
	}

	inline void PopEnv(bool merge = true)
	{
		if (merge && envs.size() > 1)
		{
			StaticEnvironment &from = envs.front();
			StaticEnvironment &to = *++envs.begin();

			to.Merge(from);
		}

		envs.pop_front();
	}

	inline void OverwriteEnv()
	{
		StaticEnvironment &env = envs.front();
		StaticEnvironment &prev = *++envs.begin();

		prev = env;

		envs.pop_front();
	}
};

void Optimize(Program *prog, int level)
{
	OptimizeVisitor visitor(level);
	prog->Accept(&visitor);
}
