#pragma once

#include <unordered_map>

#include "opcode.hpp"

#include "../ast/ast.hpp"

uint8_t *CompileProgram(Program *prog, Loader *loader, size_t *size);
