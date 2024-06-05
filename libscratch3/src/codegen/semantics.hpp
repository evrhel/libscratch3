#pragma once

#include <unordered_map>

#include "../ast/ast.hpp"

class ProgramState final
{
public:
	ProgramState &operator=(const ProgramState &) = delete;
	ProgramState &operator=(ProgramState &&) = delete;

	ProgramState();
	ProgramState(const ProgramState &) = delete;
	~ProgramState();
};

void AnalyzeProgram(Program *prog);
