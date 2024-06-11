#include <cstdio>

#include <lysys/lysys.hpp>

#include <codegen/opcode.hpp>
#include <codegen/compiler.hpp>
#include <ast/ast.hpp>

#include <SDL.h>

static void usage()
{
	printf("Usage: sdisas3 [options...] <file>\n\n");
	printf("Options:\n");
	printf("  -h, --help         Show this message\n");
	printf("  -d, --disassemble  Disassemble the program\n");
}

static void disassemble(uint8_t *fileData, size_t fileSize);

int main(int argc, char *argv[])
{
	bool disas = false;
	const char *file = nullptr;
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
		{
			usage();
			return 0;
		}
		else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassemble"))
		{
			disas = true;
		}
		else if (argv[i][0] != '-')
		{
			file = argv[i];
			break;
		}
		else
		{
			printf("Unknown option `%s`\n", argv[i]);
			return 1;
		}
	}
	
	if (file == nullptr)
	{
		usage();
		return 1;
	}

	size_t size;
	uint8_t *fileData = (uint8_t *)ls_read_file(file, &size);
	if (!fileData)
	{
		printf("Failed to load file `%s`\n", file);
		return 1;
	}

	printf("Dump of file `%s`\n\n", file);

	ProgramHeader *header = (ProgramHeader *)fileData;

	printf("  Summary\n\n");

	printf("    %8X .text\n", header->text);
	printf("    %8X .stable\n", header->stable);
	printf("    %8X .data\n", header->data);
	printf("    %8X .rdata\n", header->rdata);

	if (disas)
	{
		printf("\n");
		disassemble(fileData, size);
	}

	return 0;
}

static void disassemble(uint8_t *fileData, size_t fileSize)
{
	printf("  Disassembly\n\n");

	ProgramHeader *header = (ProgramHeader *)fileData;
	uint8_t *text, *stable, *data, *rdata;

	text = fileData + header->text;
	stable = fileData + header->stable;
	data = fileData + header->data;
	rdata = fileData + header->rdata;

	String *str;
	const char *s;
	uint64_t u64;

	uint8_t *textEnd = text + header->text_size;

	uint8_t *ptr = text;
	while (ptr < textEnd)
	{
		uint8_t opcode = *ptr;
		if (opcode == Op_onflag || opcode == Op_onkey || opcode == Op_onclick || opcode == Op_onbackdropswitch || opcode == Op_ongt || opcode == Op_onevent || opcode == Op_onclone)
			printf("\n");

		printf("    %8X  ", (uint32_t)(ptr - text));

		ptr++;
		switch (opcode)
		{
		default:
			printf("%02X\n", opcode);
			break;
		case Op_noop:
			printf("noop\n");
			break;
		case Op_int:
			printf("int\n");
			break;
		case Op_varset:
			printf("varset\n");
			break;
		case Op_varadd:
			printf("varadd\n");
			break;
		case Op_varget:
			printf("varget\n");
			break;
		case Op_listcreate:
			printf("listcreate %llu\n", *(uint64_t *)ptr);
			ptr += sizeof(uint64_t);
			break;
		case Op_jmp:
			printf("jmp %llX\n", *(int64_t *)ptr);
			ptr += sizeof(int64_t);
			break;
		case Op_jz:
			printf("jz %llX\n", *(int64_t *)ptr);
			ptr += sizeof(int64_t);
			break;
		case Op_jnz:
			printf("jnz %llX\n", *(int64_t *)ptr);
			ptr += sizeof(int64_t);
			break;
		case Op_yield:
			printf("yield\n");
			break;
		case Op_pop:
			printf("pop\n");
			break;
		case Op_pushnone:
			printf("pushnone\n");
			break;
		case Op_pushint:
			printf("pushint %lld\n", *(int64_t *)ptr);
			ptr += sizeof(int64_t);
			break;
		case Op_pushreal:
			printf("pushreal %lf\n", *(double *)ptr);
			ptr += sizeof(double);
			break;
		case Op_pushtrue:
			printf("pushtrue\n");
			break;
		case Op_pushfalse:
			printf("pushfalse\n");
			break;
		case Op_pushstring:
			str = (String *)(fileData + *(uint64_t *)ptr);
			printf("pushstring %08llX -> %s\n", *(uint64_t *)ptr, str->str);
			ptr += sizeof(uint64_t);
			break;
		case Op_dup:
			printf("dup\n");
			break;
		case Op_eq:
			printf("eq\n");
			break;
		case Op_neq:
			printf("neq\n");
			break;
		case Op_gt:
			printf("gt\n");
			break;
		case Op_ge:
			printf("ge\n");
			break;
		case Op_lt:
			printf("lt\n");
			break;
		case Op_le:
			printf("le\n");
			break;
		case Op_land:
			printf("land\n");
			break;
		case Op_lor:
			printf("lor\n");
			break;
		case Op_lnot:
			printf("lnot\n");
			break;
		case Op_add:
			printf("add\n");
			break;
		case Op_sub:
			printf("sub\n");
			break;
		case Op_mul:
			printf("mul\n");
			break;
		case Op_div:
			printf("div\n");
			break;
		case Op_mod:
			printf("mod\n");
			break;
		case Op_neg:
			printf("neg\n");
			break;
		case Op_round:
			printf("round\n");
			break;
		case Op_abs:
			printf("abs\n");
			break;
		case Op_floor:
			printf("floor\n");
			break;
		case Op_ceil:
			printf("ceil\n");
			break;
		case Op_sqrt:
			printf("sqrt\n");
			break;
		case Op_sin:
			printf("sin\n");
			break;
		case Op_cos:
			printf("cos\n");
			break;
		case Op_tan:
			printf("tan\n");
			break;
		case Op_asin:
			printf("asin\n");
			break;
		case Op_acos:
			printf("acos\n");
			break;
		case Op_atan:
			printf("atan\n");
			break;
		case Op_ln:
			printf("ln\n");
			break;
		case Op_log10:
			printf("log10\n");
			break;
		case Op_exp:
			printf("exp\n");
			break;
		case Op_exp10:
			printf("exp10\n");
			break;
		case Op_strcat:
			printf("strcat\n");
			break;
		case Op_charat:
			printf("charat\n");
			break;
		case Op_strlen:
			printf("strlen\n");
			break;
		case Op_strstr:
			printf("strstr\n");
			break;
		case Op_inc:
			printf("inc\n");
			break;
		case Op_dec:
			printf("dec\n");
			break;
		case Op_movesteps:
			printf("movesteps\n");
			break;
		case Op_turndegrees:
			printf("turndegrees\n");
			break;
		case Op_goto:
			printf("goto\n");
			break;
		case Op_gotoxy:
			printf("gotoxy\n");
			break;
		case Op_glide:
			printf("glide\n");
			break;
		case Op_glidexy:
			printf("glidexy\n");
			break;
		case Op_setdir:
			printf("setdir\n");
			break;
		case Op_lookat:
			printf("lookat\n");
			break;
		case Op_addx:
			printf("addx\n");
			break;
		case Op_setx:
			printf("setx\n");
			break;
		case Op_addy:
			printf("addy\n");
			break;
		case Op_sety:
			printf("sety\n");
			break;
		case Op_bounceonedge:
			printf("bounceonedge\n");
			break;
		case Op_setrotationstyle:
			switch (*ptr)
			{
			case RotationStyle_AllAround:
				s = "allaround";
				break;
			case RotationStyle_LeftRight:
				s = "leftright";
				break;
			case RotationStyle_DontRotate:
				s = "dontrotate";
				break;
			default:
				s = "unknown";
				break;
			}
			ptr++;
			printf("setrotationstyle %s\n", s);
			break;
		case Op_getx:
			printf("getx\n");
			break;
		case Op_gety:
			printf("gety\n");
			break;
		case Op_getdir:
			printf("getdir\n");
			break;
		case Op_say:
			printf("say\n");
			break;
		case Op_think:
			printf("think\n");
			break;
		case Op_setcostume:
			printf("setcostume\n");
			break;
		case Op_findcostume:
			printf("findcostume\n");
			break;
		case Op_nextcostume:
			printf("nextcostume\n");
			break;
		case Op_setbackdrop:
			printf("setbackdrop\n");
			break;
		case Op_findbackdrop:
			printf("findbackdrop\n");
			break;
		case Op_nextbackdrop:
			printf("nextbackdrop\n");
			break;
		case Op_addsize:
			printf("addsize\n");
			break;
		case Op_setsize:
			printf("setsize\n");
			break;
		case Op_addgraphiceffect:
			printf("addgraphiceffect\n");
			// TODO: Decode graphic effect
			ptr++;
			break;
		case Op_setgraphiceffect:
			printf("setgraphiceffect\n");
			// TODO: Decode graphic effect
			ptr++;
			break;
		case Op_cleargraphiceffects:
			printf("cleargraphiceffects\n");
			break;
		case Op_show:
			printf("show\n");
			break;
		case Op_hide:
			printf("hide\n");
			break;
		case Op_gotolayer:
			switch (*ptr)
			{
			case LayerType_Front:
				s = "front";
				break;
			case LayerType_Back:
				s = "back";
				break;
			default:
				s = "unknown";
				break;
			}
			ptr++;
			printf("gotolayer %s\n", s);
			break;
		case Op_movelayer:
			switch (*ptr)
			{
			case LayerDir_Forward:
				s = "forward";
				break;
			case LayerDir_Backward:
				s = "backward";
				break;
			default:
				s = "unknown";
				break;
			}
			ptr++;
			printf("movelayer %s\n", s);
			break;
		case Op_getcostume:
			printf("getcostume\n");
			break;
		case Op_getcostumename:
			printf("getcostumename\n");
			break;
		case Op_getbackdrop:
			printf("getbackdrop\n");
			break;
		case Op_getsize:
			printf("getsize\n");
			break;
		case Op_playsoundandwait:
			printf("playsoundandwait\n");
			break;
		case Op_playsound:
			printf("playsound\n");
			break;
		case Op_findsound:
			printf("findsound\n");
			break;
		case Op_stopsound:
			printf("stopsound\n");
			break;
		case Op_addsoundeffect:
			printf("addsoundeffect\n");
			// TODO: Decode sound effect
			ptr++;
			break;
		case Op_setsoundeffect:
			printf("setsoundeffect\n");
			// TODO: Decode sound effect
			ptr++;
			break;
		case Op_clearsoundeffects:
			printf("clearsoundeffects\n");
			break;
		case Op_addvolume:
			printf("addvolume\n");
			break;
		case Op_setvolume:
			printf("setvolume\n");
			break;
		case Op_getvolume:
			printf("getvolume\n");
			break;
		case Op_onflag:
			printf("onflag\n");
			break;
		case Op_onkey:
			printf("onkey %s\n", SDL_GetScancodeName((SDL_Scancode)*(uint16_t *)ptr));
			ptr += sizeof(uint16_t);
			break;
		case Op_onclick:
			printf("onclick\n");
			break;
		case Op_onevent:
			s = (char *)(fileData + *(uint64_t *)ptr);
			printf("onevent %08llX -> %s\n", *(uint64_t *)ptr, s);
			ptr += sizeof(uint64_t);
			break;
		case Op_send:
			printf("send\n");
			break;
		case Op_sendandwait:
			printf("sendandwait\n");
			break;
		case Op_findevent:
			printf("findevent\n");
			break;
		case Op_waitsecs:
			printf("waitsecs\n");
			break;
		case Op_stopall:
			printf("stopall\n");
			break;
		case Op_stopself:
			printf("stopself\n");
			break;
		case Op_stopother:
			printf("stopother\n");
			break;
		case Op_onclone:
			printf("onclone\n");
			break;
		case Op_clone:
			printf("clone\n");
			break;
		case Op_deleteclone:
			printf("deleteclone\n");
			break;
		case Op_touching:
			printf("touching\n");
			break;
		case Op_touchingcolor:
			printf("touchingcolor\n");
			break;
		case Op_colortouching:
			printf("colortouching\n");
			break;
		case Op_distanceto:
			printf("distanceto\n");
			break;
		case Op_ask:
			printf("ask\n");
			break;
		case Op_getanswer:
			printf("getanswer\n");
			break;
		case Op_keypressed:
			printf("keypressed\n");
			break;
		case Op_mousedown:
			printf("mousedown\n");
			break;
		case Op_mousex:
			printf("mousex\n");
			break;
		case Op_mousey:
			printf("mousey\n");
			break;
		case Op_setdragmode:
			switch (*ptr)
			{
			case DragMode_Draggable:
				s = "draggable";
				break;
			case DragMode_NotDraggable:
				s = "not draggable";
				break;
			default:
				s = "unknown";
				break;
			}
			ptr++;
			printf("setdragmode %s\n", s);
			break;
		case Op_getloudness:
			printf("getloudness\n");
			break;
		case Op_gettimer:
			printf("gettimer\n");
			break;
		case Op_resettimer:
			printf("resettimer\n");
			break;
		case Op_propertyof:
			switch (*ptr)
			{
			case PropertyTarget_BackdropNumber:
				s = "backdropnumber";
				break;
			case PropertyTarget_BackdropName:
				s = "backdropname";
				break;
			case PropertyTarget_XPosition:
				s = "xposition";
				break;
			case PropertyTarget_YPosition:
				s = "yposition";
				break;
			case PropertyTarget_Direction:
				s = "direction";
				break;
			case PropertyTarget_CostumeNumber:
				s = "costumenumber";
				break;
			case PropertyTarget_CostumeName:
				s = "costumename";
				break;
			case PropertyTarget_Size:
				s = "size";
				break;
			case PropertyTarget_Volume:
				s = "volume";
				break;
			case PropertyTarget_Variable:
				s = "variable";
				break;
			default:
				s = "unknown";
				break;
			}
			ptr++;
			printf("propertyof %s\n", s);
			break;
		case Op_gettime:
			switch (*ptr)
			{
			case DateFormat_Year:
				s = "year";
				break;
			case DateFormat_Month:
				s = "month";
				break;
			case DateFormat_Date:
				s = "date";
				break;
			case DateFormat_DayOfWeek:
				s = "dayofweek";
				break;
			case DateFormat_Hour:
				s = "hour";
				break;
			case DateFormat_Minute:
				s = "minute";
				break;
			case DateFormat_Second:
				s = "second";
				break;
			default:
				s = "unknown";
				break;
			}
			ptr++;
			printf("gettime %s\n", s);
			break;
		case Op_getdayssince2000:
			printf("getdayssince2000\n");
			break;
		case Op_getusername:
			printf("getusername\n");
			break;
		case Op_rand:
			printf("rand\n");
			break;
		case Op_varshow:
			printf("varshow\n");
			break;
		case Op_varhide:
			printf("varhide\n");
			break;
		case Op_listadd:
			printf("listadd\n");
			break;
		case Op_listremove:
			printf("listremove\n");
			break;
		case Op_listclear:
			printf("listclear\n");
			break;
		case Op_listinsert:
			printf("listinsert\n");
			break;
		case Op_listreplace:
			printf("listreplace\n");
			break;
		case Op_listat:
			printf("listat\n");
			break;
		case Op_listfind:
			printf("listfind\n");
			break;
		case Op_listlen:
			printf("listlen\n");
			break;
		case Op_listcontains:
			printf("listcontains\n");
			break;
		}
	}
}
