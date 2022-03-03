#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>

#include "lua_wrapper.h"

#include "../utils/logger.h"

#include "log_export_to_lua.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif





static void printf_debug (const char* file_name, int line_num, const char* msg) 
{
	logger::log (file_name, line_num, DEBUG, msg);
}

static void printf_warning (const char* file_name, int line_num, const char* msg) 
{
	logger::log (file_name, line_num, WARNING, msg);
}

static void printf_error (const char* file_name, int line_num, const char* msg) 
{
	logger::log (file_name, line_num, LOG_ERROR, msg);
}

//获取lua栈中信息---file_name和line_name
static void do_log_message (void (*log)(const char* file_name, int line_num, const char* msg), const char* msg) 
{
	lua_Debug info;
	int depth = 0;
	while (lua_getstack (lua_wrapper::get_luaState(), depth, &info)) 
	{

		/*
	//		 lua_getinfo
	//		获取函数调用的基本信息
	//		'n': fills in the field name and namewhat;
	//		'S': fills in the fields source, linedefined, lastlinedefined, what, and short_src;(source)
	//		'l': fills in the field currentline;
	//		'u': fills in the field nups;
	//		'f': pushes onto the stack the function that is running at the given level; ->func
	//		'L': pushes onto the stack a table whose indices are the numbers of the lines that are valid on the function. (A valid line is a line with some 　
	//				associated code, that is, a line where you can put a break point. Non-valid lines include empty lines and comments.) ->activelines
	//		*/
		lua_getinfo (lua_wrapper::get_luaState(), "S", &info);
		lua_getinfo (lua_wrapper::get_luaState(), "n", &info);
		lua_getinfo (lua_wrapper::get_luaState(), "l", &info);

		if (info.source[0] == '@') {
			log (&info.source[1], info.currentline, msg);
			return;
		}

		++depth;
	}
	if (depth == 0) {
		log ("trunk", 0, msg);
	}
}

static int  lua_log_debug (lua_State* luastate) 
{
	int nargs = lua_gettop (luastate);
	std::string t;
	for (int i = 1; i <= nargs; i++)
	{
		if (lua_istable (luastate, i))
			t += "table";
		else if (lua_isnone (luastate, i))
			t += "none";
		else if (lua_isnil (luastate, i))
			t += "nil";
		else if (lua_isboolean (luastate, i))
		{
			if (lua_toboolean (luastate, i) != 0)
				t += "true";
			else
				t += "false";
		}
		else if (lua_isfunction (luastate, i))
			t += "function";
		else if (lua_islightuserdata (luastate, i))
			t += "lightuserdata";
		else if (lua_isthread (luastate, i))
			t += "thread";
		else
		{
			const char * str = lua_tostring (luastate, i);
			if (str)
				t += lua_tostring (luastate, i);
			else
				t += lua_typename (luastate, lua_type (luastate, i));
		}
		if (i != nargs)
			t += "\t";
	}
	do_log_message (printf_debug, t.c_str ());
	return 0;
}

static int  lua_log_warning (lua_State* luastate) 
{
	int nargs = lua_gettop (luastate);
	std::string t;
	for (int i = 1; i <= nargs; i++)
	{
		if (lua_istable (luastate, i))
			t += "table";
		else if (lua_isnone (luastate, i))
			t += "none";
		else if (lua_isnil (luastate, i))
			t += "nil";
		else if (lua_isboolean (luastate, i))
		{
			if (lua_toboolean (luastate, i) != 0)
				t += "true";
			else
				t += "false";
		}
		else if (lua_isfunction (luastate, i))
			t += "function";
		else if (lua_islightuserdata (luastate, i))
			t += "lightuserdata";
		else if (lua_isthread (luastate, i))
			t += "thread";
		else
		{
			const char * str = lua_tostring (luastate, i);
			if (str)
				t += lua_tostring (luastate, i);
			else
				t += lua_typename (luastate, lua_type (luastate, i));
		}
		if (i != nargs)
			t += "\t";
	}
	do_log_message (printf_warning, t.c_str ());
	return 0;
}

int  lua_log_error (lua_State* luastate) 
{
	int nargs = lua_gettop (luastate);
	std::string t;
	for (int i = 1; i <= nargs; i++)
	{
		if (lua_istable (luastate, i))
			t += "table";
		else if (lua_isnone (luastate, i))
			t += "none";
		else if (lua_isnil (luastate, i))
			t += "nil";
		else if (lua_isboolean (luastate, i))
		{
			if (lua_toboolean (luastate, i) != 0)
				t += "true";
			else
				t += "false";
		}
		else if (lua_isfunction (luastate, i))
			t += "function";
		else if (lua_islightuserdata (luastate, i))
			t += "lightuserdata";
		else if (lua_isthread (luastate, i))
			t += "thread";
		else
		{
			const char * str = lua_tostring (luastate, i);
			if (str)
				t += lua_tostring (luastate, i);
			else
				t += lua_typename (luastate, lua_type (luastate, i));
		}
		if (i != nargs)
			t += "\t";
	}
	do_log_message (printf_error, t.c_str ());
	return 0;
}

int lua_panic (lua_State* g_lua_state) 
{
	const char* msg = luaL_checkstring (g_lua_state, -1);
	if (msg){
		do_log_message (printf_error, msg);
	}
	return 0;
}

int lua_logger_init (lua_State* L) 
{
	const char* path = lua_tostring (L,1);
	if (NULL == path){
		goto lua_failed;
	}
	const char* prefix = lua_tostring (L, 2);
	if (NULL == prefix){
		goto lua_failed;
	}

	bool std_out = lua_toboolean (L, 3);

	logger::init (path, prefix, std_out);
lua_failed:
	return 0;
}

int register_logger_export (lua_State* tolua_S) 
{
	lua_wrapper::reg_func2lua ("print", lua_log_debug);

	lua_getglobal (tolua_S, "_G");
	if (lua_istable (tolua_S, -1)){
		tolua_open (tolua_S);
		tolua_module (tolua_S, "logger", 0);
		tolua_beginmodule (tolua_S, "logger");

		tolua_function (tolua_S, "debug", lua_log_debug);
		tolua_function (tolua_S, "warning", lua_log_warning);
		tolua_function (tolua_S, "error", lua_log_error);

		tolua_function (tolua_S, "init", lua_logger_init);

		tolua_endmodule (tolua_S);
	}
	lua_pop (tolua_S, 1);
	return 0;

}