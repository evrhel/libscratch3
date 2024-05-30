#include "vm.hpp"

#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <algorithm>

#include <imgui_impl_sdl2.h>

#include "../resource.hpp"
#include "../render/renderer.hpp"

#include "sprite.hpp"

#define DEG2RAD (0.017453292519943295769236907684886)
#define RAD2DEG (57.295779513082320876798154814105)

static const char True[4] = {
	't', 'r', 'u', 'e'
};

static const char False[5] = {
	'f', 'a', 'l', 's', 'e'
};

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
			vm->SetString(val, "costume1"); // always costume1
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
		std::string name = vm->ToString(vm->StackAt(0));
		vm->Pop();
			
		Sprite *sprite = vm->FindSprite(name);
		if (!sprite)
			vm->SetBool(vm->Push(), false);

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
		// TODO: implement
		vm->SetReal(vm->Push(), INFINITY);
	}

	virtual void Visit(Answer *node)
	{
		vm->Assign(vm->Push(), vm->GetAnswer());
	}

	virtual void Visit(KeyPressed *node)
	{
		node->e->Accept(this);

		Value &key = vm->StackAt(0);
		if (key.type != ValueType_String)
		{
			vm->Pop();
			vm->SetBool(vm->Push(), false);
			return;
		}

		std::string s(key.u.string->str, key.u.string->len);

		vm->Pop();

		// convert to lowercase
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
		// TODO: implement
		vm->Push(); // None
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

		if (lhs.type != ValueType_String || rhs.type != ValueType_String)
			return; // exception

		Value &v = vm->Push();
		vm->AllocString(v, lhs.u.string->len + rhs.u.string->len);
		if (v.type != ValueType_String)
			return; // exception

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
		if (v2.type != ValueType_String)
			return; // exception

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
			if (v1.type != ValueType_String)
				return; // exception
			v1.u.string->str[0] = v2.u.string->str[index - 1];
		}

		vm->Pop();
	}

	virtual void Visit(StringLength *node)
	{
		node->e->Accept(this);

		Value &v = vm->StackAt(0);
		if (v.type == ValueType_String)
			vm->SetInteger(v, v.u.string->len);
		else
			vm->SetInteger(v, 0);
	}

	virtual void Visit(StringContains *node)
	{
		node->e1->Accept(this);
		node->e2->Accept(this);

		Value &v1 = vm->StackAt(1);
		Value &v2 = vm->StackAt(0);

		vm->CvtString(v1);
		vm->CvtString(v2);

		if (v1.type != ValueType_String || v2.type != ValueType_String)
			return; // exception
		
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

		double secs = vm->ToReal(vm->StackAt(1));
		std::string dest = vm->ToString(vm->StackAt(0));

		double x = 0, y = 0;

		std::transform(dest.begin(), dest.end(), dest.begin(), ::tolower);
		if (dest == "random position")
		{
			x = ls_rand_int(-240, 240);
			y = ls_rand_int(-180, 180);
		}
		else if (dest == "mouse-pointer")
		{
			x = vm->GetMouseX();
			y = vm->GetMouseY();
		}
		else
		{
			Sprite *s = vm->FindSprite(dest);
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

		std::string mstr = vm->ToString(message);
		double secs = vm->ToReal(duration);

		vm->Pop();
		vm->Pop();

		printf("%s saying \"%s\" for %g secs\n",
			script->sprite->GetName().c_str(),
			mstr.c_str(),
			secs);

		if (mstr.size())
			script->sprite->SetMessage(mstr, MESSAGE_STATE_SAY);
		else
			script->sprite->ClearMessage();

		vm->Sleep(secs);
	}

	virtual void Visit(Say *node)
	{
		node->e->Accept(this); // message

		Value &message = vm->StackAt(0);

		std::string mstr = vm->ToString(message);

		vm->Pop();

		printf("%s saying \"%s\"\n",
			script->sprite->GetName().c_str(),
			mstr.c_str());

		if (mstr.size())
			script->sprite->SetMessage(mstr, MESSAGE_STATE_SAY);
		else
			script->sprite->ClearMessage();
	}

	virtual void Visit(ThinkForSecs *node)
	{
		node->e1->Accept(this); // message
		node->e2->Accept(this); // duration

		Value &message = vm->StackAt(1);
		Value &duration = vm->StackAt(0);

		std::string mstr = vm->ToString(message);
		double secs = vm->ToReal(duration);

		vm->Pop();
		vm->Pop();

		printf("%s thinking \"%s\" for %g secs\n",
			script->sprite->GetName().c_str(),
			mstr.c_str(),
			secs);

		if (mstr.size())
			script->sprite->SetMessage(mstr, MESSAGE_STATE_THINK);
		else
			script->sprite->ClearMessage();

		vm->Sleep(secs);
	}

	virtual void Visit(Think *node)
	{
		node->e->Accept(this); // message

		Value &message = vm->StackAt(0);

		std::string mstr = vm->ToString(message);

		vm->Pop();

		printf("%s thinking \"%s\"\n",
			script->sprite->GetName().c_str(),
			mstr.c_str());

		if (mstr.size())
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
			// TODO: lookup costume by name
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

		printf("%s = %s\n",
			node->name.c_str(),
			vm->ToString(vm->StackAt(0)).c_str());

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
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(GlideReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(PointTowardsReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(CostumeReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(BackdropReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(SoundReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(BroadcastReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(CloneReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(TouchingReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(DistanceReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(KeyReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(PropertyOfReporter *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(ArgReporterStringNumber *node)
	{
		vm->SetString(vm->Push(), node->value);
	}

	virtual void Visit(ArgReporterBoolean *node)
	{
		vm->SetString(vm->Push(), node->value);
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

	_loader = loader;
	
	bool foundStage = false;
	for (AutoRelease<SpriteDef> &def : defs)
	{
		auto it = _sprites.find(def->name);
		if (it != _sprites.end())
		{
			// duplicate sprite name
			Cleanup();
			return -1;
		}

		Sprite *sprite = new Sprite(*def);
		_sprites[sprite->GetName()] = sprite;

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

		if (def->variables)
		{
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

				if (v.type == ValueType_Exception)
				{
					Cleanup();
					return -1;
				}
			}
		}

		if (def->scripts)
		{
			for (AutoRelease<StatementList> &sl : def->scripts->sll)
			{
				Script script = { 0 };

				script.state = EMBRYO;
				script.sprite = sprite;
				script.entry = *sl;
				//script.pc = 1;

				script.lock = ls_lock_create();
				if (!script.lock)
				{
					Cleanup();
					return -1;
				}

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
	ls_lock(_lock);
	if (_running)
	{
		ls_unlock(_lock);
		return -1;
	}

	_shouldStop = false;

	ls_handle thread = ls_thread_create(&ThreadProc, this);
	if (!thread)
	{
		ls_unlock(_lock);
		return -1;
	}

	SendFlagClicked();

	while (!_running)
		ls_cond_wait(_cond, _lock);
	ls_unlock(_lock);

	return 0;
}

void VirtualMachine::VMTerminate()
{
	ls_lock(_lock);
	_shouldStop = true;
	ls_unlock(_lock);
}

int VirtualMachine::VMWait(unsigned long ms)
{
	int rc = 0;

	ls_lock(_lock);
	_waitCount++;
	while (_running)
	{
		rc = ls_cond_timedwait(_cond, _lock, ms);
		if (rc == -1)
			break;
	}
	_waitCount--;
	ls_unlock(_lock);

	return rc;
}

void VirtualMachine::VMSuspend()
{
	ls_lock(_lock);

	if (!_suspend)
	{
		_suspend = true;
		_suspendStart = ls_time64();

		SDL_SetWindowTitle(_renderer->GetWindow(), "Scratch 3 [Suspended]");
	}

	ls_unlock(_lock);
}

void VirtualMachine::VMResume()
{
	ls_lock(_lock);

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

	ls_unlock(_lock);
}

void VirtualMachine::SendFlagClicked()
{
	for (Script &script : _scripts)
	{
		ls_lock(script.lock);

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

		ls_unlock(script.lock);
	}
}

void VirtualMachine::Sleep(double seconds)
{
	_current->sleepUntil = _time + seconds;
	_current->state = WAITING;
}

void VirtualMachine::WaitUntil(Expression *expr)
{
	_current->waitExpr = expr;
	_current->state = WAITING;

	//printf("%p in %s is waiting\n",
	//	_current,
	//	_current->sprite->name.c_str());
}

void VirtualMachine::AskAndWait()
{
	// TODO: implement
}

void VirtualMachine::Terminate()
{
	_current->state = TERMINATED;

	printf("%p in %s terminated\n",
		_current,
		_current->sprite->GetName().c_str());
}

void VirtualMachine::TerminateScript(unsigned long id)
{
	// TODO: implement
}

static void DumpScript(Script *script)
{
	static const char *STATES[] = {
		"EMBRYO",
		"RUNNABLE",
		"WAITING",
		"SUSPENDED",
		"TERMINATED"
	};

	printf("Script %p\n", script);

	if (script->state >= EMBRYO && script->state <= TERMINATED)
		printf("    state = %s\n", STATES[script->state]);
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
		printf("        sl = %p\n", frame.sl);
		printf("        pc = %lld (%s)\n", frame.pc, frame.sl ? AstTypeString(frame.sl->sl[frame.pc]->GetType()) : "null");
		printf("        count = %lld\n", frame.count);
		printf("        flags = %x\n", frame.flags);
	}}

Value &VirtualMachine::Raise(ExceptionType type)
{
#if _DEBUG
	// TODO: report this in a cleaner way through api calls
	if (type != Exception_None)
	{
		printf("<EXCEPTION> ");
		switch (type)
		{
		case OutOfMemory:
			printf("Out of memory\n");
			break;
		case StackOverflow:
			printf("Stack overflow\n");
			break;
		case StackUnderflow:
			printf("Stack underflow\n");
			break;
		case VariableNotFound:
			printf("Variable not found\n");
			break;
		case IllegalOperation:
			printf("Illegal operation\n");
			break;
		case InvalidArgument:
			printf("Invalid argument\n");
			break;
		case UnsupportedOperation:
			printf("Unsupported operation\n");
			break;
		case NotImplemented:
			printf("Not implemented\n");
			break;
		case VMError:
			printf("VM error\n");
			break;
		default:
			printf("Unknown exception\n");
			break;
		}

		if (_current)
		{
			Script *script = _current;
			printf("Exception information:\n");
			DumpScript(_current);
		}
		else
			printf("No current script\n");

		abort();
	}

#endif // _DEBUG

	if (_exception.u.exception == Exception_None)
		_exception.u.exception = type;
	return _exception;
}

Value &VirtualMachine::Push()
{
	if (_exception.u.exception != Exception_None)
		return _exception;

	if (_current->sp < _current->sp)		
		return Raise(StackOverflow);
	_current->sp--;
	*_current->sp = { 0 }; // zero out the value
	return *_current->sp;
}

bool VirtualMachine::Pop()
{
	if (_exception.u.exception != Exception_None)
		return false;

	if (_current->sp >= _current->stack + STACK_SIZE)
	{
		Raise(StackUnderflow);
		return false;
	}

	ReleaseValue(*_current->sp);

	// fill with garbage
	memset(_current->sp, 0xab, sizeof(Value));

	_current->sp++;

	return true;
}

Value &VirtualMachine::StackAt(size_t i)
{
	if (_exception.u.exception != Exception_None)
		return _exception;

	if (_current->sp + i >= _current->stack + STACK_SIZE)
		return Raise(StackUnderflow);

	return _current->sp[i];
}

void VirtualMachine::PushFrame(StatementList *sl, int64_t count, uint32_t flags)
{
	if (_exception.u.exception != Exception_None && count > 0)
		return;

	if (_current->fp >= SCRIPT_DEPTH - 1)
	{
		Raise(StackOverflow);
		return;
	}

	if (sl == nullptr)
	{
		// empty loop
		_current->frames[_current->fp].pc -= 1; // loop back to the same statement

		// Raise(InvalidArgument);
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
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return false;

	if (val.type == ValueType_Exception)
		return false;

	if (val.type == ValueType_Bool)
		return val.u.boolean;

	if (val.type == ValueType_String)
	{

		if (val.u.string->len != 4)
			return false;

		for (size_t i = 0; i < 4; i++)
		{
			if (tolower(val.u.string->str[i]) != True[i])
				return false;
		}

		return true;
	}

	return false;
}

bool VirtualMachine::Equals(const Value &lhs, const Value &rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
		return false;

	if (lhs.type == ValueType_Exception || rhs.type == ValueType_Exception)
		return false;

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
	case ValueType_String: {
		// String comparisions are case-insensitive and ignore
		// leading and trailing whitespace

		String *lstr = lhs.u.string;
		String *rstr = rhs.u.string;

		if (lstr == rstr)
			return true;

		char *lstart = lstr->str;
		while (isspace(*lstart))
			lstart++;

		char *rend = lstart;
		while (!isspace(*rend))
			rend++;

		char *rstart = rstr->str;
		while (isspace(*rstart))
			rstart++;

		char *lend = rend;
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
	default:
		return false;
	}
}

Value &VirtualMachine::Assign(Value &lhs, const Value &rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
	{
		ReleaseValue(lhs);
		return _exception;
	}

	if (lhs.type == ValueType_Exception)
		return lhs;

	if (&lhs == &rhs)
		return lhs;

	if (lhs.u.ref == rhs.u.ref)
		return RetainValue(lhs);

	ReleaseValue(lhs);
	return RetainValue(lhs = rhs);
}


Value &VirtualMachine::SetInteger(Value &lhs, int64_t val)
{
	// exceptions should propagate
	if (_exception.u.exception != Exception_None)
	{
		ReleaseValue(lhs);
		return _exception;
	}

	if (lhs.type == ValueType_Exception)
		return lhs;

	ReleaseValue(lhs);
	lhs.type = ValueType_Integer;
	lhs.u.integer = val;
	return lhs;
}

Value &VirtualMachine::SetReal(Value &lhs, double rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
	{
		ReleaseValue(lhs);
		return _exception;
	}

	if (lhs.type == ValueType_Exception)
		return lhs;

	ReleaseValue(lhs);
	lhs.type = ValueType_Real;
	lhs.u.real = rhs;
	return lhs;
}

Value &VirtualMachine::SetBool(Value &lhs, bool rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
	{
		ReleaseValue(lhs);
		return _exception;
	}

	if (lhs.type == ValueType_Exception)
		return lhs;

	ReleaseValue(lhs);
	lhs.type = ValueType_Bool;
	lhs.u.boolean = rhs;
	return lhs;
}

Value &VirtualMachine::SetString(Value &lhs, const std::string &rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
	{
		ReleaseValue(lhs);
		return _exception;
	}

	if (lhs.type == ValueType_Exception)
		return lhs;

	ReleaseValue(lhs);

	if (rhs.size() == 0)
		return Assign(lhs, _emptyString);

	AllocString(lhs, rhs.size());
	if (lhs.type != ValueType_String)
		return lhs;

	memcpy(lhs.u.string->str, rhs.data(), rhs.size());

	return lhs;
}

Value &VirtualMachine::SetParsedString(Value &lhs, const std::string &rhs)
{
	// exceptions should propagate

	if (_exception.u.exception != Exception_None)
	{
		ReleaseValue(lhs);
		return _exception;
	}

	if (lhs.type == ValueType_Exception)
		return lhs;

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
				if (tolower(str[i]) != True[i])
					break;

			if (i == 4)
				return SetBool(lhs, true);
		}

		if (str.size() == 5)
		{
			size_t i;
			for (i = 0; i < 5; i++)
				if (tolower(str[i]) != False[i])
					break;

			if (i == 5)
				return SetBool(lhs, false);
		}
	}

	return SetString(lhs, rhs);
}


Value &VirtualMachine::SetEmpty(Value &lhs)
{
	return Assign(lhs, _emptyString);
}

std::string VirtualMachine::ToString(const Value &val)
{
	if (_exception.u.exception != Exception_None)
		return std::string();

	switch (val.type)
	{
	default:
	case ValueType_Exception:
	case ValueType_None:
		return std::string();
	case ValueType_Integer:
		return std::to_string(val.u.integer);
	case ValueType_Real: {
		char buf[64];
		int cch = snprintf(buf, sizeof(buf), "%.8g", val.u.real);
		return std::string(buf, cch);
	}
	case ValueType_Bool:
		return val.u.boolean ? std::string(True, 4) : std::string(False, 5);
	case ValueType_String:
		return std::string(val.u.string->str, val.u.string->len);
	}
}

void VirtualMachine::CvtString(Value &v)
{
	if (_exception.u.exception != Exception_None)
		return;

	switch (v.type)
	{
	default:
	case ValueType_Exception:
	case ValueType_String:
		break;
	case ValueType_Integer: {
		int cch;
		cch = snprintf(nullptr, 0, "%" PRId64, v.u.integer);
		if (cch < 0)
		{
			Raise(OutOfMemory);
			break;
		}

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
		{
			Raise(OutOfMemory);
			break;
		}

		AllocString(v, cch);
		if (v.type != ValueType_String)
			break; // exception

		snprintf(v.u.string->str, cch + 1, "%.8g", v.u.real);

		break;
	}
	case ValueType_Bool:
		Assign(v, v.u.boolean ? _trueString : _falseString);
		break;
	}
}

int64_t VirtualMachine::ToInteger(const Value &val)
{
	if (_exception.u.exception != Exception_None)
		return 0;

	switch (val.type)
	{
	default:
	case ValueType_Exception:
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
	if (_exception.u.exception != Exception_None)
		return 0.0;

	switch (val.type)
	{
	default:
	case ValueType_Exception:
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

	v.type = ValueType_String;
	v.u.string = (String *)calloc(1, offsetof(String, str) + len + 1);
	if (!v.u.string)
		return Raise(OutOfMemory);

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
		return Raise(VariableNotFound);
	return it->second;
}

Sprite *VirtualMachine::FindSprite(const std::string &name)
{
	auto it = _sprites.find(name);
	return it != _sprites.end() ? it->second : nullptr;
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

VirtualMachine::VirtualMachine()
{
	_prog = nullptr;
	_loader = nullptr;

	_renderer = nullptr;

	_answer = { 0 };
	_mouseDown = false;
	_mouseX = 0;
	_mouseY = 0;
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
	_exception = { 0 };

	_current = nullptr;
	_time = 0.0;
	_lastTime = 0.0;
	_nextExecution = 0.0;

	_lock = ls_lock_create();
	_cond = ls_cond_create();

	_emptyString = {};
	AllocString(_emptyString, 0);

	_trueString = {};
	AllocString(_trueString, sizeof(True));
	if (_trueString.type == ValueType_String)
		memcpy(_trueString.u.string->str, True, sizeof(True));

	_falseString = {};
	AllocString(_falseString, sizeof(False));
	if (_falseString.type == ValueType_String)
		memcpy(_falseString.u.string->str, False, sizeof(False));
}

VirtualMachine::~VirtualMachine()
{
	Cleanup();

	ReleaseValue(_falseString);
	ReleaseValue(_trueString);
	ReleaseValue(_emptyString);

	ReleaseValue(_username);
	ReleaseValue(_answer);

	Release(_prog);

	ls_close(_cond);
	ls_close(_lock);
}

bool VirtualMachine::InitGraphics()
{
	return false;
}

void VirtualMachine::DestroyGraphics()
{
	for (auto &p : _sprites)
		delete p.second;
	_sprites.clear();

	if (_renderer)
		delete _renderer, _renderer = nullptr;
}

void VirtualMachine::PollEvents()
{
	if (!_renderer)
		return;

	SDL_Event evt;

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
				_mouseDown = true;
			break;
		case SDL_MOUSEBUTTONUP:
			if (evt.button.button == SDL_BUTTON_LEFT)
				_mouseDown = false;
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
		}
	}
}

void VirtualMachine::Render()
{
	_renderer->BeginRender();

	int spritesVisible = 0;

	for (auto &p : _sprites)
	{
		p.second->Update();
		spritesVisible += p.second->IsShown();
	}

	_renderer->Render();

	double dt = _time - _lastTime;
	double fps = 1 / dt;

	if (ImGui::Begin("Debug"))
	{
		SDL_Window *window = _renderer->GetWindow();
		int width, height;
		SDL_GL_GetDrawableSize(window, &width, &height);

		int left = _renderer->GetLogicalLeft();
		int right = _renderer->GetLogicalRight();
		int top = _renderer->GetLogicalTop();
		int bottom = _renderer->GetLogicalBottom();

		ImGui::SeparatorText("Graphics");
		ImGui::LabelText("Framerate", "%.2f (%d ms)", fps, (int)(dt * 1000));
		ImGui::LabelText("Resolution", "%d x %d", width, height);
		ImGui::LabelText("Viewport Size", "%d x %d", right - left, top - bottom);
		ImGui::LabelText("Sprites visible", "%d/%d", spritesVisible, (int)_sprites.size());

		ImGui::SeparatorText("I/O");
		ImGui::LabelText("Mouse Down", "%s", _mouseDown ? "true" : "false");
		ImGui::LabelText("Mouse", "%d, %d", (int)_mouseX, (int)_mouseY);
		ImGui::LabelText("Keys Pressed", "%d", _keysPressed);
		ImGui::LabelText("Timer", "%.2f", _timer);

		ImGui::SeparatorText("Virtual Machine");
		ImGui::LabelText("Program Name", "%s", _progName.c_str());
		ImGui::LabelText("Active Scripts", "%d/%d", (int)_activeScripts, (int)_scripts.size());
		ImGui::LabelText("Waiting Scripts", "%d", (int)_waitCount);
		ImGui::LabelText("Execution Rate", "%d Hz", (int)FRAMERATE);
		ImGui::LabelText("Variable Count", "%d", (int)_variables.size());

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

		ImGui::SameLine();


		if (ImGui::Button("Terminate"))
			VMTerminate();
	}
	ImGui::End();

	_renderer->EndRender();
}

void VirtualMachine::Cleanup()
{
	for (auto &p : _variables)
		ReleaseValue(p.second);

	for (Script &script : _scripts)
	{
		ls_close(script.lock);
		free(script.stack);
	}

	DestroyGraphics();

	_loader = nullptr;
}

void VirtualMachine::Scheduler()
{
	constexpr double kMinExecutionTime = 1.0 / FRAMERATE;

	Executor executor;

	executor.vm = this;

	ls_lock(_lock);

	int64_t spriteCount = _sprites.size();
	_renderer = new GLRenderer(spriteCount - 1); // exclude the stage

	// Initialize graphics resources
	for (auto &p : _sprites)
		p.second->InitGraphics(_loader, _renderer);

	SDL_Window *window = _renderer->GetWindow();
	SDL_SetWindowTitle(window, "Scratch 3");

	_time = ls_time64();
	_timerStart = _time;

	_running = true;
	ls_cond_signal(_cond);

	_nextExecution = _time;

	for (;;)
	{
		if (_exception.u.exception != Exception_None)
		{
			printf("<EXCEPTION> %u\n", _exception.u.exception);
			break;
		}

		PollEvents();

		bool suspend = _suspend;

		if (_shouldStop)
			break;

		// Nothing running and threads are waiting
		if (!suspend && _activeScripts == 0 && _waitCount != 0)
			break;

		size_t activeScripts = -1;

		ls_unlock(_lock);
		if (!suspend)
		{
			_lastTime = _time;
			_time = ls_time64();
			_timer = _time - _timerStart;

			if (_time >= _nextExecution)
			{
				_nextExecution = _time + kMinExecutionTime;

				activeScripts = 0;
				for (Script &script : _scripts)
				{
					ls_lock(script.lock);

					_current = &script;

					// Handle waiting scripts
					if (script.state == WAITING)
					{
						if (script.waitExpr)
						{
							// Execute the wait expression
							executor.script = &script;
							script.waitExpr->Accept(&executor);

							// If true, awaken the script
							if (Truth(script.sp[0]))
								script.state = RUNNABLE;
						}
						else if (script.sleepUntil <= _time)
						{
							// Wake up the script
							script.state = RUNNABLE;
						}
					}

					// Don't execute scripts that are not runnable
					if (script.state != RUNNABLE)
					{
						if (script.state == WAITING)
							activeScripts++;

						_current = nullptr;
						ls_unlock(script.lock);
						continue;
					}

					// Pop frames until we find one that can execute
					Frame *f = &script.frames[script.fp];
					while (f->pc >= f->sl->sl.size())
					{
						// top of the stack, script is done
						if (script.fp == 0)
						{
							script.state = TERMINATED;
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
					if (script.state == RUNNABLE)
					{
						activeScripts++;

						executor.script = &script;

						if (f->pc < f->sl->sl.size())
						{
							// Run the statement
							Statement *stmt = *f->sl->sl[f->pc];
							stmt->Accept(&executor);

							// Exception handling
							if (_exception.u.exception == Exception_None)
							{
								if (script.sp != script.stack + STACK_SIZE)
									Raise(VMError); // stack should be empty
								else if (script.fp >= SCRIPT_DEPTH)
									Raise(VMError); // invalid frame pointer
							}

							f->pc++;
						}
						else
							Raise(VMError);
					}

					_current = nullptr;

					ls_unlock(script.lock);
				}

				for (auto &p : _sprites)
				{
					Sprite *s = p.second;
					GlideInfo &g = *s->GetGlide();

					if (g.start < 0.0)
						continue; // not gliding

					double t = (_time - g.start) / (g.end - g.start);
					if (t >= 1.0)
					{
						// done gliding
						s->SetXY(g.x1, g.y1);
						g.start = -1.0;
					}
					else
					{
						// linear interpolation
						s->SetXY(g.x0 + (g.x1 - g.x0) * t, g.y0 + (g.y1 - g.y0) * t);
					}
				}
			}
		}

		Render();

		ls_lock(_lock);

		if (activeScripts != -1)
			_activeScripts = activeScripts;
	}

	// Lock is held here

	_activeScripts = 0;
	_running = false;

	DestroyGraphics(); // clean up graphics resources

	ls_cond_broadcast(_cond); // notify waiting threads
	ls_unlock(_lock);
}

int VirtualMachine::ThreadProc(void *data)
{
	VirtualMachine *vm = (VirtualMachine *)data;
	vm->Scheduler();
	return 0;
}
