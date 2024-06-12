#pragma once

#include <scratch3/scratch3.h>

#include "astdef.hpp"
#include "astnode.hpp"
#include "expression.hpp"
#include "program.hpp"
#include "statement.hpp"
#include "visitor.hpp"

//! Parse JSON string into AST.
//! 
//! \param jsonString The JSON string to parse.
//! \param length The length of the JSON string.
//! \param log The log callback.
//! \param up The user pointer.
//! 
//! \return The AST.
Program *ParseAST(const char *jsonString, size_t length,
	Scratch3LogFn log, void *up, const Scratch3CompilerOptions *options);
