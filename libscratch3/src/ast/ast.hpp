#pragma once

#include "astdef.hpp"
#include "astnode.hpp"
#include "expression.hpp"
#include "program.hpp"
#include "statement.hpp"

enum MessageType
{
	MessageType_Error,
	MessageType_Warning
};

struct Message
{
	MessageType type = MessageType_Error;
	std::string message;
};

// Parse AST from project.json
Program *ParseAST(const char *jsonString, size_t length, std::vector<Message> *log = nullptr);
