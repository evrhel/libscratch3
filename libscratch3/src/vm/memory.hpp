#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <mutil/mutil.h>

#include "../defs.hpp"

// initial capacity for lists
#define INITIAL_CAPACITY 8

#define VALUE_STATIC 0x01 // value is statically allocated

using namespace mutil;

enum ValueType
{
	ValueType_Undefined = -1, // undefined value

	ValueType_None = 0, // no value

	/* Numeric types */

	ValueType_Integer, // int64_t
	ValueType_Real, // double
	ValueType_Bool, // bool

	/* Reference types */

	ValueType_String, // String *
	ValueType_List, // List *

	/* Internal types */
	ValueType_IntPtr // intptr_t
};

struct Reference;
struct String;
struct List;

struct Value
{
	uint16_t type; // Type of the value, as a ValueType
	uint16_t flags; // Flags for the value

	uint8_t __padding[4]; // Padding for alignment

	union
	{
		int64_t integer; // ValueType_Integer
		double real; // ValueType_Real
		bool boolean; // ValueType_Bool

		Reference *ref; // (any reference type)
		String *string; // ValueType_String
		List *list; // ValueType_List

		intptr_t intptr; // ValueType_IntPtr
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
	int64_t len; // length of the string (excluding null terminator)
	int64_t hash; // hash of the string
	char str[1]; // string data, null-terminated
};

struct List
{
	Reference ref;
	int64_t len; // number of elements in the list
	int64_t capacity; // capacity of the list
	Value *values; // array of values, of length capacity
};

//! \brief Hash a string
//!
//! \param s The string to hash
//!
//! \return The hash of the string
constexpr uint32_t HashString(const char *s)
{
	uint32_t hash = 1315423911;
	while (*s)
		hash ^= ((hash << 5) + *s++ + (hash >> 2));
	return hash;
}

//! \brief Compare strings case-insensitively
//!
//! \param lstr The left string
//! \param rstr The right string
//!
//! \return true if the strings are equal, false otherwise
bool StringEqualsRaw(const char *lstr, const char *rstr);

//! \brief Compare strings with Scratch semantics
//!
//! Scratch strings are case-insensitive and ignore leading and trailing
//! whitespace.
//!
//! \param lstr The left string
//! \param rstr The right string
//!
//! \return true if the strings are equal, false otherwise
bool StringEquals(const char *lstr, const char *rstr);

//! \brief Evaluate the truthiness of a value
//!
//! \param val The value to evaluate
//!
//! \return true if the value is truthy, false otherwise
bool Truth(const Value &val);

//! \brief Compare two values
//!
//! \param lhs The left value
//! \param rhs The right value
//!
//! \return true if the values are equal, false otherwise
bool Equals(const Value &lhs, const Value &rhs);

//
/////////////////////////////////////////////////////////////////////
// Assignment operations
//

Value &Assign(Value &lhs, const Value &rhs);
Value &SetInteger(Value &lhs, int64_t rhs);
Value &SetReal(Value &lhs, double rhs);
Value &SetBool(Value &lhs, bool rhs);
Value &SetChar(Value &lhs, char c);
Value &SetString(Value &lhs, const char *rhs, size_t len);
Value &SetString(Value &lhs, const char *rhs);
Value &SetString(Value &lhs, const std::string &rhs);
Value &SetStaticString(Value &lhs, String *rhs);
Value &SetParsedString(Value &lhs, const std::string &rhs);
Value &SetParsedString(Value &lhs, const char *rhs);
Value &SetIntPtr(Value &lhs, intptr_t rhs);
Value &SetEmpty(Value &lhs);

//
/////////////////////////////////////////////////////////////////////
// List operations
//

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

//
/////////////////////////////////////////////////////////////////////
// String operations
//

Value &CvtString(Value &v);
int64_t ValueLength(const Value &v);
Value &ConcatValue(Value &lhs, const Value &rhs);
char ValueCharAt(const Value &v, int64_t index);
bool ValueContains(const Value &lhs, const Value &rhs);

//
/////////////////////////////////////////////////////////////////////
// Conversion operations
//

int64_t ToInteger(const Value &v);
double ToReal(const Value &v);
const char *ToString(const Value &v, int64_t *len = nullptr);
IntVector4 ToRGBA(const Value &v);
IntVector3 ToRGB(const Value &v);

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

//
/////////////////////////////////////////////////////////////////////
// Operators
//

Value &ValueAdd(Value &lhs, const Value &rhs);
Value &ValueSub(Value &lhs, const Value &rhs);
Value &ValueMul(Value &lhs, const Value &rhs);
Value &ValueDiv(Value &lhs, const Value &rhs);
Value &ValueMod(Value &lhs, const Value &rhs);
Value &ValueNeg(Value &lhs);

Value &ValueGreater(Value &lhs, const Value &rhs);
Value &ValueLess(Value &lhs, const Value &rhs);

Value &ValueDeepCopy(Value &lhs, const Value &rhs);

//
/////////////////////////////////////////////////////////////////////
// Allocation operations
//

//! \brief Allocate a string
//!
//! Allocates a String object with enough space to store a string of
//! the given length, including its null terminator. The contents of
//! the string are initialized to zero.
//!
//! If allocation fails, the function will assign a None value to the
//! given value.
//!
//! \param v Value to store the string
//! \param len Length of the string
//!
//! \return v
Value &AllocString(Value &v, int64_t len);

//! \brief Allocate a list
//!
//! Allocates a List object of the given length. All entries in the
//! list are initialized to None.
//!
//! \param v Value to store the list
//! \param len Length of the list
//!
//! \return v
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
	v.flags = 0;
	v.__padding[0] = 0;
	v.__padding[1] = 0;
	v.__padding[2] = 0;
	v.__padding[3] = 0;
	v.u.ref = nullptr;
	return v;
}

//! \brief Increment the reference count of a value
//!
//! \param v Value to retain
//!
//! \return v
Value &RetainValue(Value &v);

//! \brief Decrement the reference count of a value
//!
//! The referenced value will be freed if the reference count reaches
//! zero. Regardless of the reference count, the value will be
//! assigned to a None value on return. Calling this function on a
//! non-reference type or statically allocated value will have no
//! effect.
//!
//! \param v Value to release
void ReleaseValue(Value &v);

//! \brief Free a value, immediately
//!
//! The value will be freed immediately, regardless of its reference
//! count. The contents of the value are undefined after this function
//! is called. Calling on a statically allocated value will result in
//! undefined behavior.
void FreeValue(Value &v);

//! \brief Hashes a string for use in a hash table
struct _StringHasher
{
	constexpr size_t operator()(const String *str) const
	{
		return static_cast<size_t>(str->hash);
	}
};

//! \brief Compares two strings for use in a hash table
struct _StringEqual
{
	constexpr bool operator()(const String *lhs, const String *rhs) const
	{
		if (lhs == rhs)
			return true;

		if (lhs->hash != rhs->hash || lhs->len != rhs->len)
			return false;

		for (int64_t i = 0; i < lhs->len; i++)
		{
			if (lhs->str[i] != rhs->str[i])
				return false;
		}

		return true;
	}
};

template <typename T>
using StringMap = std::unordered_map<const String *, T, _StringHasher, _StringEqual>;

class OptionalValue
{
public:
	constexpr bool HasValue() const { return _hasValue; }
	constexpr ValueType Type() const { return _type; }
	constexpr const Value &GetValue() const { return _value; }

	inline OptionalValue &Erase()
	{
		if (_hasValue)
		{
			ReleaseValue(_value);
			_hasValue = false;
		}

		return *this;
	}

	inline OptionalValue &SetUndefined()
	{
		Erase();
		_type = ValueType_Undefined;

		return *this;
	}

	inline OptionalValue &SetEmpty()
	{
		if (!_hasValue)
			InitializeValue(_value);
		else
			::SetEmpty(_value);

		_hasValue = true;
		_type = ValueType_None;

		return *this;
	}

	inline OptionalValue &SetInteger(int64_t val)
	{
		if (!_hasValue)
			InitializeValue(_value);

		::SetInteger(_value, val);
		_hasValue = true;
		_type = ValueType_Integer;

		return *this;
	}

	inline OptionalValue &SetInteger()
	{
		if (_hasValue)
			ReleaseValue(_value);

		_hasValue = false;
		_type = ValueType_Integer;

		return *this;
	}

	inline OptionalValue &SetReal(double val)
	{
		if (!_hasValue)
			InitializeValue(_value);

		::SetReal(_value, val);
		_hasValue = true;
		_type = ValueType_Real;

		return *this;
	}

	inline OptionalValue &SetReal()
	{
		if (_hasValue)
			ReleaseValue(_value);

		_hasValue = false;
		_type = ValueType_Real;

		return *this;
	}

	inline OptionalValue &SetBool(bool val)
	{
		if (!_hasValue)
			InitializeValue(_value);

		::SetBool(_value, val);
		_hasValue = true;
		_type = ValueType_Bool;

		return *this;
	}
	
	inline OptionalValue &SetBool()
	{
		if (_hasValue)
			ReleaseValue(_value);

		_hasValue = false;
		_type = ValueType_Bool;

		return *this;
	}

	inline OptionalValue &SetString(const char *val)
	{
		if (!_hasValue)
			InitializeValue(_value);

		::SetString(_value, val);
		_hasValue = true;
		_type = ValueType_String;

		return *this;
	}

	inline OptionalValue &SetString(const std::string &val)
	{
		if (!_hasValue)
			InitializeValue(_value);

		::SetString(_value, val);
		_hasValue = true;
		_type = ValueType_String;

		return *this;
	}

	inline OptionalValue &SetString()
	{
		if (_hasValue)
			ReleaseValue(_value);

		_hasValue = false;
		_type = ValueType_String;

		return *this;
	}

	inline OptionalValue &SetList()
	{
		if (_hasValue)
			ReleaseValue(_value);

		_hasValue = false;
		_type = ValueType_List;

		return *this;
	}

	inline OptionalValue &SetParsedString(const char *val)
	{
		if (!_hasValue)
			InitializeValue(_value);

		::SetParsedString(_value, val);
		_hasValue = true;
		_type = (ValueType)_value.type;

		return *this;
	}

	inline OptionalValue &SetValue(const Value &val)
	{
		if (!_hasValue)
			InitializeValue(_value);

		Assign(_value, val);
		_hasValue = true;
		_type = (ValueType)val.type;

		return *this;
	}

	inline OptionalValue &SetType(ValueType type)
	{
		if (_hasValue)
			ReleaseValue(_value);

		_type = type;
		_hasValue = false;

		return *this;
	}

	inline OptionalValue &operator=(const OptionalValue &opt)
	{
		if (this == &opt)
			return *this;

		SetEmpty();

		_hasValue = opt._hasValue;
		_type = opt._type;

		if (_hasValue)
			Assign(_value, opt._value);

		return *this;
	}

	inline OptionalValue &operator=(OptionalValue &&opt) noexcept
	{
		if (this == &opt)
			return *this;
		
		if (_hasValue)
			ReleaseValue(_value);

		_value = opt._value;
		_type = opt._type;
		_hasValue = opt._hasValue;

		opt._hasValue = false;
		opt._type = ValueType_Undefined;

		return *this;
	}

	inline OptionalValue &operator=(const Value &val)
	{
		if (!_hasValue)
			InitializeValue(_value);

		Assign(_value, val);
		_hasValue = true;
		_type = (ValueType)val.type;

		return *this;
	}

	constexpr bool IsZeroLike() const
	{
		if (_type == ValueType_None || _type == ValueType_String)
			return true;

		if (!_hasValue)
			return false;

		switch (_type)
		{
		default:
			return false;
		case ValueType_Integer:
			return _value.u.integer == 0;
		case ValueType_Real:
			return _value.u.real == 0.0;
		case ValueType_Bool:
			return !_value.u.boolean;
		}
	}

	constexpr bool IsZero() const
	{
		if (_type == ValueType_None)
			return true;

		if (!_hasValue)
			return false;

		switch (_type)
		{
		default:
			return false;
		case ValueType_Integer:
			return _value.u.integer == 0;
		case ValueType_Real:
			return _value.u.real == 0.0;
		case ValueType_Bool:
			return !_value.u.boolean;
		}
	}

	constexpr bool IsOne() const
	{
		if (!_hasValue)
			return false;

		switch (_type)
		{
		default:
			return false;
		case ValueType_Integer:
			return _value.u.integer == 1;
		case ValueType_Real:
			return _value.u.real == 1.0;
		case ValueType_Bool:
			return _value.u.boolean;
		}
	}

	constexpr bool IsNegativeOne() const
	{
		if (!_hasValue)
			return false;

		switch (_type)
		{
		default:
			return false;
		case ValueType_Integer:
			return _value.u.integer == -1;
		case ValueType_Real:
			return _value.u.real == -1.0;
		}
	}

	constexpr bool IsNegative() const
	{
		if (!_hasValue)
			return false;

		switch (_type)
		{
		default:
			return false;
		case ValueType_Integer:
			return _value.u.integer < 0;
		case ValueType_Real:
			return _value.u.real < 0.0;
		}
	}

	constexpr bool IsNegativeOrZero() const
	{
		if (_type == ValueType_None)
			return true;

		if (!_hasValue)
			return false;

		switch (_type)
		{
		default:
			return false;
		case ValueType_Bool:
			return !_value.u.boolean;
		case ValueType_Integer:
			return _value.u.integer <= 0;
		case ValueType_Real:
			return _value.u.real <= 0.0;
		}
	}

	constexpr bool IsPositive() const
	{
		if (!_hasValue)
			return false;

		switch (_type)
		{
		default:
			return false;
		case ValueType_Bool:
			return _value.u.boolean;
		case ValueType_Integer:
			return _value.u.integer > 0;
		case ValueType_Real:
			return _value.u.real > 0.0;
		}
	}

	constexpr bool IsPositiveOrZero() const
	{
		if (_type == ValueType_None || _type == ValueType_Bool)
			return true;

		if (!_hasValue)
			return false;

		switch (_type)
		{
		default:
			return false;
		case ValueType_Integer:
			return _value.u.integer >= 0;
		case ValueType_Real:
			return _value.u.real >= 0.0;
		}
	}

	constexpr bool IsPossibleNumeric() const
	{
		return _type == ValueType_Undefined || _type == ValueType_None || _type == ValueType_Integer || _type == ValueType_Real;
	}

	constexpr bool IsNone() const
	{
		return _type == ValueType_None;
	}

	inline OptionalValue() : _hasValue(false), _type(ValueType_Undefined)
	{
		memset(&_value, 0, sizeof(Value));
	}

	inline OptionalValue(ValueType type) : _hasValue(false), _type(type)
	{
		memset(&_value, 0, sizeof(Value));
	}

	inline OptionalValue(int64_t val) : _hasValue(true), _type(ValueType_Integer)
	{
		InitializeValue(_value);
		::SetInteger(_value, val);
	}

	inline OptionalValue(double val) : _hasValue(true), _type(ValueType_Real)
	{
		InitializeValue(_value);
		::SetReal(_value, val);
	}

	inline OptionalValue(bool val) : _hasValue(true), _type(ValueType_Bool)
	{
		InitializeValue(_value);
		::SetBool(_value, val);
	}

	inline OptionalValue(const char *val) : _hasValue(true), _type(ValueType_String)
	{
		InitializeValue(_value);
		::SetString(_value, val);
	}

	inline OptionalValue(const std::string &val) : _hasValue(true), _type(ValueType_String)
	{
		InitializeValue(_value);
		::SetString(_value, val);
	}

	inline OptionalValue(const Value &val) : _hasValue(true), _type((ValueType)val.type)
	{
		InitializeValue(_value);
		Assign(_value, val);
	}

	inline OptionalValue(const OptionalValue &opt) : _hasValue(opt._hasValue), _type(opt._type)
	{
		if (_hasValue)
		{
			InitializeValue(_value);
			Assign(_value, opt._value);
		}
		else
			memset(&_value, 0, sizeof(Value));
	}

	inline ~OptionalValue()
	{
		if (_hasValue)
			ReleaseValue(_value);
	}
private:
	bool _hasValue;
	ValueType _type;
	Value _value;
};

inline OptionalValue operator+(const OptionalValue &lhs, const OptionalValue &rhs)
{
	if (!lhs.HasValue() || !rhs.HasValue())
	{
		if (lhs.IsPossibleNumeric() && rhs.IsPossibleNumeric())
			return OptionalValue(ValueType_Real);
		return OptionalValue();
	}

	Value result;
	InitializeValue(result);
	Assign(result, lhs.GetValue());
	ValueAdd(result, rhs.GetValue());

	OptionalValue opt;
	opt.SetValue(result);

	ReleaseValue(result);

	return opt;
}

inline OptionalValue operator-(const OptionalValue &lhs, const OptionalValue &rhs)
{
	if (!lhs.HasValue() || !rhs.HasValue())
	{
		if (lhs.IsPossibleNumeric() && rhs.IsPossibleNumeric())
			return OptionalValue(ValueType_Real);
		return OptionalValue();
	}

	Value result;
	InitializeValue(result);
	Assign(result, lhs.GetValue());
	ValueSub(result, rhs.GetValue());

	OptionalValue opt;
	opt.SetValue(result);

	ReleaseValue(result);

	return opt;
}

inline OptionalValue operator*(const OptionalValue &lhs, const OptionalValue &rhs)
{
	if (!lhs.HasValue() || !rhs.HasValue())
	{
		if (lhs.IsPossibleNumeric() && rhs.IsPossibleNumeric())
			return OptionalValue(ValueType_Real);
		return OptionalValue();
	}

	Value result;
	InitializeValue(result);
	Assign(result, lhs.GetValue());
	ValueMul(result, rhs.GetValue());

	OptionalValue opt;
	opt.SetValue(result);

	ReleaseValue(result);

	return opt;
}

inline OptionalValue operator/(const OptionalValue &lhs, const OptionalValue &rhs)
{
	if (!lhs.HasValue() || !rhs.HasValue())
	{
		if (lhs.IsPossibleNumeric() && rhs.IsPossibleNumeric())
			return OptionalValue(ValueType_Real);
		return OptionalValue();
	}

	Value result;
	InitializeValue(result);
	Assign(result, lhs.GetValue());
	ValueDiv(result, rhs.GetValue());

	OptionalValue opt;
	opt.SetValue(result);

	ReleaseValue(result);

	return opt;
}

inline OptionalValue operator-(const OptionalValue &val)
{
	if (!val.HasValue())
	{
		if (val.IsPossibleNumeric())
			return val;
		return OptionalValue();
	}

	Value result;
	InitializeValue(result);
	Assign(result, val.GetValue());
	ValueNeg(result);

	OptionalValue opt;
	opt.SetValue(result);

	ReleaseValue(result);

	return opt;
}
