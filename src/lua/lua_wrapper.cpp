#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua_wrapper.h"
#include "../utils/logger.h"
#include "tolua_fix.h"
#include "mysql_export_to_lua.h"
#include "redis_export_to_lua.h"
#include "service_export_to_lua.h"
#include "session_export_to_lua.h"
#include "log_export_to_lua.h"
#include "scheduler_export_to_lua.h"
#include "netbus_export_to_lua.h"
#include "proto_man_export_to_lua.h"


lua_State* lua_wrapper::g_lua_State = nullptr;

extern "C"
{
	//static void print_error(const char* file_name, int line_num, const char* msg)
	//{
	//	logger::log(file_name, line_num, LOG_ERROR, msg);
	//}

	//static void print_warning(const char* file_name, int line_num, const char* msg)
	//{
	//	logger::log(file_name, line_num, WARNING, msg);
	//}

	//static void print_debug(const char* file_name, int line_num, const char* msg)
	//{
	//	logger::log(file_name, line_num, DEBUG, msg);
	//}
	//
	////获取lua栈中信息---file_name和line_name
	//static void do_log_message(void(*log)(const char* file_name, int line_num, const char* msg), const char* msg) 
	//{
	//	lua_Debug info;
	//	int depth = 0;
	//	while (lua_getstack(lua_wrapper::get_luaState(), depth, &info)) 
	//	{
	//		/*
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
	//		lua_getinfo(lua_wrapper::get_luaState(), "S" ,&info);
	//		lua_getinfo(lua_wrapper::get_luaState(), "n" ,&info);
	//		lua_getinfo(lua_wrapper::get_luaState(), "l" ,&info);

	//		if (info.source[0] == '@') {
	//			log(&info.source[1], info.currentline, msg);
	//			return;
	//		}

	//		++depth;
	//	}
	//	if (depth == 0) {
	//		log("trunk", 0, msg);
	//	}
	//}

	//static int lua_log_debug(lua_State* L)
	//{
	//	/*
	//	luaL_checkstring
	//	检查lua栈中数据，如果为string类型则返回取得的数据，否则返回null
	//	参数：
	//	1--栈
	//	2：栈中位置：-1栈顶，1栈底
	//	*/
	//	const char* msg = luaL_checkstring(L, -1);
	//	if (msg)//访问lua的调用信息
	//	{
	//		do_log_message(print_debug, msg);
	//	}
	//	return 0;
	//}

	//static int lua_log_warning(lua_State* L)
	//{

	//	const char* msg = luaL_checkstring(L, -1);
	//	if (msg)//访问lua的调用信息
	//	{
	//		do_log_message(print_warning, msg);
	//	}
	//	return 0;
	//}

	//static int lua_log_error(lua_State* L)
	//{
	//	const char* msg = luaL_checkstring(L, -1);
	//	if (msg)//访问lua的调用信息
	//	{
	//		do_log_message(print_error, msg);
	//	}
	//	return 0;
	//}

	////重定义lua_panic，防止lua执行出错从而杀死整个进程
	//static int lua_panic(lua_State* g_lua_state) 
	//{
	//	const char* msg = luaL_checkstring(g_lua_state, -1);
	//	if (msg) {
	//		do_log_message(print_error, msg);
	//	}
	//	return 0;
	//}
	//将nHandler对应的函数push到lua栈中
	static bool pushFunctionByHandler(int nHandler)
	{
		toluafix_get_function_by_refid(lua_wrapper::get_luaState(), nHandler);                  /* L: ... func */
		if (!lua_isfunction(lua_wrapper::get_luaState(), -1))
		{
			log_error("[LUA ERROR] function refid '%d' does not reference a Lua function", nHandler);
			lua_pop(lua_wrapper::get_luaState(), 1);
			return false;
		}
		return true;
	}
	//执行lua函数
	static int executeFunction(int numArgs)
	{
		lua_State* g_lua_state = lua_wrapper::get_luaState();
		int functionIndex = -(numArgs + 1);
		if (!lua_isfunction(g_lua_state, functionIndex))
		{
			log_error("value at stack [%d] is not function", functionIndex);
			lua_pop(g_lua_state, numArgs + 1); // remove function and arguments
			return 0;
		}

		int traceback = 0;
		lua_getglobal(g_lua_state, "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
		if (!lua_isfunction(g_lua_state, -1))
		{
			lua_pop(g_lua_state, 1);                                            /* L: ... func arg1 arg2 ... */
		}
		else
		{
			lua_insert(g_lua_state, functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
			traceback = functionIndex - 1;
		}

		int error = 0;
		error = lua_pcall(g_lua_state, numArgs, 1, traceback);                  /* L: ... [G] ret */
		if (error)
		{
			if (traceback == 0)
			{
				log_error("[LUA ERROR] %s", lua_tostring(g_lua_state, -1));        /* L: ... error */
				lua_pop(g_lua_state, 1); // remove error message from stack
			}
			else                                                            /* L: ... G error */
			{
				lua_pop(g_lua_state, 2); // remove __G__TRACKBACK__ and error message from stack
			}
			return 0;
		}

		// get return value
		int ret = 0;
		if (lua_isnumber(g_lua_state, -1))
		{
			ret = (int)lua_tointeger(g_lua_state, -1);
		}
		else if (lua_isboolean(g_lua_state, -1))
		{
			ret = (int)lua_toboolean(g_lua_state, -1);
		}
		// remove return value from stack
		lua_pop(g_lua_state, 1);                                                /* L: ... [G] */

		if (traceback)
		{
			lua_pop(g_lua_state, 1); // remove __G__TRACKBACK__ from stack      /* L: ... */
		}

		return ret;
	}

	static int lua_add_search_path(lua_State* L) {
		const char* path = luaL_checkstring(L, 1);
		if (path) {
			std::string search_path = path;
			lua_wrapper::add_search_path(search_path);
		}
		return 0;
	}
}

void lua_wrapper::init()
{
	g_lua_State = luaL_newstate();//创建lua虚拟机
	lua_atpanic(g_lua_State, lua_panic); //重定义lua_panic，防止lua执行出错,从而调用abort杀死整个进程
	luaL_openlibs(g_lua_State);//打开lua基础库
	toluafix_open(g_lua_State);

	lua_wrapper::reg_func2lua("add_search_path", lua_add_search_path);//注册lua_add_search_path

	register_logger_export(g_lua_State);//注册logger
	register_scheduler_export(g_lua_State);//注册scheduler
	register_netbus_export(g_lua_State);//注册netbus
	register_session_export(g_lua_State);//注册session
	register_proto_man_export(g_lua_State);//注册proto_man
	register_raw_cmd_export(g_lua_State);
	register_service_export(g_lua_State);//注册service
	register_mysql_export(g_lua_State);//注册mysql
	register_redis_export(g_lua_State);//注册redis

	

	//lua_wrapper::reg_func2lua("log_error", lua_log_error);
	//lua_wrapper::reg_func2lua("log_debug", lua_log_debug);
	//lua_wrapper::reg_func2lua("log_warning", lua_log_warning);
}

void lua_wrapper::exit()
{
	if (g_lua_State)
	{
		lua_close(g_lua_State);
		g_lua_State = nullptr;
	}
}

bool lua_wrapper::do_file(const std::string& file)
{
	if (luaL_dofile(g_lua_State, file.c_str()))
	{
		//执行失败
		lua_log_error(g_lua_State);//打印错误信息
		return false;
	}
	return true;
}

void lua_wrapper::reg_func2lua(const char* name, int (*func) (lua_State* L))
{
	lua_pushcfunction(g_lua_State, func);//将函数压入lua栈
	lua_setglobal(g_lua_State, name);//将函数设为全局
}

int lua_wrapper::execute_script_Handle(int nHandle, int numArgs)
{
	int ret = 0;
	if (pushFunctionByHandler(nHandle))                                /* L: ... arg1 arg2 ... func */
	{
		if (numArgs > 0)
		{
			lua_insert(g_lua_State, -(numArgs + 1));                        /* L: ... func arg1 arg2 ... */
		}
		ret = executeFunction(numArgs);
	}
	lua_settop(g_lua_State, 0);
	return ret;
}

void lua_wrapper::remove_script_Handle(int nHandle)
{
	toluafix_remove_function_by_refid(g_lua_State, nHandle);
}

void lua_wrapper::add_search_path(const std::string& path)
{
	char strPath[1024] = { 0 };
	sprintf(strPath, "local path = string.match([[%s]],[[(.*)/[^/]*$]])\n package.path = package.path .. [[;]] .. path .. [[/?.lua;]] .. path .. [[/?/init.lua]]\n", path.c_str());
	luaL_dostring(g_lua_State, strPath);
}
