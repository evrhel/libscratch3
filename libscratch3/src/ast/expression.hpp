#pragma once

#include "astnode.hpp"

#include <stdexcept>

#include <semantics/syminfo.hpp>
#include <semantics/invariant.hpp>

#define S3_DEG_TO_RAD (3.14159265358979323846 / 180.0)
#define S3_RAD_TO_DEG (180.0 / 3.14159265358979323846)

// expression
struct Expression : public ASTNode
{
	AST_IMPL(Expression, ASTNode);

	//! \brief Attempt to collapse the expression into a constant expression.
	//! 
	//! \return The collapsed expression, or this if the expression cannot be collapsed.
	inline virtual Expression *Collapse() { return this; }

	constexpr bool IsNumeric() const
	{
		return syminfo.type == SymbolType_Int || syminfo.type == SymbolType_Number;
	}

	SymInfo syminfo = SymbolType_Any;
};

// expression evaluatable to a constant expression
// at compile time, given its arguments are constant
// expressions
struct Consteval : public Expression
{
	AST_IMPL(Consteval, Expression);
};

// constant expression
struct Constexpr : public Consteval
{
	static inline Constexpr *OfString(const std::string &value)
	{
		Constexpr *ce = new Constexpr();

		if (value.empty())
		{
			ce->syminfo = SymbolType_String;
			return ce;
		}

		ce->value = value;

		// attempt to parse a "true" or "false" boolean literal
		if (value == "true")
		{
			ce->bvalue = true;
			ce->syminfo = SymbolType_Bool;
			return ce;
		}

		if (value == "false")
		{
			ce->bvalue = false;
			ce->syminfo = SymbolType_Bool;
			return ce;
		}

		// attempt to parse as a integer
		try
		{
			size_t idx;
			int ival = std::stoi(value, &idx);
			if (idx != value.size())
				throw std::invalid_argument("invalid integer literal");

			ce->ivalue = static_cast<int64_t>(ival);
			ce->dvalue = static_cast<double>(ival);
			ce->syminfo = SymbolType_Int;
			return ce;
		}
		catch (std::invalid_argument) {}
		catch (std::out_of_range) {}

		// attempt to parse as a number
		try
		{
			size_t idx;
			double dval = std::stod(value, &idx);
			if (idx != value.size())
				throw std::invalid_argument("invalid number literal");

			ce->dvalue = dval;
			ce->ivalue = static_cast<int64_t>(dval);
			ce->syminfo = SymbolType_Number;
			return ce;
		}
		catch (std::invalid_argument) {}
		catch (std::out_of_range) {}

		// finally, it's just a string

		ce->syminfo = SymbolType_String;
		return ce;
	}

	static inline Constexpr *OfInt(int64_t value)
	{
		Constexpr *ce = new Constexpr();

		ce->value = std::to_string(value);
		ce->dvalue = static_cast<double>(value);
		ce->bvalue = false;
		ce->syminfo = SymbolType_Int;
		return ce;
	}

	static inline Constexpr *OfNumber(double value)
	{
		// check if the number is an integer
		if (floor(value) == value)
			return OfInt(static_cast<int64_t>(value));

		Constexpr *ce = new Constexpr();

		size_t len = snprintf(NULL, 0, "%.11g", value);
		std::string str(len, '\0');
		sprintf_s(&str[0], len + 1, "%.11g", value);

		ce->value = str;
		ce->ivalue = static_cast<int64_t>(value);
		ce->dvalue = value;
		ce->bvalue = false;
		ce->syminfo = SymbolType_Number;
		return ce;
	}

	static inline Constexpr *OfBool(bool value)
	{
		Constexpr *ce = new Constexpr();

		if (value)
			ce->value = "true";
		else
			ce->value = "false";
		ce->ivalue = 0;
		ce->dvalue = 0.0;
		ce->bvalue = value;
		ce->syminfo = SymbolType_Bool;
		return ce;
	}

	AST_IMPL(Constexpr, Consteval);
	AST_ACCEPTOR;

	std::string value;
	int64_t ivalue = 0;
	double dvalue = 0.0;
	bool bvalue = false;
};

// list of expressions
struct ExpressionList : public ASTNode
{
	AST_IMPL(ExpressionList, ASTNode);
	AST_ACCEPTOR;

	std::vector<AutoRelease<Expression>> expressions;
};

// (x position)
struct XPos : public Expression
{
	EXPR_IMPL(XPos, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (y position)
struct YPos : public Expression
{
	EXPR_IMPL(YPos, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (direction)
struct Direction : public Expression
{
	EXPR_IMPL(Direction, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (costume ?type)
struct CurrentCostume : public Expression
{
	EXPR_IMPL(CurrentCostume, Expression, SymbolType_String);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "NUMBER_NAME")
		{
			if (type == PropGetType_Unknown)
			{
				if (value == "number")
					type = PropGetType_Number, syminfo = SymbolType_Number;
				else if (value == "name")
					type = PropGetType_Name, syminfo = SymbolType_String;
				else
					return false;

				return true;
			}
		}

		return false;
	}

	PropGetType type = PropGetType_Unknown;
};

// (backdrop ?type)
struct CurrentBackdrop : public Expression
{
	EXPR_IMPL(CurrentBackdrop, Expression, SymbolType_String);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "NUMBER_NAME")
		{
			if (type == PropGetType_Unknown)
			{
				if (value == "number")
					type = PropGetType_Number, syminfo = SymbolType_Number;
				else if (value == "name")
					type = PropGetType_Name, syminfo = SymbolType_String;
				else
					return false;

				return true;
			}
		}

		return false;
	}

	PropGetType type = PropGetType_Unknown;
};

// (size)
struct Size : public Expression
{
	EXPR_IMPL(Size, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (volume)
struct Volume : public Expression
{
	EXPR_IMPL(Volume, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// <touching $e>
struct Touching : public Expression
{
	EXPR_IMPL(Touching, Expression, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, value)
	{
		if (key == "TOUCHINGOBJECTMENU")
		{
			if (!e)
				e = value->As<Expression>();
			return !!e;
		}

		return false;
	}

	AutoRelease<Expression> e; // object
};

// <touching color $e ?>
struct TouchingColor : public Expression
{
	EXPR_IMPL(TouchingColor, Expression, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "COLOR")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AutoRelease<Expression> e; // color
};

// <color $e1 is touching $e2 ?>
struct ColorTouching : public Expression
{
	EXPR_IMPL(ColorTouching, Expression, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "COLOR")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "COLOR2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual ~ColorTouching()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr; // color
	Expression *e2 = nullptr; // color
};

// (distance to $e)
struct DistanceTo : public Expression
{
	EXPR_IMPL(DistanceTo, Expression, SymbolType_Number);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, value)
	{
		if (key == "DISTANCETOMENU")
		{
			if (!e)
				e = value->As<Expression>();
			return !!e;
		}

		return false;
	}

	AutoRelease<Expression> e; // string
};

// (answer)
struct Answer : public Expression
{
	EXPR_IMPL(Answer, Expression, SymbolType_String);
	AST_ACCEPTOR;
};

// <key $e pressed>
struct KeyPressed : public Expression
{
	EXPR_IMPL(KeyPressed, Expression, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, value)
	{
		if (key == "KEY_OPTION")
		{
			if (!e)
				e = value->As<Expression>();
			return !!e;
		}

		return false;
	}

	AutoRelease<Expression> e; // string
};

// <mouse down>
struct MouseDown : public Expression
{
	EXPR_IMPL(MouseDown, Expression, SymbolType_Bool);
	AST_ACCEPTOR;
};

// (mouse x)
struct MouseX : public Expression
{
	EXPR_IMPL(MouseX, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (mouse y)
struct MouseY : public Expression
{
	EXPR_IMPL(MouseY, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (loudness)
struct Loudness : public Expression
{
	EXPR_IMPL(Loudness, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (timer)
struct TimerValue : public Expression
{
	EXPR_IMPL(TimerValue, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (?target of $e)
struct PropertyOf : public Expression
{
	EXPR_IMPL(PropertyOf, Expression, SymbolType_Any);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, value)
	{
		if (key == "OBJECT")
		{
			if (!e)
				e = value->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "PROPERTY")
		{
			if (target == PropertyTarget_Unknown)
			{
				if (value == "backdrop #")
				{
					target = PropertyTarget_BackdropNumber;
					syminfo = SymbolType_Number;
				}
				else if (value == "backdrop name")
				{
					target = PropertyTarget_BackdropName;
					syminfo = SymbolType_String;
				}
				else if (value == "x position")
				{
					target = PropertyTarget_XPosition;
					syminfo = SymbolType_Number;
				}
				else if (value == "y position")
				{
					target = PropertyTarget_YPosition;
					syminfo = SymbolType_Number;
				}
				else if (value == "direction")
				{
					target = PropertyTarget_Direction;
					syminfo = SymbolType_Number;
				}
				else if (value == "costume #")
				{
					target = PropertyTarget_CostumeNumber;
					syminfo = SymbolType_Number;
				}
				else if (value == "costume name")
				{
					target = PropertyTarget_CostumeName;
					syminfo = SymbolType_String;
				}
				else if (value == "size")
				{
					target = PropertyTarget_Size;
					syminfo = SymbolType_Number;
				}
				else if (value == "volume")
				{
					target = PropertyTarget_Volume;
					syminfo = SymbolType_Number;
				}
				else
				{
					target = PropertyTarget_Variable;
					syminfo = SymbolType_Any;
					name = value;
					this->id = id;
				}

				return true;
			}
		}

		return false;
	}

	PropertyTarget target = PropertyTarget_Unknown;
	std::string id, name; // id/name of variable if target is a variable
	AutoRelease<Expression> e; // string
};

// (current $format)
struct CurrentDate : public Expression
{
	EXPR_IMPL(CurrentDate, Expression, SymbolType_Number);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "CURRENTMENU")
		{
			if (format == DateFormat_Unknown)
			{
				if (value == "YEAR")
					format = DateFormat_Year;
				else if (value == "MONTH")
					format = DateFormat_Month;
				else if (value == "DATE")
					format = DateFormat_Date;
				else if (value == "DAYOFWEEK")
					format = DateFormat_DayOfWeek;
				else if (value == "HOUR")
					format = DateFormat_Hour;
				else if (value == "MINUTE")
					format = DateFormat_Minute;
				else if (value == "SECOND")
					format = DateFormat_Second;
				else
					return false;

				return true;
			}
		}

		return false;
	}

	DateFormat format = DateFormat_Unknown;
};

// (days since 2000)
struct DaysSince2000 : public Expression
{
	EXPR_IMPL(DaysSince2000, Expression, SymbolType_Number);
	AST_ACCEPTOR;
};

// (username)
struct Username : public Expression
{
	EXPR_IMPL(Username, Expression, SymbolType_String);
	AST_ACCEPTOR;
};

// ($e1 + $e2)
struct Add : public Consteval
{
	EXPR_IMPL(Add, Consteval, SymbolType_Number);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "NUM1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "NUM2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
			return Constexpr::OfNumber(c1->dvalue + c2->dvalue);

		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 - $e2)
struct Sub : public Consteval
{
	EXPR_IMPL(Sub, Consteval, SymbolType_Number);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "NUM1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "NUM2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
			return Constexpr::OfNumber(c1->dvalue - c2->dvalue);

		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 * $e2)
struct Mul : public Consteval
{
	EXPR_IMPL(Mul, Consteval, SymbolType_Number);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "NUM1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "NUM2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
			return Constexpr::OfNumber(c1->dvalue * c2->dvalue);

		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 / $e2)
struct Div : public Consteval
{
	EXPR_IMPL(Div, Consteval, SymbolType_Number);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "NUM1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "NUM2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
			return Constexpr::OfNumber(c1->dvalue / c2->dvalue);

		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// (pick random $e1 to $e2)
struct Random : public Expression
{
	EXPR_IMPL(Random, Expression, SymbolType_Number);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "FROM")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "TO")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();
		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 > $e2)
struct Greater : public Consteval
{
	EXPR_IMPL(Greater, Consteval, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "OPERAND1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "OPERAND2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		// TODO: handle string comparison, its not as simple as numeric comparison
		
		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 < $e2)
struct Less : public Consteval
{
	EXPR_IMPL(Less, Consteval, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "OPERAND1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "OPERAND2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		// TODO: same thing as Greater

		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 = $e2)
struct Equal : public Consteval
{
	EXPR_IMPL(Equal, Consteval, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "OPERAND1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "OPERAND2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
		{
			if (c1->IsNumeric() && c2->IsNumeric())
				return Constexpr::OfBool(c1->dvalue == c2->dvalue);

			// TODO: case insensitive string comparison
			return Constexpr::OfBool(c1->value == c2->value);
		}

		return this;
	}

	AutoRelease<Expression> e1; // any
	AutoRelease<Expression> e2; // any
};

// ($e1 and $e2)
struct LogicalAnd : public Consteval
{
	EXPR_IMPL(LogicalAnd, Consteval, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "OPERAND1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "OPERAND2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
			return Constexpr::OfBool(c1->bvalue && c2->bvalue);

		return this;
	}

	AutoRelease<Expression> e1; // bool
	AutoRelease<Expression> e2; // bool
};

// ($e1 or $e2)
struct LogicalOr : public Consteval
{
	EXPR_IMPL(LogicalOr, Consteval, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "OPERAND1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "OPERAND2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
			return Constexpr::OfBool(c1->bvalue || c2->bvalue);

		return this;
	}

	AutoRelease<Expression> e1; // bool
	AutoRelease<Expression> e2; // bool
};

// (not $e)
struct LogicalNot : public Consteval
{
	EXPR_IMPL(LogicalNot, Consteval, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "OPERAND")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e = e->Collapse();

		Constexpr *c = e->As<Constexpr>();

		if (c)
			return Constexpr::OfBool(c->bvalue);

		return this;
	}

	AutoRelease<Expression> e; // bool
};

// (join $e1 $e2)
struct Concat : public Consteval
{
	EXPR_IMPL(Concat, Consteval, SymbolType_String);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "STRING1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "STRING2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
			return Constexpr::OfString(c1->value + c2->value);

		return this;
	}

	AutoRelease<Expression> e1; // string
	AutoRelease<Expression> e2; // string
};

// (letter $e1 of $e2)
struct CharAt : public Consteval
{
	EXPR_IMPL(CharAt, Consteval, SymbolType_String);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "LETTER")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "STRING")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
		{
			int64_t index = static_cast<int64_t>(round(c1->dvalue));
			if (index < 1 || index >= (int64_t)c2->value.size())
				return new Constexpr(); // empty string
			return Constexpr::OfString(std::string(1, c2->value[index - 1]));
		}

		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // string
};

// (length of $e)
struct StringLength : public Consteval
{
	EXPR_IMPL(StringLength, Consteval, SymbolType_Int);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "STRING")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e = e->Collapse();

		Constexpr *c = e->As<Constexpr>();

		if (c)
			return Constexpr::OfInt(c->value.size());

		return this;
	}

	AutoRelease<Expression> e; // string
};

// <$e1 contains $e2 ?>
struct StringContains : public Consteval
{
	EXPR_IMPL(StringContains, Consteval, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "STRING1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "STRING2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
		{
			size_t pos = c1->value.find(c2->value);
			return Constexpr::OfBool(pos != std::string::npos);
		}

		return this;
	}

	AutoRelease<Expression> e1; // string
	AutoRelease<Expression> e2; // string
};

// ($e1 mod $e2)
struct Mod : public Consteval
{
	EXPR_IMPL(Mod, Consteval, SymbolType_Number);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "NUM1")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "NUM2")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e1 = e1->Collapse();
		e2 = e2->Collapse();

		Constexpr *c1 = e1->As<Constexpr>();
		Constexpr *c2 = e2->As<Constexpr>();

		if (c1 && c2)
			return Constexpr::OfNumber(fmod(c1->dvalue, c2->dvalue));

		return this;
	}

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// (round $e)
struct Round : public Consteval
{
	EXPR_IMPL(Round, Consteval, SymbolType_Int);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "NUM")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e = e->Collapse();

		Constexpr *c = e->As<Constexpr>();

		if (c)
			return Constexpr::OfNumber(round(c->dvalue));

		return this;
	}

	AutoRelease<Expression> e; // number
};

// (?func of $e)
struct MathFunc : public Consteval
{
	EXPR_IMPL(MathFunc, Consteval, SymbolType_Number);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "NUM")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "OPERATOR")
		{
			if (func != MathFuncType_Unknown)
				return false;
			func = MathFuncFromString(value);
			return func != MathFuncType_Unknown;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e = e->Collapse();
		Constexpr *c = e->As<Constexpr>();

		if (c)
		{
			switch (func)
			{
			case MathFuncType_Abs:
				return Constexpr::OfNumber(fabs(c->dvalue));
			case MathFuncType_Floor:
				return Constexpr::OfNumber(floor(c->dvalue));
			case MathFuncType_Ceil:
				return Constexpr::OfNumber(ceil(c->dvalue));
			case MathFuncType_Sqrt:
				return Constexpr::OfNumber(sqrt(c->dvalue));
			case MathFuncType_Sin:
				return Constexpr::OfNumber(sin(c->dvalue * S3_DEG_TO_RAD));
			case MathFuncType_Cos:
				return Constexpr::OfNumber(cos(c->dvalue * S3_DEG_TO_RAD));
			case MathFuncType_Tan:
				return Constexpr::OfNumber(tan(c->dvalue * S3_DEG_TO_RAD));
			case MathFuncType_Asin:
				return Constexpr::OfNumber(asin(c->dvalue) * S3_RAD_TO_DEG);
			case MathFuncType_Acos:
				return Constexpr::OfNumber(acos(c->dvalue) * S3_RAD_TO_DEG);
			case MathFuncType_Atan:
				return Constexpr::OfNumber(atan(c->dvalue) * S3_RAD_TO_DEG);
			case MathFuncType_Ln:
				return Constexpr::OfNumber(log(c->dvalue));
			case MathFuncType_Log:
				return Constexpr::OfNumber(log10(c->dvalue));
			case MathFuncType_Exp:
				return Constexpr::OfNumber(exp(c->dvalue));
			case MathFuncType_Exp10:
				return Constexpr::OfNumber(pow(10, c->dvalue));
			default:
				break; // runtime error
			}
		}

		return this;
	}

	MathFuncType func = MathFuncType_Unknown;
	AutoRelease<Expression> e; // number
};

// (?id)
struct VariableExpr : public Expression
{
	EXPR_IMPL(VariableExpr, Expression, SymbolType_Any);
	AST_ACCEPTOR;

	std::string id, name;
};

// broadcast id
struct BroadcastExpr : public Expression
{
	EXPR_IMPL(BroadcastExpr, Expression, SymbolType_Any);
	AST_ACCEPTOR;

	std::string id, name;
};

// (?id)
struct ListExpr : public Expression
{
	EXPR_IMPL(ListExpr, Expression, SymbolType_String);
	AST_ACCEPTOR;

	std::string id, name;
};

// (item $e of ?id)
struct ListAccess : public Expression
{
	EXPR_IMPL(ListAccess, Expression, SymbolType_Any);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "INDEX")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "LIST")
		{
			if (!this->id.empty())
				return false;
			this->id = id;
			this->name = value;
			return true;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e = e->Collapse();
		return this;
	}

	AutoRelease<Expression> e; // positive int
	std::string id, name;
};

// (item # of $e in ?id)
struct IndexOf : public Expression
{
	EXPR_IMPL(IndexOf, Expression, SymbolType_Int);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "ITEM")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "LIST")
		{
			if (!this->id.empty())
				return false;
			this->id = id;
			this->name = value;
			return true;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e = e->Collapse();
		return this;
	}

	AutoRelease<Expression> e; // any
	std::string id, name;
};

// (length of ?id)
struct ListLength : public Expression
{
	EXPR_IMPL(ListLength, Expression, SymbolType_Int);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "LIST")
		{
			if (!this->id.empty())
				return false;
			this->id = id;
			this->name = value;
			return true;
		}

		return false;
	}

	std::string id, name;
};

// <?id contains $e>
struct ListContains : public Expression
{
	EXPR_IMPL(ListContains, Expression, SymbolType_Bool);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "ITEM")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "LIST")
		{
			if (!this->id.empty())
				return false;
			this->id = id;
			this->name = value;
			return true;
		}

		return false;
	}

	inline virtual Expression *Collapse() override
	{
		e = e->Collapse();
		return this;
	}
	
	std::string id, name;
	AutoRelease<Expression> e; // any
};
