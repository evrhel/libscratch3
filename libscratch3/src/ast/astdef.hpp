#pragma once

// Definitions for AST nodes representing a Scratch program
// Loosely based on the Scratch 3.0 format (sb3)
// See: https://en.scratch-wiki.info/wiki/Scratch_File_Format

#include <string>

class Visitor;

/////////////////////////////////////////////////////////////////////////////////
// AST Node Types
//

struct ASTNode;

struct SymbolName;

/////////////////////////////////////////////////////////////////////////////////
// Expressions
//

struct Expression;
struct Consteval;
struct Constexpr;

// Motion Expressions
struct XPos;
struct YPos;
struct Direction;

// Looks Expressions
struct CurrentCostume;
struct CurrentBackdrop;
struct Size;

// Sound Expressions
struct Volume;

// Events Expressions
// (none)

// Sensing Expressions
struct Touching;
struct TouchingColor;
struct ColorTouching;
struct DistanceTo;
struct Answer;
struct KeyPressed;
struct MouseDown;
struct MouseX;
struct MouseY;
struct Loudness;
struct TimerValue;
struct PropertyOf;
struct CurrentDate;
struct DaysSince2000;
struct Username;

// Operators Expressions
struct Add;
struct Sub;
struct Mul;
struct Div;
struct Neg;
struct Random;
struct Greater;
struct Less;
struct Equal;
struct LogicalAnd;
struct LogicalOr;
struct LogicalNot;
struct Concat;
struct CharAt;
struct StringLength;
struct StringContains;
struct Mod;
struct Round;
struct MathFunc;

// Variables Expressions
struct VariableExpr;
struct BroadcastExpr;
struct ListExpr;
struct ListAccess;
struct IndexOf;
struct ListLength;
struct ListContains;

struct PenMenuColorProperty;

//
/////////////////////////////////////////////////////////////////////////////////
// Internal Reporters
// 
// Reporters are internal expressions used in Scratch in the form of dropdown
// menus that contain a list of options.
//

struct Reporter;

struct GotoReporter;
struct GlideReporter;
struct PointTowardsReporter;
struct CostumeReporter;
struct BackdropReporter;
struct SoundReporter;
struct BroadcastReporter;
struct CloneReporter;
struct TouchingReporter;
struct DistanceReporter;
struct KeyReporter;
struct PropertyOfReporter;
struct ArgReporterStringNumber;
struct ArgReporterBoolean;

//
/////////////////////////////////////////////////////////////////////////////////
// Statements
//

struct Statement;
struct StatementList;

// Motion Blocks
struct MoveSteps;
struct TurnDegrees;
struct TurnNegDegrees;
struct Goto;
struct GotoXY;
struct Glide;
struct GlideXY;
struct PointDir;
struct PointTowards;
struct ChangeX;
struct SetX;
struct ChangeY;
struct SetY;
struct BounceIfOnEdge;
struct SetRotationStyle;

// Looks Blocks
struct SayForSecs;
struct Say;
struct ThinkForSecs;
struct Think;
struct SwitchCostume;
struct NextCostume;
struct SwitchBackdrop;
struct SwitchBackdropAndWait;
struct NextBackdrop;
struct ChangeSize;
struct SetSize;
struct ChangeGraphicEffect;
struct SetGraphicEffect;
struct ClearGraphicEffects;
struct ShowSprite;
struct HideSprite;
struct GotoLayer;
struct MoveLayer;

// Sound Blocks
struct PlaySoundUntilDone;
struct StartSound;
struct StopAllSounds;
struct ChangeSoundEffect;
struct SetSoundEffect;
struct ClearSoundEffects;
struct ChangeVolume;
struct SetVolume;

// Events Blocks
struct OnFlagClicked;
struct OnKeyPressed;
struct OnSpriteClicked;
struct OnStageClicked;
struct OnBackdropSwitch;
struct OnGreaterThan;
struct OnEvent;
struct Broadcast;
struct BroadcastAndWait;

// Control Blocks
struct WaitSecs;
struct Repeat;
struct Forever;
struct If;
struct IfElse;
struct WaitUntil;
struct RepeatUntil;
struct Stop;
struct CloneStart;
struct CreateClone;
struct DeleteClone;

// Sensing Blocks
struct AskAndWait;
struct SetDragMode;
struct ResetTimer;

// Operators Blocks
// (none)

// Variables Blocks
struct SetVariable;
struct ChangeVariable;
struct ShowVariable;
struct HideVariable;
struct AppendToList;
struct DeleteFromList;
struct DeleteAllList;
struct InsertInList;
struct ReplaceInList;
struct ShowList;
struct HideList;

// Custom Blocks
struct ProcProto;
struct DefineProc;
struct Call;

// Pen
struct PenClear;
struct PenStamp;
struct PenDown;
struct PenUp;
struct SetPenColor;
struct ChangePenProperty;
struct SetPenProperty;
struct ChangePenSize;
struct SetPenSize;

// Top Level
struct VariableDef;
struct VariableDefList;

struct ListDef;
struct ListDefList;

struct StatementListList;

struct CostumeDef;
struct CostumeDefList;

struct SoundDef;
struct SoundDefList;

struct SpriteDef;
struct SpriteDefList;
struct StageDef;

struct ValMonitor;
struct ValMonitorList;

struct Program;

//
/////////////////////////////////////////////////////////////////////////////////
// macros to reduce boilerplate code
//

#define AST_ACCEPTOR inline virtual void Accept(Visitor *v) override { v->Visit(this); }
#define AST_INPUT_SETTER(_Key, _Val) inline virtual bool SetInput(const std::string &_Key, ASTNode *_Val) override
#define AST_FIELD_SETTER(_Key, _Value, _Id) inline virtual bool SetField(const std::string & _Key , const std::string & _Value , const std::string & _Id ) override
#define AST_TOSTRING() virtual std::string ToString() const override
#define VISITOR_IMPL(_Deriver, _NodeT) void _Deriver :: Visit(_NodeT)
#define VISITOR(_NodeT) virtual void Visit( _NodeT *v) override
#define VISITOR_INTERFACE(_NodeT) virtual void Visit( _NodeT *node);

#define AST(_Type) Ast_##_Type

// annotation for AST node classes
#define AST_IMPL(_Type, _Parent) \
	static constexpr AstType TYPE = AST( _Type ); \
	inline _Type () { ASTNode::SetType(AST( _Type )); }

#define EXPR_IMPL(_Type, _Parent) \
	static constexpr AstType TYPE = AST( _Type ); \
	inline _Type () { ASTNode::SetType(AST( _Type )); }

#define REPORTER_IMPL(_Type, _Parent, _Key) \
	static constexpr AstType TYPE = AST( _Type ); \
	inline _Type () { ASTNode::SetType(AST( _Type )); } \
	AST_FIELD_SETTER(key, value, id) \
	{ \
		if ( #_Key == key ) \
		{ \
			if (!value.empty()) \
			{ \
				this->value = value; \
				return true; \
			} \
			return false; \
		} \
		return false; \
	} \
	std::string value;

//
/////////////////////////////////////////////////////////////////////////////////
// AST Node Types
//

enum AstType
{
	Ast_ASTNode,

	Ast_SymbolName,

	Ast_Expression,
	Ast_Consteval,
	Ast_Constexpr,

	Ast_XPos,
	Ast_YPos,
	Ast_Direction,

	Ast_CurrentCostume,
	Ast_CurrentBackdrop,
	Ast_Size,

	Ast_Volume,

	Ast_Touching,
	Ast_TouchingColor,
	Ast_ColorTouching,
	Ast_DistanceTo,
	Ast_Answer,
	Ast_KeyPressed,
	Ast_MouseDown,
	Ast_MouseX,
	Ast_MouseY,
	Ast_Loudness,
	Ast_TimerValue,
	Ast_PropertyOf,
	Ast_CurrentDate,
	Ast_DaysSince2000,
	Ast_Username,

	Ast_Add,
	Ast_Sub,
	Ast_Mul,
	Ast_Div,
	Ast_Neg,
	Ast_Random,
	Ast_Greater,
	Ast_Less,
	Ast_Equal,
	Ast_LogicalAnd,
	Ast_LogicalOr,
	Ast_LogicalNot,
	Ast_Concat,
	Ast_CharAt,
	Ast_StringLength,
	Ast_StringContains,
	Ast_Mod,
	Ast_Round,
	Ast_MathFunc,

	Ast_VariableExpr,
	Ast_BroadcastExpr,
	Ast_ListExpr,
	Ast_ListAccess,
	Ast_IndexOf,
	Ast_ListLength,
	Ast_ListContains,

	Ast_PenMenuColorProperty,

	Ast_Reporter,
	
	Ast_GotoReporter,
	Ast_GlideReporter,
	Ast_PointTowardsReporter,
	Ast_CostumeReporter,
	Ast_BackdropReporter,
	Ast_SoundReporter,
	Ast_BroadcastReporter,
	Ast_CloneReporter,
	Ast_TouchingReporter,
	Ast_DistanceReporter,
	Ast_KeyReporter,
	Ast_PropertyOfReporter,
	Ast_ArgReporterStringNumber,
	Ast_ArgReporterBoolean,

	Ast_Statement,
	Ast_StatementList,

	Ast_MoveSteps,
	Ast_TurnDegrees,
	Ast_TurnNegDegrees,
	Ast_Goto,
	Ast_GotoXY,
	Ast_Glide,
	Ast_GlideXY,
	Ast_PointDir,
	Ast_PointTowards,
	Ast_ChangeX,
	Ast_SetX,
	Ast_ChangeY,
	Ast_SetY,
	Ast_BounceIfOnEdge,
	Ast_SetRotationStyle,

	Ast_SayForSecs,
	Ast_Say,
	Ast_ThinkForSecs,
	Ast_Think,
	Ast_SwitchCostume,
	Ast_NextCostume,
	Ast_SwitchBackdrop,
	Ast_SwitchBackdropAndWait,
	Ast_NextBackdrop,
	Ast_ChangeSize,
	Ast_SetSize,
	Ast_ChangeGraphicEffect,
	Ast_SetGraphicEffect,
	Ast_ClearGraphicEffects,
	Ast_ShowSprite,
	Ast_HideSprite,
	Ast_GotoLayer,
	Ast_MoveLayer,

	Ast_PlaySoundUntilDone,
	Ast_StartSound,
	Ast_StopAllSounds,
	Ast_ChangeSoundEffect,
	Ast_SetSoundEffect,
	Ast_ClearSoundEffects,
	Ast_ChangeVolume,
	Ast_SetVolume,

	Ast_OnFlagClicked,
	Ast_OnKeyPressed,
	Ast_OnSpriteClicked,
	Ast_OnStageClicked,
	Ast_OnBackdropSwitch,
	Ast_OnGreaterThan,
	Ast_OnEvent,
	Ast_Broadcast,
	Ast_BroadcastAndWait,

	Ast_WaitSecs,
	Ast_Repeat,
	Ast_Forever,
	Ast_If,
	Ast_IfElse,
	Ast_WaitUntil,
	Ast_RepeatUntil,
	Ast_Stop,
	Ast_CloneStart,
	Ast_CreateClone,
	Ast_DeleteClone,

	Ast_AskAndWait,
	Ast_SetDragMode,
	Ast_ResetTimer,

	Ast_SetVariable,
	Ast_ChangeVariable,
	Ast_ShowVariable,
	Ast_HideVariable,
	Ast_AppendToList,
	Ast_DeleteFromList,
	Ast_DeleteAllList,
	Ast_InsertInList,
	Ast_ReplaceInList,
	Ast_ShowList,
	Ast_HideList,

	Ast_ProcProto,
	Ast_DefineProc,
	Ast_Call,

	Ast_PenClear,
	Ast_PenStamp,
	Ast_PenDown,
	Ast_PenUp,
	Ast_SetPenColor,
	Ast_ChangePenProperty,
	Ast_SetPenProperty,
	Ast_ChangePenSize,
	Ast_SetPenSize,

	Ast_VariableDef,
	Ast_VariableDefList,

	Ast_ListDef,
	Ast_ListDefList,

	Ast_StatementListList,

	Ast_CostumeDef,
	Ast_CostumeDefList,

	Ast_SoundDef,
	Ast_SoundDefList,

	Ast_SpriteDef,
	Ast_SpriteDefList,
	Ast_StageDef,

	Ast_ValMonitor,
	Ast_ValMonitorList,

	Ast_Program
};

//
/////////////////////////////////////////////////////////////////////////////////
// Constants
//

enum BlockType
{
	BlockType_Unknown = 0,

	BlockType_Shadow = 1,
	BlockType_NoShadow = 2,
	BlockType_ShadowObscured = 3,
	BlockType_Number = 4,
	BlockType_PositiveNumber = 5,
	BlockType_PositiveInt = 6,
	BlockType_Int = 7,
	BlockType_Angle = 8,
	BlockType_Color = 9,
	BlockType_String = 10,
	BlockType_Broadcast = 11,
	BlockType_Variable = 12,
	BlockType_List = 13
};

enum RotationStyle
{
	RotationStyle_Unknown,

	RotationStyle_LeftRight,
	RotationStyle_DontRotate,
	RotationStyle_AllAround,

	RotationStyle_Count
};

extern const char *const RotationStyleStrings[];
RotationStyle RotationStyleFromString(const std::string &str);

enum GraphicEffect
{
	GraphicEffect_Unknown,

	GraphicEffect_Color,
	GraphicEffect_Fisheye,
	GraphicEffect_Whirl,
	GraphicEffect_Pixelate,
	GraphicEffect_Mosaic,
	GraphicEffect_Brightness,
	GraphicEffect_Ghost,

	GraphicEffect_Count
};

extern const char *const GraphicEffectStrings[GraphicEffect_Count];
GraphicEffect GraphicEffectFromString(const std::string &str);

enum LayerType
{
	LayerType_Unknown,

	LayerType_Front,
	LayerType_Back,

	LayerType_Count
};

extern const char *const LayerTypeStrings[];

enum LayerDir
{
	LayerDir_Unknown,

	LayerDir_Forward,
	LayerDir_Backward,

	LayerDir_Count
};

extern const char *const LayerDirStrings[];

enum PropGetType
{
	PropGetType_Unknown,

	PropGetType_Number,
	PropGetType_Name,

	PropGetType_Count
};

extern const char *const PropGetTypeStrings[];

enum SoundEffect
{
	SoundEffect_Unknown,

	SoundEffect_Pitch,
	SoundEffect_Pan,

	SoundEffect_Count
};

extern const char *const SoundEffectStrings[];
SoundEffect SoundEffectFromString(const std::string &str);

enum ListenValueType
{
	ListenValueType_Unknown,

	ListenValueType_Loudness,
	ListenValueType_Timer,

	ListenValueType_Count
};

extern const char *const ListenValueTypeStrings[];
ListenValueType ListenValueTypeFromString(const std::string &str);

enum StopMode
{
	StopMode_Unknown,

	StopMode_All,
	StopMode_ThisScript,
	StopMode_OtherScriptsInSprite,

	StopMode_Count
};

extern const char *const StopModeStrings[];
StopMode StopModeFromString(const std::string &str);

enum DragMode
{
	DragMode_Unknown,

	DragMode_Draggable,
	DragMode_NotDraggable,

	DragMode_Count
};

extern const char *const DragModeStrings[];
DragMode DragModeFromString(const std::string &str);

enum PropertyTarget
{
	PropertyTarget_Unknown,

	PropertyTarget_BackdropNumber,
	PropertyTarget_BackdropName,

	PropertyTarget_XPosition,
	PropertyTarget_YPosition,
	PropertyTarget_Direction,
	PropertyTarget_CostumeNumber,
	PropertyTarget_CostumeName,
	PropertyTarget_Size,
	PropertyTarget_Volume,

	PropertyTarget_Variable,

	PropertyTarget_Count
};

extern const char *const PropertyTargetStrings[];

enum DateFormat
{
	DateFormat_Unknown,

	DateFormat_Year,
	DateFormat_Month,
	DateFormat_Date,
	DateFormat_DayOfWeek,
	DateFormat_Hour,
	DateFormat_Minute,
	DateFormat_Second,

	DateFormat_Count
};

extern const char *const DateFormatStrings[];

enum MathFuncType
{
	MathFuncType_Unknown,

	MathFuncType_Abs,
	MathFuncType_Floor,
	MathFuncType_Ceil,
	MathFuncType_Sqrt,
	MathFuncType_Sin,
	MathFuncType_Cos,
	MathFuncType_Tan,
	MathFuncType_Asin,
	MathFuncType_Acos,
	MathFuncType_Atan,
	MathFuncType_Ln,
	MathFuncType_Log,
	MathFuncType_Exp,
	MathFuncType_Exp10,

	MathFuncType_Count
};

extern const char *const MathFuncStrings[];
MathFuncType MathFuncFromString(const std::string &str);

enum VideoState
{
	VideoState_Unknown,

	VideoState_On,
	VideoState_Off,
	VideoState_OnFlipped,

	VideoState_Count
};

extern const char *const VideoStateStrings[];

enum MonitorMode
{
	MonitorMode_Unknown,

	MonitorMode_Default,
	MonitorMode_Large,
	MonitorMode_Slider,
	MonitorMode_List,

	MonitorMode_Count
};

extern const char *const MonitorModeStrings[];

enum PenProperty
{
	PenProperty_Unknown,

	PenProperty_Color,
	PenProperty_Saturation,
	PenProperty_Brightness,
	PenProperty_Transparency,

	PenProperty_Count
};

extern const char *const PenPropertyStrings[];
PenProperty PenPropertyFromString(const std::string &str);

//
/////////////////////////////////////////////////////////////////////////////////
// Utility Functions
//

//! \brief Get the string representation of an AST type.
//! 
//! \param type The AST type.
//! 
//! \return The string representation.
const char *AstTypeString(AstType type);

//
/////////////////////////////////////////////////////////////////////////////////
