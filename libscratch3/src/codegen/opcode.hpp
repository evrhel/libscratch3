#pragma once

#include <cstdint>

enum Opcode
{
	Op_noop = 0x00,

	Op_int = 0x01,

	Op_pop = 0x09,

	Op_getstatic = 0x2a,
	Op_getfield = 0x2b,
	Op_getsprite = 0x2c,
	Op_getreporter = 0x2d,
	Op_findfield = 0x2e,

	Op_setstatic = 0x2f,
	Op_addstatic = 0x30,
	Op_setfield = 0x31,
	Op_addfield = 0x32,

	Op_eq = 0x40,
	Op_neq = 0x41,
	Op_gt = 0x42,
	Op_ge = 0x43,
	Op_lt = 0x44,
	Op_le = 0x45,
	Op_land = 0x46,
	Op_lor = 0x47,
	Op_lnot = 0x48,
	Op_add = 0x49,
	Op_sub = 0x4a,
	Op_mul = 0x4b,
	Op_div = 0x4c,
	Op_mod = 0x4d,
	Op_round = 0x4e,
	Op_abs = 0x4f,
	Op_floor = 0x50,
	Op_ceil = 0x51,
	Op_sqrt = 0x52,
	Op_sin = 0x53,
	Op_cos = 0x54,
	Op_tan = 0x55,
	Op_asin = 0x56,
	Op_acos = 0x57,
	Op_atan = 0x58,
	Op_ln = 0x59,
	Op_log10 = 0x5a,
	Op_exp = 0x5b,
	Op_exp10 = 0x5c,
	Op_strcat = 0x5d,
	Op_charat = 0x5e,
	Op_strlen = 0x5f,
	Op_strstr = 0x60,

	Op_inc = 0x64,
	Op_addi = 0x65,
	Op_addr = 0x66,
	Op_dec = 0x67,
	Op_subi = 0x68,
	Op_subr = 0x69,
	Op_muli = 0x6a,
	Op_mulr = 0x6b,
	Op_divi = 0x6c,
	Op_divr = 0x6d,

	Op_movesteps = 0x6e,
	Op_turndegrees = 0x6f,
	Op_goto = 0x70,
	Op_gotoxy = 0x71,
	Op_glide = 0x72,
	Op_glidexy = 0x73,
	Op_setdir = 0x74,
	Op_lookat = 0x75,
	Op_addx = 0x76,
	Op_setx = 0x77,
	Op_addy = 0x78,
	Op_sety = 0x79,
	Op_bounceonedge = 0x7a,
	Op_setrotationstyle = 0x7b,
	Op_getx = 0x7c,
	Op_gety = 0x7d,
	Op_getdir = 0x7e,

	Op_saysecs = 0x7f,
	Op_say = 0x80,
	Op_thinksecs = 0x81,
	Op_think = 0x82,
	Op_setcostume = 0x83,
	Op_findcostume = 0x84,
	Op_nextcostume = 0x85,
	Op_setbackdrop = 0x86,
	Op_findbackdrop = 0x87,
	Op_nextbackdrop = 0x88,
	Op_addsize = 0x89,
	Op_setsize = 0x8a,
	Op_addgraphiceffect = 0x8b,
	Op_setgraphiceffect = 0x8c,
	Op_cleargraphiceffects = 0x8d,
	Op_show = 0x8e,
	Op_hide = 0x8f,
	Op_gotolayer = 0x90,
	Op_movelayer = 0x91,
	Op_getcostume = 0x92,
	Op_getcostumename = 0x93,
	Op_getbackdrop = 0x94,
	Op_getsize = 0x95,
	
	Op_playsoundandwait = 0x96,
	Op_playsound = 0x97,
	Op_findsound = 0x98,
	Op_stopsound = 0x99,
	Op_addsoundeffect = 0x9a,
	Op_setsoundeffect = 0x9b,
	Op_clearsoundeffects = 0x9c,
	Op_addvolume = 0x9d,
	Op_setvolume = 0x9e,
	Op_getvolume = 0x9f,

	Op_onflag = 0xa0,
	Op_onkey = 0xa1,
	Op_onclick = 0xa2,
	Op_onbackdropswitch = 0xa3,
	Op_ongt = 0xa4,
	Op_onevent = 0xa5,
	Op_send = 0xa6,
	Op_sendandwait = 0xa7,

	Op_waitsecs = 0xa8,
	Op_waituntil = 0xa9,
	Op_stopall = 0xaa,
	Op_stopself = 0xab,
	Op_stopother = 0xac,
	Op_onclone = 0xad,
	Op_clone = 0xae,
	Op_deleteclone = 0xaf,

	Op_touching = 0xb0,
	Op_touchingcolor = 0xb1,
	Op_colortouching = 0xb2,
	Op_distanceto = 0xb3,
	Op_ask = 0xb4,
	Op_getanswer = 0xb5,
	Op_keypressed = 0xb6,
	Op_mousedown = 0xb7,
	Op_mousex = 0xb8,
	Op_mousey = 0xb9,
	Op_setdragmode = 0xba,
	Op_getloudness = 0xbb,
	Op_gettimer = 0xbc,
	Op_resettimer = 0xbd,
	Op_gettime = 0xbe,
	Op_getusername = 0xbf,

	Op_rand = 0xc1,

	Op_showstatic = 0xc2,
	Op_showfield = 0xc3,
	Op_hidestatic = 0xc4,
	Op_hidefield = 0xc5,

	Op_listadd = 0xc6,
	Op_listremove = 0xc7,
	Op_listclear = 0xc8,
	Op_listinsert = 0xc9,
	Op_listreplace = 0xca,
	Op_listat = 0xcb,
	Op_listfind = 0xcc,
	Op_listlen = 0xcd,
	Op_listcontains = 0xce,
	Op_listshow = 0xcf,
	Op_listhide = 0xd0,

	Op_invoke = 0xd1,

	Op_ext = 0xff // Extension operation, check next 2 byte (extension id, extension opcode)
};

enum ExtId
{
	Ext_inval = 0x00,

	Ext_pen = 0x01
};

enum Opcode_Pen
{
	Op_Pen_noop = 0x00,

	Op_Pen_erase = 0x10,
	Op_Pen_stamp = 0x11,
	Op_Pen_pendown = 0x12,
	Op_Pen_penup = 0x13,
	Op_Pen_addparam = 0x14,
	Op_Pen_setparam = 0x15,
	Op_Pen_findparam = 0x16,
	Op_Pen_addsize = 0x17,
	Op_Pen_setsize = 0x18
};

struct Instruction
{
	uint8_t opcode;
};

struct Instruction_Ext
{
	uint8_t opcode;
	uint8_t ext;
};
