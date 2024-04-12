#include "syminfo.hpp"

#include <vector>

std::string SymInfo::ToString() const
{
	if (type & SymbolType_Literal)
	{
		switch (type)
		{
		case SymbolType_String: return "string[" + literal + "]";
		case SymbolType_Number: return "number[" + literal + "]";
		case SymbolType_Int: return "int[" + literal + "]";
		case SymbolType_Bool: return "bool[" + literal + "]";
		default: return "literal[" + literal + "]";
		};
	}

	std::vector<std::string> parts;

	if (type & SymbolType_String) parts.push_back("string");
	if (type & SymbolType_Number) parts.push_back("number");
	if (type & SymbolType_Int) parts.push_back("int");
	if (type & SymbolType_Bool) parts.push_back("bool");

	if (parts.size() == 0 || parts.size() == NUM_SYMBOL_TYPES)
		return "any";

	std::string result = parts[0];
	for (size_t i = 1; i < parts.size(); i++)
		result += " | " + parts[i];

	return result;
}

SymInfo::SymInfo(int type) : type((SymbolType)type) {}

SymInfo::SymInfo(int type, const std::string &literal) :
	type((SymbolType)type), literal(literal) {}
