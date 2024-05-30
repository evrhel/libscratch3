#include "astdef.hpp"

#include <algorithm>

const char *const RotationStyleStrings[RotationStyle_Count] = {
	"unknown",
	"left-right",
	"don't rotate",
	"all around"
};

RotationStyle RotationStyleFromString(const std::string &str)
{
	for (int i = 0; i < RotationStyle_Count; i++)
		if (str == RotationStyleStrings[i])
			return (RotationStyle)i;
	return RotationStyle_Unknown;
}

const char *const GraphicEffectStrings[GraphicEffect_Count] = {
	"unknown",
	"COLOR",
	"FISHEYE",
	"WHIRL",
	"PIXELATE",
	"MOSAIC",
	"BRIGHTNESS",
	"GHOST"
};

GraphicEffect GraphicEffectFromString(const std::string &str)
{
	std::string cap(str);
	std::transform(cap.begin(), cap.end(), cap.begin(), ::toupper);

	for (int i = 0; i < GraphicEffect_Count; i++)
		if (cap == GraphicEffectStrings[i])
			return (GraphicEffect)i;
	return GraphicEffect_Unknown;
}

const char *const LayerTypeStrings[LayerType_Count] = {
	"unknown",
	"front",
	"back"
};

const char *const LayerDirStrings[LayerDir_Count] = {
	"unknown",
	"forward",
	"backward"
};

const char *const PropGetTypeStrings[PropGetType_Count] = {
	"unknown",
	"number",
	"name"
};

const char *const SoundEffectStrings[] = {
	"unknown",
	"pitch",
	"pan"
};

SoundEffect SoundEffectFromString(const std::string &str)
{
	for (int i = 0; i < SoundEffect_Count; i++)
		if (str == SoundEffectStrings[i])
			return (SoundEffect)i;
	return SoundEffect_Unknown;
}

Key KeyFromString(const std::string &str)
{
	if (str.length() == 1)
	{
		char c = str[0];
		if (c >= '0' && c <= '9')
			return (Key)(Key_0 + (c - '0'));
		if (c >= 'A' && c <= 'Z')
			return (Key)(Key_A + (c - 'A'));
		return Key_Unknown;
	}

	if (str == "space")
		return Key_Space;
	if (str == "up arrow")
		return Key_Up;
	if (str == "down arrow")
		return Key_Down;
	if (str == "left arrow")
		return Key_Left;
	if (str == "right arrow")
		return Key_Right;

	if (str == "any")
		return Key_Any;

	return Key_Unknown;
}

const char *GetKeyName(Key key)
{
	static const char *Numbers[] = {
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
	};

	static const char *Letters[] = {
		"a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
		"k", "l", "m", "n", "o", "p", "q", "r", "s", "t",
		"u", "v", "w", "x", "y", "z"
	};

	if (key >= Key_0 && key <= Key_9)
		return Numbers[key - Key_0];

	if (key >= Key_A && key <= Key_Z)
		return Letters[key - Key_A];

	if (key == Key_Up)
		return "Up";
	if (key == Key_Down)
		return "Down";
	if (key == Key_Left)
		return "Left";
	if (key == Key_Right)
		return "Right";

	if (key == Key_Space)
		return "Space";

	if (key == Key_Any)
		return "Any";

	return "unknown";
}

const char *const ListenValueTypeStrings[ListenValueType_Count] = {
	"unknown",
	"loudness",
	"timer"
};

ListenValueType ListenValueTypeFromString(const std::string &str)
{
	for (int i = 0; i < ListenValueType_Count; i++)
		if (str == ListenValueTypeStrings[i])
			return (ListenValueType)i;
	return ListenValueType_Unknown;
}

const char *const StopModeStrings[StopMode_Count] = {
	"unknown",
	"all",
	"this script",
	"other scripts in sprite"
};

StopMode StopModeFromString(const std::string &str)
{
	for (int i = 0; i < StopMode_Count; i++)
		if (str == StopModeStrings[i])
			return (StopMode)i;
	return StopMode_Unknown;
}

const char *const DragModeStrings[DragMode_Count] = {
	"unknown",
	"draggable",
	"not draggable"
};

DragMode DragModeFromString(const std::string &str)
{
	for (int i = 0; i < DragMode_Count; i++)
		if (str == DragModeStrings[i])
			return (DragMode)i;
	return DragMode_Unknown;
}

const char *const PropertyTargetStrings[PropertyTarget_Count] = {
	"unknown",
	"backdrop #",
	"backdrop name",
	"x position",
	"y position",
	"direction",
	"costume #",
	"costume name",
	"size",
	"volume",
	"variable" // actual value is stored in a PropertyOf node
};

const char *const DateFormatStrings[DateFormat_Count] = {
	"unknown",
	"YEAR",
	"MONTH",
	"DATE",
	"DAYOFWEEK",
	"HOUR",
	"MINUTE",
	"SECOND"
};

const char *const MathFuncStrings[MathFuncType_Count] = {
	"unknown",
	"abs",
	"floor",
	"ceiling",
	"sqrt",
	"sin",
	"cos",
	"tan",
	"asin",
	"acos",
	"atan",
	"ln",
	"log",
	"e ^",
	"10 ^"
};

MathFuncType MathFuncFromString(const std::string &str)
{
	for (int i = 0; i < MathFuncType_Count; i++)
		if (str == MathFuncStrings[i])
			return (MathFuncType)i;
	return MathFuncType_Unknown;
}

const char *const VideoStateStrings[VideoState_Count] = {
	"unknown",
	"ON",
	"OFF",
	"ON_FLIPPED"
};

const char *const MonitorModeStrings[MonitorMode_Count] = {
	"unknown",
	"DEFAULT",
	"LARGE",
	"SLIDER",
	"LIST"
};

const char *AstTypeString(AstType type)
{
	switch (type)
	{
	default: return "<unknown>";
	case Ast_ASTNode: return "ASTNode";
	case Ast_SymbolName: return "SymbolName";
	case Ast_Expression: return "Expression";
	case Ast_Consteval: return "Consteval";
	case Ast_Constexpr: return "Constexpr";
	case Ast_XPos: return "XPos";
	case Ast_YPos: return "YPos";
	case Ast_Direction: return "Direction";
	case Ast_CurrentCostume: return "CurrentCostume";
	case Ast_CurrentBackdrop: return "CurrentBackdrop";
	case Ast_Size: return "Size";
	case Ast_Volume: return "Volume";
	case Ast_Touching: return "Touching";
	case Ast_TouchingColor: return "TouchingColor";
	case Ast_ColorTouching: return "ColorTouching";
	case Ast_DistanceTo: return "DistanceTo";
	case Ast_Answer: return "Answer";
	case Ast_KeyPressed: return "KeyPressed";
	case Ast_MouseDown: return "MouseDown";
	case Ast_MouseX: return "MouseX";
	case Ast_MouseY: return "MouseY";
	case Ast_Loudness: return "Loudness";
	case Ast_TimerValue: return "TimerValue";
	case Ast_PropertyOf: return "PropertyOf";
	case Ast_CurrentDate: return "CurrentDate";
	case Ast_DaysSince2000: return "DaysSince2000";
	case Ast_Username: return "Username";
	case Ast_Add: return "Add";
	case Ast_Sub: return "Sub";
	case Ast_Mul: return "Mul";
	case Ast_Div: return "Div";
	case Ast_Random: return "Random";
	case Ast_Less: return "Less";
	case Ast_Equal: return "Equal";
	case Ast_LogicalAnd: return "LogicalAnd";
	case Ast_LogicalOr: return "LogicalOr";
	case Ast_LogicalNot: return "LogicalNot";
	case Ast_Concat: return "Concat";
	case Ast_CharAt: return "CharAt";
	case Ast_StringLength: return "StringLength";
	case Ast_StringContains: return "StringContains";
	case Ast_Mod: return "Mod";
	case Ast_Round: return "Round";
	case Ast_MathFunc: return "MathFunc";
	case Ast_VariableExpr: return "VariableExpr";
	case Ast_BroadcastExpr: return "BroadcastExpr";
	case Ast_ListExpr: return "ListExpr";
	case Ast_ListAccess: return "ListAccess";
	case Ast_IndexOf: return "IndexOf";
	case Ast_ListLength: return "ListLength";
	case Ast_ListContains: return "ListContains";
	case Ast_Statement: return "Statement";
	case Ast_StatementList: return "StatementList";
	case Ast_MoveSteps: return "MoveSteps";
	case Ast_TurnDegrees: return "TurnDegrees";
	case Ast_TurnNegDegrees: return "TurnNegDegrees";
	case Ast_Goto: return "Goto";
	case Ast_GotoXY: return "GotoXY";
	case Ast_Glide:	return "Glide";
	case Ast_GlideXY: return "GlideXY";
	case Ast_PointDir: return "PointDir";
	case Ast_PointTowards: return "PointTowards";
	case Ast_ChangeX: return "ChangeX";
	case Ast_SetX: return "SetX";
	case Ast_ChangeY: return "ChangeY";
	case Ast_SetY: return "SetY";
	case Ast_BounceIfOnEdge: return "BounceIfOnEdge";
	case Ast_SetRotationStyle: return "SetRotationStyle";
	case Ast_SayForSecs: return "SayForSecs";
	case Ast_Say: return "Say";
	case Ast_ThinkForSecs: return "ThinkForSecs";
	case Ast_Think: return "Think";
	case Ast_SwitchCostume: return "SwitchCostume";
	case Ast_NextCostume: return "NextCostume";
	case Ast_SwitchBackdrop: return "SwitchBackdrop";
	case Ast_NextBackdrop: return "NextBackdrop";
	case Ast_ChangeSize: return "ChangeSize";
	case Ast_SetSize: return "SetSize";
	case Ast_ChangeGraphicEffect: return "ChangeGraphicEffect";
	case Ast_SetGraphicEffect: return "SetGraphicEffect";
	case Ast_ClearGraphicEffects: return "ClearGraphicEffects";
	case Ast_ShowSprite: return "ShowSprite";
	case Ast_HideSprite: return "HideSprite";
	case Ast_GotoLayer: return "GotoLayer";
	case Ast_MoveLayer: return "MoveLayer";
	case Ast_PlaySoundUntilDone: return "PlaySoundUntilDone";
	case Ast_StartSound: return "StartSound";
	case Ast_StopAllSounds: return "StopAllSounds";
	case Ast_ChangeSoundEffect: return "ChangeSoundEffect";
	case Ast_SetSoundEffect: return "SetSoundEffect";
	case Ast_ClearSoundEffects: return "ClearSoundEffects";
	case Ast_ChangeVolume: return "ChangeVolume";
	case Ast_SetVolume: return "SetVolume";
	case Ast_OnFlagClicked: return "OnFlagClicked";
	case Ast_OnKeyPressed: return "OnKeyPressed";
	case Ast_OnSpriteClicked: return "OnSpriteClicked";
	case Ast_OnBackdropSwitch: return "OnBackdropSwitch";
	case Ast_OnGreaterThan: return "OnGreaterThan";
	case Ast_OnEvent: return "OnEvent";
	case Ast_Broadcast: return "Broadcast";
	case Ast_BroadcastAndWait: return "BroadcastAndWait";
	case Ast_WaitSecs: return "WaitSecs";
	case Ast_Repeat: return "Repeat";
	case Ast_Forever: return "Forever";
	case Ast_If: return "If";
	case Ast_IfElse: return "IfElse";
	case Ast_WaitUntil: return "WaitUntil";
	case Ast_RepeatUntil: return "RepeatUntil";
	case Ast_Stop: return "Stop";
	case Ast_CloneStart: return "CloneStart";
	case Ast_CreateClone: return "CreateClone";
	case Ast_DeleteClone: return "DeleteClone";
	case Ast_AskAndWait: return "AskAndWait";
	case Ast_SetDragMode: return "SetDragMode";
	case Ast_ResetTimer: return "ResetTimer";
	case Ast_SetVariable: return "SetVariable";
	case Ast_ChangeVariable: return "ChangeVariable";
	case Ast_ShowVariable: return "ShowVariable";
	case Ast_HideVariable: return "HideVariable";
	case Ast_AppendToList: return "AppendToList";
	case Ast_DeleteFromList: return "DeleteFromList";
	case Ast_DeleteAllList: return "DeleteAllList";
	case Ast_InsertInList: return "InsertInList";
	case Ast_ReplaceInList: return "ReplaceInList";
	case Ast_ShowList: return "ShowList";
	case Ast_HideList: return "HideList";
	case Ast_ProcProto: return "ProcProto";
	case Ast_DefineProc: return "DefineProc";
	case Ast_Call: return "Call";
	case Ast_VariableDef: return "VariableDef";
	case Ast_VariableDefList: return "VariableDefList";
	case Ast_ListDef: return "ListDef";
	case Ast_ListDefList: return "ListDefList";
	case Ast_StatementListList: return "StatementListList";
	case Ast_SpriteDef: return "SpriteDef";
	case Ast_SpriteDefList: return "SpriteDefList";
	case Ast_StageDef: return "StageDef";
	case Ast_ValMonitor: return "ValMonitor";
	case Ast_ValMonitorList: return "ValMonitorList";
	case Ast_Program: return "Program";
	}
}
