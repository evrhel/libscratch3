#pragma once

#include "expression.hpp"

// TODO: should this be an expression?
struct Reporter : public Expression
{
	AST_IMPL(Reporter, Expression);
};

// Reporter for (goto)
struct GotoReporter : public Reporter
{
	REPORTER_IMPL(GotoReporter, Reporter, TO);
	AST_ACCEPTOR;
};

// Reporter for (glide () secs to ())
struct GlideReporter : public Reporter
{
	REPORTER_IMPL(GlideReporter, Reporter, TO);
	AST_ACCEPTOR;
};

// Reporter for (point towards ())
struct PointTowardsReporter : public Reporter
{
	REPORTER_IMPL(PointTowardsReporter, Reporter, TOWARDS);
	AST_ACCEPTOR;
};

// Reporter for (switch costume to ())
struct CostumeReporter : public Reporter
{
	REPORTER_IMPL(CostumeReporter, Reporter, COSTUME);
	AST_ACCEPTOR;
};

// Reporter for (switch backdrop to ())
// or (switch backdrop to () and wait)
struct BackdropReporter : public Reporter
{
	REPORTER_IMPL(BackdropReporter, Reporter, BACKDROP);
	AST_ACCEPTOR;
};

// Reporter for (play sound () until done) or
// (play sound ())
struct SoundReporter : public Reporter
{
	REPORTER_IMPL(SoundReporter, Reporter, SOUND_MENU);
	AST_ACCEPTOR;
};

// Reporter for (broadcast ()) or (broadcast () and wait)
struct BroadcastReporter : public Reporter
{
	AST_IMPL(BroadcastReporter, Reporter);
	AST_ACCEPTOR;
};

// Reporter for (create clone of ())
struct CloneReporter : public Reporter
{
	REPORTER_IMPL(CloneReporter, Reporter, CLONE_OPTION);
	AST_ACCEPTOR;
};

// Reporter for <touching ()?>
struct TouchingReporter : public Reporter
{
	REPORTER_IMPL(TouchingReporter, Reporter, TOUCHINGOBJECTMENU);
	AST_ACCEPTOR;
};

// Reporter for (distance to ())
struct DistanceReporter : public Reporter
{
	REPORTER_IMPL(DistanceReporter, Reporter, DISTANCETOMENU);
	AST_ACCEPTOR;
};

// Reporter for <key () pressed?>
struct KeyReporter : public Reporter
{
	REPORTER_IMPL(KeyReporter, Reporter, KEY_OPTION);
	AST_ACCEPTOR;
};

// Reporter for (() of ())
struct PropertyOfReporter : public Reporter
{
	REPORTER_IMPL(PropertyOfReporter, Reporter, OBJECT);
	AST_ACCEPTOR;
};

// Reporter for string/number argument in a custom block
struct ArgReporterStringNumber : public Reporter
{
	REPORTER_IMPL(ArgReporterStringNumber, Reporter, VALUE);
	AST_ACCEPTOR;
};

// Reporter for boolean argument in a custom block
struct ArgReporterBoolean : public Reporter
{
	REPORTER_IMPL(ArgReporterBoolean, Reporter, VALUE);
	AST_ACCEPTOR;
};
