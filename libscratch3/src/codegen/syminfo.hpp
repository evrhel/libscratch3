#pragma once

#include "../ref.hpp"
#include "../vm/memory.hpp"

class KnownState;

class SymInfo final
{
public:
	constexpr bool HasValue() const { return _hasValue; }
	constexpr const Value &GetValue() const { return _value; }

	Value &SetValue(const Value &value);
	void ClearValue();

	SymInfo &operator=(const SymInfo &si);
	SymInfo &operator=(SymInfo &&si) noexcept;

	SymInfo();
	SymInfo(const SymInfo &si);
	~SymInfo();
private:
	bool _hasValue;
	Value _value;
};
