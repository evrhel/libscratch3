#include "ref.hpp"

#include <cassert>

void RefCounted::Retain() const
{
	assert(_refCount >= 0);

	_refCount++;
}

void RefCounted::Release() const
{
	assert(_refCount > 0);

	_refCount--;
	if (_refCount == 0)
		delete this;
}

RefCounted::RefCounted() :
	_refCount(0) {}

RefCounted::~RefCounted()
{
	assert(_refCount == 0);
}
