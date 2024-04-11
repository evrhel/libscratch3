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
	AST_IMPL(GotoReporter, Reporter);
	AST_ACCEPTOR;
};

// Reporter for (glide () secs to ())
struct GlideReporter : public Reporter
{
	AST_IMPL(GlideReporter, Reporter);
	AST_ACCEPTOR;
};

// Reporter for (point towards ())
struct PointTowardsReporter : public Reporter
{
	AST_IMPL(PointTowardsReporter, Reporter);
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
	AST_IMPL(SoundReporter, Reporter);
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
	AST_IMPL(CloneReporter, Reporter);
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
	AST_IMPL(DistanceReporter, Reporter);
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
