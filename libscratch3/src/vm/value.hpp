#pragma once

#include <cstdint>

enum ValueType
{
	ValueType_None = 0,

	/* Numeric types */

	ValueType_Integer,
	ValueType_Real,
	ValueType_Bool,

	/* Reference types */

	ValueType_String,

	/* Other types */

	ValueType_BasicString,
	ValueType_ConstString
};

struct Reference;
struct String;
struct List;

struct Value
{
	uint16_t type;
	uint8_t __padding[2];

	uint32_t hash;

	union
	{
		int64_t integer;
		double real;
		bool boolean;

		Reference *ref;
		String *string;

		const char *basic_string; // weak reference
		const std::string *const_string; // weak reference
	} u;
};

struct Reference
{
	int32_t count;
	uint32_t flags;
};

struct String
{
	Reference ref;
	int64_t len;
	char str[1];
};
