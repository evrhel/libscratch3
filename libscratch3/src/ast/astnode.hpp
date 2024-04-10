#pragma once

#include <vector>
#include <string>
#include <cstdlib>
#include <memory>
#include <unordered_map>

#include "visitor.hpp"

class Visitor;
class ASTStack;

struct ASTNode
{
	static constexpr AstType TYPE = Ast_ASTNode;
	
	// Accept a visitor
	inline virtual void Accept(Visitor *visitor) { }

	// Set an input value
	inline virtual bool SetInput(const std::string &key, Expression *value) { return false; }

	inline virtual bool SetField(const std::string &key,
		const std::string &value, const std::string &id) { return false; }

	// Return a string representation of the node.
	inline virtual std::string ToString() { return AstTypeString(_types[0]); }

	// Return the type of the node.
	inline AstType GetType() const { return _types[0]; }

	// Return true if the node is of the given type.
	inline bool Is(AstType type) const
	{
		// Walk inheritance tree
		for (auto t : _types)
			if (t == type)
				return true;
		return false;
	}

	// Convert the node to the given type, returns nullptr if the node is not
	// of the given type.
	template <typename T>
	inline T *As()
	{
		if (Is(T::TYPE))
			return static_cast<T *>(this);
		return 0;
	}

	std::string nodeid;

	inline ASTNode() { }
	virtual ~ASTNode() { }
protected:
	inline void SetType(AstType type) { _types.insert(_types.begin(), type); }
private:
	std::vector<AstType> _types;
};

// Convert node to string, returns "(null)" if node is nullptr.
inline std::string AsString(ASTNode *node)
{
	return node ? node->ToString() : "(null)";
}

// Stack of AST nodes
class ASTStack
{
public:
	inline size_t GetSize() const { return _stack.size(); }
	inline bool IsEmpty() const { return _stack.empty(); }
	inline ASTNode *GetTop() const { return _stack.back(); }
	inline ASTStack &Push(ASTNode *node) { _stack.push_back(node); return *this; }
	inline ASTStack &Pop() { _stack.pop_back(); return *this; }
	inline ASTStack &Pop(size_t count) { _stack.resize(_stack.size() - count); return *this; }
	inline ASTStack &Clear() { _stack.clear(); return *this; }
	inline ASTNode *&operator[](size_t index) { return _stack[index]; }
private:
	std::vector<ASTNode *> _stack;
};
