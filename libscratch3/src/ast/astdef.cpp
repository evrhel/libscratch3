#include "astdef.hpp"

GraphicEffect GraphicEffectFromString(const std::string &str)
{
	if (str == "COLOR") return GraphicEffect_Color;
	if (str == "FISHEYE") return GraphicEffect_Fisheye;
	if (str == "WHIRL") return GraphicEffect_Whirl;
	if (str == "PIXELATE") return GraphicEffect_Pixelate;
	if (str == "MOSAIC") return GraphicEffect_Mosaic;
	if (str == "BRIGHTNESS") return GraphicEffect_Brightness;
	if (str == "GHOST") return GraphicEffect_Ghost;
	return GraphicEffect_Unknown;
}

const char *GraphicEffectToString(GraphicEffect effect)
{
	switch (effect)
	{
	default: return "<unknown>";
	case GraphicEffect_Color: return "COLOR";
	case GraphicEffect_Fisheye: return "FISHEYE";
	case GraphicEffect_Whirl: return "WHIRL";
	case GraphicEffect_Pixelate: return "PIXELATE";
	case GraphicEffect_Mosaic: return "MOSAIC";
	case GraphicEffect_Brightness: return "BRIGHTNESS";
	case GraphicEffect_Ghost: return "GHOST";
	}
}

const char *AstTypeString(AstType type)
{
	switch (type)
	{
	default: return "<unknown>";
	case Ast_ASTNode: return "ASTNode";
	case Ast_Unevaluated: return "Unevaluated";
	case Ast_SymbolName: return "SymbolName";
	case Ast_Expression: return "Expression";
	case Ast_Consteval: return "Consteval";
	case Ast_ExpressionList: return "ExpressionList";
	case Ast_Constexpr: return "Constexpr";
	case Ast_Number: return "Number";
	case Ast_PositiveNumber: return "PositiveNumber";
	case Ast_PositiveInt: return "PositiveInt";
	case Ast_Int: return "Int";
	case Ast_Angle: return "Angle";
	case Ast_Color: return "Color";
	case Ast_String: return "String";
	case Ast_True: return "True";
	case Ast_False: return "False";
	case Ast_Null: return "Null";
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
