#pragma once

#include <type_traits>
#include <cstdint>

//! \brief Object that maintains a reference count.
//! 
//! Used to implement automatic memory management. Objects
//! are initalized with a reference count of 0. If you want
//! to keep the object around, you must call Retain. When
//! you are done with the object, you must call Release. If
//! the object has not been retained, you must make sure
//! you manually delete it.
class RefCounted
{
public:
	//! \brief Retains the object, incrementing the reference
	//! count.
	void Retain() const;

	//! \brief Releases the object, deleting it if the
	//! reference count reaches zero.
	//! 
	//! Using the object after calling Release is undefined.
	void Release() const;

	RefCounted();
	virtual ~RefCounted();
private:
	mutable int64_t _refCount;
};

//! \brief Shortcut for Retain, but performs nullptr check.
//! 
//! \param obj The object to retain.
//! 
//! \return obj.
template <typename T>
inline T *Retain(T *obj)
{
	static_assert(std::is_base_of<RefCounted, T>::value, "T must be derived from RefCounted");
	if (obj)
		obj->Retain();
	return obj;
}

//! \brief Shortcut for Release, but performs nullptr check.
//! 
//! \param obj The object to release.
//! 
//! \return nullptr.
template <typename T>
inline T *Release(T *obj)
{
	static_assert(std::is_base_of<RefCounted, T>::value, "T must be derived from RefCounted");
	if (obj)
		obj->Release();
	return nullptr;
}

//! \brief Automatically releases an object when it goes out of scope.
//! 
//! When the object is created, it retains the object.
template <typename T>
class AutoRelease
{
public:
	constexpr T *operator->() const { return _obj; }
	constexpr T &operator*() const { return *_obj; }
	
	//! \brief Return a raw pointer to the object.
	//!
	//! The object will not be retained before being returned.
	//!
	//! \return A raw pointer to the object.
	constexpr T *get() const { return _obj; }

	template <typename U>
	constexpr AutoRelease<U> cast() const { return reinterpret_cast<U *>(_obj); }

	template <typename U>
	inline AutoRelease<T> &set(AutoRelease<U> &other)
	{
		if (_obj != other.get())
		{
			Release(_obj);
			_obj = Retain(reinterpret_cast<T *>(other.get()));
		}
		return *this;
	}

	template <typename U>
	inline AutoRelease<T> &set(U *other)
	{
		if (_obj != other)
		{
			Release(_obj);
			_obj = Retain(reinterpret_cast<T *>(other));
		}
		return *this;
	}

	//! \brief Check if the object is not nullptr.
	//! 
	//! \return True if the object is not nullptr.
	constexpr operator bool() const { return _obj != nullptr; }

	inline AutoRelease<T> &operator=(std::nullptr_t n)
	{
		_obj = Release(_obj);
		return *this;
	}

	template <typename U>
	inline AutoRelease<T> &operator=(U *obj)
	{
		if (_obj != obj)
		{
			_obj = Release(_obj);
			_obj = Retain(static_cast<T *>(obj));
		}
		return *this;
	}

	template <typename U>
	inline AutoRelease<T> &operator=(AutoRelease<U> &other)
	{
		if (this != reinterpret_cast<AutoRelease<T> *>(&other))
		{
			_obj = Release(_obj);
			_obj = Retain(static_cast<T *>(other.get()));
		}
		return *this;
	}

	template <typename U>
	inline AutoRelease<T> &operator=(AutoRelease<U> &&other)
	{
		AutoRelease<T> *tmp = reinterpret_cast<AutoRelease<T> *>(&other);
		if (this != tmp)
		{
			Release(_obj);
			_obj = tmp->_obj;
			tmp->_obj = nullptr;
		}

		return *this;
	}
	
	constexpr AutoRelease() : _obj(nullptr) {}

	constexpr AutoRelease(std::nullptr_t n) : _obj(nullptr) {}

	template <typename U>
	inline AutoRelease(U *obj) : _obj(Retain(static_cast<T *>(obj))) {}
	
	template <typename U>
	inline AutoRelease(AutoRelease<U> &other) : _obj(Retain(static_cast<T *>(other.get()))) {}

	inline AutoRelease(const AutoRelease &other) : _obj(Retain(other._obj)) {}
	constexpr AutoRelease(AutoRelease &&other) : _obj(other._obj) { other._obj = nullptr; }
	inline ~AutoRelease() { _obj = Release(_obj); }
private:
	T *_obj;
};

template <typename T, typename U>
constexpr bool operator==(const AutoRelease<T> &a, const AutoRelease<U> &b)
{
	return *a == *b;
}

template <typename T>
constexpr bool operator==(const AutoRelease<T> &a, const void *b)
{
	return *a == b;
}

template <typename T>
constexpr bool operator==(const AutoRelease<T> &a, std::nullptr_t b)
{
	return a.get() == nullptr;
}

template <typename T>
constexpr bool operator==(const void *a, const AutoRelease<T> &b)
{
	return a == *b;
}

template <typename T>
constexpr bool operator==(std::nullptr_t a, const AutoRelease<T> &b)
{
	return b.get() == nullptr;
}

template <typename T, typename U>
constexpr bool operator!=(const AutoRelease<T> &a, const AutoRelease<U> &b)
{
	return *a != *b;
}

template <typename T>
constexpr bool operator!=(const AutoRelease<T> &a, const void *b)
{
	return *a != b;
}

template <typename T>
constexpr bool operator!=(const void *a, const AutoRelease<T> &b)
{
	return a != *b;
}

template <typename T>
constexpr bool operator!=(const AutoRelease<T> &a, std::nullptr_t b)
{
	return a.get() != nullptr;
}

template <typename T>
constexpr bool operator!=(std::nullptr_t a, const AutoRelease<T> &b)
{
	return b.get() != nullptr;
}
