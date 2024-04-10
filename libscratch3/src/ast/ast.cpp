#include "ast.hpp"

#include <cstdarg>
#include <cstdio>
#include <cassert>

#include <rapidjson/document.h>

static ASTNode *NodeFromOpcode(const std::string &opcode);

class Parser
{
public:
	Program *Parse(rapidjson::Value &doc)
	{
		_defs.clear();

		Program *p = new Program();

		if (doc.HasMember("targets"))
		{
			rapidjson::Value &targets = doc["targets"];
			if (targets.IsArray())
			{
				p->sprites = new SpriteDefList();
				ParseTargets(targets, p);
			}
			else
				Error("Expected array parsing targets");
		}
		else
			Warn("Missing `targets` member");

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

	Parser(std::vector<Message> *log) :
		_log(log) {}
private:
	std::unordered_map<std::string, ASTNode *> _defs;

	std::vector<Message> *_log;
	bool are_errors = false;

	void Error(const char *format, ...)
	{
		char error[512];

		va_list args;
		va_start(args, format);
		vsprintf_s(error, format, args);
		va_end(args);

		if (_log)
			_log->push_back({ MessageType_Error, error });
		are_errors = true;
	}

	void Warn(const char *format, ...)
	{
		char error[512];

		va_list args;
		va_start(args, format);
		vsprintf_s(error, format, args);
		va_end(args);

		if (_log)
			_log->push_back({ MessageType_Warning, error });
	}
	
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

	SpriteDef *ParseSprite(rapidjson::Value &target)
	{
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

		if (target.HasMember("blocks"))
		{
			rapidjson::Value &blocks = target["blocks"];
			if (!blocks.IsObject())
			{
				Error("Expected object parsing blocks in target");
				goto failure;
			}

			_defs.clear();

			for (auto it = blocks.MemberBegin(); it != blocks.MemberEnd(); ++it)
			{
				rapidjson::Value &target = it->value;
				std::string id = it->name.GetString();

				if (!target.IsObject())
				{
					Error("Invalid target `%s`", id.c_str());
					continue;
				}

				if (!target.HasMember("topLevel"))
				{
					Error("Missing `topLevel` member in target `%s`", id.c_str());
					continue;
				}

				if (!target["topLevel"].GetBool())
					continue; // ignore non-top-level targets

				StatementList *sl = new StatementList();
				ParseScript(blocks, id, sl);
				sd->scripts->sll.push_back(sl);
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

	void ParseScript(rapidjson::Value &blocks, const std::string &first, StatementList *sl)
	{
		Statement *s;
		
		std::string id = first;
		for (;;)
		{
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

			ASTNode *node = ParseBlock(blocks, id);
			if (!node)
				break;

			s = node->As<Statement>();
			if (!s)
			{
				Error("Expected statement, got %s", AstTypeString(node->GetType()));
				_defs.erase(id), delete node;
				break;
			}

			sl->sl.push_back(s);

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

	ASTNode *ParseBlock(rapidjson::Value &blocks, const std::string &id)
	{
		auto it = _defs.find(id);
		if (it != _defs.end())
		{
			Error("Circular reference `%s`", id.c_str());
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

		ASTNode *n = NodeFromOpcode(opcode.GetString());
		if (!n)
		{
			Error("Unknown opcode `%s` in target `%s`", opcode.GetString(), id.c_str());
			return nullptr;
		}

		if (block.HasMember("inputs"))
		{
			rapidjson::Value &inputs = block["inputs"];
			if (!inputs.IsObject())
			{
				Error("Expected object parsing inputs in block `%s`", id.c_str());
				delete n;
				return nullptr;
			}

			for (auto it = inputs.MemberBegin(); it != inputs.MemberEnd(); ++it)
			{
				std::string key = it->name.GetString();
				Expression *val = ParseExpression(blocks, it->value);
				if (!val)
				{
					Error("Failed to parse input `%s` in block `%s`", key.c_str(), id.c_str());
					continue;
				}

				if (!n->SetInput(key, val))
					Warn("Unknown input `%s` in block `%s`", key.c_str(), id.c_str());
			}
		}
		else
			Warn("Missing `inputs` member in block `%s`", id.c_str());

		if (block.HasMember("fields"))
		{
			rapidjson::Value &fields = block["fields"];
			if (!fields.IsObject())
			{
				Error("Expected object parsing fields in block `%s`", id.c_str());
				delete n;
				return nullptr;
			}

			for (auto it = fields.MemberBegin(); it != fields.MemberEnd(); ++it)
			{
				std::string key = it->name.GetString();
				if (!it->value.IsArray())
				{
					Warn("Expected array parsing field `%s` in block `%s`", key.c_str(), id.c_str());
					continue;
				}

				std::string svalue, sid;

				rapidjson::Value &field = it->value;

				if (field.Size() < 1)
				{
					Warn("Expected at least 1 element parsing field `%s` in block `%s`", key.c_str(), id.c_str());
					continue;
				}

				rapidjson::Value &valueval = field[0];
				if (!valueval.IsString())
				{
					Warn("Expected string parsing value in field `%s` in block `%s`", key.c_str(), id.c_str());
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
							Warn("Expected string parsing id in field `%s` in block `%s`", key.c_str(), id.c_str());
							continue;
						}

						sid = idval.GetString();
					}
				}

				if (!n->SetField(key, svalue, sid))
					Warn("Unknown field `%s` in block `%s`", key.c_str(), id.c_str());
			}
		}
		else
			Warn("Missing `fields` member in block `%s`", id.c_str());

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

	// parse an expression
	Expression *ParseExpression(rapidjson::Value &blocks, rapidjson::Value &v)
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
				return ParseExpression(blocks, v[1]);
			
			if (!v[1].IsString())
			{
				Error("Expected string parsing shadow block");
				return nullptr;
			}

			ASTNode *node = ParseBlock(blocks, v[1].GetString());
			Expression *e = node->As<Expression>();
			if (!e)
			{
				Error("Expected expression, got %s", AstTypeString(node->GetType()));
				return nullptr;
			}

			return e;
		}
		case BlockType_NoShadow: {
			if (v.Size() < 2)
			{
				Error("Expected block id parsing no shadow block");
				return nullptr;
			}

			if (v[1].IsArray())
				return ParseExpression(blocks, v[1]);

			if (!v[1].IsString())
			{
				Error("Expected string parsing no shadow block");
				return nullptr;
			}

			ASTNode *node = ParseBlock(blocks, v[1].GetString());
			Expression *e = node->As<Expression>();
			if (!e)
			{
				Error("Expected expression, got %s", AstTypeString(node->GetType()));
				return nullptr;
			}

			return e;
		}
		case BlockType_ShadowObscured: {
			if (v.Size() < 2)
			{
				Error("Expected block id parsing shadow obscured block");
				return nullptr;
			}

			if (v[1].IsArray())
				return ParseExpression(blocks, v[1]);

			if (!v[1].IsString())
			{
				Error("Expected string parsing shadow obscured block");
				return nullptr;
			}

			ASTNode *node = ParseBlock(blocks, v[1].GetString());
			if (!node)
			{
				Error("Failed to parse block `%s`", v[1].GetString());
				return nullptr;
			}

			Expression *e = node->As<Expression>();
			if (!e)
			{
				Error("Expected expression, got %s", AstTypeString(node->GetType()));
				return nullptr;
			}

			return e;
		}
		case BlockType_Number: {
			if (v.Size() < 2)
			{
				Error("Expected value parsing number block");
				return nullptr;
			}

			if (!v[1].IsString())
			{
				Error("Expected string parsing number block");
				return nullptr;
			}

			Number *n = new Number();
			n->value = v[1].GetString();
			return n;
		}
		case BlockType_PositiveNumber: {
			if (v.Size() < 2)
			{
				Error("Expected value parsing positive number block");
				return nullptr;
			}

			if (!v[1].IsString())
			{
				Error("Expected string parsing positive number block");
				return nullptr;
			}

			PositiveNumber *pn = new PositiveNumber();
			pn->value = v[1].GetString();

			return pn;
		}
		case BlockType_PositiveInt: {
			if (v.Size() < 2)
			{
				Error("Expected value parsing positive int block");
				return nullptr;
			}

			if (!v[1].IsString())
			{
				Error("Expected string parsing positive int block");
				return nullptr;
			}

			PositiveInt *pi = new PositiveInt();
			pi->value = v[1].GetString();
			return pi;
		}
		case BlockType_Int: {
			if (v.Size() < 2)
			{
				Error("Expected value parsing int block");
				return nullptr;
			}

			if (!v[1].IsString())
			{
				Error("Expected string parsing int block");
				return nullptr;
			}

			Int *i = new Int();
			i->value = v[1].GetString();
			return i;
		}
		case BlockType_Angle: {
			if (v.Size() < 2)
			{
				Error("Expected value parsing angle block");
				return nullptr;
			}

			if (!v[1].IsString())
			{
				Error("Expected string parsing angle block");
				return nullptr;
			}

			Angle *a = new Angle();
			a->value = v[1].GetString();
			return a;
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
		case BlockType_String: {
			if (v.Size() < 2)
			{
				Error("Expected value parsing string block");
				return nullptr;
			}

			if (!v[1].IsString())
			{
				Error("Expected string parsing string block");
				return nullptr;
			}

			String *s = new String();
			s->value = v[1].GetString();
			return s;
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

			VariableExpr *var = new VariableExpr();
			var->id = id.GetString();
			var->name = name.GetString();
			return var;
		}
		case BlockType_List: {
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

#include <stdio.h>

Program *ParseAST(const char *jsonString, size_t length, std::vector<Message> *log)
{
	rapidjson::Document doc;

	doc.Parse(jsonString, length);
	if (doc.HasParseError())
		return nullptr;

	Parser parser(log);
	return parser.Parse(doc);
}

static ASTNode *NodeFromOpcode(const std::string &opcode)
{
	if (opcode == "motion_movesteps") return new MoveSteps();
	if (opcode == "motion_turnright") return new TurnDegrees();
	if (opcode == "motion_turnleft") return new TurnNegDegrees();
	if (opcode == "motion_goto") return new Goto();
	if (opcode == "motion_gotoxy") return new GotoXY();
	if (opcode == "motion_glide") return new Glide();
	if (opcode == "motion_glidexy") return new GlideXY();
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
	if (opcode == "control_repeat") return new Repeat(); // NOTE: the Scratch Wiki has the this as "motion_turnright" but it's actually "control_repeat"
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
	// argument_reporter_string_number
	// argument_reporter_boolean

	// No extension blocks are supported

	return nullptr; // unknown opcode
}
