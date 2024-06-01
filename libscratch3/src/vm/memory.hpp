#pragma once

#include <cstdint>
#include <string>

#include <lysys/lysys.hpp>

#define INITIAL_CAPACITY 8

enum ValueType
{
	ValueType_None = 0,

	/* Numeric types */

	ValueType_Integer,
	ValueType_Real,
	ValueType_Bool,

	/* Reference types */

	ValueType_String,
	ValueType_List,

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
		List *list;

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

struct List
{
	Reference ref;
	int64_t len;
	int64_t capacity;
	Value *values;
};

bool StringEquals(const char *lstr, const char *rstr);

bool Truth(const Value &val);
bool Equals(const Value &lhs, const Value &rhs);
Value &Assign(Value &lhs, const Value &rhs);
Value &SetInteger(Value &lhs, int64_t rhs);
Value &SetReal(Value &lhs, double rhs);
Value &SetBool(Value &lhs, bool rhs);
Value &SetChar(Value &lhs, char c);
Value &SetString(Value &lhs, const char *rhs, size_t len);
Value &SetString(Value &lhs, const char *rhs);
Value &SetString(Value &lhs, const std::string &rhs);
Value &SetBasicString(Value &lhs, const char *rhs);
Value &SetConstString(Value &lhs, const std::string *rhs);
Value &SetParsedString(Value &lhs, const std::string &rhs);
Value &SetParsedBasicString(Value &lhs, const char *rhs);
Value &SetParsedConstString(Value &lhs, const std::string *rhs);
Value &SetEmpty(Value &lhs);

Value &ListGet(Value &lhs, const Value &list, int64_t index);
void ListSet(Value &list, int64_t index, const Value &v);
int64_t ListIndexOf(const Value &list, const Value &v);
int64_t ListGetLength(const Value &list);
bool ListContainsValue(const Value &list, const Value &v);
void ListAppend(const Value &list, const Value &v);
void ListDelete(const Value &list, int64_t index);
void ListDelete(const Value &list, const Value &index);
void ListClear(const Value &list);
void ListInsert(const Value &list, int64_t index, const Value &v);

void CvtString(Value &v);
int64_t ValueLength(const Value &v);
Value &ConcatValue(Value &lhs, const Value &rhs);
char ValueCharAt(const Value &v, int64_t index);
bool ValueContains(const Value &lhs, const Value &rhs);

int64_t ToInteger(const Value &v);
double ToReal(const Value &v);
const char *ToString(const Value &v, int64_t *len = nullptr);

//! \brief Returns a trivial string representation of a value
//! 
//! If the value requires processing to convert to a string (such
//! as for integers and reals), the function will return an
//! empty string.
//! 
//! \param v Value to convert
//! \param len Pointer to store the length of the string
//! 
//! \return A string representation of the value
const char *GetRawString(const Value &v, int64_t *len = nullptr);

Value &AllocString(Value &v, int64_t len);

Value &AllocList(Value &v, int64_t len);

//! \brief Initialize a value
//! 
//! Only call this function on a value that has not been initialized.
//! Doing so may cause a memory leak.
//! 
//! \param v Value to initialize
//! 
//! \return v
constexpr Value &InitializeValue(Value &v)
{
	v.type = ValueType_None;
	v.u.ref = nullptr;
	return v;
}

Value &RetainValue(Value &v);
void ReleaseValue(Value &v);
void FreeValue(Value &v);
