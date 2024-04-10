#pragma once

#include "astnode.hpp"

// expression
struct Expression : public ASTNode
{
	AST_IMPL(Expression, ASTNode);
};

// expression evaluatable at compile time, given
// its arguments are constant expressions
struct Consteval : public Expression
{
	AST_IMPL(Consteval, Expression);
};

// unevaluated expression, used in some expressions that require
// an expression that does not yet exist on the stack.
struct Unevaluated : public Expression
{
	AST_IMPL(Unevaluated, Expression);
};

// list of expressions
struct ExpressionList : public ASTNode
{
	AST_IMPL(ExpressionList, ASTNode);
	AST_ACCEPTOR;

	inline virtual ~ExpressionList()
	{
		for (auto e : expressions)
			delete e;
	}

	std::vector<Expression *> expressions;
};

// constant expression
struct Constexpr : public Consteval
{
	AST_IMPL(Constexpr, Consteval);

	std::string value; // empty string
};

// number
struct Number : public Constexpr
{
	AST_IMPL(Number, Constexpr);
	AST_ACCEPTOR;
};

// positive number
struct PositiveNumber : public Constexpr
{
	AST_IMPL(PositiveNumber, Constexpr);
	AST_ACCEPTOR;
};

// positive int
struct PositiveInt : public Constexpr
{
	AST_IMPL(PositiveInt, Constexpr);
	AST_ACCEPTOR;
};

// int
struct Int : public Constexpr
{
	AST_IMPL(Int, Constexpr);
	AST_ACCEPTOR;
};

// angle
struct Angle : public Constexpr
{
	AST_IMPL(Angle, Constexpr);
	AST_ACCEPTOR;
};

// color
struct Color : public Constexpr
{
	AST_IMPL(Color, Constexpr);
	AST_ACCEPTOR;
};

// string
struct String : public Constexpr
{
	AST_IMPL(String, Constexpr);
	AST_ACCEPTOR;
};

// true
struct True : public Constexpr
{
	AST_IMPL(True, Constexpr);
	AST_ACCEPTOR;
};

// false
struct False : public Constexpr
{
	AST_IMPL(False, Constexpr);
	AST_ACCEPTOR;
};

// emtpy
struct Null : public Constexpr
{
	AST_IMPL(Null, Constexpr);
	AST_ACCEPTOR;
};

// (x position)
struct XPos : public Expression
{
	AST_IMPL(XPos, Expression);
	AST_ACCEPTOR;
};

// (y position)
struct YPos : public Expression
{
	AST_IMPL(YPos, Expression);
	AST_ACCEPTOR;
};

// (direction)
struct Direction : public Expression
{
	AST_IMPL(Direction, Expression);
	AST_ACCEPTOR;
};

// (costume ?type)
struct CurrentCostume : public Expression
{
	AST_IMPL(CurrentCostume, Expression);
	AST_ACCEPTOR;

	PropGetType type;
};

// (backdrop ?type)
struct CurrentBackdrop : public Expression
{
	AST_IMPL(CurrentBackdrop, Expression);
	AST_ACCEPTOR;

	PropGetType type;
};

// (size)
struct Size : public Expression
{
	AST_IMPL(Size, Expression);
	AST_ACCEPTOR;
};

// (volume)
struct Volume : public Expression
{
	AST_IMPL(Volume, Expression);
	AST_ACCEPTOR;
};

// <touching $e>
struct Touching : public Expression
{
	AST_IMPL(Touching, Expression);
	AST_ACCEPTOR;

	inline virtual ~Touching()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// <touching color $e ?>
struct TouchingColor : public Expression
{
	AST_IMPL(TouchingColor, Expression);
	AST_ACCEPTOR;

	inline virtual ~TouchingColor()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// <color $e1 is touching $e2 ?>
struct ColorTouching : public Expression
{
	AST_IMPL(ColorTouching, Expression);
	AST_ACCEPTOR;

	inline virtual ~ColorTouching()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// (distance to $e)
struct DistanceTo : public Expression
{
	AST_IMPL(DistanceTo, Expression);
	AST_ACCEPTOR;

	inline virtual ~DistanceTo()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// (answer)
struct Answer : public Expression
{
	AST_IMPL(Answer, Expression);
	AST_ACCEPTOR;
};

// <key $e pressed>
struct KeyPressed : public Expression
{
	AST_IMPL(KeyPressed, Expression);
	AST_ACCEPTOR;

	inline virtual ~KeyPressed()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// <mouse down>
struct MouseDown : public Expression
{
	AST_IMPL(MouseDown, Expression);
	AST_ACCEPTOR;
};

// (mouse x)
struct MouseX : public Expression
{
	AST_IMPL(MouseX, Expression);
	AST_ACCEPTOR;
};

// (mouse y)
struct MouseY : public Expression
{
	AST_IMPL(MouseY, Expression);
	AST_ACCEPTOR;
};

// (loudness)
struct Loudness : public Expression
{
	AST_IMPL(Loudness, Expression);
	AST_ACCEPTOR;
};

// (timer)
struct TimerValue : public Expression
{
	AST_IMPL(TimerValue, Expression);
	AST_ACCEPTOR;
};

// (?target of $e)
struct PropertyOf : public Expression
{
	AST_IMPL(PropertyOf, Expression);
	AST_ACCEPTOR;

	inline virtual ~PropertyOf()
	{
		delete e;
	}

	PropertyTarget target = PropertyTarget_Unknown;
	std::string id, name; // id/name of variable if target is a variable
	Expression *e = nullptr;
};

// (current $format)
struct CurrentDate : public Expression
{
	AST_IMPL(CurrentDate, Expression);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "CURRENTMENU")
		{
			if (format.empty())
			{
				format = value;
				return true;
			}
		}

		return false;
	}

	std::string format;
};

// (days since 2000)
struct DaysSince2000 : public Expression
{
	AST_IMPL(DaysSince2000, Expression);
	AST_ACCEPTOR;
};

// (username)
struct Username : public Expression
{
	AST_IMPL(Username, Expression);
	AST_ACCEPTOR;
};

// ($e1 + $e2)
struct Add : public Consteval
{
	AST_IMPL(Add, Consteval);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, expr)
	{
		if (key == "NUM1")
		{
			if (!e1)
			{
				e1 = expr;
				return true;
			}
		}
		else if (key == "NUM2")
		{
			if (!e2)
			{
				e2 = expr;
				return true;
			}
		}

		return false;
	}

	inline virtual ~Add()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 - $e2)
struct Sub : public Consteval
{
	AST_IMPL(Sub, Consteval);
	AST_ACCEPTOR;

	inline virtual ~Sub()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 * $e2)
struct Mul : public Consteval
{
	AST_IMPL(Mul, Consteval);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, expr)
	{
		if (key == "NUM1")
		{
			if (e1)
				return false;

			e1 = expr;
			return true;
		}

		if (key == "NUM2")
		{
			if (e2)
				return false;

			e2 = expr;
			return true;
		}

		return false;
	}

	inline virtual ~Mul()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 / $e2)
struct Div : public Consteval
{
	AST_IMPL(Div, Consteval);
	AST_ACCEPTOR;

	inline virtual ~Div()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// (pick random $e1 to $e2)
struct Random : public Expression
{
	AST_IMPL(Random, Expression);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, expr)
	{
		if (key == "FROM")
		{
			if (!e1)
			{
				e1 = expr;
				return true;
			}
		}
		else if (key == "TO")
		{
			if (!e2)
			{
				e2 = expr;
				return true;
			}
		}

		return false;
	}

	inline virtual ~Random()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 > $e2)
struct Greater : public Consteval
{
	AST_IMPL(Greater, Consteval);
	AST_ACCEPTOR;

	inline virtual ~Greater()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 < $e2)
struct Less : public Consteval
{
	AST_IMPL(Less, Consteval);
	AST_ACCEPTOR;

	inline virtual ~Less()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 = $e2)
struct Equal : public Consteval
{
	AST_IMPL(Equal, Consteval);
	AST_ACCEPTOR;

	inline virtual ~Equal()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 and $e2)
struct LogicalAnd : public Consteval
{
	AST_IMPL(LogicalAnd, Consteval);
	AST_ACCEPTOR;

	inline virtual ~LogicalAnd()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 or $e2)
struct LogicalOr : public Consteval
{
	AST_IMPL(LogicalOr, Consteval);
	AST_ACCEPTOR;

	inline virtual ~LogicalOr()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// (not $e)
struct LogicalNot : public Consteval
{
	AST_IMPL(LogicalNot, Consteval);
	AST_ACCEPTOR;

	inline virtual ~LogicalNot()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// (join $e1 $e2)
struct Concat : public Consteval
{
	AST_IMPL(Concat, Consteval);
	AST_ACCEPTOR;

	inline virtual ~Concat()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// (letter $e1 of $e2)
struct CharAt : public Consteval
{
	AST_IMPL(CharAt, Consteval);
	AST_ACCEPTOR;

	inline virtual ~CharAt()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// (length of $e)
struct StringLength : public Consteval
{
	AST_IMPL(StringLength, Consteval);
	AST_ACCEPTOR;

	inline virtual ~StringLength()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// <$e1 contains $e2 ?>
struct StringContains : public Consteval
{
	AST_IMPL(StringContains, Consteval);
	AST_ACCEPTOR;

	inline virtual ~StringContains()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// ($e1 mod $e2)
struct Mod : public Consteval
{
	AST_IMPL(Mod, Consteval);
	AST_ACCEPTOR;

	inline virtual ~Mod()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// (round $e)
struct Round : public Consteval
{
	AST_IMPL(Round, Consteval);
	AST_ACCEPTOR;

	inline virtual ~Round()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// (?func of $e)
struct MathFunc : public Consteval
{
	AST_IMPL(MathFunc, Consteval);
	AST_ACCEPTOR;

	inline virtual ~MathFunc()
	{
		delete e;
	}

	std::string func;
	Expression *e = nullptr;
};

// (?id)
struct VariableExpr : public Expression
{
	AST_IMPL(VariableExpr, Expression);
	AST_ACCEPTOR;

	std::string id, name;
};

// (?id)
struct ListExpr : public Expression
{
	AST_IMPL(ListExpr, Expression);
	AST_ACCEPTOR;

	std::string id, name;
};

// (item $e of ?id)
struct ListAccess : public Expression
{
	AST_IMPL(ListAccess, Expression);
	AST_ACCEPTOR;

	inline virtual ~ListAccess()
	{
		delete e;
	}

	Expression *e = nullptr;
	std::string id, name;
};

// (item # of $e in ?id)
struct IndexOf : public Expression
{
	AST_IMPL(IndexOf, Expression);
	AST_ACCEPTOR;

	inline virtual ~IndexOf()
	{
		delete e;
	}

	Expression *e = nullptr;
	std::string id, name;
};

// (length of ?id)
struct ListLength : public Expression
{
	AST_IMPL(ListLength, Expression);
	AST_ACCEPTOR;

	std::string id, name;
};

// <?id contains $e>
struct ListContains : public Expression
{
	AST_IMPL(ListContains, Expression);
	AST_ACCEPTOR;

	inline virtual ~ListContains()
	{
		delete e;
	}

	std::string id, name;
	Expression *e = nullptr;
};
