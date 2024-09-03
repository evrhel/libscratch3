#include "memory.hpp"

#include <cmath>
#include <cassert>

#include <lysys/lysys.hpp>

#define TRUE_STRING "true"
#define FALSE_STRING "false"

#define TRUE_SIZE (sizeof(TRUE_STRING) - 1)
#define FALSE_SIZE (sizeof(FALSE_STRING) - 1)

bool StringEqualsRaw(const char *lstr, const char *rstr)
{
	while (*lstr && *rstr)
	{
		if (tolower(*lstr) != tolower(*rstr))
			return false;

		lstr++;
		rstr++;
	}

	return *lstr == *rstr;
}

bool StringEquals(const char *lstr, const char *rstr)
{
	if (lstr == rstr)
		return true;

	const char *lstart = lstr;
	while (*lstart && isspace(*lstart))
		lstart++;

	const char *lend = lstart;
	while (*lend && !isspace(*lend))
		lend++;

	const char *rstart = rstr;
	while (*rstart && isspace(*rstart))
		rstart++;

	const char *rend = rstart;
	while (*rend && !isspace(*rend))
		rend++;

	if (lend - lstart != rend - rstart)
		return false;

	size_t len = lend - lstart;
	for (size_t i = 0; i < len; i++)
	{
		if (tolower(lstart[i]) != tolower(rstart[i]))
			return false;
	}

	return true;
}

bool Truth(const Value &val)
{
	switch (val.type)
	{
	default:
		return false;
	case ValueType_Integer:
		return val.u.integer != 0;
	case ValueType_Real:
		return val.u.real != 0.0;
	case ValueType_Bool:
		return val.u.boolean;
	case ValueType_String:
		return StringEquals(val.u.string->str, TRUE_STRING);
	}
}

bool Equals(const Value &lhs, const Value &rhs)
{
	switch (lhs.type)
	{
	default:
		return false;
	case ValueType_Integer:
		if (rhs.type == ValueType_Integer)
			return lhs.u.integer == rhs.u.integer;
		if (rhs.type == ValueType_Real)
			return lhs.u.integer == rhs.u.real;
		if (rhs.type == ValueType_Bool)
			return rhs.u.boolean ? lhs.u.integer == 1 : lhs.u.integer == 0;
		return false;
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return lhs.u.real == rhs.u.real;
		if (rhs.type == ValueType_Integer)
			return lhs.u.real == rhs.u.integer;
		if (rhs.type == ValueType_Bool)
			return rhs.u.boolean ? lhs.u.real == 1.0 : lhs.u.real == 0.0;
		return false;
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return lhs.u.boolean == rhs.u.boolean;
		else if (rhs.type == ValueType_Integer)
			return lhs.u.boolean ? 1 == rhs.u.integer : 0 == rhs.u.integer;
		else if (rhs.type == ValueType_Real)
			return lhs.u.boolean ? 1.0 == rhs.u.real : 0.0 == rhs.u.real;
		return false;
	case ValueType_String:
		if (rhs.type == ValueType_String && lhs.u.string->hash == rhs.u.string->hash)
			return StringEquals(lhs.u.string->str, rhs.u.string->str);
		return false;
	case ValueType_List:
		if (rhs.type != ValueType_List)
			return false;

		if (lhs.u.list == rhs.u.list)
			return true;

		if (lhs.u.list->len != rhs.u.list->len)
			return false;

		for (int64_t i = 0; i < lhs.u.list->len; i++)
		{
			if (!Equals(lhs.u.list->values[i], rhs.u.list->values[i]))
				return false;
		}

		return true;
	}
}

Value &Assign(Value &lhs, const Value &rhs)
{
	if (&lhs == &rhs)
		return lhs;

	ReleaseValue(lhs);
	return RetainValue(lhs = rhs);
}

Value &SetInteger(Value &lhs, int64_t rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_Integer;
	lhs.u.integer = rhs;
	return lhs;
}

Value &SetReal(Value &lhs, double rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_Real;
	lhs.u.real = rhs;
	return lhs;
}

Value &SetBool(Value &lhs, bool rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_Bool;
	lhs.u.boolean = rhs;
	return lhs;
}

Value &SetChar(Value &lhs, char c)
{
	if (c == 0)
		return SetEmpty(lhs);

	AllocString(lhs, 1);
	if (lhs.type != ValueType_String)
		return SetEmpty(lhs);
	lhs.u.string->str[0] = c;
	return lhs;
}

Value &SetString(Value &lhs, const char *rhs, size_t len)
{
	if (len == 0)
		return SetEmpty(lhs);

	AllocString(lhs, len);
	if (lhs.type != ValueType_String)
		return SetEmpty(lhs);

	memcpy(lhs.u.string->str, rhs, len);
	lhs.u.string->hash = HashString(lhs.u.string->str);
	return lhs;
}

Value &SetString(Value &lhs, const char *rhs)
{
	return SetString(lhs, rhs, strlen(rhs));
}

Value &SetString(Value &lhs, const std::string &rhs)
{
	return SetString(lhs, rhs.data(), rhs.size());
}

Value &SetStaticString(Value &lhs, String *rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_String;
	lhs.flags = VALUE_STATIC;
	lhs.u.string = rhs;
	return lhs;
}

static std::string Trim(const std::string &str, const std::string &ws = " \t\n\r")
{
	size_t start = str.find_first_not_of(ws);
	if (start == std::string::npos)
		return "";

	size_t end = str.find_last_not_of(ws);
	return str.substr(start, end - start + 1);
}

static int ParseString(Value &lhs, const std::string &rhs)
{
	std::string str = Trim(rhs);
	if (str.size() != 0)
	{
		char *end;

		int64_t integer = strtoll(str.c_str(), &end, 10);
		if (*end == 0)
		{
			SetInteger(lhs, integer);
			return ValueType_Integer;
		}

		double real = strtod(str.c_str(), &end);
		if (*end == 0)
		{
			SetReal(lhs, real);
			return ValueType_Real;
		}

		if (str.size() == 4)
		{
			size_t i;
			for (i = 0; i < 4; i++)
				if (tolower(str[i]) != TRUE_STRING[i])
					break;

			if (i == 4)
			{
				SetBool(lhs, true);
				return ValueType_Bool;
			}
		}

		if (str.size() == 5)
		{
			size_t i;
			for (i = 0; i < 5; i++)
				if (tolower(str[i]) != FALSE_STRING[i])
					break;

			if (i == 5)
			{
				SetBool(lhs, false);
				return ValueType_Bool;
			}
		}
	}

	return ValueType_None;
}

Value &SetParsedString(Value &lhs, const std::string &rhs)
{
	if (ParseString(lhs, rhs) != ValueType_None)
		return lhs;
	return SetString(lhs, rhs);
}

Value &SetParsedString(Value &lhs, const char *rhs)
{
	if (ParseString(lhs, rhs) != ValueType_None)
		return lhs;
	return SetString(lhs, rhs);
}

Value &SetIntPtr(Value &lhs, intptr_t intptr)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_IntPtr;
	lhs.u.intptr = intptr;
	return lhs;
}

Value &SetEmpty(Value &lhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_None;
	return lhs;
}

Value &ListGet(Value &lhs, const Value &list, int64_t index)
{
	if (index < 1 || list.type != ValueType_List)
		return SetEmpty(lhs);

	List *l = list.u.list;
	if (index > l->len)
		return SetEmpty(lhs);

	return Assign(lhs, l->values[index - 1]);
}

void ListSet(Value &list, int64_t index, const Value &v)
{
	if (index < 1 || list.type != ValueType_List)
		return;

	List *l = list.u.list;
	if (index > l->len)
		return;

	Assign(l->values[index - 1], v);
}

int64_t ListIndexOf(const Value &list, const Value &v)
{
	if (list.type != ValueType_List)
		return 0;

	List *l = list.u.list;
	for (int64_t i = 0; i < l->len; i++)
	{
		if (Equals(l->values[i], v))
			return i + 1;
	}

	return 0;
}

int64_t ListGetLength(const Value &list)
{
	if (list.type != ValueType_List)
		return 0;
	return list.u.list->len;
}

bool ListContainsValue(const Value &list, const Value &v)
{
	return ListIndexOf(list, v) != 0;
}

static bool ListGrow(const Value &list)
{
	assert(list.type == ValueType_List);

	List *l = list.u.list;

	int64_t newLen = l->len + 1;
	if (l->capacity < newLen)
	{
		int64_t newCapacity = l->capacity * 2;
		if (newCapacity < newLen)
			newCapacity = newLen;

		Value *newValues = (Value *)realloc(l->values, newCapacity * sizeof(Value));
		if (!newValues)
			return false;

		l->values = newValues;
		l->capacity = newCapacity;
	}

	InitializeValue(l->values[l->len]); // uninitialized data
	l->len = newLen;

	return true;
}

void ListAppend(const Value &list, const Value &v)
{
	if (list.type != ValueType_List)
		return;

	if (!ListGrow(list))
		return;

	List *l = list.u.list;	
	Assign(l->values[l->len - 1], v);
}

void ListDelete(const Value &list, int64_t index)
{
	if (index < 1 || list.type != ValueType_List)
		return;

	List *l = list.u.list;
	if (index > l->len)
		return;

	int64_t i = index - 1;

	ReleaseValue(l->values[i]);

	for (; i < l->len; i++)
		l->values[i] = l->values[i + 1]; // direct assignment, no need to retain/release

	l->len--;
	InitializeValue(l->values[l->len]); // clear the last value
}

void ListDelete(const Value &list, const Value &index)
{
	if (index.type == ValueType_String)
	{
		const char *position = GetRawString(index, nullptr);
		if (StringEquals(position, "first"))
		{
			ListDelete(list, 1);
			return;
		}
		else if (StringEquals(position, "last"))
		{
			ListDelete(list, ListGetLength(list));
			return;
		}
		else if (StringEquals(position, "all"))
		{
			ListClear(list);
			return;
		}
	}
	
	ListDelete(list, ToInteger(index));
}

void ListClear(const Value &list)
{
	if (list.type != ValueType_List)
		return;

	List *l = list.u.list;
	for (int64_t i = 0; i < l->len; i++)
		ReleaseValue(l->values[i]);
	l->len = 0;
}

void ListInsert(const Value &list, int64_t index, const Value &v)
{
	if (index < 1 || list.type != ValueType_List)
		return;

	List *l = list.u.list;

	if (index > l->len + 1)
		return;

	if (!ListGrow(list))
		return;

	int64_t target = index - 1;
	for (int64_t i = l->len - 1; i > target; i--)
		l->values[i] = l->values[i - 1]; // direct assignment, no need to retain/release

	InitializeValue(l->values[target]); // prevents double release
	Assign(l->values[target], v);
}

Value &CvtString(Value &v)
{
	char buf[64];
	int cch;

	switch (v.type)
	{
	default:
	case ValueType_String:
		break;
	case ValueType_Integer:
		cch = snprintf(buf, sizeof(buf), "%lld", v.u.integer);
		SetString(v, buf, cch);
		break;
	case ValueType_Real:
		if (v.u.real == NAN)
		{
			SetString(v, "NaN");
			break;
		}

		if (v.u.real == INFINITY)
		{
			SetString(v, "Infinity");
			break;
		}

		if (v.u.real == -INFINITY)
		{
			SetString(v, "-Infinity");
			break;
		}

		cch = snprintf(buf, sizeof(buf), "%.8g", v.u.real);
		SetString(v, buf, cch);
		break;
	case ValueType_Bool:
		SetString(v, v.u.boolean ? TRUE_STRING : FALSE_STRING);
		break;
	case ValueType_List:
		SetString(v, "<list>");
		break;
	}

	return v;
}

int64_t ValueLength(const Value &v)
{
	Value tmp;
	int64_t len;

	switch (v.type)
	{
	default:
	case ValueType_None:
		return 0;
	case ValueType_Integer:
	case ValueType_Real:
	case ValueType_List:
		InitializeValue(tmp);
		Assign(tmp, v), CvtString(tmp);
		len = tmp.u.string->len; // will always be type ValueType_String
		ReleaseValue(tmp);
		return len;
	case ValueType_Bool:
		return v.u.boolean ? TRUE_SIZE : FALSE_SIZE - 1;
	case ValueType_String:
		return v.u.string->len;
	}
}

Value &ConcatValue(Value &lhs, const Value &rhs)
{
	Value a, b;
	InitializeValue(a);
	InitializeValue(b);

	// convert each value to a string
	Assign(a, lhs), CvtString(a);
	Assign(b, rhs), CvtString(b);

	int64_t len1, len2;
	const char *s1, *s2;

	// get the raw strings (CvtString already performed conversion)
	s1 = GetRawString(a, &len1);
	s2 = GetRawString(b, &len2);

	// allocate a new string
	AllocString(lhs, len1 + len2);
	if (lhs.type != ValueType_String)
	{
		ReleaseValue(b);
		ReleaseValue(a);
		return lhs;
	}

	// copy the strings into the new string
	memcpy(lhs.u.string->str, s1, len1);
	memcpy(lhs.u.string->str + len1, s2, len2);

	ReleaseValue(b);
	ReleaseValue(a);
	return lhs;
}

char ValueCharAt(const Value &v, int64_t index)
{
	if (index < 1)
		return 0;

	Value a;
	InitializeValue(a);

	Assign(a, v), CvtString(a);

	int64_t len;
	const char *s = GetRawString(a, &len);

	if (index > len)
	{
		ReleaseValue(a);
		return 0;
	}

	char c = s[index - 1];

	ReleaseValue(a);
	return c;
}

bool ValueContains(const Value &lhs, const Value &rhs)
{
	Value a, b;
	InitializeValue(a);
	InitializeValue(b);

	Assign(a, lhs), CvtString(a);
	Assign(b, rhs), CvtString(b);

	int64_t len1, len2;
	const char *s1, *s2;

	s1 = GetRawString(a, &len1);
	s2 = GetRawString(b, &len2);

	if (len2 == 0)
	{
		ReleaseValue(b);
		ReleaseValue(a);
		return true;
	}

	int64_t i, j;
	for (i = 0; i < len1; i++)
	{
		char c = tolower(s1[i]);
		for (j = 0; j < len2; j++)
		{
			if (c != tolower(s2[j]))
				break;
		}

		if (j == len2)
		{
			ReleaseValue(b);
			ReleaseValue(a);
			return true;
		}
	}

	ReleaseValue(b);
	ReleaseValue(a);
	return false;
}

int64_t ToInteger(const Value &v)
{
	switch (v.type)
	{
	default:
	case ValueType_None:
	case ValueType_String:
	case ValueType_List:
		return 0;
	case ValueType_Real:
		return static_cast<int64_t>(round(v.u.real));
	case ValueType_Integer:
		return v.u.integer;
	case ValueType_Bool:
		return v.u.boolean ? 1 : 0;
	}
}

double ToReal(const Value &v)
{
	switch (v.type)
	{
	default:
	case ValueType_None:
	case ValueType_String:
	case ValueType_List:
		return 0.0;
	case ValueType_Real:
		return v.u.real;
	case ValueType_Integer:
		return static_cast<double>(v.u.integer);
	case ValueType_Bool:
		return v.u.boolean ? 1.0 : 0.0;
	}
}

const char *ToString(const Value &v, int64_t *len)
{
	static SCRATCH3_STORAGE char buf[64];
	int cch;

	switch (v.type)
	{
	default:
	case ValueType_None:
		if (len) *len = 0;
		return "";
	case ValueType_Integer:
		cch = snprintf(buf, sizeof(buf), "%lld", v.u.integer);
		if (len) *len = cch;
		return buf;
	case ValueType_Real:
		cch = snprintf(buf, sizeof(buf), "%.8g", v.u.real);
		if (len) *len = cch;
		return buf;
	case ValueType_Bool:
		if (len) *len = v.u.boolean ? TRUE_SIZE : FALSE_SIZE;
		return v.u.boolean ? TRUE_STRING : FALSE_STRING;
	case ValueType_String:
		if (len) *len = v.u.string->len;
		return v.u.string->str;
	case ValueType_List:
		if (len) *len = 6;
		return "<list>";
	}
}

IntVector4 ToRGBA(const Value &v)
{
	int64_t icol;

	switch (v.type)
	{
	default:
	case ValueType_None:
		return IntVector4(0);
	case ValueType_Integer:
	case ValueType_Real:
	case ValueType_Bool:
		icol = ToInteger(v);
		break;
	case ValueType_String:
	case ValueType_List: {
		int64_t len;
		const char *s = ToString(v, &len);

		if (len == 0 || s[0] != '#')
			return IntVector4(0);

		// parse hex color
		char *end;
		icol = strtoll(s + 1, &end, 16);
		
		if (*end != 0) // invalid hex color
			return IntVector4(0);

		break;
	}
	}

	IntVector4 color;
	color.r = (icol >> 16) & 0xff;
	color.g = (icol >> 8) & 0xff;
	color.b = icol & 0xff;
	color.a = (icol >> 24) & 0xff;

	return color;
}

IntVector3 ToRGB(const Value &v)
{
	return IntVector3(ToRGBA(v));
}

const char *GetRawString(const Value &v, int64_t *len)
{
	switch (v.type)
	{
	default:
	case ValueType_None:
	case ValueType_Integer:
	case ValueType_Real:
	case ValueType_List:
		if (len) *len = 0;
		return "";
	case ValueType_Bool:
		if (len) *len = v.u.boolean ? sizeof(TRUE_STRING) - 1 : sizeof(FALSE_STRING) - 1;
		return v.u.boolean ? TRUE_STRING : FALSE_STRING;
	case ValueType_String:
		if (len) *len = v.u.string->len;
		return v.u.string->str;
	}
}

Value &ValueAdd(Value &lhs, const Value &rhs)
{
	switch (lhs.type)
	{
	default:
		if (rhs.type == ValueType_Integer || rhs.type == ValueType_Real)
			return Assign(lhs, rhs);
		return SetInteger(lhs, 0);
	case ValueType_Integer:
		if (rhs.type == ValueType_Integer)
			return SetInteger(lhs, lhs.u.integer + rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, lhs.u.integer + rhs.u.real);
		if (rhs.type == ValueType_Bool)
			return SetInteger(lhs, lhs.u.integer + (rhs.u.boolean ? 1 : 0));
		return lhs;
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, lhs.u.real + rhs.u.real);
		if (rhs.type == ValueType_Integer)
			return SetReal(lhs, lhs.u.real + rhs.u.integer);
		if (rhs.type == ValueType_Bool)
			return SetReal(lhs, lhs.u.real + (rhs.u.boolean ? 1.0 : 0.0));
		return lhs;
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return SetInteger(lhs, (lhs.u.boolean ? 1 : 0) + (rhs.u.boolean ? 1 : 0));
		if (rhs.type == ValueType_Integer)
			return SetInteger(lhs, (lhs.u.boolean ? 1 : 0) + rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, (lhs.u.boolean ? 1 : 0) + rhs.u.real);
		return lhs;
	}
}

Value &ValueSub(Value &lhs, const Value &rhs)
{
	switch (lhs.type)
	{
	default:
		return ValueNeg(Assign(lhs, rhs));
	case ValueType_Integer:
		if (rhs.type == ValueType_Integer)
			return SetInteger(lhs, lhs.u.integer - rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, lhs.u.integer - rhs.u.real);
		return lhs;
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, lhs.u.real - rhs.u.real);
		if (rhs.type == ValueType_Integer)
			return SetReal(lhs, lhs.u.real - rhs.u.integer);
		return lhs;
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return SetInteger(lhs, (lhs.u.boolean ? 1 : 0) - (rhs.u.boolean ? 1 : 0));
		if (rhs.type == ValueType_Integer)
			return SetInteger(lhs, (lhs.u.boolean ? 1 : 0) - rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, (lhs.u.boolean ? 1 : 0) - rhs.u.real);
		return lhs;
	}
}

Value &ValueMul(Value &lhs, const Value &rhs)
{
	switch (lhs.type)
	{
	default:
		return SetInteger(lhs, 0);
	case ValueType_Integer:
		if (rhs.type == ValueType_Integer)
			return SetInteger(lhs, lhs.u.integer * rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, lhs.u.integer * rhs.u.real);
		return SetInteger(lhs, 0);
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, lhs.u.real * rhs.u.real);
		if (rhs.type == ValueType_Integer)
			return SetReal(lhs, lhs.u.real * rhs.u.integer);
		return SetInteger(lhs, 0);
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return SetInteger(lhs, (lhs.u.boolean ? 1 : 0) * (rhs.u.boolean ? 1 : 0));
		if (rhs.type == ValueType_Integer)
			return SetInteger(lhs, (lhs.u.boolean ? 1 : 0) * rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetReal(lhs, (lhs.u.boolean ? 1 : 0) * rhs.u.real);
		return SetInteger(lhs, 0);
	}
}

Value &ValueDiv(Value &lhs, const Value &rhs)
{
	switch (rhs.type) // Notice switch on rhs
	{
	default:
		goto handle_div0;
	case ValueType_Integer:
		if (rhs.u.integer == 0)
			goto handle_div0;

		if (lhs.type == ValueType_Integer)
			return SetInteger(lhs, lhs.u.integer / rhs.u.integer);
		if (lhs.type == ValueType_Real)
			return SetReal(lhs, lhs.u.real / rhs.u.integer);
		return SetInteger(lhs, 0);
	case ValueType_Real:
		if (rhs.u.real == 0.0)
			goto handle_div0;

		if (lhs.type == ValueType_Real)
			return SetReal(lhs, lhs.u.real / rhs.u.real);
		if (rhs.type == ValueType_Integer)
			return SetReal(lhs, lhs.u.integer / rhs.u.real);
		return SetInteger(lhs, 0);
	case ValueType_Bool:
		if (!rhs.u.boolean)
			goto handle_div0;

		if (lhs.type == ValueType_Integer || lhs.type == ValueType_Real)
			return lhs;

		return SetInteger(lhs, 0);
	}

handle_div0:
	if (lhs.type == ValueType_Integer)
	{
		if (lhs.u.integer == 0)
			return SetReal(lhs, NAN);
		if (lhs.u.integer < 0)
			return SetReal(lhs, -INFINITY);
		return SetReal(lhs, INFINITY);
	}

	if (lhs.type == ValueType_Real)
	{
		if (lhs.u.real == 0.0)
			return SetReal(lhs, NAN);
		if (lhs.u.real < 0.0)
			return SetReal(lhs, -INFINITY);
		return SetReal(lhs, INFINITY);
	}

	if (lhs.type == ValueType_Bool)
		return SetReal(lhs, lhs.u.boolean ? INFINITY : NAN);

	return SetReal(lhs, NAN);
}

Value &ValueMod(Value &lhs, const Value &rhs)
{
	switch (rhs.type) // Notice switch on rhs
	{
	default:
		return SetReal(lhs, NAN);
	case ValueType_Integer:
		if (rhs.u.integer == 0)
			return SetReal(lhs, NAN);

		if (lhs.type == ValueType_Integer)
			return SetInteger(lhs, lhs.u.integer % rhs.u.integer);
		if (lhs.type == ValueType_Real)
			return SetReal(lhs, fmod(lhs.u.real, (double)rhs.u.integer));
		return SetInteger(lhs, 0);
	case ValueType_Real:
		if (rhs.u.real == 0.0)
			return SetReal(lhs, NAN);

		if (lhs.type == ValueType_Real)
			return SetReal(lhs, fmod(lhs.u.real, rhs.u.real));
		if (rhs.type == ValueType_Integer)
			return SetReal(lhs, fmod((double)lhs.u.integer, rhs.u.real));
		return SetInteger(lhs, 0);
	case ValueType_Bool:
		if (!rhs.u.boolean)
			return SetReal(lhs, NAN);
		return SetInteger(lhs, 0);
	}
}

Value &ValueNeg(Value &lhs)
{
	switch (lhs.type)
	{
	default:
		return SetInteger(lhs, 0);
	case ValueType_Integer:
		return SetInteger(lhs, -lhs.u.integer);
	case ValueType_Real:
		return SetReal(lhs, -lhs.u.real);
	case ValueType_Bool:
		return SetInteger(lhs, lhs.u.boolean ? -1 : 0);
	}
}

Value &ValueGreater(Value &lhs, const Value &rhs)
{
	switch (lhs.type)
	{
	default:
		return SetBool(lhs, true);
	case ValueType_Integer:
		if (rhs.type == ValueType_Integer)
			return SetBool(lhs, lhs.u.integer > rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetBool(lhs, lhs.u.integer > rhs.u.real);
		if (rhs.type == ValueType_Bool)
			return SetBool(lhs, lhs.u.integer > (rhs.u.boolean ? 1 : 0));
		return SetBool(lhs, false);
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return SetBool(lhs, lhs.u.real > rhs.u.real);
		if (rhs.type == ValueType_Integer)
			return SetBool(lhs, lhs.u.real > rhs.u.integer);
		if (rhs.type == ValueType_Bool)
			return SetBool(lhs, lhs.u.real > (rhs.u.boolean ? 1.0 : 0.0));
		return SetBool(lhs, false);
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return SetBool(lhs, lhs.u.boolean && !rhs.u.boolean);
		if (rhs.type == ValueType_Integer)
			return SetBool(lhs, (lhs.u.boolean ? 1 : 0) > rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetBool(lhs, (lhs.u.boolean ? 1 : 0) > rhs.u.real);
		return SetBool(lhs, false);
	}
}

Value &ValueLess(Value &lhs, const Value &rhs)
{
	switch (lhs.type)
	{
	default:
		return SetBool(lhs, false);
	case ValueType_Integer:
		if (rhs.type == ValueType_Integer)
			return SetBool(lhs, lhs.u.integer < rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetBool(lhs, lhs.u.integer < rhs.u.real);
		if (rhs.type == ValueType_Bool)
			return SetBool(lhs, lhs.u.integer < (rhs.u.boolean ? 1 : 0));
		return SetBool(lhs, true);
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return SetBool(lhs, lhs.u.real < rhs.u.real);
		if (rhs.type == ValueType_Integer)
			return SetBool(lhs, lhs.u.real < rhs.u.integer);
		if (rhs.type == ValueType_Bool)
			return SetBool(lhs, lhs.u.real < (rhs.u.boolean ? 1.0 : 0.0));
		return SetBool(lhs, true);
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return SetBool(lhs, !lhs.u.boolean && rhs.u.boolean);
		if (rhs.type == ValueType_Integer)
			return SetBool(lhs, (lhs.u.boolean ? 1 : 0) < rhs.u.integer);
		if (rhs.type == ValueType_Real)
			return SetBool(lhs, (lhs.u.boolean ? 1 : 0) < rhs.u.real);
		return SetBool(lhs, true);
	}
}

Value &AllocString(Value &v, int64_t len)
{
	if (len <= 0)
		return SetEmpty(v);

	ReleaseValue(v);

	v.type = ValueType_String;
	v.u.string = (String *)calloc(1, offsetof(String, str) + len + 1);
	if (!v.u.string)
	{
		v.type = ValueType_None;
		return v;
	}

	v.u.string->len = len;
	v.u.ref->count = 1;

	return v;
}

Value &AllocList(Value &v, int64_t len)
{
	ReleaseValue(v);

	if (len < 0)
		len = 0;

	v.type = ValueType_List;
	v.u.list = (List *)calloc(1, sizeof(List));
	if (!v.u.list)
	{
		v.type = ValueType_None;
		return v;
	}

	List *list = v.u.list;

	list->ref.count = 1;
	list->len = len;
	list->capacity = std::max<int64_t>(INITIAL_CAPACITY, len);
	list->values = (Value *)calloc(list->capacity, sizeof(Value));
	if (!list->values)
	{
		free(list);
		v.type = ValueType_None;
		return v;
	}

	for (int64_t i = 0; i < len; i++)
		InitializeValue(list->values[i]);

	return v;
}

Value &RetainValue(Value &v)
{
	if (!(v.flags & VALUE_STATIC))
	{
		if (v.type == ValueType_String || v.type == ValueType_List)
		{
			assert(v.u.ref->count > 0);
			v.u.ref->count++;
		}
	}

	return v;
}

void ReleaseValue(Value &v)
{
	if (!(v.flags & VALUE_STATIC))
	{
		if (v.type == ValueType_String || v.type == ValueType_List)
		{
			assert(v.u.ref->count > 0);

			if (--v.u.ref->count == 0)
				FreeValue(v);
		}
	}

	v.type = ValueType_None;
	v.u.ref = nullptr;
}

void FreeValue(Value &v)
{
	assert(!(v.flags & VALUE_STATIC));

	if (v.type == ValueType_String)
	{
		assert(v.u.ref->count == 0);
		free(v.u.string);
		v.u.ref = nullptr;
		v.type = ValueType_None;
	}
	else if (v.type == ValueType_List)
	{
		assert(v.u.ref->count == 0);

		for (int64_t i = 0; i < v.u.list->len; i++)
			ReleaseValue(v.u.list->values[i]);
		free(v.u.list);
	}
}
