#pragma once

#include <cstdint>

enum ValueType
{
	ValueType_Exception = -1,

	ValueType_None = 0,

	/* Numeric types */

	ValueType_Integer,
	ValueType_Real,
	ValueType_Bool,

	/* Reference types */

	ValueType_String
};

struct Reference;
struct String;
struct List;

struct Value
{
	uint16_t type;
	uint8_t __padding[6];

	union
	{
		int64_t integer;
		double real;
		bool boolean;

		uint32_t exception;

		Reference *ref;
		String *string;
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