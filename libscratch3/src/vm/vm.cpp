#include "vm.hpp"

#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <algorithm>

#include <imgui_impl_sdl2.h>

#include "../resource.hpp"
#include "../render/renderer.hpp"

#include "sprite.hpp"

#define TRUE_STRING "true"
#define FALSE_STRING "false"

#define DEG2RAD (0.017453292519943295769236907684886)
#define RAD2DEG (57.295779513082320876798154814105)

static const char *States[] = {
	"EMBRYO",
	"RUNNABLE",
	"WAITING",
	"SUSPENDED",
	"TERMINATED"
};

static const char *ExceptionString(ExceptionType type)
{
	switch (type)
	{
	default:
		return "Unknown exception";
	case Exception_None:
		return "No exception";
	case OutOfMemory:
		return "Out of memory";
	case StackOverflow:
		return "Stack overflow";
	case StackUnderflow:
		return "Stack underflow";
	case VariableNotFound:
		return "Variable not found";
	case IllegalOperation:
		return "Illegal operation";
	case InvalidArgument:
		return "Invalid argument";
	case UnsupportedOperation:
		return "Unsupported operation";
	case NotImplemented:
		return "Not implemented";
	case VMError:
		return "VM error";
	}
}

static bool StringEquals(const char *lstr, const char *rstr)
{
	if (lstr == rstr)
		return true;

	const char *lstart = lstr;
	while (isspace(*lstart))
		lstart++;

	const char *rend = lstart;
	while (!isspace(*rend))
		rend++;

	const char *rstart = rstr;
	while (isspace(*rstart))
		rstart++;

	const char *lend = rend;
	while (!isspace(*lend))
		lend++;

	if (rend - lstart != lend - rstart)
		return false;

	size_t len = rend - lstart;
	for (size_t i = 0; i < len; i++)
	{
		if (tolower(lstart[i]) != tolower(rstart[i]))
			return false;
	}

	return true;
}

static constexpr uint32_t HashString(const char *s)
{
	uint32_t hash = 1315423911;
	while (*s)
		hash ^= ((hash << 5) + *s++ + (hash >> 2));
	return hash;
}

static constexpr uint32_t kTrueHash = HashString(TRUE_STRING);
static constexpr uint32_t kFalseHash = HashString(FALSE_STRING);

class Executor : public Visitor
{
public:

	//
	/////////////////////////////////////////////////////////////////
	// Expressions
	//

	virtual void Visit(Constexpr *node)
	{
		vm->SetParsedString(vm->Push(), node->value);
	}

	virtual void Visit(XPos *node)
	{
		vm->SetReal(vm->Push(), script->sprite->GetX());
	}

	virtual void Visit(YPos *node)
	{
		vm->SetReal(vm->Push(), script->sprite->GetY());
	}

	virtual void Visit(Direction *node)
	{
		vm->SetReal(vm->Push(), script->sprite->GetDirection());
	}

	virtual void Visit(CurrentCostume *node)
	{
		Value &val = vm->Push();

		switch (node->type)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case PropGetType_Number:
			vm->SetInteger(val, script->sprite->GetCostume());
			break;
		case PropGetType_Name:
			// TODO: implement
			vm->SetBasicString(val, "costume1"); // always costume1
			break;
		}
	}

	virtual void Visit(CurrentBackdrop *node) {}

	virtual void Visit(Size *node)
	{
		vm->SetReal(vm->Push(), script->sprite->GetSize());
	}

	virtual void Visit(Volume *node)
	{
		vm->SetReal(vm->Push(), script->sprite->GetVolume());
	}

	virtual void Visit(Touching *node)
	{
		// TODO: implement
		node->e->Accept(this);
		const char *name = vm->ToString(vm->StackAt(0));
		vm->Pop();

		if (!strcmp(name, "_mouse_"))
		{
			vm->SetBool(vm->Push(), script->sprite->TouchingPoint(Vector2(vm->GetMouseX(), vm->GetMouseY())));
			return;
		}
			
		Sprite *sprite = vm->FindSprite(name);
		if (!sprite)
		{
			vm->SetBool(vm->Push(), false);
			return;
		}

		vm->SetBool(vm->Push(), script->sprite->TouchingSprite(sprite));
	}

	virtual void Visit(TouchingColor *node)
	{
		// TODO: implement
		node->e->Accept(this);
		int64_t color = vm->ToInteger(vm->StackAt(0));
		vm->Pop();

		vm->SetBool(vm->Push(), script->sprite->TouchingColor(color));
	}

	virtual void Visit(ColorTouching *node)
	{
		// TODO: implement
		vm->SetBool(vm->Push(), false);
	}

	virtual void Visit(DistanceTo *node)
	{
		int64_t len;
		node->e->Accept(this);
		const char *name = vm->ToString(vm->StackAt(0), &len);
		vm->Pop();

		Sprite *s = vm->FindSprite(std::string(name, len));
		if (!s)
		{
			vm->SetReal(vm->Push(), -1.0);
			return;
		}

		double dx = s->GetX() - script->sprite->GetX();
		double dy = s->GetY() - script->sprite->GetY();

		vm->SetReal(vm->Push(), sqrt(dx * dx + dy * dy));
	}

	virtual void Visit(Answer *node)
	{
		vm->Assign(vm->Push(), vm->GetAnswer());
	}

	virtual void Visit(KeyPressed *node)
	{
		int64_t len;
		node->e->Accept(this);
		const char *key = vm->ToString(vm->StackAt(0), &len);
		vm->Pop();

		// convert to lowercase
		std::string s(key, len);
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);

		// convert to scancode
		int scancode;
		if (s.size() == 1)
		{
			char c = s[0];
			if (c >= 'a' && c <= 'z')
				scancode = SDL_SCANCODE_A + (c - 'a');
			else if (c >= '0' && c <= '9')
				scancode = SDL_SCANCODE_0 + (c - '0');
			else
			{
				vm->SetBool(vm->Push(), false);
				return;
			}
		}
		else if (s == "space")
			scancode = SDL_SCANCODE_SPACE;
		else if (s == "up arrow")
			scancode = SDL_SCANCODE_UP;
		else if (s == "down arrow")
			scancode = SDL_SCANCODE_DOWN;
		else if (s == "right arrow")
			scancode = SDL_SCANCODE_RIGHT;
		else if (s == "left arrow")
			scancode = SDL_SCANCODE_LEFT;
		else if (s == "any")
			scancode = -1;
		else
		{
			vm->SetBool(vm->Push(), false);
			return;
		}

		vm->SetBool(vm->Push(), vm->GetKey(scancode));
	}

	virtual void Visit(MouseDown *node)
	{
		vm->SetBool(vm->Push(), vm->GetMouseDown());
	}

	virtual void Visit(MouseX *node)
	{
		vm->SetInteger(vm->Push(), vm->GetMouseX());
	}

	virtual void Visit(MouseY *node)
	{
		vm->SetInteger(vm->Push(), vm->GetMouseY());
	}

	virtual void Visit(Loudness *node)
	{
		vm->SetReal(vm->Push(), vm->GetLoudness());
	}

	virtual void Visit(TimerValue *node)
	{
		vm->SetReal(vm->Push(), vm->GetTimer());
	}

	virtual void Visit(PropertyOf *node)
	{
		int64_t len;
		node->e->Accept(this);
		const char *name = vm->ToString(vm->StackAt(0), &len);
		vm->Pop();

		Sprite *s = vm->FindSprite(std::string(name, len));
		if (!s)
		{
			vm->Push();
			return;
		}
		
		switch (node->target)
		{
		default:
			vm->Push();
			break;
		case PropertyTarget_BackdropNumber:
			vm->SetInteger(vm->Push(), 1);
			break;
		case PropertyTarget_BackdropName:
			vm->SetBasicString(vm->Push(), "backdrop1");
			break;
		case PropertyTarget_XPosition:
			vm->SetReal(vm->Push(), s->GetX());
			break;
		case PropertyTarget_YPosition:
			vm->SetReal(vm->Push(), s->GetY());
			break;
		case PropertyTarget_Direction:
			vm->SetReal(vm->Push(), s->GetDirection());
			break;
		case PropertyTarget_CostumeNumber:
			vm->SetInteger(vm->Push(), s->GetCostume());
			break;
		case PropertyTarget_CostumeName:
			vm->SetConstString(vm->Push(), &s->GetCostumeName());
			break;
		case PropertyTarget_Size:
			vm->SetReal(vm->Push(), s->GetSize());
			break;
		case PropertyTarget_Volume:
			vm->SetReal(vm->Push(), s->GetVolume());
			break;
		case PropertyTarget_Variable:
			vm->Push();
			break;
		}
	}

	virtual void Visit(CurrentDate *node)
	{
		Value &val = vm->Push();

		struct ls_timespec ts;
		ls_get_local_time(&ts);

		switch (node->format)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case DateFormat_Year:
			vm->SetInteger(val, ts.year);
			break;
		case DateFormat_Month:
			vm->SetInteger(val, ts.month);
			break;
		case DateFormat_Date:
			vm->SetInteger(val, ts.day);
			break;
		case DateFormat_DayOfWeek:
			// TODO: implement
			vm->SetInteger(val, 4); // always Thursday
			break;
		case DateFormat_Hour:
			vm->SetInteger(val, ts.hour);
			break;
		case DateFormat_Minute:
			vm->SetInteger(val, ts.minute);
			break;
		case DateFormat_Second:
			vm->SetInteger(val, ts.second);
			break;
		}
	}

	virtual void Visit(DaysSince2000 *node)
	{
		// TODO: implement
		vm->SetReal(vm->Push(), 0.0);
	}

	virtual void Visit(Username *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(Add *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = vm->ToReal(lhs) + vm->ToReal(rhs);

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(Sub *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = vm->ToReal(lhs) - vm->ToReal(rhs);

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(Mul *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = vm->ToReal(lhs) * vm->ToReal(rhs);

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(Div *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = vm->ToReal(lhs) / vm->ToReal(rhs);

		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(Random *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &from = vm->StackAt(1);
		Value &to = vm->StackAt(0);

		if (from.type == ValueType_Real || to.type == ValueType_Real)
		{
			double f = vm->ToReal(from);
			double t = vm->ToReal(to);

			if (t < f) // swap
			{
				double temp = f;
				f = t;
				t = temp;
			}

			vm->Pop();
			vm->Pop();

			double r = f + (ls_rand_double() * (t - f));

			vm->SetReal(vm->Push(), r);
		}
		else
		{
			int64_t f = vm->ToInteger(from);
			int64_t t = vm->ToInteger(to);

			if (t < f) // swap
			{
				int64_t temp = f;
				f = t;
				t = temp;
			}

			vm->Pop();
			vm->Pop();

			int64_t r = f + (ls_rand_uint64() % (t - f + 1));

			vm->SetInteger(vm->Push(), r);
		}
	}

	virtual void Visit(Greater *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		double lhs = vm->ToReal(vm->StackAt(1));
		double rhs = vm->ToReal(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetBool(vm->Push(), lhs > rhs);
	}

	virtual void Visit(Less *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);

		double lhs = vm->ToReal(vm->StackAt(1));
		double rhs = vm->ToReal(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetBool(vm->Push(), lhs < rhs);
	}

	virtual void Visit(Equal *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
	
		bool equal = vm->Equals(vm->StackAt(1), vm->StackAt(0));

		vm->Pop();
		vm->Pop();
		
		vm->SetBool(vm->Push(), equal);
	}

	virtual void Visit(LogicalAnd *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		bool lhs = vm->Truth(vm->StackAt(1));
		bool rhs = vm->Truth(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetBool(vm->Push(), lhs && rhs);
	}

	virtual void Visit(LogicalOr *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);

		bool lhs = vm->Truth(vm->StackAt(1));
		bool rhs = vm->Truth(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		vm->SetBool(vm->Push(), lhs || rhs);
	}

	virtual void Visit(LogicalNot *node)
	{
		node->e->Accept(this);

		bool truth = vm->Truth(vm->StackAt(0));

		vm->Pop();

		vm->SetBool(vm->Push(), !truth);
	}

	virtual void Visit(Concat *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
	
		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		vm->CvtString(lhs);
		vm->CvtString(rhs);

		Value &v = vm->Push();
		vm->AllocString(v, lhs.u.string->len + rhs.u.string->len);

		memcpy(v.u.string->str, lhs.u.string->str, lhs.u.string->len);
		memcpy(v.u.string->str + lhs.u.string->len, rhs.u.string->str, rhs.u.string->len);

		vm->Assign(lhs, v);

		vm->Pop();
		vm->Pop();
	}

	virtual void Visit(CharAt *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &v1 = vm->StackAt(1);
		Value &v2 =	vm->StackAt(0);

		vm->CvtString(v2);

		int64_t index = 0;
		switch (v1.type)
		{
		default:
			break;
		case ValueType_Integer: {
			index = v1.u.integer;
			break;
		}
		case ValueType_Real: {
			index = static_cast<int64_t>(v1.u.real);
			break;
		}
		}

		if (index < 1 || index > v2.u.string->len)
			vm->SetEmpty(v1);
		else
		{
			vm->AllocString(v1, 1);
			v1.u.string->str[0] = v2.u.string->str[index - 1];
		}

		vm->Pop();
	}

	virtual void Visit(StringLength *node)
	{
		int64_t len;
		node->e->Accept(this);
		vm->ToString(vm->StackAt(0), &len);
		vm->SetInteger(vm->StackAt(0), len);
	}

	virtual void Visit(StringContains *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &v1 = vm->StackAt(1);
		Value &v2 = vm->StackAt(0);

		vm->CvtString(v1);
		vm->CvtString(v2);
				
		// find v2 in v1, case-insensitive
		bool found = false;
		for (int64_t i = 0; i < v1.u.string->len; i++)
		{
			if (tolower(v1.u.string->str[i]) == tolower(v2.u.string->str[0]))
			{
				found = true;
				for (int64_t j = 1; j < v2.u.string->len; j++)
				{
					if (tolower(v1.u.string->str[i + j]) != tolower(v2.u.string->str[j]))
					{
						found = false;
						break;
					}
				}

				if (found)
					break;
			}
		}

		vm->SetBool(v1, found);
		vm->Pop();
	}

	virtual void Visit(Mod *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);
		double lhs = vm->ToReal(vm->StackAt(1));
		double rhs = vm->ToReal(vm->StackAt(0));
		vm->Pop();
		vm->Pop();

		vm->SetReal(vm->Push(), fmod(lhs, rhs));
	}

	virtual void Visit(Round *node)
	{
		node->e->Accept(this);
		double val = vm->ToReal(vm->StackAt(0));
		vm->Pop();
		vm->SetInteger(vm->Push(), static_cast<int64_t>(round(val)));
	}

	virtual void Visit(MathFunc *node)
	{
		node->e->Accept(this);
		double val = vm->ToReal(vm->StackAt(0));
		vm->Pop();

		double r;
		switch (node->func)
		{
		default:
			vm->Raise(InvalidArgument), r = NAN;
			return;
		case MathFuncType_Abs:
			r = abs(val);
			break;
		case MathFuncType_Floor:
			r = floor(val);
			break;
		case MathFuncType_Ceil:
			r = ceil(val);
			break;
		case MathFuncType_Sqrt:
			r = sqrt(val);
			break;
		case MathFuncType_Sin:
			r = sin(val * DEG2RAD);
			break;
		case MathFuncType_Cos:
			r = cos(val * DEG2RAD);
			break;
		case MathFuncType_Tan:
			r = tan(val * DEG2RAD);
			break;
		case MathFuncType_Asin:
			r = asin(val) * RAD2DEG;
			break;
		case MathFuncType_Acos:
			r = acos(val) * RAD2DEG;
			break;
		case MathFuncType_Atan:
			r = atan(val) * RAD2DEG;
			break;
		case MathFuncType_Ln:
			r = log(val);
			break;
		case MathFuncType_Log:
			r = log10(val);
			break;
		case MathFuncType_Exp:
			r = exp(val);
			break;
		case MathFuncType_Exp10:
			r = pow(10, val);
			break;
		}

		vm->SetReal(vm->Push(), r);
	}

	virtual void Visit(VariableExpr *node)
	{
		// TODO: replace with id
		Value &var = vm->FindVariable(node->name);
		vm->Assign(vm->Push(), var);
	}

	virtual void Visit(BroadcastExpr *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(ListExpr *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(ListAccess *node)
	{
		// TODO: implement
		vm->Push(); // None
	}

	virtual void Visit(IndexOf *node)
	{
		// TODO: implement
		vm->SetInteger(vm->Push(), 0);
	}

	virtual void Visit(ListLength *node)
	{
		// TODO: implement
		vm->SetInteger(vm->Push(), 0);
	}

	virtual void Visit(ListContains *node)
	{
		// TODO: implement
		vm->SetBool(vm->Push(), false);
	}

	//
	/////////////////////////////////////////////////////////////////
	// Statements
	//

	virtual void Visit(StatementList *node) {}

	virtual void Visit(MoveSteps *node)
	{
		node->e->Accept(this);
		double steps = vm->ToReal(vm->StackAt(0));
		vm->Pop();

		Sprite *s = script->sprite;

		const double dir = s->GetDirection() * DEG2RAD;
		double dx = steps * cos(dir);
		double dy = steps * sin(dir);

		s->SetXY(s->GetX() + dx, s->GetY() + dy);
	}

	virtual void Visit(TurnDegrees *node)
	{
		node->e->Accept(this);
		Sprite *s = script->sprite;
		s->SetDirection(s->GetDirection() + vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(TurnNegDegrees *node)
	{
		node->e->Accept(this);
		Sprite *s = script->sprite;
		s->SetDirection(s->GetDirection() - vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(Goto *node)
	{
		// TODO: implement
	}

	virtual void Visit(GotoXY *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
		Sprite *s = script->sprite;
		s->SetXY(vm->ToReal(vm->StackAt(1)), vm->ToReal(vm->StackAt(0)));
		vm->Pop();
		vm->Pop();
	}

	virtual void Visit(Glide *node)
	{
		node->e1->Accept(this); // seconds
		node->e2->Accept(this); // destination

		int64_t len;
		double secs = vm->ToReal(vm->StackAt(1));
		const char *dest = vm->ToString(vm->StackAt(0), &len);

		double x = 0, y = 0;

		if (!strcmp(dest, "random position"))
		{
			x = ls_rand_int(-240, 240);
			y = ls_rand_int(-180, 180);
		}
		else if (!strcmp(dest, "mouse-pointer"))
		{
			x = vm->GetMouseX();
			y = vm->GetMouseY();
		}
		else
		{
			Sprite *s = vm->FindSprite(std::string(dest, len));
			if (s != nullptr)
			{
				x = s->GetX();
				y = s->GetY();
			}
		}

		vm->Glide(script->sprite, x, y, secs);

		vm->Pop();
		vm->Pop();
	}

	virtual void Visit(GlideXY *node)
	{
		node->e1->Accept(this); // seconds
		node->e2->Accept(this); // x
		node->e3->Accept(this); // y

		double secs = vm->ToReal(vm->StackAt(2));
		double x = vm->ToReal(vm->StackAt(1));
		double y = vm->ToReal(vm->StackAt(0));

		vm->Glide(script->sprite, x, y, secs);

		vm->Pop();
		vm->Pop();
		vm->Pop();
	}

	virtual void Visit(PointDir *node)
	{
		node->e->Accept(this);
		script->sprite->SetDirection(vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	
	}

	virtual void Visit(PointTowards *node)
	{
		// TODO: implement
	}

	virtual void Visit(ChangeX *node)
	{
		node->e->Accept(this);
		Sprite *s = script->sprite;
		s->SetX(s->GetX() + vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(SetX *node)
	{
		node->e->Accept(this);
		script->sprite->SetX(vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(ChangeY *node)
	{
		node->e->Accept(this);
		Sprite *s = script->sprite;
		s->SetY(s->GetY() + vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(SetY *node)
	{
		node->e->Accept(this);
		script->sprite->SetY(vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(BounceIfOnEdge *node)
	{
		// TODO: implement
	}

	virtual void Visit(SetRotationStyle *node)
	{
		// TODO: implement
	}

	virtual void Visit(SayForSecs *node)
	{
		node->e1->Accept(this); // message
		node->e2->Accept(this); // duration

		Value &message = vm->StackAt(1);
		Value &duration = vm->StackAt(0);

		int64_t len;
		const char *mstr = vm->ToString(message, &len);
		double secs = vm->ToReal(duration);

		vm->Pop();
		vm->Pop();

		printf("%s saying \"%s\" for %g secs\n",
			script->sprite->GetName().c_str(),
			mstr, secs);

		if (len)
			script->sprite->SetMessage(std::string(mstr, len), MESSAGE_STATE_SAY);
		else
			script->sprite->ClearMessage();

		vm->Sleep(secs);
	}

	virtual void Visit(Say *node)
	{
		node->e->Accept(this); // message

		Value &message = vm->StackAt(0);

		int64_t len;
		const char *mstr = vm->ToString(message, &len);

		vm->Pop();

		printf("%s saying \"%s\"\n",
			script->sprite->GetName().c_str(),
			mstr);

		if (len)
			script->sprite->SetMessage(std::string(mstr, len), MESSAGE_STATE_SAY);
		else
			script->sprite->ClearMessage();
	}

	virtual void Visit(ThinkForSecs *node)
	{
		node->e1->Accept(this); // message
		node->e2->Accept(this); // duration

		Value &message = vm->StackAt(1);
		Value &duration = vm->StackAt(0);

		int64_t len;
		const char *mstr = vm->ToString(message, &len);
		double secs = vm->ToReal(duration);

		vm->Pop();
		vm->Pop();

		printf("%s thinking \"%s\" for %g secs\n",
			script->sprite->GetName().c_str(),
			mstr, secs);

		if (len)
			script->sprite->SetMessage(mstr, MESSAGE_STATE_THINK);
		else
			script->sprite->ClearMessage();

		vm->Sleep(secs);
	}

	virtual void Visit(Think *node)
	{
		node->e->Accept(this); // message

		Value &message = vm->StackAt(0);

		int64_t len;
		const char *mstr = vm->ToString(message, &len);

		vm->Pop();

		printf("%s thinking \"%s\"\n",
			script->sprite->GetName().c_str(),
			mstr);

		if (len)
			script->sprite->SetMessage(mstr, MESSAGE_STATE_THINK);
		else
			script->sprite->ClearMessage();
	}

	virtual void Visit(SwitchCostume *node)
	{
		node->e->Accept(this);

		Value &costume = vm->StackAt(0);

		switch (costume.type)
		{
		case ValueType_Integer:
			script->sprite->SetCostume(costume.u.integer);
			break;
		case ValueType_Real:
			script->sprite->SetCostume(static_cast<int64_t>(costume.u.real));
			break;
		case ValueType_String:
		case ValueType_BasicString:
		case ValueType_ConstString:
			script->sprite->SetCostume(vm->ToString(costume));
			break;
		default:
			break; // do nothing
		}

		vm->Pop();
	}

	virtual void Visit(NextCostume *node)
	{
		Sprite *s = script->sprite;
		s->SetCostume(s->GetCostume() + 1);
	}

	virtual void Visit(SwitchBackdrop *node)
	{
		node->e->Accept(this);

		// TODO: implement

		vm->Pop();
	}

	virtual void Visit(SwitchBackdropAndWait *node)
	{
		node->Accept(this);

		// TODO: implement

		vm->Pop();
	}

	virtual void Visit(NextBackdrop *node)
	{
		// TODO: implement
	}

	virtual void Visit(ChangeSize *node)
	{
		node->e->Accept(this);
		Sprite *s = script->sprite;
		s->SetSize(s->GetSize() + vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(SetSize *node)
	{
		node->e->Accept(this);
		script->sprite->SetSize(vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(ChangeGraphicEffect *node)
	{
		node->e->Accept(this);
		double val = vm->ToReal(vm->StackAt(0));
		vm->Pop();

		Sprite *s = script->sprite;
		switch (node->effect)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case GraphicEffect_Color:
			s->SetColorEffect(s->GetColorEffect() + val);
			break;
		case GraphicEffect_Fisheye:
			s->SetFisheyeEffect(s->GetFisheyeEffect() + val);
			break;
		case GraphicEffect_Whirl:
			s->SetWhirlEffect(s->GetWhirlEffect() + val);
			break;
		case GraphicEffect_Pixelate:
			s->SetPixelateEffect(s->GetPixelateEffect() + val);
			break;
		case GraphicEffect_Mosaic:
			s->SetMosaicEffect(s->GetMosaicEffect() + val);
			break;
		case GraphicEffect_Brightness:
			s->SetBrightnessEffect(s->GetBrightnessEffect() + val);
			break;
		case GraphicEffect_Ghost:
			s->SetGhostEffect(s->GetGhostEffect() + val);
			break;
		}
	}

	virtual void Visit(SetGraphicEffect *node)
	{
		node->e->Accept(this);
		double val = vm->ToReal(vm->StackAt(0));
		vm->Pop();

		Sprite *s = script->sprite;
		switch (node->effect)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case GraphicEffect_Color:
			s->SetColorEffect(val);
			break;
		case GraphicEffect_Fisheye:
			s->SetFisheyeEffect(val);
			break;
		case GraphicEffect_Whirl:
			s->SetWhirlEffect(val);
			break;
		case GraphicEffect_Pixelate:
			s->SetPixelateEffect(val);
			break;
		case GraphicEffect_Mosaic:
			s->SetMosaicEffect(val);
			break;
		case GraphicEffect_Brightness:
			s->SetBrightnessEffect(val);
			break;
		case GraphicEffect_Ghost:
			s->SetGhostEffect(val);
			break;
		}
	}

	virtual void Visit(ClearGraphicEffects *node)
	{
		Sprite *s = script->sprite;
		s->SetColorEffect(0);
		s->SetFisheyeEffect(0);
		s->SetWhirlEffect(0);
		s->SetPixelateEffect(0);
		s->SetMosaicEffect(0);
		s->SetBrightnessEffect(0);
		s->SetGhostEffect(0);
	}

	virtual void Visit(ShowSprite *node)
	{
		script->sprite->SetShown(true);
	}

	virtual void Visit(HideSprite *node)
	{
		script->sprite->SetShown(false);
	}

	virtual void Visit(GotoLayer *node)
	{
		switch (node->layer)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case LayerType_Front:
			script->sprite->SetLayer(1);
			break;
		case LayerType_Back:
			script->sprite->SetLayer(-1);
			break;
		}
	}

	virtual void Visit(MoveLayer *node)
	{
		node->e->Accept(this);
		int64_t amount = vm->ToInteger(vm->StackAt(0));
		vm->Pop();

		switch (node->direction)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case LayerDir_Forward:
			script->sprite->MoveLayer(amount);
			break;
		case LayerDir_Backward:
			script->sprite->MoveLayer(-amount);
			break;
		}
	}

	virtual void Visit(PlaySoundUntilDone *node) {}
	virtual void Visit(StartSound *node) {}

	virtual void Visit(StopAllSounds *node) {}
	virtual void Visit(ChangeSoundEffect *node) {}
	virtual void Visit(SetSoundEffect *node) {}
	virtual void Visit(ClearSoundEffects *node) {}

	virtual void Visit(ChangeVolume *node)
	{
		node->e->Accept(this);
		Sprite *s = script->sprite;
		s->SetVolume(s->GetVolume() + vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(SetVolume *node)
	{
		node->e->Accept(this);
		script->sprite->SetVolume(vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	/* these are handled by the VM */
	virtual void Visit(OnFlagClicked *node) {}
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
		node->e->Accept(this);
		vm->Sleep(vm->ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(Repeat *node)
	{
		node->e->Accept(this);
		int64_t count = vm->ToInteger(vm->StackAt(0));
		vm->Pop();
		vm->PushFrame(*node->sl, count, 0);
	}

	virtual void Visit(Forever *node)
	{
		vm->PushFrame(*node->sl, 0, FRAME_EXEC_FOREVER);
	}

	virtual void Visit(If *node)
	{
		node->e->Accept(this);
		bool truth = vm->Truth(vm->StackAt(0));
		vm->Pop();

		if (truth)
			vm->PushFrame(*node->sl, 1, 0);
	}

	virtual void Visit(IfElse *node)
	{
		node->e->Accept(this);
		bool truth = vm->Truth(vm->StackAt(0));
		vm->Pop();

		if (truth)
			vm->PushFrame(*node->sl1, 1, 0);
		else
			vm->PushFrame(*node->sl2, 1, 0);
	}

	virtual void Visit(WaitUntil *node)
	{
		// NOTE: don't evaluate the expression here
		vm->WaitUntil(*node->e);
	}

	virtual void Visit(RepeatUntil *node)
	{
		node->e->Accept(this);

		bool truth = vm->Truth(vm->StackAt(0));
		vm->Pop();

		if (!truth)
			vm->PushFrame(*node->sl, 1, FRAME_EXEC_AGAIN);
	}

	virtual void Visit(Stop *node)
	{
		switch (node->mode)
		{
		default:
			vm->Raise(InvalidArgument);
			break;
		case StopMode_All:
			vm->VMTerminate();
			break;
		case StopMode_ThisScript:
			vm->Terminate();
			break;
		case StopMode_OtherScriptsInSprite:
			// TODO: implement
			break;
		}
	}

	virtual void Visit(CloneStart *node)
	{
		// TODO: implement
	}

	virtual void Visit(CreateClone *node)
	{
		// TODO: implement
	}

	virtual void Visit(DeleteClone *node)
	{
		// TODO: implement
		vm->Terminate();
	}

	virtual void Visit(AskAndWait *node)
	{
		vm->AskAndWait();
	}

	virtual void Visit(SetDragMode *node)
	{
		// TODO: implement
	}

	virtual void Visit(ResetTimer *node)
	{
		vm->ResetTimer();
	}

	virtual void Visit(SetVariable *node)
	{
		// TODO: replace with id
		Value &var = vm->FindVariable(node->name);
		node->e->Accept(this);
		vm->Assign(var, vm->StackAt(0));
		vm->Pop();
	}

	virtual void Visit(ChangeVariable *node)
	{
		// TODO: replace with id
		Value &var = vm->FindVariable(node->name);
		node->e->Accept(this);
		double lhs = vm->ToReal(var);
		double rhs = vm->ToReal(vm->StackAt(0));
		vm->Pop();
		vm->SetReal(var, lhs + rhs);
	}

	virtual void Visit(ShowVariable *node) {}
	virtual void Visit(HideVariable *node) {}

	virtual void Visit(AppendToList *node) {}
	virtual void Visit(DeleteFromList *node) {}
	virtual void Visit(DeleteAllList *node) {}
	virtual void Visit(InsertInList *node) {}
	virtual void Visit(ReplaceInList *node) {}
	virtual void Visit(ShowList *node) {}
	virtual void Visit(HideList *node) {}

	virtual void Visit(ProcProto *node) {}
	virtual void Visit(DefineProc *node) {}
	virtual void Visit(Call *node) {}

	//
	/////////////////////////////////////////////////////////////////
	// Reporters
	//

	virtual void Visit(GotoReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(GlideReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(PointTowardsReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(CostumeReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(BackdropReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(SoundReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(BroadcastReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(CloneReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(TouchingReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(DistanceReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(KeyReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(PropertyOfReporter *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(ArgReporterStringNumber *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(ArgReporterBoolean *node)
	{
		vm->SetConstString(vm->Push(), &node->value);
	}

	VirtualMachine *vm = nullptr;
	Script *script = nullptr;
};

static std::string trim(const std::string &str, const std::string &ws = " \t\n\r")
{
	size_t start = str.find_first_not_of(ws);
	if (start == std::string::npos)
		return "";

	size_t end = str.find_last_not_of(ws);
	return str.substr(start, end - start + 1);
}

int VirtualMachine::Load(Program *prog, const std::string &name, Loader *loader)
{
	if (_prog || !prog)
		return -1;

	std::vector<AutoRelease<SpriteDef>> &defs = prog->sprites->sprites;
	size_t nSprites = defs.size();

	if (nSprites == 0)
	{
		// no sprites
		return -1;
	}

	_loader = loader;

	_sprites = new Sprite[nSprites];
	_spritesEnd = _sprites + nSprites;
	
	bool foundStage = false;
	intptr_t nextSpriteId = 1;
	for (AutoRelease<SpriteDef> &def : defs)
	{
		auto it = _spriteNames.find(def->name);
		if (it != _spriteNames.end())
		{
			// duplicate sprite name
			Cleanup();
			return -1;
		}

		Sprite *sprite = _sprites + nextSpriteId - 1;
		_spriteNames[def->name] = nextSpriteId++;

		sprite->Init(*def);

		if (sprite->IsStage())
		{
			if (foundStage)
			{
				// multiple stages
				Cleanup();
				return -1;
			}

			foundStage = true;
		}

		for (AutoRelease<VariableDef> &vdef : def->variables->variables)
		{
			auto it = _variables.find(vdef->name);
			if (it != _variables.end())
			{
				// duplicate variable name
				Cleanup();
				return -1;
			}

			Value &v = _variables[vdef->name] = { 0 };
			SetParsedString(v, vdef->value->value);
		}

		for (AutoRelease<StatementList> &sl : def->scripts->sll)
		{
			Script script = { 0 };

			script.vm = this;

			script.state = EMBRYO;
			script.sprite = sprite;
			script.entry = *sl;

			script.fiber = nullptr;

			script.sleepUntil = 0.0;
			script.waitExpr = nullptr;

			script.stack = (Value *)malloc(sizeof(Value) * STACK_SIZE);
			if (!script.stack)
			{
				Cleanup();
				return -1;
			}

			// fill stack with garbage
			memset(script.stack, 0xab, sizeof(Value) * STACK_SIZE);

			script.sp = script.stack + STACK_SIZE;

			_scripts.push_back(script);
		}
	}

	if (!foundStage)
	{
		// no stage
		Cleanup();
		return -1;
	}

	size_t len;
	len = ls_username(nullptr, 0);
	if (len != -1)
	{
		ReleaseValue(_username);
		AllocString(_username, len);
		ls_username(_username.u.string->str, len);
		_username.hash = HashString(_username.u.string->str);
	}

	_prog = Retain(prog);
	_progName = name;
	return 0;
}

int VirtualMachine::VMStart()
{
	if (_running || _panicing)
		return -1;

	_shouldStop = false;

	_thread = ls_thread_create(&ThreadProc, this);
	if (!_thread)
		return -1;

	return 0;
}

void VirtualMachine::VMTerminate()
{
	_shouldStop = true;
}

int VirtualMachine::VMWait(unsigned long ms)
{
	if (!_thread)
		return 0;
	return ls_timedwait(_thread, ms);
}

void VirtualMachine::VMSuspend()
{
	if (!_suspend)
	{
		_suspend = true;
		_suspendStart = ls_time64();

		SDL_SetWindowTitle(_renderer->GetWindow(), "Scratch 3 [Suspended]");
	}
}

void VirtualMachine::VMResume()
{
	if (_suspend)
	{
		_suspend = false;

		double suspendTime = ls_time64() - _suspendStart;

		// adjust timers to account for suspension
		_time += suspendTime;
		_timerStart += suspendTime;
		_nextExecution += suspendTime;

		SDL_SetWindowTitle(_renderer->GetWindow(), "Scratch 3");
	}
}

void VirtualMachine::SendFlagClicked()
{
	for (Script &script : _scripts)
	{
		AutoRelease<Statement> &s = script.entry->sl[0];
		OnFlagClicked *fc = s->As<OnFlagClicked>();
		if (fc)
		{
			script.state = RUNNABLE;
			script.sleepUntil = 0.0;
			script.waitExpr = nullptr;

			script.frames[0].sl = script.entry;
			script.frames[0].pc = 1;
			script.frames[0].count = 0;
			script.frames[0].flags = 0;
			script.fp = 0;
		}
	}
}

void VirtualMachine::Send(const std::string &message)
{

}

void VirtualMachine::SendAndWait(const std::string &message)
{

}

void VirtualMachine::SendKeyPressed(int scancode)
{

}

void VirtualMachine::SendSpriteClicked(Sprite *sprite)
{

}

void VirtualMachine::Sleep(double seconds)
{
	if (_current == nullptr)
		Panic();

	_current->sleepUntil = _time + seconds;
	_current->state = WAITING;
	Sched();
}

void VirtualMachine::WaitUntil(Expression *expr)
{
	if (_current == nullptr)
		Panic();

	_current->waitExpr = expr;
	_current->state = WAITING;
	Sched();
}

void VirtualMachine::AskAndWait()
{
	if (_current == nullptr)
		Panic();

	// TODO: implement
}

void VirtualMachine::Terminate()
{
	if (_current == nullptr)
		Panic();

	_current->state = TERMINATED;
	Sched();
}

static void DumpScript(Script *script)
{
	printf("Script %p\n", script);

	if (script->state >= EMBRYO && script->state <= TERMINATED)
		printf("    state = %s\n", States[script->state]);
	else
		printf("    state = Unknown\n");

	printf("    sprite = %s\n", script->sprite ? script->sprite->GetName().c_str() : "(null)");

	printf("    sleepUntil = %g\n", script->sleepUntil);
	printf("    waitExpr = %p (%s)\n", script->waitExpr, script->waitExpr ? AstTypeString(script->waitExpr->GetType()) : "null");
	printf("    waitInput = %d\n", (int)script->waitInput);
	printf("    stack = %p\n", script->stack);
	printf("    sp = %p\n", script->sp);
	printf("    fp = %lld\n", script->fp);

	size_t n = script->fp+1 < SCRIPT_DEPTH ? script->fp+1 : SCRIPT_DEPTH;
	for (size_t i = 0; i < n; i++)
	{
		printf("    frames[%lld]\n", i);

		const Frame &frame = script->frames[i];

		const char *astTypeString;
		if (!frame.sl)
			astTypeString = "null";
		else if (frame.pc < frame.sl->sl.size())
			astTypeString = AstTypeString(frame.sl->sl[frame.pc]->GetType());
		else
			astTypeString = "???";

		printf("        sl = %p\n", frame.sl);
		printf("        pc = %lld (%s)\n", frame.pc, astTypeString);
		printf("        count = %lld\n", frame.count);
		printf("        flags = %x\n", frame.flags);
	}}

void LS_NORETURN VirtualMachine::Raise(ExceptionType type, const char *message)
{
	if (_current == nullptr)
		Panic(); // thrown outside of a script

	_exceptionType = type;
	_exceptionMessage = message;

	Terminate();
}

void LS_NORETURN VirtualMachine::Panic(const char *message)
{
	_panicing = true;
	_panicMessage = message;

	if (_current)
	{
		ls_fiber_sched();
		abort(); // should be unreachable
	}

	longjmp(_panicJmp, 1);
}

Value &VirtualMachine::Push()
{
	assert(_current != nullptr);

	if (_current->sp < _current->sp)		
		Raise(StackOverflow);
	_current->sp--;
	*_current->sp = { 0 }; // zero out the value
	return *_current->sp;
}

static void PopUnsafe(VirtualMachine *vm, Script *script)
{
	if (script->sp >= script->stack + STACK_SIZE)
		abort();
	vm->ReleaseValue(*script->sp);
	memset(script->sp, 0xab, sizeof(Value));
	script->sp++;
}

void VirtualMachine::Pop()
{
	assert(_current != nullptr);

	if (_current->sp >= _current->stack + STACK_SIZE)
		Raise(StackUnderflow);

	ReleaseValue(*_current->sp);

	// fill with garbage
	memset(_current->sp, 0xab, sizeof(Value));

	_current->sp++;
}

Value &VirtualMachine::StackAt(size_t i)
{
	if (_current->sp + i >= _current->stack + STACK_SIZE)
		Raise(StackUnderflow);

	return _current->sp[i];
}

void VirtualMachine::PushFrame(StatementList *sl, int64_t count, uint32_t flags)
{
	if (_current->fp >= SCRIPT_DEPTH - 1)
		Raise(StackOverflow);

	if (sl == nullptr)
	{
		// empty loop
		_current->frames[_current->fp].pc -= 1; // loop back to the same statement
		return;
	}

	Frame &f = _current->frames[++_current->fp];
	f.sl = sl;
	f.pc = 0;
	f.count = count;
	f.flags = flags;
}

bool VirtualMachine::Truth(const Value &val)
{
	switch (val.type)
	{
	default:
		return false;
	case ValueType_Bool:
		return val.u.boolean;
	case ValueType_String:
		return StringEquals(val.u.string->str, TRUE_STRING);
	case ValueType_BasicString:
		return StringEquals(val.u.basic_string, TRUE_STRING);
	case ValueType_ConstString:
		return StringEquals(val.u.const_string->c_str(), TRUE_STRING);
	}
}

bool VirtualMachine::Equals(const Value &lhs, const Value &rhs)
{
	switch (lhs.type)
	{
	case ValueType_Integer: {
		if (rhs.type == ValueType_Integer)
			return lhs.u.integer == rhs.u.integer;
		if (rhs.type == ValueType_Real)
			return lhs.u.integer == rhs.u.real;
		return false;
	}
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return lhs.u.real == rhs.u.real;
		if (rhs.type == ValueType_Integer)
			return lhs.u.real == rhs.u.integer;
		return false;
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return lhs.u.boolean == rhs.u.boolean;
		return false;
	case ValueType_String:
		if (lhs.hash != rhs.hash)
			return false;

		if (rhs.type == ValueType_String)
			return StringEquals(lhs.u.string->str, rhs.u.string->str);
		else if (rhs.type == ValueType_BasicString)
			return StringEquals(lhs.u.string->str, rhs.u.basic_string);
		else if (rhs.type == ValueType_ConstString)
			return StringEquals(lhs.u.string->str, rhs.u.const_string->c_str());
		return false;
	case ValueType_BasicString:
		if (lhs.hash != rhs.hash)
			return false;

		if (rhs.type == ValueType_String)
			return StringEquals(lhs.u.basic_string, rhs.u.string->str);
		else if (rhs.type == ValueType_BasicString)
			return StringEquals(lhs.u.basic_string, rhs.u.basic_string);
		else if (rhs.type == ValueType_ConstString)
			return StringEquals(lhs.u.basic_string, rhs.u.const_string->c_str());
		return false;
	case ValueType_ConstString:
		if (lhs.hash != rhs.hash)
			return false;

		if (rhs.type == ValueType_String)
			return StringEquals(lhs.u.const_string->c_str(), rhs.u.string->str);
		else if (rhs.type == ValueType_BasicString)
			return StringEquals(lhs.u.const_string->c_str(), rhs.u.basic_string);
		else if (rhs.type == ValueType_ConstString)
			return StringEquals(lhs.u.const_string->c_str(), rhs.u.const_string->c_str());
		return false;
	default:
		return false;
	}
}

Value &VirtualMachine::Assign(Value &lhs, const Value &rhs)
{
	if (&lhs == &rhs)
		return lhs;

	if (lhs.u.ref == rhs.u.ref)
		return RetainValue(lhs);

	ReleaseValue(lhs);
	return RetainValue(lhs = rhs);
}


Value &VirtualMachine::SetInteger(Value &lhs, int64_t val)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_Integer;
	lhs.u.integer = val;
	return lhs;
}

Value &VirtualMachine::SetReal(Value &lhs, double rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_Real;
	lhs.u.real = rhs;
	return lhs;
}

Value &VirtualMachine::SetBool(Value &lhs, bool rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_Bool;
	lhs.u.boolean = rhs;
	return lhs;
}

Value &VirtualMachine::SetString(Value &lhs, const std::string &rhs)
{
	ReleaseValue(lhs);

	if (rhs.size() == 0)
		return SetEmpty(lhs);

	AllocString(lhs, rhs.size());
	if (lhs.type != ValueType_String)
		return lhs;

	memcpy(lhs.u.string->str, rhs.data(), rhs.size());
	lhs.hash = HashString(lhs.u.string->str);
	return lhs;
}

Value &VirtualMachine::SetBasicString(Value &lhs, const char *rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_BasicString;
	lhs.u.basic_string = rhs;
	lhs.hash = HashString(lhs.u.basic_string);
	return lhs;
}

Value &VirtualMachine::SetConstString(Value &lhs, const std::string *rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_ConstString;
	lhs.u.const_string = rhs;
	lhs.hash = HashString(lhs.u.const_string->c_str());
	return lhs;
}

Value &VirtualMachine::SetParsedString(Value &lhs, const std::string &rhs)
{
	std::string str = trim(rhs);
	if (str.size() != 0)
	{
		char *end;

		int64_t integer = strtoll(str.c_str(), &end, 10);
		if (*end == 0)
			return SetInteger(lhs, integer);

		double real = strtod(str.c_str(), &end);
		if (*end == 0)
			return SetReal(lhs, real);

		if (str.size() == 4)
		{
			size_t i;
			for (i = 0; i < 4; i++)
				if (tolower(str[i]) != TRUE_STRING[i])
					break;

			if (i == 4)
				return SetBool(lhs, true);
		}

		if (str.size() == 5)
		{
			size_t i;
			for (i = 0; i < 5; i++)
				if (tolower(str[i]) != FALSE_STRING[i])
					break;

			if (i == 5)
				return SetBool(lhs, false);
		}
	}

	return SetString(lhs, rhs);
}

Value &VirtualMachine::SetEmpty(Value &lhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_None;
	return lhs;
}

const char *VirtualMachine::ToString(const Value &val, int64_t *len)
{
	static LS_THREADLOCAL char buf[64];
	int cch;

	switch (val.type)
	{
	default:
	case ValueType_None:
		if (len) *len = 0;
		return "";
	case ValueType_Integer:
		cch = snprintf(buf, sizeof(buf), "%" PRId64, val.u.integer);
		if (len) *len = cch;
		return buf;
	case ValueType_Real:
		cch = snprintf(buf, sizeof(buf), "%.8g", val.u.real);
		if (len) *len = cch;
		return buf;
	case ValueType_Bool:
		return val.u.boolean ? TRUE_STRING : FALSE_STRING;
	case ValueType_String:
		if (len) *len = val.u.string->len;
		return val.u.string->str;
	case ValueType_BasicString:
		if (len) *len = strlen(val.u.basic_string);
		return val.u.basic_string;
	case ValueType_ConstString:
		if (len) *len = val.u.const_string->size();
		return val.u.const_string->c_str();
	}
}

void VirtualMachine::CvtString(Value &v)
{
	switch (v.type)
	{
	default:
	case ValueType_String:
		break;
	case ValueType_Integer: {
		int cch;
		cch = snprintf(nullptr, 0, "%" PRId64, v.u.integer);
		if (cch < 0)
			Raise(OutOfMemory);

		AllocString(v, cch);
		if (v.type != ValueType_String)
			break; // exception

		snprintf(v.u.string->str, cch + 1, "%" PRId64, v.u.integer);

		break;
	}
	case ValueType_Real: {
		int cch;
		cch = snprintf(nullptr, 0, "%.8g", v.u.real);
		if (cch < 0)
			Raise(OutOfMemory);

		AllocString(v, cch);
		snprintf(v.u.string->str, cch + 1, "%.8g", v.u.real);

		break;
	}
	case ValueType_Bool:
		SetBasicString(v, v.u.boolean ? TRUE_STRING : FALSE_STRING);
		break;
	}
}

int64_t VirtualMachine::ToInteger(const Value &val)
{
	switch (val.type)
	{
	default:
	case ValueType_None:
	case ValueType_Bool:
	case ValueType_String:
		return 0;
	case ValueType_Real:
		return static_cast<int64_t>(round(val.u.real));
	case ValueType_Integer:
		return val.u.integer;
	}
}

double VirtualMachine::ToReal(const Value &val)
{
	switch (val.type)
	{
	default:
	case ValueType_None:
	case ValueType_Bool:
	case ValueType_String:
		return 0.0;
	case ValueType_Real:
		return val.u.real;
	case ValueType_Integer:
		return static_cast<double>(val.u.integer);
	}
}

Value &VirtualMachine::AllocString(Value &v, size_t len)
{
	ReleaseValue(v);

	_allocations++;

	v.type = ValueType_String;
	v.u.string = (String *)calloc(1, offsetof(String, str) + len + 1);
	if (!v.u.string)
		Raise(OutOfMemory);

	v.u.string->len = len;
	v.u.ref->count = 1;

	return v;
}

Value &VirtualMachine::RetainValue(Value &val)
{
	if (val.type == ValueType_String)
	{
		assert(val.u.string->ref.count > 0);
		val.u.ref->count++;
	}
	return val;
}

void VirtualMachine::ReleaseValue(Value &val)
{
	if (val.type == ValueType_String)
	{
		assert(val.u.string->ref.count > 0);

		if (--val.u.ref->count == 0)
			FreeValue(val);
	}
}

void VirtualMachine::FreeValue(Value &val)
{
	if (val.type == ValueType_String)
	{
		assert(val.u.string->ref.count == 0);
		
		// printf("Free: `%s`\n", val.u.string->str);
				
		free(val.u.string);

		// for safety
		val.u.ref = nullptr;
	}
}

Value &VirtualMachine::FindVariable(const std::string &id)
{
	auto it = _variables.find(id);
	if (it == _variables.end())
		Raise(VariableNotFound);
	return it->second;
}

Sprite *VirtualMachine::FindSprite(const std::string &name)
{
	auto it = _spriteNames.find(name);
	return it != _spriteNames.end() ? FindSprite(it->second) : nullptr;
}

Sprite *VirtualMachine::FindSprite(intptr_t id)
{
	Sprite *s = _sprites + id - 1;
	if (s >= _sprites && s < _spritesEnd)
		return s;
	return nullptr;
}

void VirtualMachine::ResetTimer()
{
	_timerStart = _time;
}

void VirtualMachine::Glide(Sprite *sprite, double x, double y, double s)
{
	if (s <= 0.0)
	{
		sprite->SetXY(x, y);
		return;
	}

	GlideInfo &glide = *sprite->GetGlide();

	glide.x0 = sprite->GetX();
	glide.y0 = sprite->GetY();
	glide.x1 = x;
	glide.y1 = y;
	glide.start = _time;
	glide.end = _time + s;
}

void VirtualMachine::Sched()
{
	if (_current == nullptr)
		Panic();

	_current = nullptr;
	ls_fiber_sched();
}

VirtualMachine::VirtualMachine()
{
	_prog = nullptr;
	_loader = nullptr;

	_sprites = nullptr;
	_spritesEnd = nullptr;

	_renderer = nullptr;

	_answer = { 0 };
	_mouseDown = false;
	_lastDown = false;
	_mouseX = 0;
	_mouseY = 0;
	_clickX = 0;
	_clickY = 0;
	_clicked = false;
	memset(_keyStates, 0, sizeof(_keyStates));
	_keysPressed = 0;
	_loudness = 0.0;
	_timer = 0.0;
	_username = { 0 };

	_suspend = false;
	_suspendStart = 0.0;

	_timerStart = 0;

	_shouldStop = false;
	_waitCount = 0;

	_running = false;
	_activeScripts = 0;
	_waitingScripts = 0;
	_exceptionType = Exception_None;
	_exceptionMessage = nullptr;

	_panicing = false;
	_panicMessage = nullptr;
	memset(_panicJmp, 0, sizeof(_panicJmp));

	_current = nullptr;
	_time = 0;
	_lastTime = 0;
	_nextExecution = 0;
	_executionTime = 0;

	_allocations = 0;

	_thread = nullptr;
}

VirtualMachine::~VirtualMachine()
{
	Cleanup();

	ReleaseValue(_username);
	ReleaseValue(_answer);

	Release(_prog);
}

void VirtualMachine::DestroyGraphics()
{
	if (_sprites)
	{
		delete[] _sprites, _sprites = nullptr;
		_spritesEnd = nullptr;
	}

	if (_renderer)
		delete _renderer, _renderer = nullptr;
}

void VirtualMachine::PollEvents()
{
	if (!_renderer)
		return;

	SDL_Event evt;

	_lastDown = _mouseDown;

	while (SDL_PollEvent(&evt))
	{
		ImGui_ImplSDL2_ProcessEvent(&evt);

		switch (evt.type)
		{
		case SDL_QUIT:
			VMTerminate();
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				_mouseDown = true;
				_renderer->ScreenToStage(evt.button.x, evt.button.y, &_clickX, &_clickY);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				_mouseDown = false;
				_clickX = _clickY = 0;
			}
			break;
		case SDL_MOUSEMOTION:
			_renderer->ScreenToStage(evt.motion.x, evt.motion.y, &_mouseX, &_mouseY);
			break;
		case SDL_KEYDOWN:
			if (!_keyStates[evt.key.keysym.scancode])
				_keysPressed++;

			_keyStates[evt.key.keysym.scancode] = true;
			break;
		case SDL_KEYUP:
			if (_keyStates[evt.key.keysym.scancode])
				_keysPressed--;
			_keyStates[evt.key.keysym.scancode] = false;
			break;
		case SDL_WINDOWEVENT:
			if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
				_renderer->Resize();
		}
	}

	// clicks occur on the transition from up to down
	_clicked = _mouseDown && !_lastDown;
}

void VirtualMachine::Render()
{
	_renderer->BeginRender();

	int spritesVisible = 0;

	for (Sprite *s = _sprites; s < _spritesEnd; s++)
	{
		s->Update();
		spritesVisible += s->IsShown();
	}

	_renderer->Render();

	double dt = _time - _lastTime;
	double fps = 1 / dt;

	if (ImGui::Begin("Debug"))
	{
		if (ImGui::BeginTabBar("DebugTabs"))
		{
			SDL_Window *window = _renderer->GetWindow();
			int width, height;
			SDL_GL_GetDrawableSize(window, &width, &height);

			if (ImGui::BeginTabItem("System"))
			{
				struct ls_meminfo mi;
				struct ls_cpuinfo ci;

				ls_get_meminfo(&mi);
				ls_get_cpuinfo(&ci);

				const char *archString;
				switch (ci.arch)
				{
				default:
					archString = "unknown";
					break;
				case LS_ARCH_AMD64:
					archString = "x86_64";
					break;
				case LS_ARCH_ARM:
					archString = "arm";
					break;
				case LS_ARCH_ARM64:
					archString = "arm64";
					break;
				case LS_ARCH_X86:
					archString = "x86";
					break;
				case LS_ARCH_IA64:
					archString = "ia64";
					break;
				}

				ImGui::SeparatorText("Host");
				ImGui::LabelText("Name", LS_OS);
				ImGui::LabelText("Architecture", archString);
				ImGui::LabelText("Processor Count", "%d", ci.num_cores);
				ImGui::LabelText("Total Physical", "%llu MiB", mi.total / 1024 / 1024);

				ImGui::SeparatorText("Target");
				ImGui::LabelText("Compiler", LS_COMPILER);
				ImGui::LabelText("Target Architecture", LS_ARCH);
				ImGui::LabelText("Build Date", __TIMESTAMP__);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Graphics"))
			{
				int left = _renderer->GetLogicalLeft();
				int right = _renderer->GetLogicalRight();
				int top = _renderer->GetLogicalTop();
				int bottom = _renderer->GetLogicalBottom();

				ImGui::SeparatorText("Performance");
				ImGui::LabelText("Framerate", "%.2f (%d ms)", fps, (int)(dt * 1000));
				ImGui::LabelText("Resolution", "%dx%d", width, height);
				ImGui::LabelText("Viewport Size", "%dx%d", right - left, top - bottom);
				ImGui::LabelText("Visible Objects", "%d/%d", spritesVisible, (int)(_spritesEnd - _sprites));

				ImGui::SeparatorText("Device");
				ImGui::LabelText("OpenGL", "%s", glGetString(GL_VERSION));
				ImGui::LabelText("OpenGL Vendor", "%s", glGetString(GL_VENDOR));
				ImGui::LabelText("OpenGL Renderer", "%s", glGetString(GL_RENDERER));
				ImGui::LabelText("OpenGL Shading Language", "%s", glGetString(GL_SHADING_LANGUAGE_VERSION));
				ImGui::LabelText("Window Driver", "%s", SDL_GetVideoDriver(0));

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("I/O"))
			{
				ImGui::SeparatorText("Mouse");
				ImGui::LabelText("Mouse Down", "%s", _mouseDown ? "true" : "false");
				ImGui::LabelText("Mouse", "%d, %d", (int)_mouseX, (int)_mouseY);
				ImGui::LabelText("Click", "%d, %d", (int)_clickX, (int)_clickY);

				ImGui::SeparatorText("Keyboard");
				ImGui::LabelText("Keys Pressed", "%d", _keysPressed);

				std::string keys;
				for (int i = 0; i < SDL_NUM_SCANCODES; i++)
				{
					if (_keyStates[i])
					{
						if (keys.size() > 0)
							keys += ", ";
						keys += SDL_GetScancodeName((SDL_Scancode)i);
					}
				}
				ImGui::LabelText("Keys", "%s", keys.c_str());

				struct ls_timespec ts;
				ls_get_time(&ts);

				ImGui::SeparatorText("Timers");
				ImGui::LabelText("Timer", "%.2f", _timer);
				ImGui::LabelText("Year", "%d", ts.year);
				ImGui::LabelText("Month", "%d", ts.month);
				ImGui::LabelText("Date", "%d", ts.day);
				ImGui::LabelText("Day of Week", "%d", 4); // TODO: implement
				ImGui::LabelText("Hour", "%d", ts.hour);
				ImGui::LabelText("Minute", "%d", ts.minute);
				ImGui::LabelText("Second", "%d", ts.second);
				ImGui::LabelText("Days Since 2000", "%d", 0); // TODO: implement

				ImGui::SeparatorText("Sound");
				ImGui::LabelText("Loudness", "%.2f", _loudness);

				ImGui::SeparatorText("Other");
				ImGui::LabelText("Username", "%s", ToString(_username));
				ImGui::LabelText("Answer", "%s", ToString(_answer));

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Virtual Machine"))
			{
				ImGui::SeparatorText("Information");
				ImGui::LabelText("Program Name", "%s", _progName.c_str());

				ImGui::SeparatorText("Performance");

				if (FRAMERATE == 0)
					ImGui::LabelText("Clock Speed", "(unlimited)");
				else
					ImGui::LabelText("Clock Speed", "%u Hz", (unsigned)FRAMERATE);

				ImGui::LabelText("Interpreter Time", "%.2f ms", (_executionTime * 1000));
				ImGui::LabelText("Utilization", "%.2f%%", _executionTime * FRAMERATE * 100.0);
				ImGui::LabelText("Allocations", "%d", _allocations);

				ImGui::SeparatorText("Scheduler");
				ImGui::LabelText("Suspended", "%s", _suspend ? "true" : "false");
				ImGui::LabelText("Script Count", "%d", (int)_scripts.size());
				ImGui::LabelText("Running", "%d", _activeScripts);
				ImGui::LabelText("Waiting", "%d", _waitingScripts);

				ImGui::SeparatorText("Globals");
				for (auto &p : _variables)
				{
					Value &v = p.second;
					const char *name = p.first.c_str();

					switch (v.type)
					{
					default:
						ImGui::LabelText(name, "<unknown>");
						break;
					case ValueType_None:
						ImGui::LabelText(name, "None");
						break;
					case ValueType_Integer:
						ImGui::LabelText(name, "%llu", v.u.integer);
						break;
					case ValueType_Real:
						ImGui::LabelText(name, "%g", v.u.real);
						break;
					case ValueType_Bool:
						ImGui::LabelText(name, "%s", v.u.boolean ? "true" : "false");
						break;
					case ValueType_String:
						ImGui::LabelText(name, "\"%s\"", v.u.string->str);
						break;
					case ValueType_BasicString:
						ImGui::LabelText(name, "\"%s\"", v.u.basic_string);
						break;
					case ValueType_ConstString:
						ImGui::LabelText(name, "\"%s\"", v.u.const_string->c_str());
						break;
					}
				}

				ImGui::SeparatorText("Control");

				if (ImGui::Button("Send Flag Clicked"))
					SendFlagClicked();

				if (_suspend)
				{
					if (ImGui::Button("Resume"))
						VMResume();
				}
				else
				{
					if (ImGui::Button("Suspend"))
						VMSuspend();
				}

				if (ImGui::Button("Terminate"))
					VMTerminate();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Sprites"))
			{
				ImGui::SeparatorText("Information");
				ImGui::LabelText("Sprite Count", "%d", (int)(_spritesEnd - _sprites - 1));

				ImGui::SeparatorText("Sprites");
				for (Sprite *s = _sprites; s < _spritesEnd; s++)
				{
					if (ImGui::CollapsingHeader(s->GetName().c_str()))
						s->DebugUI();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Scripts"))
			{
				static bool onlyRunning = true;
				ImGui::Checkbox("Only Running", &onlyRunning);

				for (Script &script : _scripts)
				{
					bool running = script.state == RUNNABLE || script.state == WAITING;
					if (onlyRunning && !running)
						continue;

					char name[128];
					snprintf(name, sizeof(name), "%p (%s)", &script, script.sprite->GetName().c_str());

					if (ImGui::CollapsingHeader(name))
					{
						ImGui::LabelText("State", States[script.state]);
						ImGui::LabelText("Sprite", "%s", script.sprite->GetName().c_str());
						ImGui::LabelText("Root", "%s", script.entry->sl[0]->ToString().c_str());
						ImGui::LabelText("Wakeup", "%.2f", script.sleepUntil);

						if (script.waitExpr)
							ImGui::LabelText("Wait", "%s", script.waitExpr->ToString().c_str());
						else
							ImGui::LabelText("Wait", "(none)");

						ImGui::LabelText("Wait Input", script.waitInput ? "true" : "false");

						ImGui::LabelText("Frame", "%d", (int)script.fp);

						for (uintptr_t fp = 0; fp <= script.fp; fp++)
						{
							Frame &f = script.frames[fp];
							if (!f.sl)
								continue;

							if (f.pc == 0)
								ImGui::Text("[%d] (start)", (int)fp);
							else
							{
								Statement *stmt = *f.sl->sl[f.pc-1];
								ImGui::Text("[%d] %s", (int)fp, stmt->ToString().c_str());
							}
						}
					}
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	const ImVec2 padding(5, 5);
	const ImU32 textColor = IM_COL32(255, 255, 255, 255);
	const ImU32 hiddenColor = IM_COL32(128, 128, 128, 255);
	const ImU32 backColor = IM_COL32(0, 0, 0, 128);

	ImDrawList *drawList = ImGui::GetBackgroundDrawList();
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
	{
		int x, y;
		_renderer->StageToScreen(s->GetX(), s->GetY(), &x, &y);

		ImVec2 position(x, y);

		const char *text = s->GetName().c_str();
		ImVec2 textSize = ImGui::CalcTextSize(text);
		
		ImVec2 topLeft(position.x - padding.x, position.y - padding.y);
		ImVec2 botRight(position.x + textSize.x + padding.x, position.y + textSize.y + padding.y);

		drawList->AddRectFilled(topLeft, botRight, backColor);
		drawList->AddText(position, s->IsShown() ? textColor : hiddenColor, s->GetName().c_str());
	}

	_renderer->EndRender();
}

void VirtualMachine::Cleanup()
{
	if (_thread && ls_thread_id_self() != ls_thread_id(_thread))
	{
		_shouldStop = true;
		ls_wait(_thread);
		ls_close(_thread), _thread = nullptr;
	}

	_listeners.clear();
	_keyListeners.clear();
	_clickListeners.clear();

	for (auto &p : _variables)
		ReleaseValue(p.second);
	_variables.clear();

	for (Script &script : _scripts)
	{
		assert(script.fiber == nullptr);
		free(script.stack);
	}
	_scripts.clear();

	DestroyGraphics();

	_loader = nullptr;
}

static int ScriptMain(void *up)
{
	Script &script = *(Script *)up;
	VirtualMachine &vm = *script.vm;

	Executor executor;
	executor.script = &script;
	executor.vm = &vm;

	for (;;)
	{
		// Check if we should wait
		if (script.waitExpr)
		{
			script.waitExpr->Accept(&executor);
			bool truth = vm.Truth(vm.StackAt(0));
			vm.Pop();

			if (!truth)
			{
				vm.Sched();
				continue;
			}

			script.waitExpr = nullptr;
		}

		// Pop frames until we find one that we can execute
		Frame *f = &script.frames[script.fp];
		while (f->pc >= f->sl->sl.size())
		{
			// top of the stack, script is done
			if (script.fp == 0)
			{
				script.state = TERMINATED;
				vm.Sched();
				break;
			}

			// this script should execute forever, so reset the program counter
			if (f->flags & FRAME_EXEC_FOREVER)
			{
				f->pc = 0;
				break;
			}

			// decrement the repeat count, if it's zero, pop the frame
			f->count--;
			if (f->count > 0)
			{
				f->pc = 0;
				break;
			}

			bool again = (f->flags & FRAME_EXEC_AGAIN) != 0;

			// Pop the frame
			script.fp--;
			f = &script.frames[script.fp];

			// Counteracts the increment from the last iteration
			if (again)
				f->pc--;
		}

		// Execute the script
		if (f->pc < f->sl->sl.size())
		{
			// Run the statement
			Statement *stmt = *f->sl->sl[f->pc];
			stmt->Accept(&executor);

			// Check stack
			if (script.sp != script.stack + STACK_SIZE)
				vm.Raise(VMError, "Nonempty stack");

			// Check frame pointer
			if (script.fp >= SCRIPT_DEPTH)
				vm.Raise(VMError, "Invalid frame pointer");

			f->pc++;

			vm.Sched();
		}
		else
			vm.Raise(VMError);
	}

	return 0;
}

void VirtualMachine::ShutdownThread()
{
	_activeScripts = 0;
	_waitingScripts = 0;
	_running = false;

	DestroyGraphics(); // clean up graphics resources

	// clean up fibers
	for (Script &s : _scripts)
		ls_close(s.fiber), s.fiber = nullptr;
	ls_convert_to_thread();
}

void VirtualMachine::Scheduler()
{
	int activeScripts = 0, waitingScripts = 0;

	for (Script &script : _scripts)
	{
		if (!script.fiber)
			continue; // cannot be scheduled

		_current = &script;

		if (script.state == WAITING)
		{
			if (script.sleepUntil <= _time)
			{
				// wake up
				script.state = RUNNABLE;
			}
		}

		if (script.state != RUNNABLE)
		{
			if (script.state == WAITING)
				waitingScripts++;
			continue;
		}

		activeScripts++;

		// schedule the script
		ls_fiber_switch(script.fiber);

		assert(_current == nullptr);

		if (_exceptionType != Exception_None)
		{
			// script raised an exception
			printf("<EXCEPTION> %s\n", _exceptionMessage);
			printf("Exception information:\n");
			DumpScript(_current);

			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", _exceptionMessage, _renderer->GetWindow());
			
			_activeScripts = 0;
			_waitingScripts = 0;
			return;
		}

		if (script.state == TERMINATED)
		{
			// script was just terminated
			script.sleepUntil = 0.0;
			script.waitExpr = nullptr;
			script.waitInput = false;

			// clean up the stack
			while (script.sp < script.stack + STACK_SIZE)
				PopUnsafe(this, &script);

			memset(script.frames, 0, sizeof(script.frames));
			script.fp = 0;

			script.state = EMBRYO;
		}
	}

	_activeScripts = activeScripts;
	_waitingScripts = waitingScripts;
}

void VirtualMachine::Main()
{
#if FRAMERATE != 0
	constexpr double kMinExecutionTime = 1.0 / FRAMERATE;
#else
	constexpr double kMinExecutionTime = 0.0;
#endif

	memset(_panicJmp, 0, sizeof(_panicJmp));
	int rc = setjmp(_panicJmp);
	if (rc == 1)
	{
		assert(_panicing);

		// panic
		printf("<PANIC> %s\n", _panicMessage);
		ShutdownThread();
		return;
	}

	if (ls_convert_to_fiber(NULL) != 0)
		Panic("Failed to convert to fiber");

	for (Script &script : _scripts)
	{
		script.fiber = ls_fiber_create(ScriptMain, &script);
		if (!script.fiber)
			Panic("Failed to create fiber");
	}

	_renderer = new GLRenderer(_spritesEnd - _sprites - 1); // exclude the stage
	if (_renderer->HasError())
		Panic("Failed to initialize graphics");

	// Initialize graphics resources
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
		s->Load(_loader, _renderer);

	SDL_Window *window = _renderer->GetWindow();
	SDL_SetWindowTitle(window, "Scratch 3");

	_time = ls_time64();
	_timerStart = _time;

	_running = true;

	_nextExecution = _time;
	
	SendFlagClicked();

	for (;;)
	{
		_lastTime = _time;
		_time = ls_time64();

		PollEvents();

		if (_shouldStop)
			break;

		if (!_suspend && _exceptionType == Exception_None)
		{
			_timer = _time - _timerStart;

			if (_time >= _nextExecution)
			{
				double start = ls_time64();

				_nextExecution = _time + kMinExecutionTime;
				Scheduler();

				_executionTime = ls_time64() - start;
			}
		}

		Render();

		_allocations = 0;
	}

	ShutdownThread();
}

int VirtualMachine::ThreadProc(void *data)
{
	VirtualMachine *vm = (VirtualMachine *)data;
	vm->Main();
	return 0;
}
