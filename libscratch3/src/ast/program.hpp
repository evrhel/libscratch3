#pragma once

#include "statement.hpp"

// variable definition in a sprite/stage
struct VariableDef : public ASTNode
{
	AST_IMPL(VariableDef, ASTNode);
	AST_ACCEPTOR;

	std::string id, name;
	AutoRelease<Constexpr> value;
};

// list of variable definitions
struct VariableDefList : public ASTNode
{
	AST_IMPL(VariableDefList, ASTNode);
	AST_ACCEPTOR;

	inline const AutoRelease<VariableDef> &Find(const std::string &id) const
	{
		for (auto &v : variables)
			if (v->id == id)
				return v;
		return 0;
	}

	std::vector<AutoRelease<VariableDef>> variables;
};

// list definition in a sprite/stage
struct ListDef : public ASTNode
{
	AST_IMPL(ListDef, ASTNode);
	AST_ACCEPTOR;

	std::string id, name;
	std::vector<AutoRelease<Constexpr>> value;
};

// list of list definitions
struct ListDefList : public ASTNode
{
	AST_IMPL(ListDefList, ASTNode);
	AST_ACCEPTOR;

	inline const AutoRelease<ListDef> &Find(const std::string &id) const
	{
		for (auto &l : lists)
			if (l->id == id)
				return l;
		return nullptr;
	}

	std::vector<AutoRelease<ListDef>> lists;
};

struct StatementListList : public ASTNode
{
	AST_IMPL(StatementListList, ASTNode);
	AST_ACCEPTOR;

	std::vector<AutoRelease<StatementList>> sll;
};

// sprite definition
struct SpriteDef : public ASTNode
{
	AST_IMPL(SpriteDef, ASTNode);
	AST_ACCEPTOR;
	
	std::string name;

	AutoRelease<VariableDefList> variables;
	AutoRelease<ListDefList> lists;
	AutoRelease<StatementListList> scripts;

	int64_t currentCostume;

	double volume = 100.0;
	int64_t layer = 1;
	bool visible = true;
	double x = 0.0, y = 0.0;
	double size = 100.0;
	double direction = 90.0;
	bool draggable = false;
	std::string rotationStyle = "all around";
};

// list of sprite definitions
struct SpriteDefList : public ASTNode
{
	AST_IMPL(SpriteDefList, ASTNode);
	AST_ACCEPTOR;

	inline const AutoRelease<SpriteDef> &Find(const std::string &name) const
	{
		for (auto &s : sprites)
			if (s->name == name)
				return s;
		return nullptr;
	}

	std::vector<AutoRelease<SpriteDef>> sprites;
};

// stage definition
struct StageDef : public ASTNode
{
	AST_IMPL(StageDef, ASTNode);
	AST_ACCEPTOR;

	AutoRelease<VariableDefList> variables;
	AutoRelease<ListDefList> lists;
	AutoRelease<StatementListList> scripts;

	double volume = 100;
	int64_t layer = 0;
	double tempoBPM = 60;
	double videoTransparency = 50;
	VideoState videoState = VideoState_Off;
};

struct ValMonitor : public ASTNode
{
	AST_IMPL(ValMonitor, ASTNode);
	AST_ACCEPTOR;

	std::string id; // variable id

	// options

	std::string mode;
	bool visible = false;
	double sliderMin = 0.0, sliderMax = 100.0;
	bool isDiscrete = false;
	double x = 0.0, y = 0.0;
};

struct ValMonitorList : public ASTNode
{
	AST_IMPL(ValMonitorList, ASTNode);
	AST_ACCEPTOR;

	std::vector<AutoRelease<ValMonitor>> monitors;
};

// program definition
struct Program : public ASTNode
{
	AST_IMPL(Program, ASTNode);
	AST_ACCEPTOR;

	AutoRelease<SpriteDefList> sprites;
	AutoRelease<StageDef> stage;
	AutoRelease<ValMonitorList> monitors;
};
