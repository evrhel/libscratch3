#pragma once

enum ExceptionType
{
	Exception_None,

	OutOfMemory,
	StackOverflow,
	StackUnderflow,
	VariableNotFound,
	IllegalOperation,
	InvalidArgument,
	UnsupportedOperation,

	NotImplemented,

	VMError // Internal error
};

//! \brief Get a string representation of an exception type.
//!
//! \param type The exception type.
//!
//! \return A string representation of the exception type.
const char *ExceptionString(ExceptionType type);
