#pragma once

#include <scratch3/scratch3.h>

#include "astdef.hpp"
#include "astnode.hpp"
#include "expression.hpp"
#include "program.hpp"
#include "statement.hpp"
#include "visitor.hpp"
#include "optimize.hpp"

//! Parse JSON string into AST.
Program *ParseAST(Scratch3 *S, const char *jsonString, size_t length, const Scratch3CompilerOptions *options);
