#include "vm.hpp"

#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <algorithm>

#include <imgui_impl_sdl2.h>

#include "../resource.hpp"
#include "../render/renderer.hpp"

#include "sprite.hpp"
#include "io.hpp"
#include "debug.hpp"

#define DEG2RAD (0.017453292519943295769236907684886)
#define RAD2DEG (57.295779513082320876798154814105)

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

class Executor : public Visitor
{
public:

	//
	/////////////////////////////////////////////////////////////////
	// Expressions
	//

	virtual void Visit(Constexpr *node)
	{
		SetParsedString(vm->Push(), node->value);
	}

	virtual void Visit(XPos *node)
	{
		SetReal(vm->Push(), script->sprite->GetX());
	}

	virtual void Visit(YPos *node)
	{
		SetReal(vm->Push(), script->sprite->GetY());
	}

	virtual void Visit(Direction *node)
	{
		SetReal(vm->Push(), script->sprite->GetDirection());
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
			SetInteger(val, script->sprite->GetCostume());
			break;
		case PropGetType_Name:
			// TODO: implement
			SetBasicString(val, "costume1"); // always costume1
			break;
		}
	}

	virtual void Visit(CurrentBackdrop *node) {}

	virtual void Visit(Size *node)
	{
		SetReal(vm->Push(), script->sprite->GetSize());
	}

	virtual void Visit(Volume *node)
	{
		SetReal(vm->Push(), script->sprite->GetVolume());
	}

	virtual void Visit(Touching *node)
	{
		// TODO: implement
		node->e->Accept(this);
		int64_t len;
		const char *name = ToString(vm->StackAt(0), &len);
		vm->Pop();

		if (!strcmp(name, "_mouse_"))
		{
			auto &io = vm->GetIO();
			SetBool(vm->Push(), script->sprite->TouchingPoint(Vector2(io.GetMouseX(), io.GetMouseY())));
			return;
		}
			
		Sprite *sprite = vm->FindSprite(std::string(name, len));
		if (!sprite)
		{
			SetBool(vm->Push(), false);
			return;
		}

		SetBool(vm->Push(), script->sprite->TouchingSprite(sprite));
	}

	virtual void Visit(TouchingColor *node)
	{
		// TODO: implement
		node->e->Accept(this);
		int64_t color = ToInteger(vm->StackAt(0));
		vm->Pop();

		SetBool(vm->Push(), script->sprite->TouchingColor(color));
	}

	virtual void Visit(ColorTouching *node)
	{
		// TODO: implement
		SetBool(vm->Push(), false);
	}

	virtual void Visit(DistanceTo *node)
	{
		node->e->Accept(this);
		int64_t len;
		const char *name = ToString(vm->StackAt(0), &len);
		vm->Pop();

		Sprite *s = vm->FindSprite(std::string(name, len));
		if (!s)
		{
			SetReal(vm->Push(), -1.0);
			return;
		}

		double dx = s->GetX() - script->sprite->GetX();
		double dy = s->GetY() - script->sprite->GetY();

		SetReal(vm->Push(), sqrt(dx * dx + dy * dy));
	}

	virtual void Visit(Answer *node)
	{
		auto &io = vm->GetIO();
		Assign(vm->Push(), io.GetAnswer());
	}

	virtual void Visit(KeyPressed *node)
	{
		node->e->Accept(this);
		int64_t len;
		const char *key = ToString(vm->StackAt(0), &len);
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
				SetBool(vm->Push(), false);
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
			SetBool(vm->Push(), false);
			return;
		}

		SetBool(vm->Push(), vm->GetIO().GetKey(scancode));
	}

	virtual void Visit(MouseDown *node)
	{
		SetBool(vm->Push(), vm->GetIO().IsMouseDown());
	}

	virtual void Visit(MouseX *node)
	{
		SetInteger(vm->Push(), vm->GetIO().GetMouseX());
	}

	virtual void Visit(MouseY *node)
	{
		SetInteger(vm->Push(), vm->GetIO().GetMouseY());
	}

	virtual void Visit(Loudness *node)
	{
		SetReal(vm->Push(), vm->GetIO().GetLoudness());
	}

	virtual void Visit(TimerValue *node)
	{
		SetReal(vm->Push(), vm->GetIO().GetTimer());
	}

	virtual void Visit(PropertyOf *node)
	{
		node->e->Accept(this);
		int64_t len;
		const char *name = ToString(vm->StackAt(0), &len);
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
			SetInteger(vm->Push(), 1);
			break;
		case PropertyTarget_BackdropName:
			SetBasicString(vm->Push(), "backdrop1");
			break;
		case PropertyTarget_XPosition:
			SetReal(vm->Push(), s->GetX());
			break;
		case PropertyTarget_YPosition:
			SetReal(vm->Push(), s->GetY());
			break;
		case PropertyTarget_Direction:
			SetReal(vm->Push(), s->GetDirection());
			break;
		case PropertyTarget_CostumeNumber:
			SetInteger(vm->Push(), s->GetCostume());
			break;
		case PropertyTarget_CostumeName:
			SetConstString(vm->Push(), &s->GetCostumeName());
			break;
		case PropertyTarget_Size:
			SetReal(vm->Push(), s->GetSize());
			break;
		case PropertyTarget_Volume:
			SetReal(vm->Push(), s->GetVolume());
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
			SetInteger(val, ts.year);
			break;
		case DateFormat_Month:
			SetInteger(val, ts.month);
			break;
		case DateFormat_Date:
			SetInteger(val, ts.day);
			break;
		case DateFormat_DayOfWeek:
			// TODO: implement
			SetInteger(val, 0);
			break;
		case DateFormat_Hour:
			SetInteger(val, ts.hour);
			break;
		case DateFormat_Minute:
			SetInteger(val, ts.minute);
			break;
		case DateFormat_Second:
			SetInteger(val, ts.second);
			break;
		}
	}

	virtual void Visit(DaysSince2000 *node)
	{
		// TODO: implement
		SetReal(vm->Push(), 0.0);
	}

	virtual void Visit(Username *node)
	{
		// TODO: implement
		Assign(vm->Push(), vm->GetIO().GetUsername());
	}

	virtual void Visit(Add *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = ToReal(lhs) + ToReal(rhs);

		vm->Pop();
		vm->Pop();

		SetReal(vm->Push(), r);
	}

	virtual void Visit(Sub *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = ToReal(lhs) - ToReal(rhs);

		vm->Pop();
		vm->Pop();

		SetReal(vm->Push(), r);
	}

	virtual void Visit(Mul *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = ToReal(lhs) * ToReal(rhs);

		vm->Pop();
		vm->Pop();

		SetReal(vm->Push(), r);
	}

	virtual void Visit(Div *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		double r = ToReal(lhs) / ToReal(rhs);

		vm->Pop();
		vm->Pop();

		SetReal(vm->Push(), r);
	}

	virtual void Visit(Random *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &from = vm->StackAt(1);
		Value &to = vm->StackAt(0);

		if (from.type == ValueType_Real || to.type == ValueType_Real)
		{
			double f = ToReal(from);
			double t = ToReal(to);

			if (t < f) // swap
			{
				double temp = f;
				f = t;
				t = temp;
			}

			vm->Pop();
			vm->Pop();

			double r = f + (ls_rand_double() * (t - f));

			SetReal(vm->Push(), r);
		}
		else
		{
			int64_t f = ToInteger(from);
			int64_t t = ToInteger(to);

			if (t < f) // swap
			{
				int64_t temp = f;
				f = t;
				t = temp;
			}

			vm->Pop();
			vm->Pop();

			int64_t r = f + (ls_rand_uint64() % (t - f + 1));

			SetInteger(vm->Push(), r);
		}
	}

	virtual void Visit(Greater *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		double lhs = ToReal(vm->StackAt(1));
		double rhs = ToReal(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		SetBool(vm->Push(), lhs > rhs);
	}

	virtual void Visit(Less *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);

		double lhs = ToReal(vm->StackAt(1));
		double rhs = ToReal(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		SetBool(vm->Push(), lhs < rhs);
	}

	virtual void Visit(Equal *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
	
		bool equal = Equals(vm->StackAt(1), vm->StackAt(0));

		vm->Pop();
		vm->Pop();
		
		SetBool(vm->Push(), equal);
	}

	virtual void Visit(LogicalAnd *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		bool lhs = Truth(vm->StackAt(1));
		bool rhs = Truth(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		SetBool(vm->Push(), lhs && rhs);
	}

	virtual void Visit(LogicalOr *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);

		bool lhs = Truth(vm->StackAt(1));
		bool rhs = Truth(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		SetBool(vm->Push(), lhs || rhs);
	}

	virtual void Visit(LogicalNot *node)
	{
		node->e->Accept(this);

		bool truth = Truth(vm->StackAt(0));

		vm->Pop();

		SetBool(vm->Push(), !truth);
	}

	virtual void Visit(Concat *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);
	
		Value &lhs = vm->StackAt(1);
		Value &rhs = vm->StackAt(0);

		ConcatValue(lhs, rhs);

		vm->Pop(); // rhs
	}

	virtual void Visit(CharAt *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &string = vm->StackAt(1);
		Value &index = vm->StackAt(0);

		char c = ValueCharAt(string, ToInteger(index));

		vm->Pop();
		vm->Pop();

		SetChar(vm->Push(), c);
	}

	virtual void Visit(StringLength *node)
	{
		node->e->Accept(this);
		int64_t len = ValueLength(vm->StackAt(0));
		vm->Pop();
		SetInteger(vm->Push(), len);
	}

	virtual void Visit(StringContains *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &v1 = vm->StackAt(1);
		Value &v2 = vm->StackAt(0);

		bool found = ValueContains(v1, v2);

		vm->Pop();
		vm->Pop();

		SetBool(vm->Push(), found);
	}

	virtual void Visit(Mod *node)
{
		node->e1->Accept(this);
		node->e2->Accept(this);

		double lhs = ToReal(vm->StackAt(1));
		double rhs = ToReal(vm->StackAt(0));

		vm->Pop();
		vm->Pop();

		SetReal(vm->Push(), fmod(lhs, rhs));
	}

	virtual void Visit(Round *node)
	{
		node->e->Accept(this);
		double val = ToReal(vm->StackAt(0));
		vm->Pop();
		SetInteger(vm->Push(), static_cast<int64_t>(round(val)));
	}

	virtual void Visit(MathFunc *node)
	{
		node->e->Accept(this);
		double val = ToReal(vm->StackAt(0));
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

		SetReal(vm->Push(), r);
	}

	virtual void Visit(VariableExpr *node)
	{
		// TODO: replace with id
		Assign(vm->Push(), vm->FindVariable(node->name));
	}

	virtual void Visit(BroadcastExpr *node)
	{
		SetConstString(vm->Push(), &node->name);
	}

	virtual void Visit(ListExpr *node)
	{
		vm->FindList(node->name);
	}

	virtual void Visit(ListAccess *node)
	{
		Value &list = vm->FindList(node->name);
		node->e->Accept(this); // index
		ListGet(vm->StackAt(0), list, ToInteger(vm->StackAt(0)));
	}

	virtual void Visit(IndexOf *node)
	{
		Value &list = vm->FindList(node->name);
		node->e->Accept(this); // value
		SetInteger(vm->StackAt(0), ListIndexOf(list, vm->StackAt(0)));
	}

	virtual void Visit(ListLength *node)
	{
		Value &list = vm->FindList(node->name);
		SetInteger(vm->Push(), ListGetLength(list));
	}

	virtual void Visit(ListContains *node)
	{
		Value &list = vm->FindList(node->name);
		node->e->Accept(this);
		SetBool(vm->StackAt(0), ListContainsValue(list, vm->StackAt(0)));
	}

	//
	/////////////////////////////////////////////////////////////////
	// Statements
	//

	virtual void Visit(StatementList *node) {}

	virtual void Visit(MoveSteps *node)
	{
		node->e->Accept(this);
		double steps = ToReal(vm->StackAt(0));
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
		s->SetDirection(s->GetDirection() + ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(TurnNegDegrees *node)
	{
		node->e->Accept(this);
		Sprite *s = script->sprite;
		s->SetDirection(s->GetDirection() - ToReal(vm->StackAt(0)));
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
		s->SetXY(ToReal(vm->StackAt(1)), ToReal(vm->StackAt(0)));
		vm->Pop();
		vm->Pop();
	}

	virtual void Visit(Glide *node)
	{
		node->e1->Accept(this); // seconds
		node->e2->Accept(this); // destination

		int64_t len;
		double secs = ToReal(vm->StackAt(1));
		const char *dest = ToString(vm->StackAt(0), &len);

		double x = 0, y = 0;

		if (!strcmp(dest, "random position"))
		{
			x = ls_rand_int(-240, 240);
			y = ls_rand_int(-180, 180);
		}
		else if (!strcmp(dest, "mouse-pointer"))
		{
			x = vm->GetIO().GetMouseX();
			y = vm->GetIO().GetMouseY();
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

		double secs = ToReal(vm->StackAt(2));
		double x = ToReal(vm->StackAt(1));
		double y = ToReal(vm->StackAt(0));

		vm->Glide(script->sprite, x, y, secs);

		vm->Pop();
		vm->Pop();
		vm->Pop();
	}

	virtual void Visit(PointDir *node)
	{
		node->e->Accept(this);
		script->sprite->SetDirection(ToReal(vm->StackAt(0)));
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
		s->SetX(s->GetX() + ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(SetX *node)
	{
		node->e->Accept(this);
		script->sprite->SetX(ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(ChangeY *node)
	{
		node->e->Accept(this);
		Sprite *s = script->sprite;
		s->SetY(s->GetY() + ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(SetY *node)
	{
		node->e->Accept(this);
		script->sprite->SetY(ToReal(vm->StackAt(0)));
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
		const char *mstr = ToString(message, &len);
		double secs = ToReal(duration);

		printf("%s saying \"%s\" for %g secs\n",
			script->sprite->GetName().c_str(),
			mstr, secs);

		vm->Pop();
		vm->Pop();

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
		const char *mstr = ToString(message, &len);

		printf("%s saying \"%s\"\n",
			script->sprite->GetName().c_str(),
			mstr);

		vm->Pop();

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
		const char *mstr = ToString(message, &len);
		double secs = ToReal(duration);

		printf("%s thinking \"%s\" for %g secs\n",
			script->sprite->GetName().c_str(),
			mstr, secs);

		vm->Pop();
		vm->Pop();

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
		const char *mstr = ToString(message, &len);

		printf("%s thinking \"%s\"\n",
			script->sprite->GetName().c_str(),
			mstr);

		vm->Pop();

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
			script->sprite->SetCostume(ToString(costume));
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

		Sprite *stage = vm->FindSprite("Stage");
		if (!stage)
			vm->Raise(VariableNotFound, "Stage");

		Value &v = vm->StackAt(0);
		switch (v.type)
		{
		default:
			break;
		case ValueType_Integer:
		case ValueType_Real:
			stage->SetCostume(ToInteger(v));
			break;
		case ValueType_Bool:
		case ValueType_String:
		case ValueType_BasicString:
		case ValueType_ConstString: {
			int64_t len;
			const char *name = ToString(v, &len);
			stage->SetCostume(std::string(name, len));
			break;
		}
		}

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
		s->SetSize(s->GetSize() + ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(SetSize *node)
	{
		node->e->Accept(this);
		script->sprite->SetSize(ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(ChangeGraphicEffect *node)
	{
		node->e->Accept(this);
		double val = ToReal(vm->StackAt(0));
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
		double val = ToReal(vm->StackAt(0));
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
		int64_t amount = ToInteger(vm->StackAt(0));
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
		s->SetVolume(s->GetVolume() + ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(SetVolume *node)
	{
		node->e->Accept(this);
		script->sprite->SetVolume(ToReal(vm->StackAt(0)));
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

	virtual void Visit(Broadcast *node)
	{
		node->e->Accept(this);

		int64_t len;
		const char *message = ToString(vm->StackAt(0), &len);

		vm->Pop();

		vm->Send(std::string(message, len));
	}

	virtual void Visit(BroadcastAndWait *node) {}

	virtual void Visit(WaitSecs *node)
	{
		node->e->Accept(this);
		vm->Sleep(ToReal(vm->StackAt(0)));
		vm->Pop();
	}

	virtual void Visit(Repeat *node)
	{
		node->e->Accept(this);
		int64_t count = ToInteger(vm->StackAt(0));
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
		bool truth = Truth(vm->StackAt(0));
		vm->Pop();

		if (truth)
			vm->PushFrame(*node->sl, 1, 0);
	}

	virtual void Visit(IfElse *node)
	{
		node->e->Accept(this);
		bool truth = Truth(vm->StackAt(0));
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

		bool truth = Truth(vm->StackAt(0));
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
			vm->Raise(NotImplemented);
			break;
		case StopMode_ThisScript:
			vm->Terminate();
			break;
		case StopMode_OtherScriptsInSprite:
			vm->Raise(NotImplemented);
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
		node->e->Accept(this);

		int64_t len;
		const char *question = ToString(vm->StackAt(0), &len);
		vm->AskAndWait(std::string(question, len));
		vm->Pop();
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
		Assign(var, vm->StackAt(0));
		vm->Pop();
	}

	virtual void Visit(ChangeVariable *node)
	{
		// TODO: replace with id
		Value &var = vm->FindVariable(node->name);
		node->e->Accept(this);
		double lhs = ToReal(var);
		double rhs = ToReal(vm->StackAt(0));
		vm->Pop();
		SetReal(var, lhs + rhs);
	}

	virtual void Visit(ShowVariable *node) {}
	virtual void Visit(HideVariable *node) {}

	virtual void Visit(AppendToList *node)
	{
		Value &list = vm->FindList(node->name);

		node->e->Accept(this);
		ListAppend(list, vm->StackAt(0));
		vm->Pop();
	}

	virtual void Visit(DeleteFromList *node)
	{
		Value &list = vm->FindList(node->name);

		node->e->Accept(this);
		ListDelete(list, vm->StackAt(0));
		vm->Pop();
	}

	virtual void Visit(DeleteAllList *node)
	{
		Value &list = vm->FindList(node->name);
		ListClear(list);
	}

	virtual void Visit(InsertInList *node)
	{
		Value &list = vm->FindList(node->name);
		node->e1->Accept(this); // value
		node->e2->Accept(this); // index
		ListInsert(list, ToInteger(vm->StackAt(0)), vm->StackAt(1));
	}

	virtual void Visit(ReplaceInList *node)
	{
		Value &list = vm->FindList(node->name);
		node->e1->Accept(this); // index
		node->e2->Accept(this); // value
		ListSet(list, ToInteger(vm->StackAt(1)), vm->StackAt(0));
	}

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
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(GlideReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(PointTowardsReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(CostumeReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(BackdropReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(SoundReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(BroadcastReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(CloneReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(TouchingReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(DistanceReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(KeyReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(PropertyOfReporter *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(ArgReporterStringNumber *node)
	{
		SetConstString(vm->Push(), &node->value);
	}

	virtual void Visit(ArgReporterBoolean *node)
	{
		SetConstString(vm->Push(), &node->value);
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

			Value &v = _variables[vdef->name];
			InitializeValue(v);
			SetParsedString(v, vdef->value->value);
		}

		for (AutoRelease<ListDef> &ldef : def->lists->lists)
		{
			auto it = _lists.find(ldef->name);
			if (it != _lists.end())
			{
				// duplicate list name
				Cleanup();
				return -1;
			}

			Value &v = _lists[ldef->name];
			InitializeValue(v);
			AllocList(v, ldef->value.size());
			if (v.type != ValueType_List)
			{
				Cleanup();
				return -1;
			}

			for (int64_t i = 0; i < ldef->value.size(); i++)
			{
				AutoRelease<Constexpr> &elem = ldef->value[i];
				SetParsedString(v.u.list->values[i], elem->value);
			}
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

		SDL_SetWindowTitle(_render->GetWindow(), "Scratch 3 [Suspended]");
	}
}

void VirtualMachine::VMResume()
{
	if (_suspend)
	{
		_suspend = false;

		double suspendTime = ls_time64() - _suspendStart;

		// adjust timers to account for suspension
		_epoch += suspendTime;

		SDL_SetWindowTitle(_render->GetWindow(), "Scratch 3");
	}
}

void VirtualMachine::SendFlagClicked()
{
	printf("Flag clicked\n");
	_flagClicked = true;
}

void VirtualMachine::Send(const std::string &message)
{
	printf("Send: %s\n", message.c_str());
	_toSend.insert(message);
}

void VirtualMachine::SendAndWait(const std::string &message)
{

}

void VirtualMachine::SendKeyPressed(int scancode)
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

void VirtualMachine::AskAndWait(const std::string &question)
{
	if (_current == nullptr)
		Panic();

	_current->askInput = true;
	_current->state = WAITING;
	_askQueue.push(std::make_pair(_current, question));
	Sched();
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
		printf("    state = %s\n", GetStateName(script->state));
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

	printf("<EXCEPTION> %s: %s\n", ExceptionString(type), message);

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
	ReleaseValue(*script->sp);
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

Value &VirtualMachine::FindVariable(const std::string &id)
{
	auto it = _variables.find(id);
	if (it == _variables.end())
		Raise(VariableNotFound);
	return it->second;
}

Value &VirtualMachine::FindList(const std::string &id)
{
	auto it = _lists.find(id);
	if (it == _lists.end())
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

	_current->state = WAITING;

	Sched();
}

void VirtualMachine::Sched()
{
	if (_current == nullptr)
		Panic();

	_current = nullptr;
	ls_fiber_sched();
}

void VirtualMachine::OnClick(int64_t x, int64_t y)
{
	Vector2 point(x, y);
	for (const int64_t *id = _render->RenderOrderEnd() - 1; id >= _render->RenderOrderBegin(); id--)
	{
		Sprite *sprite = reinterpret_cast<Sprite *>(_render->GetRenderInfo(*id)->userData);
		if (sprite->TouchingPoint(point))
		{
			// sprite was clicked
			printf("Clicked %s\n", sprite->GetName().c_str());
			for (Script *script : sprite->GetClickListeners())
				_clickQueue.push(script);
			break;
		}
	}
}

void VirtualMachine::OnKeyDown(int scancode)
{
}

VirtualMachine::VirtualMachine() :
	_io(this), _debug(this)
{
	_prog = nullptr;
	_loader = nullptr;

	_sprites = nullptr;
	_spritesEnd = nullptr;

	_flagClicked = false;

	_asker = nullptr;
	memset(_inputBuf, 0, sizeof(_inputBuf));

	_render = nullptr;

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
	_epoch = 0;
	_time = 0;

	_interpreterTime = 0;
	_deltaExecution = 0;

	_thread = nullptr;
}

VirtualMachine::~VirtualMachine()
{
	Cleanup();
}

void VirtualMachine::Render()
{
	_render->BeginRender();

	for (Sprite *s = _sprites; s < _spritesEnd; s++)
		s->Update();

	_render->Render();

	_io.RenderIO();

	const ImVec2 padding(5, 5);
	const ImU32 textColor = IM_COL32(255, 255, 255, 255);
	const ImU32 hiddenColor = IM_COL32(128, 128, 128, 255);
	const ImU32 backColor = IM_COL32(0, 0, 0, 128);

	ImDrawList *drawList = ImGui::GetBackgroundDrawList();
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
	{
		int x, y;
		_render->StageToScreen(s->GetX(), s->GetY(), &x, &y);

		ImVec2 position(x, y);

		const char *text = s->GetName().c_str();
		ImVec2 textSize = ImGui::CalcTextSize(text);
		
		ImVec2 topLeft(position.x - padding.x, position.y - padding.y);
		ImVec2 botRight(position.x + textSize.x + padding.x, position.y + textSize.y + padding.y);

		drawList->AddRectFilled(topLeft, botRight, backColor);
		drawList->AddText(position, s->IsShown() ? textColor : hiddenColor, s->GetName().c_str());
	}

	_debug.Render();

	_render->EndRender();
}

void VirtualMachine::Cleanup()
{
	if (_thread && ls_thread_id_self() != ls_thread_id(_thread))
	{
		_shouldStop = true;
		ls_wait(_thread);
		ls_close(_thread), _thread = nullptr;
	}

	_flagClicked = false;
	_toSend.clear();
	_askQueue = std::queue<std::pair<Script *, std::string>>();
	_asker = nullptr;
	_question.clear();
	memset(_inputBuf, 0, sizeof(_inputBuf));

	_messageListeners.clear();
	_keyListeners.clear();

	for (auto &p : _variables)
		ReleaseValue(p.second);
	_variables.clear();

	for (auto &p : _lists)
		ReleaseValue(p.second);
	_lists.clear();

	for (Script &script : _scripts)
	{
		assert(script.fiber == nullptr);
		free(script.stack);
	}
	_scripts.clear();

	_io.Release();

	if (_render)
		delete _render, _render = nullptr;

	_loader = nullptr;

	Release(_prog), _prog = nullptr;
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
			bool truth = Truth(vm.StackAt(0));
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
			vm.Raise(VMError, "Invalid program counter");
	}

	return 0;
}

void VirtualMachine::ShutdownThread()
{
	_activeScripts = 0;
	_waitingScripts = 0;
	_running = false;

	_io.Release();

	if (_render)
		delete _render, _render = nullptr;

	// clean up fibers
	for (Script &s : _scripts)
		ls_close(s.fiber), s.fiber = nullptr;
	ls_convert_to_thread();
}

void VirtualMachine::ResetScript(Script &script)
{
	script.sleepUntil = 0.0;
	script.waitExpr = nullptr;
	script.waitInput = false;

	// clean up the stack
	while (script.sp < script.stack + STACK_SIZE)
		PopUnsafe(this, &script);

	memset(script.frames, 0, sizeof(script.frames));

	// reset the frame pointer to the entry point
	script.frames[0].sl = script.entry;
	script.frames[0].pc = 1;
	script.frames[0].count = 0;
	script.frames[0].flags = 0;
	script.fp = 0;

	script.state = EMBRYO;
}

void VirtualMachine::StartScript(Script &script)
{
	ResetScript(script);
	script.state = RUNNABLE;
}

void VirtualMachine::DispatchEvents()
{
	// Flag clicked
	if (_flagClicked)
	{
		for (Script *script : _flagListeners)
			StartScript(*script);
		_flagClicked = false;
	}

	// Broadcasts
	if (_toSend.size() != 0)
	{
		for (const std::string &message : _toSend)
		{
			auto it = _messageListeners.find(message);
			if (it == _messageListeners.end())
				continue;

			for (Script *script : it->second)
				StartScript(*script);
		}

		_toSend.clear();
	}

	// Sprite clicked
	while (!_clickQueue.empty())
	{
		Script *s = _clickQueue.front();
		StartScript(*s);
		_clickQueue.pop();
	}

	// Ask input
	if (_askQueue.size() != 0 && _asker == nullptr)
	{
		std::pair<Script *, std::string> &top = _askQueue.front();
		_asker = top.first;
		_question = top.second;
		memset(_inputBuf, 0, sizeof(_inputBuf));
		_askQueue.pop();
	}
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
			bool gliding = script.sprite->GetGlide()->end > _time;

			if (!gliding && !script.waitExpr && !script.waitInput && !script.askInput && script.sleepUntil <= _time)
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

			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", _exceptionMessage, _render->GetWindow());
			
			_activeScripts = 0;
			_waitingScripts = 0;
			return;
		}

		if (script.state == TERMINATED)
			ResetScript(script);
	}

	_activeScripts = activeScripts;
	_waitingScripts = waitingScripts;
}

void VirtualMachine::Main()
{
#if CLOCK_SPEED != 0
	constexpr double kMinExecutionTime = 1.0 / CLOCK_SPEED;
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

	_render = new GLRenderer(_spritesEnd - _sprites - 1); // exclude the stage
	if (_render->HasError())
		Panic("Failed to initialize graphics");

	// Initialize graphics resources
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
		s->Load(this);

	_messageListeners.clear();
	_keyListeners.clear();
	_flagListeners.clear();

	// Find listeners
	for (Script &script : _scripts)
	{
		AutoRelease<Statement> &s = script.entry->sl[0];
		OnEvent *oe = s->As<OnEvent>();
		if (oe)
		{
			_messageListeners[oe->message].push_back(&script);
			continue;
		}

		OnKeyPressed *okp = s->As<OnKeyPressed>();
		if (okp)
		{
			SDL_Scancode sc = SDL_GetScancodeFromName(okp->key.c_str());
			if (sc != SDL_SCANCODE_UNKNOWN)
				_keyListeners[sc].push_back(&script);
			continue;
		}

		OnFlagClicked *ofc = s->As<OnFlagClicked>();
		if (ofc)
		{
			_flagListeners.push_back(&script);
			continue;
		}
	}

	SDL_Window *window = _render->GetWindow();
	SDL_SetWindowTitle(window, "Scratch 3");

	_flagClicked = false;
	_toSend.clear();
	_askQueue = std::queue<std::pair<Script *, std::string>>();
	_asker = nullptr;
	_question.clear();
	memset(_inputBuf, 0, sizeof(_inputBuf));

	_time = 0.0;
	_timerStart = 0.0;
	_deltaExecution = 0.0;

	_running = true;

	_epoch = ls_time64();
	
	SendFlagClicked();

	double lastTime = _time;
	double lastExecution = _time;
	double nextExecution = _time;

	for (;;)
	{
		lastTime = _time;
		_time = ls_time64() - _epoch;

		_io.PollEvents();

		if (_shouldStop)
			break;

		DispatchEvents();

		if (!_suspend && _exceptionType == Exception_None)
		{
			if (_time >= nextExecution)
			{
				double start = ls_time64();

				_deltaExecution = _time - lastExecution;
				lastExecution = _time;
				nextExecution = _time + kMinExecutionTime;

				Scheduler();

				_interpreterTime = ls_time64() - start;
			}
		}

		Render();
	}

	ShutdownThread();
}

int VirtualMachine::ThreadProc(void *data)
{
	VirtualMachine *vm = (VirtualMachine *)data;
	vm->Main();
	return 0;
}
