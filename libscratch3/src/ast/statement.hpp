#pragma once

#include "reporter.hpp"

// executable statement
struct Statement : public ASTNode
{
	AST_IMPL(Statement, ASTNode);

	bool topLevel = false;
};

// statement list
struct StatementList : public ASTNode
{
	AST_IMPL(StatementList, ASTNode);
	AST_ACCEPTOR;

	inline virtual ~StatementList()
	{
		for (auto s : sl)
			delete s;
	}

	std::vector<Statement *> sl;
};

// [move $e steps]
struct MoveSteps : public Statement
{
	AST_IMPL(MoveSteps, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "STEPS")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~MoveSteps()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [turn cw $e degrees]
struct TurnDegrees : public Statement
{
	AST_IMPL(TurnDegrees, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "DEGREES")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~TurnDegrees()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [turn ccw $e degrees]
struct TurnNegDegrees : public Statement
{
	AST_IMPL(TurnNegDegrees, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "DEGREES")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~TurnNegDegrees()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [go to $e]
struct Goto : public Statement
{
	AST_IMPL(Goto, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "TO")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~Goto()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [go to x: $e1 y: $e2]
struct GotoXY : public Statement
{
	AST_IMPL(GotoXY, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "X")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "Y")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual ~GotoXY()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// [glide $e1 secs to $e2]
struct Glide : public Statement
{
	AST_IMPL(Glide, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "SECS")
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

	inline virtual ~Glide()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// [glide $e1 secs to x: $e2 y: $e3]
struct GlideXY : public Statement
{
	AST_IMPL(GlideXY, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "SECS")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "X")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		if (key == "Y")
		{
			if (!e3)
				e3 = val->As<Expression>();
			return !!e3;
		}

		return false;
	}

	inline virtual ~GlideXY()
	{
		delete e1;
		delete e2;
		delete e3;
	}

	Expression *e1 = nullptr, *e2 = nullptr, *e3 = nullptr;
};

// [point in direction $e]
struct PointDir : public Statement
{
	AST_IMPL(PointDir, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "DIRECTION")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~PointDir()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [point towards $e]
struct PointTowards : public Statement
{
	AST_IMPL(PointTowards, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "TOWARDS")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~PointTowards()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [change x by $e]
struct ChangeX : public Statement
{
	AST_IMPL(ChangeX, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "DX")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~ChangeX()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [set x to $e]
struct SetX : public Statement
{
	AST_IMPL(SetX, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "X")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~SetX()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [change y by $e]
struct ChangeY : public Statement
{
	AST_IMPL(ChangeY, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "DY")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~ChangeY()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [set y to $e]
struct SetY : public Statement
{
	AST_IMPL(SetY, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "Y")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~SetY()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [if on edge, bounce]
struct BounceIfOnEdge : public Statement
{
	AST_IMPL(BounceIfOnEdge, Statement);
	AST_ACCEPTOR;
};

// [set rotation style ?style]
struct SetRotationStyle : public Statement
{
	AST_IMPL(SetRotationStyle, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "STYLE")
		{
			style = RotationStyleFromString(value);
			return true;
		}

		return false;
	}

	RotationStyle style = RotationStyle_Unknown;
};

// [say $e1 for $e2 secs]
struct SayForSecs : public Statement
{
	AST_IMPL(SayForSecs, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "MESSAGE")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "SECS")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual ~SayForSecs()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// [say $e]
struct Say : public Statement
{
	AST_IMPL(Say, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "MESSAGE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~Say()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [think $e1 for $e2 secs]
struct ThinkForSecs : public Statement
{
	AST_IMPL(ThinkForSecs, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "MESSAGE")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "SECS")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
		}

		return false;
	}

	inline virtual ~ThinkForSecs()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
};

// [think $e]
struct Think : public Statement
{
	AST_IMPL(Think, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "MESSAGE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~Think()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [switch costume to $e]
struct SwitchCostume : public Statement
{
	AST_IMPL(SwitchCostume, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "COSTUME")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~SwitchCostume()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [next costume]
struct NextCostume : public Statement
{
	AST_IMPL(NextCostume, Statement);
	AST_ACCEPTOR;
};

// [switch backdrop to $e]
struct SwitchBackdrop : public Statement
{
	AST_IMPL(SwitchBackdrop, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "BACKDROP")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~SwitchBackdrop()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [switch backdrop to $e and wait]
struct SwitchBackdropAndWait : public Statement
{
	AST_IMPL(SwitchBackdropAndWait, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "BACKDROP")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~SwitchBackdropAndWait()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [next backdrop]
struct NextBackdrop : public Statement
{
	AST_IMPL(NextBackdrop, Statement);
	AST_ACCEPTOR;
};

// [change size by $e]
struct ChangeSize : public Statement
{
	AST_IMPL(ChangeSize, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "CHANGE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~ChangeSize()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [set size to $e]
struct SetSize : public Statement
{
	AST_IMPL(SetSize, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "SIZE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~SetSize()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [change ?effect effect by $e]
struct ChangeGraphicEffect : public Statement
{
	AST_IMPL(ChangeGraphicEffect, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "CHANGE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "EFFECT")
		{
			effect = GraphicEffectFromString(value);
			return true;
		}

		return false;
	}

	inline virtual ~ChangeGraphicEffect()
	{
		delete e;
	}

	GraphicEffect effect = GraphicEffect_Unknown;
	Expression *e = nullptr;
};

// [set ?effect effect to $e]
struct SetGraphicEffect : public Statement
{
	AST_IMPL(SetGraphicEffect, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "VALUE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "EFFECT")
		{
			effect = GraphicEffectFromString(value);
			return true;
		}

		return false;
	}

	inline virtual ~SetGraphicEffect()
	{
		delete e;
	}

	GraphicEffect effect = GraphicEffect_Unknown;
	Expression *e = nullptr;
};

// [clear graphic effects]
struct ClearGraphicEffects : public Statement
{
	AST_IMPL(ClearGraphicEffects, Statement);
	AST_ACCEPTOR;
};

// [show]
struct ShowSprite : public Statement
{
	AST_IMPL(ShowSprite, Statement);
	AST_ACCEPTOR;
};

// [hide]
struct HideSprite : public Statement
{
	AST_IMPL(HideSprite, Statement);
	AST_ACCEPTOR;
};

// [go to ?layer layer]
struct GotoLayer : public Statement
{
	AST_IMPL(GotoLayer, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "FRONT_BACK")
		{
			if (layer != LayerType_Unknown)
				return false;

			if (value == "front")
				layer = LayerType_Front;
			else if (value == "back")
				layer = LayerType_Back;
			else
				return false;

			return true;
		}

		return false;
	}

	LayerType layer = LayerType_Unknown;
};

// [go ?direction $e layers]
struct MoveLayer : public Statement
{
	AST_IMPL(MoveLayer, Statement);
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
		if (key == "FORWARD_BACKWARD")
		{
			if (direction == LayerDir_Unknown)
			{
				if (value == "forward")
					direction = LayerDir_Forward;
				else if (value == "backward")
					direction = LayerDir_Backward;
				else
					return false;
			}
			return true;
		}

		return false;
	}

	inline virtual ~MoveLayer()
	{
		delete e;
	}

	LayerDir direction = LayerDir_Unknown;
	Expression *e = nullptr;
};

// [play sound $e until done]
struct PlaySoundUntilDone : public Statement
{
	AST_IMPL(PlaySoundUntilDone, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "SOUND_MENU")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~PlaySoundUntilDone()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [start sound $e]
struct StartSound : public Statement
{
	AST_IMPL(StartSound, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "SOUND_MENU")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~StartSound()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [stop all sounds]
struct StopAllSounds : public Statement
{
	AST_IMPL(StopAllSounds, Statement);
	AST_ACCEPTOR;
};

// [change ?effect effect by $e]
struct ChangeSoundEffect : public Statement
{
	AST_IMPL(ChangeSoundEffect, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "VALUE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "EFFECT")
		{
			if (effect != SoundEffect_Unknown)
				return false;
			effect = SoundEffectFromString(value);
			return true;
		}

		return false;
	}

	inline virtual ~ChangeSoundEffect()
	{
		delete e;
	}

	SoundEffect effect = SoundEffect_Unknown;
	Expression *e = nullptr;
};

// [set ?effect effect to $e]
struct SetSoundEffect : public Statement
{
	AST_IMPL(SetSoundEffect, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "VALUE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "EFFECT")
		{
			if (effect != SoundEffect_Unknown)
				return false;
			effect = SoundEffectFromString(value);
			return true;
		}

		return false;
	}

	inline virtual ~SetSoundEffect()
	{
		delete e;
	}

	SoundEffect effect = SoundEffect_Unknown;
	Expression *e = nullptr;
};

// [clear sound effects]
struct ClearSoundEffects : public Statement
{
	AST_IMPL(ClearSoundEffects, Statement);
	AST_ACCEPTOR;
};

// [change volume by $e]
struct ChangeVolume : public Statement
{
	AST_IMPL(ChangeVolume, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "VOLUME")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~ChangeVolume()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [set volume to $e %]
struct SetVolume : public Statement
{
	AST_IMPL(SetVolume, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "VOLUME")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~SetVolume()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [when flag clicked]
struct OnFlagClicked : public Statement
{
	AST_IMPL(OnFlagClicked, Statement);
	AST_ACCEPTOR;
};

// [when ?key key pressed]
struct OnKeyPressed : public Statement
{
	AST_IMPL(OnKeyPressed, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "KEY_OPTION")
		{
			if (this->key != Key_Unknown)
				return false;
			this->key = KeyFromString(value);
			return this->key != Key_Unknown;
		}

		return false;
	}

	Key key = Key_Unknown;
};

// [when this sprite clicked]
struct OnSpriteClicked : public Statement
{
	AST_IMPL(OnSpriteClicked, Statement);
	AST_ACCEPTOR;
};

// [when stage clicked]
struct OnStageClicked : public Statement
{
	AST_IMPL(OnStageClicked, Statement);
	AST_ACCEPTOR;
};

// [when backdrop switches to ?backdrop]
struct OnBackdropSwitch : public Statement
{
	AST_IMPL(OnBackdropSwitch, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "BACKDROP")
		{
			backdrop = value;
			return true;
		}

		return false;
	}

	std::string backdrop;
};

// [when ?value > $e]
struct OnGreaterThan : public Statement
{
	AST_IMPL(OnGreaterThan, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "VALUE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "WHENGREATERTHANMENU")
		{
			if (this->value != ListenValueType_Unknown)
				return false;
			this->value = ListenValueTypeFromString(value);
			return this->value != ListenValueType_Unknown;
		}

		return false;
	}

	inline virtual ~OnGreaterThan()
	{
		delete e;
	}

	ListenValueType value = ListenValueType_Unknown;
	Expression *e = nullptr;
};

// [when I receive ?message]
struct OnEvent : public Statement
{
	AST_IMPL(OnEvent, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "BROADCAST_OPTION")
		{
			message = value;
			return true;
		}

		return false;
	}

	std::string message;
};

// [broadcast $e]
struct Broadcast : public Statement
{
	AST_IMPL(Broadcast, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "BROADCAST_INPUT")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~Broadcast()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [broadcast $e and wait]
struct BroadcastAndWait : public Statement
{
	AST_IMPL(BroadcastAndWait, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "BROADCAST_INPUT")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~BroadcastAndWait()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [wait $e secs]
struct WaitSecs : public Statement
{
	AST_IMPL(WaitSecs, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "DURATION")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~WaitSecs()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [repeat $e]
//   $sl
// [end]
struct Repeat : public Statement
{
	AST_IMPL(Repeat, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "TIMES")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		if (key == "SUBSTACK")
		{
			if (!sl)
				sl = val->As<StatementList>();
			return !!sl;
		}

		return false;
	}

	inline virtual ~Repeat()
	{
		delete e;
		delete sl;
	}

	Expression *e = nullptr;
	StatementList *sl = nullptr;
};

// [forever]
//   $sl
// [end]
struct Forever : public Statement
{
	AST_IMPL(Forever, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "SUBSTACK")
		{
			if (!sl)
			{
				if (val == nullptr)
					sl = new StatementList();
				else
					sl = val->As<StatementList>();
			}
			return !!sl;
		}

		return false;
	}

	inline virtual ~Forever()
	{
		delete sl;
	}

	StatementList *sl = nullptr;
};

// [if $e]
//   $sl
// [end]
struct If : public Statement
{
	AST_IMPL(If, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "CONDITION")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		if (key == "SUBSTACK")
		{
			if (!sl)
				sl = val->As<StatementList>();
			return !!sl;
		}

		return false;
	}

	inline virtual ~If()
	{
		delete e;
		delete sl;
	}

	Expression *e = nullptr;
	StatementList *sl = nullptr;
};

// [if $e]
//   $sl1
// [else]
//   $sl2
// [end]
struct IfElse : public Statement
{
	AST_IMPL(IfElse, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "CONDITION")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		if (key == "SUBSTACK")
		{
			if (!sl1)
				sl1 = val->As<StatementList>();
			return !!sl1;
		}

		if (key == "SUBSTACK2")
		{
			if (!sl2)
				sl2 = val->As<StatementList>();
			return !!sl2;
		}

		return false;
	}

	inline virtual ~IfElse()
	{
		delete e;
		delete sl1;
		delete sl2;
	}

	Expression *e = nullptr;
	StatementList *sl1 = nullptr, *sl2 = nullptr;
};

// [wait until $e]
struct WaitUntil : public Statement
{
	AST_IMPL(WaitUntil, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "CONDITION")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~WaitUntil()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [repeat until $e]
//   $sl
// [end]
struct RepeatUntil : public Statement
{
	AST_IMPL(RepeatUntil, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "CONDITION")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		if (key == "SUBSTACK")
		{
			if (!sl)
				sl = val->As<StatementList>();
			return !!sl;
		}

		return false;
	}

	inline virtual ~RepeatUntil()
	{
		delete e;
		delete sl;
	}

	Expression *e = nullptr;
	StatementList *sl = nullptr;
};

// [stop ?mode]
struct Stop : public Statement
{
	AST_IMPL(Stop, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "STOP_OPTION")
		{
			if (mode != StopMode_Unknown)
				return false;
			mode = StopModeFromString(value);
			return true;
		}

		return false;
	}

	StopMode mode = StopMode_Unknown;
};

// [when I start as a clone]
struct CloneStart : public Statement
{
	AST_IMPL(CloneStart, Statement);
	AST_ACCEPTOR;
};

// [create clone of $e]
struct CreateClone : public Statement
{
	AST_IMPL(CreateClone, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "CLONE_OPTION")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~CreateClone()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [delete this clone]
struct DeleteClone : public Statement
{
	AST_IMPL(DeleteClone, Statement);
	AST_ACCEPTOR;
};

// [ask $e and wait]
struct AskAndWait : public Statement
{
	AST_IMPL(AskAndWait, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "QUESTION")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	inline virtual ~AskAndWait()
	{
		delete e;
	}

	Expression *e = nullptr;
};

// [set drag mode ?mode]
struct SetDragMode : public Statement
{
	AST_IMPL(SetDragMode, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "DRAG_MODE")
		{
			if (mode != DragMode_Unknown)
				return false;
			mode = DragModeFromString(value);
			return true;
		}

		return false;
	}

	DragMode mode = DragMode_Unknown;
};

// [reset timer]
struct ResetTimer : public Statement
{
	AST_IMPL(ResetTimer, Statement);
	AST_ACCEPTOR;
};

// [set ?id to $e]
struct SetVariable : public Statement
{
	AST_IMPL(SetVariable, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "VALUE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		(void)value;

		if (key == "VARIABLE")
		{
			if (!this->id.empty())
				return false;
			this->id = id;
			this->name = value;
			return true;
		}

		return false;
	}

	inline virtual ~SetVariable()
	{
		delete e;
	}

	std::string id, name;
	Expression *e = nullptr;
};

// [change ?id by $e]
struct ChangeVariable : public Statement
{
	AST_IMPL(ChangeVariable, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "VALUE")
		{
			if (!e)
				e = val->As<Expression>();
			return !!e;
		}

		return false;
	}

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "VARIABLE")
		{
			if (!this->id.empty())
				return false;
			this->id = id;
			this->name = value;
			return true;
		}

		return false;
	}

	inline virtual ~ChangeVariable()
	{
		delete e;
	}

	std::string id, name;
	Expression *e = nullptr;
};

// [show variable ?id]
struct ShowVariable : public Statement
{
	AST_IMPL(ShowVariable, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "VARIABLE")
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

// [hide variable ?id]
struct HideVariable : public Statement
{
	AST_IMPL(HideVariable, Statement);
	AST_ACCEPTOR;

	AST_FIELD_SETTER(key, value, id)
	{
		if (key == "VARIABLE")
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

// [add $e to ?id]
struct AppendToList : public Statement
{
	AST_IMPL(AppendToList, Statement);
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

	inline virtual ~AppendToList()
	{
		delete e;
	}

	Expression *e = nullptr;
	std::string id, name;
};

// [delete $e of ?id]
struct DeleteFromList : public Statement
{
	AST_IMPL(DeleteFromList, Statement);
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

	inline virtual ~DeleteFromList()
	{
		delete e;
	}

	Expression *e = nullptr;
	std::string id, name;
};

// [delete all of ?id]
struct DeleteAllList : public Statement
{
	AST_IMPL(DeleteAllList, Statement);
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

// [insert $e1 at $e2 of ?id]
struct InsertInList : public Statement
{
	AST_IMPL(InsertInList, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "ITEM")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "INDEX")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
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

	inline virtual ~InsertInList()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr, *e2 = nullptr;
	std::string id, name;
};

// [replace item $e1 of ?id with $e2]
struct ReplaceInList : public Statement
{
	AST_IMPL(ReplaceInList, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "INDEX")
		{
			if (!e1)
				e1 = val->As<Expression>();
			return !!e1;
		}

		if (key == "ITEM")
		{
			if (!e2)
				e2 = val->As<Expression>();
			return !!e2;
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

	inline virtual ~ReplaceInList()
	{
		delete e1;
		delete e2;
	}

	Expression *e1 = nullptr;
	std::string name, id;
	Expression *e2 = nullptr;
};

// [show list ?id]
struct ShowList : public Statement
{
	AST_IMPL(ShowList, Statement);
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

// [hide list ?id]
struct HideList : public Statement
{
	AST_IMPL(HideList, Statement);
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

struct ProcProto : public Statement
{
	AST_IMPL(ProcProto, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		// TODO: the inptus will be of the form:
		// "argument_id": [
		//   1,
		//   "argument reporter block id"
		// ]
		return true;
	}

	// TODO: accept mutation

	std::string proccode;

	std::vector<std::string> argumentIds;
	std::vector<std::string> argumentNames;
	std::vector<std::string> argumentDefaults;

	bool warp;
};

// [define ?name ?params...]
struct DefineProc : public Statement
{
	AST_IMPL(DefineProc, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		if (key == "custom_block")
		{
			if (proto != nullptr)
				return false;

			// extract the proto from the statement list

			StatementList *sl = val->As<StatementList>();
			if (!sl)
				return false;

			if (!sl->sl.size() == 1)
				return false;

			proto = sl->sl[0]->As<ProcProto>();
			if (!proto)
				return false;

			// remove from the list and delete
			sl->sl.clear();
			delete sl;

			return true;
		}

		return false;
	}

	inline virtual ~DefineProc()
	{
		delete proto;
		// delete sl;
	}

	ProcProto *proto = nullptr;

	// StatementList *sl = nullptr;
};

// [?name ?args...]
struct Call : public Statement
{
	AST_IMPL(Call, Statement);
	AST_ACCEPTOR;

	AST_INPUT_SETTER(key, val)
	{
		auto it = args.find(key);
		if (it != args.end())
			return false;

		Expression *expr = val->As<Expression>();
		if (!expr)
			return false;

		args[key] = expr;
		return true;
	}

	// TODO: accept mutation

	inline virtual ~Call()
	{
		for (auto &p : args)
			delete p.second;
	}

	std::string proccode; // proccode
	std::unordered_map<std::string, Expression *> args;
};
