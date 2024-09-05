#pragma once

#include "astnode.hpp"
#include "../vm/memory.hpp"

#include <stdexcept>

// expression
struct Expression : public ASTNode
{
	AST_IMPL(Expression, ASTNode);

	OptionalValue eval; // evaluated value
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
	AST_IMPL(Constexpr, Consteval);
	AST_ACCEPTOR;

	//std::string value;
};

// (x position)
struct XPos : public Expression
{
	EXPR_IMPL(XPos, Expression);
	AST_ACCEPTOR;
};

// (y position)
struct YPos : public Expression
{
	EXPR_IMPL(YPos, Expression);
	AST_ACCEPTOR;
};

// (direction)
struct Direction : public Expression
{
	EXPR_IMPL(Direction, Expression);
	AST_ACCEPTOR;
};

// (costume ?type)
struct CurrentCostume : public Expression
{
	EXPR_IMPL(CurrentCostume, Expression);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "NUMBER_NAME")
		{
			if (type == PropGetType_Unknown)
			{
				if (value == "number")
					type = PropGetType_Number;
				else if (value == "name")
					type = PropGetType_Name;
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
	EXPR_IMPL(CurrentBackdrop, Expression);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "NUMBER_NAME")
		{
			if (type == PropGetType_Unknown)
			{
				if (value == "number")
					type = PropGetType_Number;
				else if (value == "name")
					type = PropGetType_Name;
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
	EXPR_IMPL(Size, Expression);
	AST_ACCEPTOR;
};

// (volume)
struct Volume : public Expression
{
	EXPR_IMPL(Volume, Expression);
	AST_ACCEPTOR;
};

// <touching $e>
struct Touching : public Expression
{
	EXPR_IMPL(Touching, Expression);
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
	EXPR_IMPL(TouchingColor, Expression);
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
	EXPR_IMPL(ColorTouching, Expression);
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
	EXPR_IMPL(DistanceTo, Expression);
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
	EXPR_IMPL(Answer, Expression);
	AST_ACCEPTOR;
};

// <key $e pressed>
struct KeyPressed : public Expression
{
	EXPR_IMPL(KeyPressed, Expression);
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
	EXPR_IMPL(MouseDown, Expression);
	AST_ACCEPTOR;
};

// (mouse x)
struct MouseX : public Expression
{
	EXPR_IMPL(MouseX, Expression);
	AST_ACCEPTOR;
};

// (mouse y)
struct MouseY : public Expression
{
	EXPR_IMPL(MouseY, Expression);
	AST_ACCEPTOR;
};

// (loudness)
struct Loudness : public Expression
{
	EXPR_IMPL(Loudness, Expression);
	AST_ACCEPTOR;
};

// (timer)
struct TimerValue : public Expression
{
	EXPR_IMPL(TimerValue, Expression);
	AST_ACCEPTOR;
};

// (?target of $e)
struct PropertyOf : public Expression
{
	EXPR_IMPL(PropertyOf, Expression);
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
					target = PropertyTarget_BackdropNumber;
				else if (value == "backdrop name")
					target = PropertyTarget_BackdropName;
				else if (value == "x position")
					target = PropertyTarget_XPosition;
				else if (value == "y position")
					target = PropertyTarget_YPosition;
				else if (value == "direction")
					target = PropertyTarget_Direction;
				else if (value == "costume #")
					target = PropertyTarget_CostumeNumber;
				else if (value == "costume name")
					target = PropertyTarget_CostumeName;
				else if (value == "size")
					target = PropertyTarget_Size;
				else if (value == "volume")
					target = PropertyTarget_Volume;
				else
				{
					target = PropertyTarget_Variable;
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
	EXPR_IMPL(CurrentDate, Expression);
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
	EXPR_IMPL(DaysSince2000, Expression);
	AST_ACCEPTOR;
};

// (username)
struct Username : public Expression
{
	EXPR_IMPL(Username, Expression);
	AST_ACCEPTOR;
};

// ($e1 + $e2)
struct Add : public Consteval
{
	EXPR_IMPL(Add, Consteval);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 - $e2)
struct Sub : public Consteval
{
	EXPR_IMPL(Sub, Consteval);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 * $e2)
struct Mul : public Consteval
{
	EXPR_IMPL(Mul, Consteval);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 / $e2)
struct Div : public Consteval
{
	EXPR_IMPL(Div, Consteval);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// negation
struct Neg : public Consteval
{
	EXPR_IMPL(Neg, Consteval);
	AST_ACCEPTOR;

	AutoRelease<Expression> e; // number
};

// increment by 1
struct Inc : public Consteval
{
	EXPR_IMPL(Inc, Consteval);
	AST_ACCEPTOR;

	AutoRelease<Expression> e; // number
};

// decremend by 1
struct Dec : public Consteval
{
	EXPR_IMPL(Dec, Consteval);
	AST_ACCEPTOR;

	AutoRelease<Expression> e; // number
};

// (pick random $e1 to $e2)
struct Random : public Expression
{
	EXPR_IMPL(Random, Expression);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 > $e2)
struct Greater : public Consteval
{
	EXPR_IMPL(Greater, Consteval);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 < $e2)
struct Less : public Consteval
{
	EXPR_IMPL(Less, Consteval);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// ($e1 = $e2)
struct Equal : public Consteval
{
	EXPR_IMPL(Equal, Consteval);
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

	AutoRelease<Expression> e1; // any
	AutoRelease<Expression> e2; // any
};

// ($e1 and $e2)
struct LogicalAnd : public Consteval
{
	EXPR_IMPL(LogicalAnd, Consteval);
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

	AutoRelease<Expression> e1; // bool
	AutoRelease<Expression> e2; // bool
};

// ($e1 or $e2)
struct LogicalOr : public Consteval
{
	EXPR_IMPL(LogicalOr, Consteval);
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

	AutoRelease<Expression> e1; // bool
	AutoRelease<Expression> e2; // bool
};

// (not $e)
struct LogicalNot : public Consteval
{
	EXPR_IMPL(LogicalNot, Consteval);
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

	AutoRelease<Expression> e; // bool
};

// (join $e1 $e2)
struct Concat : public Consteval
{
	EXPR_IMPL(Concat, Consteval);
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

	AutoRelease<Expression> e1; // string
	AutoRelease<Expression> e2; // string
};

// (letter $e1 of $e2)
struct CharAt : public Consteval
{
	EXPR_IMPL(CharAt, Consteval);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // string
};

// (length of $e)
struct StringLength : public Consteval
{
	EXPR_IMPL(StringLength, Consteval);
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

	AutoRelease<Expression> e; // string
};

// <$e1 contains $e2 ?>
struct StringContains : public Consteval
{
	EXPR_IMPL(StringContains, Consteval);
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

	AutoRelease<Expression> e1; // string
	AutoRelease<Expression> e2; // string
};

// ($e1 mod $e2)
struct Mod : public Consteval
{
	EXPR_IMPL(Mod, Consteval);
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

	AutoRelease<Expression> e1; // number
	AutoRelease<Expression> e2; // number
};

// (round $e)
struct Round : public Consteval
{
	EXPR_IMPL(Round, Consteval);
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

	AutoRelease<Expression> e; // number
};

// (?func of $e)
struct MathFunc : public Consteval
{
	EXPR_IMPL(MathFunc, Consteval);
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

	MathFuncType func = MathFuncType_Unknown;
	AutoRelease<Expression> e; // number
};

// (?id)
struct VariableExpr : public Expression
{
	EXPR_IMPL(VariableExpr, Expression);
	AST_ACCEPTOR;

	std::string id, name;
};

// broadcast id
struct BroadcastExpr : public Expression
{
	EXPR_IMPL(BroadcastExpr, Expression);
	AST_ACCEPTOR;

	std::string id, name;
};

// (?id)
struct ListExpr : public Expression
{
	EXPR_IMPL(ListExpr, Expression);
	AST_ACCEPTOR;

	std::string id, name;
};

// (item $e of ?id)
struct ListAccess : public Expression
{
	EXPR_IMPL(ListAccess, Expression);
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

	AutoRelease<Expression> e; // positive int
	std::string id, name;
};

// (item # of $e in ?id)
struct IndexOf : public Expression
{
	EXPR_IMPL(IndexOf, Expression);
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

	AutoRelease<Expression> e; // any
	std::string id, name;
};

// (length of ?id)
struct ListLength : public Expression
{
	EXPR_IMPL(ListLength, Expression);
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
	EXPR_IMPL(ListContains, Expression);
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
	
	std::string id, name;
	AutoRelease<Expression> e; // any
};

struct PenMenuColorProperty : public Expression
{
	EXPR_IMPL(PenMenuColorProperty, Expression);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "colorParam")
		{
			this->type = value;
			return true;
		}

		return false;
	}

	std::string type;
};
