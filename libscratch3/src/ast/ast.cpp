#include "ast.hpp"

#include <lysys/lysys.hpp>

#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <cmath>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

//! \brief Check whether an opcode is an event handler.
//! 
//! \param opcode The opcode to check. This is the value of the "opcode"
//! member in a block object, not the internal opcode values this library
//! uses.
//! 
//! \return True if the opcode is an event handler, false otherwise.
static bool IsEventHandler(const std::string &opcode);

//! \brief Create an AST node from an opcode.
//! 
//! \param opcode The opcode to create a node from. Has same restrictions
//! as IsEventHandler().
//! 
//! \return An ASTNode object representing the opcode, or nullptr if the
//! opcode is unknown or unsupported.
static ASTNode *NodeFromOpcode(const std::string &opcode);

//! \brief Interpret a string as a literal value.
//! 
//! Tries to interpret a JSON string as a literal value. The function
//! will attempt to parse the string as an integer, double, boolean,
//! or null. If the string is not a literal value, it will be interpreted
//! as a string.
//! 
//! \param v The JSON value to interpret. Must be a string.
//! 
//! \return A Constexpr node representing the literal value. Never returns
//! nullptr.
static Constexpr *InterpretString(rapidjson::Value &v)
{
	std::string val = v.GetString();

	// empty string is a None
	if (val.empty())
		return new None();

	// attempt to parse as a positive integer
	if (val.find_first_not_of("0123456789") == std::string::npos)
	{
		PositiveInt *pi = new PositiveInt();
		pi->value = val;
		return pi;
	}

	// attempt to parse as an integer
	if (val[0] == '-' && val.find_first_not_of("0123456789", 1) == std::string::npos)
	{
		Int *i = new Int();
		i->value = val;
		return i;
	}

	// attempt to parse as a positive number
	if (val.find_first_not_of("0123456789.") == std::string::npos)
	{
		PositiveNumber *pn = new PositiveNumber();
		pn->value = val;
		return pn;
	}

	// attempt to parse as a number
	if (val[0] == '-' && val.find_first_not_of("0123456789.", 1) == std::string::npos)
	{
		Number *n = new Number();
		n->value = val;
		return n;
	}

	// attempt to parse a "true" or "false" boolean literal
	if (val == "true")
	{
		True *t = new True();
		t->value = val;
		return t;
	}

	if (val == "false")
	{
		False *f = new False();
		f->value = val;
		return f;
	}

	// finally, it's just a string

	String *s = new String();
	s->value = v.GetString();
	return s;
}

//! \brief Parse a literal value from a JSON value.
//! 
//! Tries to parse a literal value from a JSON value. The value can be
//! a string, integer, double, boolean, or null. If the value is a string,
//! the function will attempt to interpret it as a more specific type
//! (e.g. integer, double, boolean, etc.).
//! 
//! \param v The JSON value to parse.
//! 
//! \return A Constexpr object representing the literal value, or nullptr
//! if parsing failed.
static Constexpr *ParseLiteral(rapidjson::Value &v)
{
	if (v.IsString())
		return InterpretString(v);

	if (v.IsInt())
	{
		int ival = v.GetInt();
		if (ival >= 0)
		{
			PositiveInt *pi = new PositiveInt();
			pi->value = std::to_string(ival);
			return pi;
		}
		else
		{
			Int *i = new Int();
			i->value = std::to_string(ival);
			return i;
		
		}
	}

	if (v.IsDouble())
	{
		double dval = v.GetDouble();
		if (dval >= 0)
		{
			PositiveNumber *pn = new PositiveNumber();
			pn->value = std::to_string(dval);
			return pn;
		}
		else
		{
			Number *n = new Number();
			n->value = std::to_string(dval);
			return n;
		}
	}

	if (v.IsBool())
	{
		bool bval = v.GetBool();
		if (bval)
		{
			True *t = new True();
			t->value = "true";
			return t;
		}
		else
		{
			False *f = new False();
			f->value = "false";
			return f;
		}
	}

	if (v.IsNull())
	{
		None *n = new None();
		n->value = "";
		return n;
	}

	// Unsupported literal type
	return nullptr;
}

class Parser
{
public:
	// Parse JSON string into AST
	Program *Parse(const char *json, size_t length)
	{
		_defs.clear();

		rapidjson::Document doc;
		doc.Parse(json, length);
		if (doc.HasParseError())
		{
			Error("Malformed JSON: %s", rapidjson::GetParseError_En(doc.GetParseError()));
			return nullptr;
		}

		Program *p = new Program();

		// "targets" member
		if (doc.HasMember("targets"))
		{
			rapidjson::Value &targets = doc["targets"];
			if (targets.IsArray())
			{
				p->sprites = new SpriteDefList();
				ParseTargets(targets, p);

				if (p->sprites->sprites.size() == 0)
				{
					Warn("No targets found in project");
					delete p->sprites;
					p->sprites = nullptr;
				}
			}
			else
				Error("Expected array parsing targets");
		}
		else
			Warn("Missing `targets` member");

		// "monitors" member
		if (doc.HasMember("monitors"))
		{
			rapidjson::Value &monitors = doc["monitors"];
			if (monitors.IsArray())
			{
				// TODO: implement monitors
			}
			else
				Error("Expected array parsing monitors");
		}
		else
			Warn("Missing `monitors` member");

		// "extensions" member
		if (doc.HasMember("extensions"))
		{
			rapidjson::Value &extensions = doc["extensions"];
			if (extensions.IsArray())
			{
				// TODO: implement extensions
			}
			else
				Error("Expected array parsing extensions");
		}
		else
			Warn("Missing `extensions` member");

		if (are_errors)
			goto failure;

		return p;
	failure:
		delete p;
		return nullptr;
	}

	Parser(Scratch3_LogCallback log, void *up) :
		_log(log), _up(up) {}
private:
	std::unordered_map<std::string, ASTNode *> _defs;

	Scratch3_LogCallback _log;
	void *_up;
	bool are_errors = false;

	// Write an error message
	void Error(const char *format, ...)
	{
		char error[512];

		are_errors = true;

		if (_log)
		{
			va_list args;
			va_start(args, format);
			vsprintf_s(error, format, args);
			va_end(args);

			_log(_up, Scratch3_Error, error);
		}
	}

	// Write a warning message
	void Warn(const char *format, ...)
	{
		char error[512];

		if (_log)
		{
			va_list args;
			va_start(args, format);
			vsprintf_s(error, format, args);
			va_end(args);

			_log(_up, Scratch3_Warning, error);
		}
	}

	// Write an informational message
	void Info(const char *format, ...)
	{
		char error[512];

		if (_log)
		{
			va_list args;
			va_start(args, format);
			vsprintf_s(error, format, args);
			va_end(args);

			_log(_up, Scratch3_Info, error);
		}
	}
	
	//! \brief Parse the "targets" member
	//! 
	//! The "targets" member is an array of objects, each representing
	//! a sprite or stage.
	//! 
	//! \param targets The "targets" member. Must be an array.
	//! \param p The program object to populate.
	void ParseTargets(rapidjson::Value &targets, Program *p)
	{
		assert(targets.IsArray());

		for (unsigned i = 0; i < targets.Size(); i++)
		{
			rapidjson::Value &target = targets[i];
			if (!target.IsObject())
			{
				Error("Expected object parsing target at index %u", i);
				continue;
			}
			
			SpriteDef *sd = ParseSprite(target);
			if (sd)
				p->sprites->sprites.push_back(sd);
		}
	}

	//! \brief Parse a sprite or stage object.
	//! 
	//! \param target The object representing the sprite or stage.
	//! Must be an object.
	//! 
	//! \return A SpriteDef object, or nullptr if parsing failed.
	SpriteDef *ParseSprite(rapidjson::Value &target)
	{
		assert(target.IsObject());

		SpriteDef *sd = new SpriteDef();
		sd->variables = new VariableDefList();
		sd->lists = new ListDefList();
		sd->scripts = new StatementListList();

		if (!target.HasMember("name"))
		{
			Error("Missing `name` member in target");
			goto failure;
		}

		if (!target["name"].IsString())
		{
			Error("Expected string parsing `name` member in target");
			goto failure;
		}

		sd->name = target["name"].GetString();

		// Variables that this target defines
		// If the target is a sprite, these are variables local
		// to the sprite. If the target is the stage, these are
		// global variables.
		if (target.HasMember("variables"))
		{
			rapidjson::Value &variables = target["variables"];
			if (!variables.IsObject())
			{
				Error("Expected object parsing variables in target");
				goto failure;
			}

			ParseVariables(variables, sd->variables);

			if (sd->variables->variables.size() == 0)
			{
				delete sd->variables;
				sd->variables = nullptr;
			}
		}

		// Lists that this target defines
		// Like variables, these are local to the sprite or global
		// to the stage.
		if (target.HasMember("lists"))
		{
			rapidjson::Value &lists = target["lists"];
			if (!lists.IsObject())
			{
				Error("Expected object parsing lists in target");
				goto failure;
			}

			ParseLists(lists, sd->lists);

			if (sd->lists->lists.size() == 0)
			{
				delete sd->lists;
				sd->lists = nullptr;
			}
		}

		// Blocks in the target
		if (target.HasMember("blocks"))
		{
			rapidjson::Value &blocks = target["blocks"];
			if (!blocks.IsObject())
			{
				Error("Expected object parsing blocks in target");
				goto failure;
			}

			_defs.clear();

			// Iterate over all blocks in the target, we only care about
			// top-level blocks that are event handlers. Non-event handlers
			// are unreachable and are discarded. If a block is not a
			// top-level block, we skip it, it will be parsed when we reach
			// it when traversing the script via the `next` member.
			for (auto it = blocks.MemberBegin(); it != blocks.MemberEnd(); ++it)
			{
				rapidjson::Value &target = it->value;
				std::string id = it->name.GetString();

				if (!target.IsObject())
				{
					Error("Expected object parsing block `%s`", id.c_str());
					continue;
				}

				if (!target.HasMember("topLevel"))
				{
					Error("Missing `topLevel` member in block `%s`", id.c_str());
					continue;
				}

				if (!target["topLevel"].GetBool())
					continue; // ignore non-top-level targets

				// retrieve the opcode

				if (!target.HasMember("opcode"))
				{
					Error("Missing `opcode` member in block `%s`", id.c_str());
					continue;
				}

				rapidjson::Value &opcode = target["opcode"];
				if (!opcode.IsString())
				{
					Error("Expected string parsing opcode in block `%s`", id.c_str());
					continue;
				}

				// check if the opcode is an event handler
				if (!IsEventHandler(opcode.GetString()))
				{
					Info("Discarding unreachable block `%s` (%s)", id.c_str(), opcode.GetString());
					continue;
				}

				// traverse the script starting from this block
				StatementList *sl = new StatementList();
				ParseScript(blocks, id, sl);

				// add the script to the list of scripts
				if (sl->sl.size() > 0)
					sd->scripts->sll.push_back(sl);
				else
				{
					Warn("Empty script `%s`", id.c_str());
					delete sl;
				}
			}
		}
		else
			Warn("Missing `blocks` member in target");

		// TODO: parse other members

		return sd;
	failure:
		delete sd;
		return nullptr;
	}

	//! \brief Parse the "variables" member of a target
	//!
	//! \param variables The "variables" member. Must be an object.
	//! \param vdl The VariableDefList object to populate.
	void ParseVariables(rapidjson::Value &variables, VariableDefList *vdl)
	{
		// Iterate over all variables in the target
		//
		// Variables are defined in the format:
		// "id": ["name", value]
		for (auto it = variables.MemberBegin(); it != variables.MemberEnd(); ++it)
		{
			std::string id = it->name.GetString();
			rapidjson::Value &arr = it->value;
			if (!arr.IsArray())
			{
				Error("Expected array parsing variable `%s`", id.c_str());
				continue;
			}

			if (arr.Size() < 2)
			{
				Error("Expected at least 2 elements parsing variable `%s`", id.c_str());
				continue;
			}

			rapidjson::Value &name = arr[0];
			if (!name.IsString())
			{
				Error("Expected string parsing name in variable `%s`", id.c_str());
				continue;
			}

			// Parse the value of the variable
			Constexpr *ce = ParseLiteral(arr[1]);
			if (!ce)
			{
				Error("Failed to parse value in variable `%s` (%s)", id.c_str());
				continue;
			}

			// Create a VariableDef node and add it to the list

			VariableDef *vd = new VariableDef();
			vd->id = id;
			vd->name = name.GetString();
			vd->value = ce;

			vdl->variables.push_back(vd);
		}
	}

	//! \brief Parse a list definition from the "lists" member of a target.
	//! 
	//! \param lists The "lists" member. Must be an object.
	//! \param ldl The ListDefList object to populate.
	void ParseLists(rapidjson::Value &lists, ListDefList *ldl)
	{
		assert(lists.IsObject());

		// Iterate over all lists in the target
		//
		// Lists are defined in the format:
		// "id": ["name", [value1, value2, ...]]
		for (auto it = lists.MemberBegin(); it != lists.MemberEnd(); ++it)
		{
			std::string id = it->name.GetString();
			rapidjson::Value &arr = it->value;
			if (!arr.IsArray())
			{
				Error("Expected array parsing list `%s`", id.c_str());
				continue;
			}

			if (arr.Size() < 2)
			{
				Error("Expected at least 2 elements parsing list `%s`", id.c_str());
				continue;
			}

			rapidjson::Value &name = arr[0];
			if (!name.IsString())
			{
				Error("Expected string parsing name in list `%s`", id.c_str());
				continue;
			}

			rapidjson::Value &value = arr[1];
			if (!value.IsArray())
			{
				Error("Expected array parsing value in list `%s`", id.c_str());
				continue;
			}

			ListDef *ld = new ListDef();

			// Parse the values in the list
			for (auto lit = value.Begin(); lit != value.End(); ++lit)
			{
				Constexpr *ce = ParseLiteral(*lit);
				if (!ce)
				{
					delete ld;
					Error("Failed to parse value in list `%s`", id.c_str());
					goto next;
				}

				ld->value.push_back(ce);
			}

			// Add the list to the list of lists

			ld->id = id;
			ld->name = name.GetString();

			ldl->lists.push_back(ld);
		next:
			(void)0; // dummy statement
		}
	}

	//! \brief Parse a script starting from a block ID.
	//! 
	//! Parses a script starting from a block ID. The script is a sequence
	//! of blocks that are connected via the "next" member. The script is
	//! stored in a StatementList object. Note that the blocks object is
	//! specific to the target, meaning that in a malicious project, the
	//! blocks object can contain arbitrary blocks that are not part of the
	//! target.
	//! 
	//! \param blocks The "blocks" member of the target. Must be an object.
	//! \param first The ID of the first block in the script.
	//! \param sl The StatementList object to populate.
	void ParseScript(rapidjson::Value &blocks, const std::string &first, StatementList *sl)
	{
		Statement *s;
		
		// Traverse the script starting from the first block
		// See ParseBlock for the format of blocks
		std::string id = first;
		for (;;)
		{
			// Check if the block exists
			if (!blocks.HasMember(id.c_str()))
			{
				Error("Missing block `%s`", first.c_str());
				return;
			}

			rapidjson::Value &block = blocks[id.c_str()];
			if (!block.IsObject())
			{
				Error("Expected object parsing block `%s`", id.c_str());
				return;
			}

			// parse the block
			ASTNode *node = ParseBlock(blocks, id, false);
			if (!node)
				break;

			s = node->As<Statement>();
			if (!s)
			{
				Error("Expected statement, got %s", AstTypeString(node->GetType()));
				_defs.erase(id), delete node;
				break;
			}

			// add the statement to the list
			sl->sl.push_back(s);

			// go to the next block

			if (!block.HasMember("next"))
			{
				Warn("Missing `next` member in block `%s`", id.c_str());
				break;
			}

			rapidjson::Value &next = block["next"];
			if (next.IsNull())
				break; // end of script

			if (!next.IsString())
			{
				Warn("Expected string parsing next block in block `%s`", id.c_str());
				break;
			}

			id = next.GetString();
		}
	}

	//! \brief Parse a block from the "blocks" member of a target.
	//! 
	//! \param blocks The "blocks" member. Must be an object.
	//! \param id The ID of the block to parse.
	//! \param createList Whether to create a list of statements from the block.
	//! If true, the resulting ASTNode object will be a StatementList, only if
	//! the parsed block is a Statement. Use this for parsing branches, such as
	//! "if" blocks.
	//! 
	//! \return An ASTNode object representing the block, or nullptr if parsing
	//! failed.
	ASTNode *ParseBlock(rapidjson::Value &blocks, const std::string &id, bool createList)
	{
		// Blocks are in the format (only relevant members are shown):
		// "id": {
		//    "opcode": "opcode",
		//    "next": "next block id" | null,
		//    "inputs": {
		//       "input1": ...,
		//       ...
		//    },
		//    "fields": {
		//       "field1": ...,
		//       ...
		//    },
		//    "topLevel": true | false
		// }

		auto it = _defs.find(id);
		if (it != _defs.end())
		{
			Error("Circular reference `%s`", id.c_str());
			return nullptr;
		}

		if (!blocks.HasMember(id.c_str()))
		{
			Error("Missing block `%s`", id.c_str());
			return nullptr;
		}

		rapidjson::Value &block = blocks[id.c_str()];
		if (!block.IsObject())
		{
			Error("Expected object paring block `%s`", id.c_str());
			return nullptr;
		}

		if (!block.HasMember("opcode"))
		{
			Error("Missing `opcode` member in block `%s`", id.c_str());
			return nullptr;
		}

		rapidjson::Value &opcode = block["opcode"];
		if (!opcode.IsString())
		{
			Error("Expected string parsing opcode in target `%s`", id.c_str());
			return nullptr;
		}

		// Create an AST node from the opcode
		ASTNode *n = NodeFromOpcode(opcode.GetString());
		if (!n)
		{
			Error("Unknown opcode `%s` in target `%s`", opcode.GetString(), id.c_str());
			return nullptr;
		}

		// Create a StatementList object if requested
		if (createList)
		{
			if (n->Is(Statement::TYPE))
			{
				delete n; // This is inefficient, we are parsing the block twice!
				n = new StatementList();
				ParseScript(blocks, id, n->As<StatementList>());
				return n;
			}

			// block is an expression, don't create a list
		}

		// Parse the inputs member (expressions)
		if (block.HasMember("inputs"))
		{
			// inputs are in the format:
			// "input": [type, value]
			// See BlockType for the types of inputs

			rapidjson::Value &inputs = block["inputs"];
			if (!inputs.IsObject())
			{
				Error("Expected object parsing inputs in block `%s` (%s)", id.c_str(), opcode.GetString());
				delete n;
				return nullptr;
			}

			// iterate over all inputs in the block
			for (auto it = inputs.MemberBegin(); it != inputs.MemberEnd(); ++it)
			{
				std::string key = it->name.GetString();
				ASTNode *val = ParseInput(blocks, it->value);
				if (!val)
				{
					//Error("Failed to parse input `%s` in block `%s` (%s)",
					//	key.c_str(), id.c_str(), opcode.GetString());
					//continue;
					
					Warn("Null input `%s` in block `%s` (%s)",
						key.c_str(), id.c_str(), opcode.GetString());
				}

				// set the input in the node
				if (!n->SetInput(key, val))
				{
					Warn("Unknown or invalid input `%s` in block `%s` (%s)",
						key.c_str(), id.c_str(), opcode.GetString());
				}
			}
		}
		else
			Warn("Missing `inputs` member in block `%s` (%s)", id.c_str(), opcode.GetString());

		// Parse the fields member (dropdowns, text fields, etc.)
		if (block.HasMember("fields"))
		{
			rapidjson::Value &fields = block["fields"];
			if (!fields.IsObject())
			{
				Error("Expected object parsing fields in block `%s` (%s)",
					id.c_str(), opcode.GetString());
				delete n;
				return nullptr;
			}

			// iterate over all fields in the block
			for (auto it = fields.MemberBegin(); it != fields.MemberEnd(); ++it)
			{
				// fields are in the format:
				// "field": ["value", "id"]
				// the "id" member is optional

				std::string key = it->name.GetString();
				if (!it->value.IsArray())
				{
					Warn("Expected array parsing field `%s` in block `%s` (%s)",
						key.c_str(), id.c_str(), opcode.GetString());
					continue;
				}

				std::string svalue, sid;

				rapidjson::Value &field = it->value;

				if (field.Size() < 1)
				{
					Warn("Expected at least 1 element parsing field `%s` in block `%s` (%s)",
						key.c_str(), id.c_str(), opcode.GetString());
					continue;
				}

				rapidjson::Value &valueval = field[0];
				if (!valueval.IsString())
				{
					Warn("Expected string parsing value in field `%s` in block `%s` (%s)",
						key.c_str(), id.c_str(), opcode.GetString());
					continue;
				}
				svalue = valueval.GetString();

				if (field.Size() > 1)
				{
					rapidjson::Value &idval = field[1];
					if (!idval.IsNull())
					{
						if (!idval.IsString())
						{
							Warn("Expected string parsing id in field `%s` in block `%s` (%s)",
								key.c_str(), id.c_str(), opcode.GetString());
							continue;
						}

						sid = idval.GetString();
					}
				}

				// set the field in the node
				if (!n->SetField(key, svalue, sid))
				{
					Warn("Unknown or invalid field `%s` in block `%s` (%s)",
						key.c_str(), id.c_str(), opcode.GetString());
				}
			}
		}
		else
		{
			Warn("Missing `fields` member in block `%s` (%s)",
				id.c_str(), opcode.GetString());
		}

		assert(block.HasMember("topLevel")); // should have been checked in ParseTargets
		
		rapidjson::Value &topLevel = block["topLevel"];
		assert(topLevel.IsBool());

		Statement *s = n->As<Statement>();
		if (s)
			s->topLevel = topLevel.GetBool();

		// we discard top-level targets that are not statements

		_defs[id] = n;
		return n;
	}

	//! \brief Parse an input from a block.
	//! 
	//! These are generally expressions, but may be statements in
	//! the case of branches, such as "if" blocks where its inputs
	//! are statement lists.
	//! 
	//! \param blocks The "blocks" member of the target.
	//! \param v The JSON value to parse.
	//! 
	//! \return An ASTNode object representing the input, or nullptr if
	//! parsing failed.
	ASTNode *ParseInput(rapidjson::Value &blocks, rapidjson::Value &v)
	{
		if (!v.IsArray())
		{
			Error("Expected array parsing expression");
			return nullptr;
		}

		if (v.Size() < 1)
		{
			Error("Expected block type");
			return nullptr;
		}

		if (!v[0].IsInt())
		{
			Error("Expected block type to be an integral value");
			return nullptr;
		}

		// block types are defined in the BlockType enum
		BlockType type = (BlockType)v[0].GetInt();
		switch (type)
		{
		case BlockType_Shadow: {
			if (v.Size() < 2)
			{
				Error("Expected block id parsing shadow block");
				return nullptr;
			}

			if (v[1].IsArray())
				return ParseInput(blocks, v[1]);

			if (v[1].IsNull())
				return nullptr;
			
			if (!v[1].IsString())
			{
				Error("Expected string parsing shadow block");
				return nullptr;
			}

			return ParseBlock(blocks, v[1].GetString(), true);
		}
		case BlockType_NoShadow: {
			if (v.Size() < 2)
			{
				Error("Expected block id parsing no shadow block");
				return nullptr;
			}

			if (v[1].IsArray())
				return ParseInput(blocks, v[1]);

			if (v[1].IsNull())
				return nullptr;

			if (!v[1].IsString())
			{
				Error("Expected string parsing no shadow block");
				return nullptr;
			}

			return ParseBlock(blocks, v[1].GetString(), true);
		}
		case BlockType_ShadowObscured: {
			if (v.Size() < 2)
			{
				Error("Expected block id parsing shadow obscured block");
				return nullptr;
			}

			if (v[1].IsArray())
				return ParseInput(blocks, v[1]);

			if (v[1].IsNull())
				return nullptr;

			if (!v[1].IsString())
			{
				Error("Expected string parsing shadow obscured block");
				return nullptr;
			}

			return ParseBlock(blocks, v[1].GetString(), true);
		}
		case BlockType_Color: {
			if (v.Size() < 2)
			{
				Error("Expected value parsing color block");
				return nullptr;
			}

			if (!v[1].IsString())
			{
				Error("Expected string parsing color block");
				return nullptr;
			}

			Color *c = new Color();
			c->value = v[1].GetString();
			return c;
		}
		case BlockType_Number:
		case BlockType_PositiveNumber:
		case BlockType_PositiveInt:
		case BlockType_Int:
		case BlockType_Angle:
		case BlockType_String: {
			// Parse the literal value
			// We ignore the block type and just parse the value
			// as a literal ourselves and to avoid malicious projects

			if (v.Size() < 2)
			{
				Error("Expected value parsing literal");
				return nullptr;
			}

			Constexpr *ce = ParseLiteral(v[1]);
			if (!ce)
			{
				Error("Failed to parse literal");
				return nullptr;
			}

			return ce;
		}
		case BlockType_Broadcast: {
			if (v.Size() < 2)
			{
				Error("Expected value parsing broadcast block");
				return nullptr;
			}

			// TODO: implement broadcast block

			Error("Broadcast block not implemented");
			return nullptr;
		}
		case BlockType_Variable: {
			// Variables are in the format:
			// [12, "name", "id"]

			if (v.Size() < 3)
			{
				Error("Expected id parsing variable block");
				return nullptr;
			}

			rapidjson::Value &id = v[2];
			if (!id.IsString())
			{
				Error("Expected string parsing id in variable block");
				return nullptr;
			}

			rapidjson::Value &name = v[1];
			if (!name.IsString())
			{
				Error("Expected string parsing name in variable block");
				return nullptr;
			}

			// Create a VariableExpr node
			VariableExpr *var = new VariableExpr();
			var->id = id.GetString();
			var->name = name.GetString();
			return var;
		}
		case BlockType_List: {
			// Lists are in the format:
			// [13, "name", "id"]

			if (v.Size() < 3)
			{
				Error("Expected id parsing list block");
				return nullptr;
			}

			rapidjson::Value &id = v[2];
			if (!id.IsString())
			{
				Error("Expected string parsing id in list block");
				return nullptr;
			}

			rapidjson::Value &name = v[1];
			if (!name.IsString())
			{
				Error("Expected string parsing name in list block");
				return nullptr;
			}

			// Create a ListExpr node
			ListExpr *list = new ListExpr();
			list->id = id.GetString();
			list->name = name.GetString();
			return list;
		}
		default:
			Error("Invalid block type");
			return nullptr;
		}

		// unreachable
	}
};

Program *ParseAST(const char *jsonString, size_t length, Scratch3_LogCallback log, void *up)
{
	Program *p;
	
	double start = ls_time64();

	// parse the AST
	Parser parser(log, up);
	p = parser.Parse(jsonString, length);

	// log the time taken to parse
	if (log)
	{
		double elapsed = ls_time64() - start;

		char message[64];
		sprintf_s(message, "Finished in %g sec", round(elapsed * 1000.0) / 1000.0);

		log(up, Scratch3_Info, message);
	}

	return p;
}

static bool IsEventHandler(const std::string &opcode)
{
	return opcode == "event_whenflagclicked" ||
		opcode == "event_whenkeypressed" ||
		opcode == "event_whenthisspriteclicked" ||
		opcode == "event_whenstageclicked" ||
		opcode == "event_whenbackdropswitchesto" ||
		opcode == "event_whengreaterthan" ||
		opcode == "event_whenbroadcastreceived" ||
		opcode == "control_start_as_clone";
}

static ASTNode *NodeFromOpcode(const std::string &opcode)
{
	// https://en.scratch-wiki.info/wiki/List_of_Block_Opcodes
	// 
	// There are some issues on the wiki as of April 2024:
	// -  The "repeat" block is listed as having opcode "motion_turnright"
	//    but it's actually "control_repeat"

	if (opcode == "motion_movesteps") return new MoveSteps();
	if (opcode == "motion_turnright") return new TurnDegrees();
	if (opcode == "motion_turnleft") return new TurnNegDegrees();
	if (opcode == "motion_goto") return new Goto();
	if (opcode == "motion_gotoxy") return new GotoXY();
	if (opcode == "motion_glideto") return new Glide();
	if (opcode == "motion_glidesecstoxy") return new GlideXY();
	if (opcode == "motion_pointindirection") return new PointDir();
	if (opcode == "motion_pointtowards") return new PointTowards();
	if (opcode == "motion_changexby") return new ChangeX();
	if (opcode == "motion_setx") return new SetX();
	if (opcode == "motion_changeyby") return new ChangeY();
	if (opcode == "motion_sety") return new SetY();
	if (opcode == "motion_ifonedgebounce") return new BounceIfOnEdge();
	if (opcode == "motion_setrotationstyle") return new SetRotationStyle();
	if (opcode == "motion_xposition") return new XPos();
	if (opcode == "motion_yposition") return new YPos();
	if (opcode == "motion_direction") return new Direction();

	if (opcode == "looks_sayforsecs") return new SayForSecs();
	if (opcode == "looks_say") return new Say();
	if (opcode == "looks_thinkforsecs") return new ThinkForSecs();
	if (opcode == "looks_think") return new Think();
	if (opcode == "looks_switchcostumeto") return new SwitchCostume();
	if (opcode == "looks_nextcostume") return new NextCostume();
	if (opcode == "looks_switchbackdropto") return new SwitchBackdrop();
	if (opcode == "looks_switchbackdroptoandwait") return new SwitchBackdropAndWait();
	if (opcode == "looks_nextbackdrop") return new NextBackdrop();
	if (opcode == "looks_changesizeby") return new ChangeSize();
	if (opcode == "looks_setsizeto") return new SetSize();
	if (opcode == "looks_changeeffectby") return new ChangeGraphicEffect();
	if (opcode == "looks_seteffectto") return new SetGraphicEffect();
	if (opcode == "looks_cleargraphiceffects") return new ClearGraphicEffects();
	if (opcode == "looks_show") return new ShowSprite();
	if (opcode == "looks_hide") return new HideSprite();
	if (opcode == "looks_gotofrontback") return new GotoLayer();
	if (opcode == "looks_goforwardbackwardlayers") return new MoveLayer();
	if (opcode == "looks_costumenumbername") return new CurrentCostume();
	if (opcode == "looks_backdropnumbername") return new CurrentBackdrop();
	if (opcode == "looks_size") return new Size();

	if (opcode == "sound_playuntildone") return new PlaySoundUntilDone();
	if (opcode == "sound_play") return new StartSound();
	if (opcode == "sound_stopallsounds") return new StopAllSounds();
	if (opcode == "sound_changeeffectby") return new ChangeSoundEffect();
	if (opcode == "sound_seteffectto") return new SetSoundEffect();
	if (opcode == "sound_cleareffects") return new ClearSoundEffects();
	if (opcode == "sound_changevolumeby") return new ChangeVolume();
	if (opcode == "sound_setvolumeto") return new SetVolume();
	if (opcode == "sound_volume") return new Volume();

	if (opcode == "event_whenflagclicked") return new OnFlagClicked();
	if (opcode == "event_whenkeypressed") return new OnKeyPressed();
	if (opcode == "event_whenthisspriteclicked") return new OnSpriteClicked();
	if (opcode == "event_whenstageclicked")	return new OnStageClicked();
	if (opcode == "event_whenbackdropswitchesto") return new OnBackdropSwitch();
	if (opcode == "event_whengreaterthan") return new OnGreaterThan();
	if (opcode == "event_whenbroadcastreceived") return new OnEvent();
	if (opcode == "event_broadcast") return new Broadcast();
	if (opcode == "event_broadcastandwait") return new BroadcastAndWait();

	if (opcode == "control_wait") return new WaitSecs();
	if (opcode == "control_repeat") return new Repeat();
	if (opcode == "control_forever") return new Forever();
	if (opcode == "control_if") return new If();
	if (opcode == "control_if_else") return new IfElse();
	if (opcode == "control_wait_until") return new WaitUntil();
	if (opcode == "control_repeat_until") return new RepeatUntil();
	if (opcode == "control_stop") return new Stop();
	if (opcode == "control_start_as_clone") return new CloneStart();
	if (opcode == "control_create_clone_of") return new CreateClone();
	if (opcode == "control_delete_this_clone") return new DeleteClone();

	if (opcode == "sensing_touchingobject") return new Touching();
	if (opcode == "sensing_touchingcolor") return new TouchingColor();
	if (opcode == "sensing_coloristouchingcolor") return new ColorTouching();
	if (opcode == "sensing_distanceto") return new DistanceTo();
	if (opcode == "sensing_askandwait") return new AskAndWait();
	if (opcode == "sensing_answer")	return new Answer();
	if (opcode == "sensing_keypressed") return new KeyPressed();
	if (opcode == "sensing_mousedown") return new MouseDown();
	if (opcode == "sensing_mousex") return new MouseX();
	if (opcode == "sensing_mousey") return new MouseY();
	if (opcode == "sensing_setdragmode") return new SetDragMode();
	if (opcode == "sensing_loudness") return new Loudness();
	if (opcode == "sensing_timer") return new TimerValue();
	if (opcode == "sensing_resettimer") return new ResetTimer();
	if (opcode == "sensing_of") return new PropertyOf();
	if (opcode == "sensing_current") return new CurrentDate();
	if (opcode == "sensing_dayssince2000") return new DaysSince2000();
	if (opcode == "sensing_username") return new Username();

	if (opcode == "operator_add") return new Add();
	if (opcode == "operator_subtract") return new Sub();
	if (opcode == "operator_multiply") return new Mul();
	if (opcode == "operator_divide") return new Div();
	if (opcode == "operator_random") return new Random();
	if (opcode == "operator_gt") return new Greater();
	if (opcode == "operator_lt") return new Less();
	if (opcode == "operator_equals") return new Equal();
	if (opcode == "operator_and") return new LogicalAnd();
	if (opcode == "operator_or") return new LogicalOr();
	if (opcode == "operator_not") return new LogicalNot();
	if (opcode == "operator_join") return new Concat();
	if (opcode == "operator_letter_of") return new CharAt();
	if (opcode == "operator_length") return new StringLength();
	if (opcode == "operator_contains") return new StringContains();
	if (opcode == "operator_mod") return new Mod();
	if (opcode == "operator_round") return new Round();
	if (opcode == "operator_mathop") return new MathFunc();

	if (opcode == "data_variable") return new VariableExpr();
	if (opcode == "data_setvariableto") return new SetVariable();
	if (opcode == "data_changevariableby") return new ChangeVariable();
	if (opcode == "data_showvariable") return new ShowVariable();
	if (opcode == "data_hidevariable") return new HideVariable();

	if (opcode == "data_listcontents") return new ListExpr();
	if (opcode == "data_addtolist") return new AppendToList();
	if (opcode == "data_deleteoflist") return new DeleteFromList();
	if (opcode == "data_deletealloflist") return new DeleteAllList();
	if (opcode == "data_insertatlist") return new InsertInList();
	if (opcode == "data_replaceitemoflist") return new ReplaceInList();
	if (opcode == "data_itemoflist") return new ListAccess();
	if (opcode == "data_itemnumoflist") return new IndexOf();
	if (opcode == "data_lengthoflist") return new ListLength();
	if (opcode == "data_listcontainsitem") return new ListContains();
	if (opcode == "data_showlist") return new ShowList();
	if (opcode == "data_hidelist") return new HideList();

	if (opcode == "procedures_definition") return new DefineProc();
	if (opcode == "procedures_call") return new Call();

	// Reporters

	if (opcode == "motion_goto_menu") return new GotoReporter();
	if (opcode == "motion_glideto_menu") return new GlideReporter();
	if (opcode == "motion_pointtowards_menu") return new PointTowardsReporter();
	if (opcode == "looks_costume") return new CostumeReporter();
	if (opcode == "looks_backdrops") return new BackdropReporter();
	if (opcode == "sound_sounds_menu") return new SoundReporter();
	if (opcode == "event_broadcast_menu") return new BroadcastReporter();
	if (opcode == "control_create_clone_of_menu") return new CloneReporter();
	if (opcode == "sensing_touchingobjectmenu") return new TouchingReporter();
	if (opcode == "sensing_distancetomenu") return new DistanceReporter();
	if (opcode == "sensing_keyoptions") return new KeyReporter();
	if (opcode == "sensing_of_object_menu") return new PropertyOfReporter();

	// No extension blocks are supported

	return nullptr; // unknown opcode
}
