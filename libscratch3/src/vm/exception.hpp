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

const char *ExceptionString(ExceptionType type);
