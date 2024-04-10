#pragma once

#include "statement.hpp"

// variable definition in a sprite/stage
struct VariableDef : public ASTNode
{
	AST_IMPL(VariableDef, ASTNode);
	AST_ACCEPTOR;

	inline virtual ~VariableDef() { delete value; }

	std::string id, name;
	Constexpr *value = 0;
};

// list of variable definitions
struct VariableDefList : public ASTNode
{
	AST_IMPL(VariableDefList, ASTNode);
	AST_ACCEPTOR;

	inline VariableDef *Find(const std::string &id) const
	{
		for (auto v : variables)
			if (v->id == id)
				return v;
		return 0;
	}

	inline virtual ~VariableDefList()
	{
		for (auto v : variables)
			delete v;
	}

	std::vector<VariableDef *> variables;
};

// list definition in a sprite/stage
struct ListDef : public ASTNode
{
	AST_IMPL(ListDef, ASTNode);
	AST_ACCEPTOR;

	inline virtual ~ListDef()
	{
		for (auto v : value)
			delete v;
	}

	std::string id, name;
	std::vector<Constexpr *> value;
};

// list of list definitions
struct ListDefList : public ASTNode
{
	AST_IMPL(ListDefList, ASTNode);
	AST_ACCEPTOR;

	inline ListDef *Find(const std::string &id) const
	{
		for (auto l : lists)
			if (l->id == id)
				return l;
		return nullptr;
	}

	inline virtual ~ListDefList()
	{
		for (auto l : lists)
			delete l;
	}

	std::vector<ListDef *> lists;
};

struct StatementListList : public ASTNode
{
	AST_IMPL(StatementListList, ASTNode);
	AST_ACCEPTOR;

	inline virtual ~StatementListList()
	{
		for (auto sl : sll)
			delete sl;
	}

	std::vector<StatementList *> sll;
};

// sprite definition
struct SpriteDef : public ASTNode
{
	AST_IMPL(SpriteDef, ASTNode);
	AST_ACCEPTOR;

	inline virtual ~SpriteDef()
	{
		delete variables;
		delete lists;
		delete scripts;
	}

	std::string name;

	VariableDefList *variables = nullptr;
	ListDefList *lists = nullptr;
	StatementListList *scripts = nullptr;

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

	inline SpriteDef *Find(const std::string &name) const
	{
		for (auto s : sprites)
			if (s->name == name)
				return s;
		return 0;
	}

	inline virtual ~SpriteDefList()
	{
		for (auto &s : sprites)
			delete s;
	}

	std::vector<SpriteDef *> sprites;
};

// stage definition
struct StageDef : public ASTNode
{
	AST_IMPL(StageDef, ASTNode);
	AST_ACCEPTOR;

	inline virtual ~StageDef()
	{
		delete variables;
		delete lists;
		delete scripts;
	}

	VariableDefList *variables = nullptr;
	ListDefList *lists = nullptr;
	StatementListList *scripts = nullptr;

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

	inline virtual ~ValMonitorList()
	{
		for (auto m : monitors)
			delete m;
	}

	std::vector<ValMonitor *> monitors;
};

// program definition
struct Program : public ASTNode
{
	AST_IMPL(Program, ASTNode);
	AST_ACCEPTOR;

	inline ~Program()
	{
		delete sprites;
		delete stage;
		delete monitors;
	}

	SpriteDefList *sprites = nullptr;
	StageDef *stage = nullptr;
	ValMonitorList *monitors = nullptr;
};
