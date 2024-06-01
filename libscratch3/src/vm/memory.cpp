#include "memory.hpp"

#include <cassert>

#include <lysys/lysys.hpp>

#define TRUE_STRING "true"
#define FALSE_STRING "false"

#define TRUE_SIZE (sizeof(TRUE_STRING) - 1)
#define FALSE_SIZE (sizeof(FALSE_STRING) - 1)

static constexpr uint32_t HashString(const char *s)
{
	uint32_t hash = 1315423911;
	while (*s)
		hash ^= ((hash << 5) + *s++ + (hash >> 2));
	return hash;
}

static constexpr uint32_t kTrueHash = HashString(TRUE_STRING);
static constexpr uint32_t kFalseHash = HashString(FALSE_STRING);

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
	case ValueType_Bool:
		return val.u.boolean;
	case ValueType_String:
		return StringEquals(val.u.string->str, TRUE_STRING);
	case ValueType_BasicString:
		return StringEquals(val.u.basic_string, TRUE_STRING);
	case ValueType_ConstString:
		return StringEquals(val.u.const_string->c_str(), TRUE_STRING);
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
		return false;
	case ValueType_Real:
		if (rhs.type == ValueType_Real)
			return lhs.u.real == rhs.u.real;
		if (rhs.type == ValueType_Integer)
			return lhs.u.real == rhs.u.integer;
		return false;
	case ValueType_Bool:
		if (rhs.type == ValueType_Bool)
			return lhs.u.boolean == rhs.u.boolean;
		return false;
	case ValueType_String:
		if (lhs.hash != rhs.hash)
			return false;

		if (rhs.type == ValueType_String)
			return StringEquals(lhs.u.string->str, rhs.u.string->str);
		else if (rhs.type == ValueType_BasicString)
			return StringEquals(lhs.u.string->str, rhs.u.basic_string);
		else if (rhs.type == ValueType_ConstString)
			return StringEquals(lhs.u.string->str, rhs.u.const_string->c_str());
		return false;
	case ValueType_BasicString:
		if (lhs.hash != rhs.hash)
			return false;

		if (rhs.type == ValueType_String)
			return StringEquals(lhs.u.basic_string, rhs.u.string->str);
		else if (rhs.type == ValueType_BasicString)
			return StringEquals(lhs.u.basic_string, rhs.u.basic_string);
		else if (rhs.type == ValueType_ConstString)
			return StringEquals(lhs.u.basic_string, rhs.u.const_string->c_str());
		return false;
	case ValueType_ConstString:
		if (lhs.hash != rhs.hash)
			return false;

		if (rhs.type == ValueType_String)
			return StringEquals(lhs.u.const_string->c_str(), rhs.u.string->str);
		else if (rhs.type == ValueType_BasicString)
			return StringEquals(lhs.u.const_string->c_str(), rhs.u.basic_string);
		else if (rhs.type == ValueType_ConstString)
			return StringEquals(lhs.u.const_string->c_str(), rhs.u.const_string->c_str());
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
	lhs.hash = HashString(lhs.u.string->str);
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

Value &SetBasicString(Value &lhs, const char *rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_BasicString;
	lhs.u.basic_string = rhs;
	lhs.hash = HashString(rhs);
	return lhs;
}

Value &SetConstString(Value &lhs, const std::string *rhs)
{
	ReleaseValue(lhs);
	lhs.type = ValueType_ConstString;
	lhs.u.const_string = rhs;
	lhs.hash = HashString(rhs->c_str());
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

Value &SetParsedBasicString(Value &lhs, const char *rhs)
{
	if (ParseString(lhs, rhs) != ValueType_None)
		return lhs;
	return SetBasicString(lhs, rhs);
}

Value &SetParsedConstString(Value &lhs, const std::string *rhs)
{
	if (ParseString(lhs, *rhs) != ValueType_None)
		return lhs;
	return SetConstString(lhs, rhs);
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
	if (index.type == ValueType_String || index.type == ValueType_BasicString || index.type == ValueType_ConstString)
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

void CvtString(Value &v)
{
	char buf[64];
	int cch;

	switch (v.type)
	{
	default:
	case ValueType_String:
	case ValueType_BasicString:
	case ValueType_ConstString:
		break;
	case ValueType_Integer:
		cch = snprintf(buf, sizeof(buf), "%lld", v.u.integer);
		SetString(v, buf, cch);
		break;
	case ValueType_Real:
		if (v.u.real == NAN)
		{
			SetBasicString(v, "NaN");
			break;
		}

		if (v.u.real == INFINITY)
		{
			SetBasicString(v, "Infinity");
			break;
		}

		if (v.u.real == -INFINITY)
		{
			SetBasicString(v, "-Infinity");
			break;
		}

		cch = snprintf(buf, sizeof(buf), "%.8g", v.u.real);
		SetString(v, buf, cch);
		break;
	case ValueType_Bool:
		SetBasicString(v, v.u.boolean ? TRUE_STRING : FALSE_STRING);
		break;
	case ValueType_List:
		SetBasicString(v, "<list>");
		break;
	}
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
	case ValueType_BasicString:
		return strlen(v.u.basic_string);
	case ValueType_ConstString:
		return v.u.const_string->size();
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
	case ValueType_Bool:
	case ValueType_String:
	case ValueType_BasicString:
	case ValueType_ConstString:
	case ValueType_List:
		return 0;
	case ValueType_Real:
		return static_cast<int64_t>(round(v.u.real));
	case ValueType_Integer:
		return v.u.integer;
	}
}

double ToReal(const Value &v)
{
	switch (v.type)
	{
	default:
	case ValueType_None:
	case ValueType_Bool:
	case ValueType_String:
	case ValueType_BasicString:
	case ValueType_ConstString:
	case ValueType_List:
		return 0.0;
	case ValueType_Real:
		return v.u.real;
	case ValueType_Integer:
		return static_cast<double>(v.u.integer);
	}
}

const char *ToString(const Value &v, int64_t *len)
{
	static LS_THREADLOCAL char buf[64];
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
	case ValueType_BasicString:
		if (len) *len = strlen(v.u.basic_string);
		return v.u.basic_string;
	case ValueType_ConstString:
		if (len) *len = v.u.const_string->size();
		return v.u.const_string->c_str();
	case ValueType_List:
		if (len) *len = 6;
		return "<list>";
	}
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
	case ValueType_BasicString:
		if (len) *len = strlen(v.u.basic_string);
		return v.u.basic_string;
	case ValueType_ConstString:
		if (len) *len = v.u.const_string->size();
		return v.u.const_string->c_str();
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
	if (v.type == ValueType_String || v.type == ValueType_List)
	{
		assert(v.u.ref->count > 0);
		v.u.ref->count++;
	}

	return v;
}

void ReleaseValue(Value &v)
{
	if (v.type == ValueType_String || v.type == ValueType_List)
	{
		assert(v.u.ref->count > 0);

		if (--v.u.ref->count == 0)
			FreeValue(v);
	}

	v.type = ValueType_None;
	v.u.ref = nullptr;
}

void FreeValue(Value &v)
{
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
