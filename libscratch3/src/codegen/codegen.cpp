#include "codegen.hpp"

#include <unordered_map>

class Environment : public Visitor
{
public:
	virtual void Visit(Constexpr *node)
	{
	}

	virtual void Visit(XPos *node)
	{
	}

	virtual void Visit(YPos *node)
	{
	}

	virtual void Visit(Direction *node)
	{
	}

	virtual void Visit(CurrentCostume *node)
	{
	}

	virtual void Visit(CurrentBackdrop *node) {}

	virtual void Visit(Size *node)
	{
	}

	virtual void Visit(Volume *node)
	{
	}

	virtual void Visit(Touching *node)
	{
	}

	virtual void Visit(TouchingColor *node)
	{
	}

	virtual void Visit(ColorTouching *node)
	{
	}

	virtual void Visit(DistanceTo *node)
	{
	}

	virtual void Visit(Answer *node)
	{
	}

	virtual void Visit(KeyPressed *node)
	{
	}

	virtual void Visit(MouseDown *node)
	{
	}

	virtual void Visit(MouseX *node)
	{
	}

	virtual void Visit(MouseY *node)
	{
	}

	virtual void Visit(Loudness *node)
	{
	}

	virtual void Visit(TimerValue *node)
	{
	}

	virtual void Visit(PropertyOf *node)
	{
	}

	virtual void Visit(CurrentDate *node)
	{
	}

	virtual void Visit(DaysSince2000 *node)
	{
	}

	virtual void Visit(Username *node)
	{
	}

	virtual void Visit(Add *node)
	{
	}

	virtual void Visit(Sub *node)
	{
	}

	virtual void Visit(Mul *node)
	{
	}

	virtual void Visit(Div *node)
	{
	}

	virtual void Visit(Random *node)
	{
	}

	virtual void Visit(Greater *node)
	{
	}

	virtual void Visit(Less *node)
	{
	}

	virtual void Visit(Equal *node)
	{
	}

	virtual void Visit(LogicalAnd *node)
	{
	}

	virtual void Visit(LogicalOr *node)
	{
	}

	virtual void Visit(LogicalNot *node)
	{
	}

	virtual void Visit(Concat *node)
	{
	}

	virtual void Visit(CharAt *node)
	{
	}

	virtual void Visit(StringLength *node)
	{
	}

	virtual void Visit(StringContains *node)
	{
	}

	virtual void Visit(Mod *node)
	{
	}

	virtual void Visit(Round *node)
	{
	}

	virtual void Visit(MathFunc *node)
	{
	}

	virtual void Visit(VariableExpr *node)
	{
	}

	virtual void Visit(BroadcastExpr *node)
	{
	}

	virtual void Visit(ListExpr *node)
	{
	}

	virtual void Visit(ListAccess *node)
	{
	}

	virtual void Visit(IndexOf *node)
	{
	}

	virtual void Visit(ListLength *node)
	{
	}

	virtual void Visit(ListContains *node)
	{
	}

	virtual void Visit(StatementList *node) {}

	virtual void Visit(MoveSteps *node)
	{
	}

	virtual void Visit(TurnDegrees *node)
	{
	}

	virtual void Visit(TurnNegDegrees *node)
	{
	}

	virtual void Visit(Goto *node)
	{
	}

	virtual void Visit(GotoXY *node)
	{
	}

	virtual void Visit(Glide *node)
	{
	}

	virtual void Visit(GlideXY *node)
	{
	}

	virtual void Visit(PointDir *node)
	{
	}

	virtual void Visit(PointTowards *node)
	{
	}

	virtual void Visit(ChangeX *node)
	{
	}

	virtual void Visit(SetX *node)
	{
	}

	virtual void Visit(ChangeY *node)
	{
	}

	virtual void Visit(SetY *node)
	{
	}

	virtual void Visit(BounceIfOnEdge *node)
	{
	}

	virtual void Visit(SetRotationStyle *node)
	{
	}

	virtual void Visit(SayForSecs *node)
	{
	}

	virtual void Visit(Say *node)
	{
	}

	virtual void Visit(ThinkForSecs *node)
	{
	}

	virtual void Visit(Think *node)
	{
	}

	virtual void Visit(SwitchCostume *node)
	{
	}

	virtual void Visit(NextCostume *node)
	{
	}

	virtual void Visit(SwitchBackdrop *node)
	{
	}

	virtual void Visit(SwitchBackdropAndWait *node)
	{
	}

	virtual void Visit(NextBackdrop *node)
	{
	}

	virtual void Visit(ChangeSize *node)
	{
	}

	virtual void Visit(SetSize *node)
	{
	}

	virtual void Visit(ChangeGraphicEffect *node)
	{
	}

	virtual void Visit(SetGraphicEffect *node)
	{
	}

	virtual void Visit(ClearGraphicEffects *node)
	{
	}

	virtual void Visit(ShowSprite *node)
	{
	}

	virtual void Visit(HideSprite *node)
	{
	}

	virtual void Visit(GotoLayer *node)
	{
	}

	virtual void Visit(MoveLayer *node)
	{
	}

	virtual void Visit(PlaySoundUntilDone *node) {}
	virtual void Visit(StartSound *node) {}

	virtual void Visit(StopAllSounds *node) {}
	virtual void Visit(ChangeSoundEffect *node) {}
	virtual void Visit(SetSoundEffect *node) {}
	virtual void Visit(ClearSoundEffects *node) {}

	virtual void Visit(ChangeVolume *node)
	{
	}

	virtual void Visit(SetVolume *node)
	{
	}

	virtual void Visit(OnFlagClicked *node) {}
	virtual void Visit(OnKeyPressed *node) {}
	virtual void Visit(OnSpriteClicked *node) {}
	virtual void Visit(OnStageClicked *node) {}
	virtual void Visit(OnBackdropSwitch *node) {}
	virtual void Visit(OnGreaterThan *node) {}
	virtual void Visit(OnEvent *node) {}

	virtual void Visit(Broadcast *node)
	{
	}

	virtual void Visit(BroadcastAndWait *node)
	{
	}

	virtual void Visit(WaitSecs *node)
	{
	}

	virtual void Visit(Repeat *node)
	{
	}

	virtual void Visit(Forever *node)
	{
	}

	virtual void Visit(If *node)
	{
	}

	virtual void Visit(IfElse *node)
	{
	}

	virtual void Visit(WaitUntil *node)
	{
	}

	virtual void Visit(RepeatUntil *node)
	{
	}

	virtual void Visit(Stop *node)
	{
	}

	virtual void Visit(CloneStart *node)
	{
	}

	virtual void Visit(CreateClone *node)
	{
	}

	virtual void Visit(DeleteClone *node)
	{
	}

	virtual void Visit(AskAndWait *node)
	{

	}

	virtual void Visit(SetDragMode *node)
	{
		
	}

	virtual void Visit(ResetTimer *node)
	{
		
	}

	virtual void Visit(SetVariable *node)
	{

	}

	virtual void Visit(ChangeVariable *node)
	{

	}

	virtual void Visit(ShowVariable *node)
	{
	}

	virtual void Visit(HideVariable *node)
	{
	}

	virtual void Visit(AppendToList *node)
	{

	}

	virtual void Visit(DeleteFromList *node)
	{

	}

	virtual void Visit(DeleteAllList *node)
	{

	}

	virtual void Visit(InsertInList *node)
	{

	}

	virtual void Visit(ReplaceInList *node)
	{

	}

	virtual void Visit(ShowList *node)
	{
	}

	virtual void Visit(HideList *node)
	{
	}

	virtual void Visit(ProcProto *node) {}
	virtual void Visit(DefineProc *node) {}
	virtual void Visit(Call *node) {}

	//
	/////////////////////////////////////////////////////////////////
	// Reporters
	//

	virtual void Visit(GotoReporter *node)
	{
	}

	virtual void Visit(GlideReporter *node)
	{
	}

	virtual void Visit(PointTowardsReporter *node)
	{
	}

	virtual void Visit(CostumeReporter *node)
	{
	}

	virtual void Visit(BackdropReporter *node)
	{
	}

	virtual void Visit(SoundReporter *node)
	{
	}

	virtual void Visit(BroadcastReporter *node)
	{
	}

	virtual void Visit(CloneReporter *node)
	{
	}

	virtual void Visit(TouchingReporter *node)
	{
	}

	virtual void Visit(DistanceReporter *node)
	{
	}

	virtual void Visit(KeyReporter *node)
	{
	}

	virtual void Visit(PropertyOfReporter *node)
	{
	}

	virtual void Visit(ArgReporterStringNumber *node)
	{
	}

	virtual void Visit(ArgReporterBoolean *node)
	{
	}

};
