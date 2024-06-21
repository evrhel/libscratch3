#pragma once

#include <cstdint>

enum Opcode : uint8_t
{
	Op_noop = 0x00,

	Op_int = 0x01,

	Op_varset,
	Op_varadd,
	Op_varget,

	Op_setstatic,
	Op_getstatic,

	Op_listcreate, // Create list

	Op_jmp, // Relative jump
	Op_jz, // Relative jump if false
	Op_jnz, // Relative jump if true

	Op_call, // Push return address and jump
	Op_ret, // Pop return address and jump

	Op_enter, // Enter function
	Op_leave, // Leave function

	Op_yield, // Yield to scheduler

	Op_pop,
	Op_pushnone,
	Op_pushint,
	Op_pushreal,
	Op_pushtrue,
	Op_pushfalse,
	Op_pushstring,

	Op_dup, // Duplicate top of stack

	Op_eq,
	Op_neq,
	Op_gt,
	Op_ge,
	Op_lt,
	Op_le,
	Op_land,
	Op_lor,
	Op_lnot,
	Op_add,
	Op_sub,
	Op_mul,
	Op_div,
	Op_mod,
	Op_neg,
	Op_round,
	Op_abs,
	Op_floor,
	Op_ceil,
	Op_sqrt,
	Op_sin,
	Op_cos,
	Op_tan,
	Op_asin,
	Op_acos,
	Op_atan,
	Op_ln,
	Op_log10,
	Op_exp,
	Op_exp10,
	Op_strcat,
	Op_charat,
	Op_strlen,
	Op_strstr,

	Op_inc,
	Op_dec,

	Op_movesteps,
	Op_turndegrees,
	Op_goto,
	Op_gotoxy,
	Op_glide,
	Op_glidexy,
	Op_setdir,
	Op_lookat,
	Op_addx,
	Op_setx,
	Op_addy,
	Op_sety,
	Op_bounceonedge,
	Op_setrotationstyle,
	Op_getx,
	Op_gety,
	Op_getdir,
	
	Op_say,
	Op_think,
	Op_setcostume,
	Op_nextcostume,
	Op_setbackdrop,
	Op_nextbackdrop,
	Op_addsize,
	Op_setsize,
	Op_addgraphiceffect,
	Op_setgraphiceffect,
	Op_cleargraphiceffects,
	Op_show,
	Op_hide,
	Op_gotolayer,
	Op_movelayer,
	Op_getcostume,
	Op_getcostumename,
	Op_getbackdrop,
	Op_getsize,
	
	Op_playsoundandwait,
	Op_playsound,
	Op_stopsound,
	Op_addsoundeffect,
	Op_setsoundeffect,
	Op_clearsoundeffects,
	Op_addvolume,
	Op_setvolume,
	Op_getvolume,

	Op_onflag,
	Op_onkey,
	Op_onclick,
	Op_onbackdropswitch,
	Op_ongt,
	Op_onevent,
	Op_send,
	Op_sendandwait,
	Op_findevent,

	Op_waitsecs,
	Op_stopall,
	Op_stopself,
	Op_stopother,
	Op_onclone,
	Op_clone,
	Op_deleteclone,

	Op_touching,
	Op_touchingcolor,
	Op_colortouching,
	Op_distanceto,
	Op_ask,
	Op_getanswer,
	Op_keypressed,
	Op_mousedown,
	Op_mousex,
	Op_mousey,
	Op_setdragmode,
	Op_getloudness,
	Op_gettimer,
	Op_resettimer,
	Op_propertyof,
	Op_gettime,
	Op_getdayssince2000,
	Op_getusername,

	Op_rand,

	Op_varshow,
	Op_varhide,

	Op_listadd,
	Op_listremove,
	Op_listclear,
	Op_listinsert,
	Op_listreplace,
	Op_listat,
	Op_listfind,
	Op_listlen,
	Op_listcontains,

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
	uint8_t ext_opcode;
};
