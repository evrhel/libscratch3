#pragma once

#include "astdef.hpp"

class Visitor
{
public:
	/////////////////////////////////////////////////////////////////
	// Expressions
	//

	inline virtual void Visit(ExpressionList *node) {}
	inline virtual void Visit(Number *node) {}
	inline virtual void Visit(PositiveNumber *node) {}
	inline virtual void Visit(PositiveInt *node) {}
	inline virtual void Visit(Int *node) {}
	inline virtual void Visit(Angle *node) {}
	inline virtual void Visit(Color *node) {}
	inline virtual void Visit(String *node) {}
	inline virtual void Visit(True *node) {}
	inline virtual void Visit(False *node) {}
	inline virtual void Visit(None *node) {}
	inline virtual void Visit(XPos *node) {}
	inline virtual void Visit(YPos *node) {}
	inline virtual void Visit(Direction *node) {}
	inline virtual void Visit(CurrentCostume *node) {}
	inline virtual void Visit(CurrentBackdrop *node) {}
	inline virtual void Visit(Size *node) {}
	inline virtual void Visit(Volume *node) {}
	inline virtual void Visit(Touching *node) {}
	inline virtual void Visit(TouchingColor *node) {}
	inline virtual void Visit(ColorTouching *node) {}
	inline virtual void Visit(DistanceTo *node) {}
	inline virtual void Visit(Answer *node) {}
	inline virtual void Visit(KeyPressed *node) {}
	inline virtual void Visit(MouseDown *node) {}
	inline virtual void Visit(MouseX *node) {}
	inline virtual void Visit(MouseY *node) {}
	inline virtual void Visit(Loudness *node) {}
	inline virtual void Visit(TimerValue *node) {}
	inline virtual void Visit(PropertyOf *node) {}
	inline virtual void Visit(CurrentDate *node) {}
	inline virtual void Visit(DaysSince2000 *node) {}
	inline virtual void Visit(Username *node) {}
	inline virtual void Visit(Add *node) {}
	inline virtual void Visit(Sub *node) {}
	inline virtual void Visit(Mul *node) {}
	inline virtual void Visit(Div *node) {}
	inline virtual void Visit(Random *node) {}
	inline virtual void Visit(Greater *node) {}
	inline virtual void Visit(Less *node) {}
	inline virtual void Visit(Equal *node) {}
	inline virtual void Visit(LogicalAnd *node) {}
	inline virtual void Visit(LogicalOr *node) {}
	inline virtual void Visit(LogicalNot *node) {}
	inline virtual void Visit(Concat *node) {}
	inline virtual void Visit(CharAt *node) {}
	inline virtual void Visit(StringLength *node) {}
	inline virtual void Visit(StringContains *node) {}
	inline virtual void Visit(Mod *node) {}
	inline virtual void Visit(Round *node) {}
	inline virtual void Visit(MathFunc *node) {}
	inline virtual void Visit(VariableExpr *node) {}
	inline virtual void Visit(BroadcastExpr *node) {}
	inline virtual void Visit(ListExpr *node) {}
	inline virtual void Visit(ListAccess *node) {}
	inline virtual void Visit(IndexOf *node) {}
	inline virtual void Visit(ListLength *node) {}
	inline virtual void Visit(ListContains *node) {}

	//
	/////////////////////////////////////////////////////////////////
	// Statements
	//

	inline virtual void Visit(StatementList *node) {}
	inline virtual void Visit(MoveSteps *node) {}
	inline virtual void Visit(TurnDegrees *node) {}
	inline virtual void Visit(TurnNegDegrees *node) {}
	inline virtual void Visit(Goto *node) {}
	inline virtual void Visit(GotoXY *node) {}
	inline virtual void Visit(Glide *node) {}
	inline virtual void Visit(GlideXY *node) {}
	inline virtual void Visit(PointDir *node) {}
	inline virtual void Visit(PointTowards *node) {}
	inline virtual void Visit(ChangeX *node) {}
	inline virtual void Visit(SetX *node) {}
	inline virtual void Visit(ChangeY *node) {}
	inline virtual void Visit(SetY *node) {}
	inline virtual void Visit(BounceIfOnEdge *node) {}
	inline virtual void Visit(SetRotationStyle *node) {}
	inline virtual void Visit(SayForSecs *node) {}
	inline virtual void Visit(Say *node) {}
	inline virtual void Visit(ThinkForSecs *node) {}
	inline virtual void Visit(Think *node) {}
	inline virtual void Visit(SwitchCostume *node) {}
	inline virtual void Visit(NextCostume *node) {}
	inline virtual void Visit(SwitchBackdrop *node) {}
	inline virtual void Visit(SwitchBackdropAndWait *node) {}
	inline virtual void Visit(NextBackdrop *node) {}
	inline virtual void Visit(ChangeSize *node) {}
	inline virtual void Visit(SetSize *node) {}
	inline virtual void Visit(ChangeGraphicEffect *node) {}
	inline virtual void Visit(SetGraphicEffect *node) {}
	inline virtual void Visit(ClearGraphicEffects *node) {}
	inline virtual void Visit(ShowSprite *node) {}
	inline virtual void Visit(HideSprite *node) {}
	inline virtual void Visit(GotoLayer *node) {}
	inline virtual void Visit(MoveLayer *node) {}
	inline virtual void Visit(PlaySoundUntilDone *node) {}
	inline virtual void Visit(StartSound *node) {}
	inline virtual void Visit(StopAllSounds *node) {}
	inline virtual void Visit(ChangeSoundEffect *node) {}
	inline virtual void Visit(SetSoundEffect *node) {}
	inline virtual void Visit(ClearSoundEffects *node) {}
	inline virtual void Visit(ChangeVolume *node) {}
	inline virtual void Visit(SetVolume *node) {}
	inline virtual void Visit(OnFlagClicked *node) {}
	inline virtual void Visit(OnKeyPressed *node) {}
	inline virtual void Visit(OnSpriteClicked *node) {}
	inline virtual void Visit(OnStageClicked *node) {}
	inline virtual void Visit(OnBackdropSwitch *node) {}
	inline virtual void Visit(OnGreaterThan *node) {}
	inline virtual void Visit(OnEvent *node) {}
	inline virtual void Visit(Broadcast *node) {}
	inline virtual void Visit(BroadcastAndWait *node) {}
	inline virtual void Visit(WaitSecs *node) {}
	inline virtual void Visit(Repeat *node) {}
	inline virtual void Visit(Forever *node) {}
	inline virtual void Visit(If *node) {}
	inline virtual void Visit(IfElse *node) {}
	inline virtual void Visit(WaitUntil *node) {}
	inline virtual void Visit(RepeatUntil *node) {}
	inline virtual void Visit(Stop *node) {}
	inline virtual void Visit(CloneStart *node) {}
	inline virtual void Visit(CreateClone *node) {}
	inline virtual void Visit(DeleteClone *node) {}
	inline virtual void Visit(AskAndWait *node) {}
	inline virtual void Visit(SetDragMode *node) {}
	inline virtual void Visit(ResetTimer *node) {}
	inline virtual void Visit(SetVariable *node) {}
	inline virtual void Visit(ChangeVariable *node) {}
	inline virtual void Visit(ShowVariable *node) {}
	inline virtual void Visit(HideVariable *node) {}
	inline virtual void Visit(AppendToList *node) {}
	inline virtual void Visit(DeleteFromList *node) {}
	inline virtual void Visit(DeleteAllList *node) {}
	inline virtual void Visit(InsertInList *node) {}
	inline virtual void Visit(ReplaceInList *node) {}
	inline virtual void Visit(ShowList *node) {}
	inline virtual void Visit(HideList *node) {}
	inline virtual void Visit(ProcProto *node) {}
	inline virtual void Visit(DefineProc *node) {}
	inline virtual void Visit(Call *node) {}

	//
	/////////////////////////////////////////////////////////////////
	// Reporters
	//

	inline virtual void Visit(GotoReporter *node) {}
	inline virtual void Visit(GlideReporter *node) {}
	inline virtual void Visit(PointTowardsReporter *node) {}
	inline virtual void Visit(CostumeReporter *node) {}
	inline virtual void Visit(BackdropReporter *node) {}
	inline virtual void Visit(SoundReporter *node) {}
	inline virtual void Visit(BroadcastReporter *node) {}
	inline virtual void Visit(CloneReporter *node) {}
	inline virtual void Visit(TouchingReporter *node) {}
	inline virtual void Visit(DistanceReporter *node) {}
	inline virtual void Visit(KeyReporter *node) {}
	inline virtual void Visit(PropertyOfReporter *node) {}
	inline virtual void Visit(ArgReporterStringNumber *node) {}
	inline virtual void Visit(ArgReporterBoolean *node) {}

	//
	/////////////////////////////////////////////////////////////////
	// Program Components
	//

	inline virtual void Visit(VariableDef *node) {}
	inline virtual void Visit(VariableDefList *node) {}
	inline virtual void Visit(ListDef *node) {}
	inline virtual void Visit(ListDefList *node) {}
	inline virtual void Visit(StatementListList *node) {}
	inline virtual void Visit(SpriteDef *node) {}
	inline virtual void Visit(SpriteDefList *node) {}
	inline virtual void Visit(StageDef *node) {}
	inline virtual void Visit(ValMonitor *node) {}
	inline virtual void Visit(ValMonitorList *node) {}
	inline virtual void Visit(Program *node) {}
private:
};

Visitor *CreateDumpVisitor();
