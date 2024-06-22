#include "exception.hpp"

const char *ExceptionString(ExceptionType type)
{
	switch (type)
	{
	default:
		return "Unknown exception";
	case Exception_None:
		return "No exception";
	case OutOfMemory:
		return "Out of memory";
	case StackOverflow:
		return "Stack overflow";
	case StackUnderflow:
		return "Stack underflow";
	case VariableNotFound:
		return "Variable not found";
	case IllegalOperation:
		return "Illegal operation";
	case InvalidArgument:
		return "Invalid argument";
	case UnsupportedOperation:
		return "Unsupported operation";
	case AccessViolation:
		return "Access violation";
	case NotImplemented:
		return "Not implemented";
	case VMError:
		return "VM error";
	}
}
