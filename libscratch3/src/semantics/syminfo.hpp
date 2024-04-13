#pragma once

#include <string>

enum SymbolType
{
	SymbolType_String = 0x01,
	SymbolType_Number = 0x02,
	SymbolType_Bool = 0x04,

	SymbolType_Any = 0x00ff,

	SymbolType_Literal = 0x100
};

#define NUM_SYMBOL_TYPES 4

struct SymInfo
{
	std::string ToString() const;

	SymInfo() = default;
	SymInfo(int type);
	SymInfo(int type, const std::string &literal);

	SymbolType type = SymbolType_Any;
	std::string literal;
};
