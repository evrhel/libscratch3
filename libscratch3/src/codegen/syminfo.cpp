#include "syminfo.hpp"

Value &SymInfo::SetValue(const Value &value)
{
	_hasValue = true;
	return Assign(_value, value);
}

void SymInfo::ClearValue()
{
	ReleaseValue(_value);
	_hasValue = false;
}

SymInfo &SymInfo::operator=(const SymInfo &si)
{
	if (this == &si)
		return *this;

	Assign(_value, si._value);

	return *this;
}

SymInfo &SymInfo::operator=(SymInfo &&si) noexcept
{
	if (this == &si)
		return *this;

	ReleaseValue(_value);

	if (si._hasValue)
		_value = si._value;

	memset(&si._value, 0, sizeof(si._value));

	return *this;
}

SymInfo::SymInfo() :
	_hasValue(false)
{
	InitializeValue(_value);
}

SymInfo::SymInfo(const SymInfo &si) :
	_hasValue(si._hasValue)
{
	InitializeValue(_value);
	
	if (_hasValue)
		Assign(_value, si._value);
}

SymInfo::~SymInfo()
{
	ReleaseValue(_value);
}
