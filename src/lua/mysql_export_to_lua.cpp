#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lua_wrapper.h"
#include "../database/mysql_wrapper.h"

#ifdef __cplusplus
extern "C" 
{
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

#include "tolua_fix.h"
#include "mysql_export_to_lua.h"

static void on_open_cb(const char* err, void* Conn, void* udata)
{
	//压入参数--begin
	if (err){
		lua_pushstring(lua_wrapper::get_luaState(),err);
		lua_pushnil(lua_wrapper::get_luaState());
	}
	else{
		lua_pushnil(lua_wrapper::get_luaState());
		tolua_pushuserdata(lua_wrapper::get_luaState(), Conn);
	}
	//end

	lua_wrapper::execute_script_Handle((int)udata,2);
	lua_wrapper::remove_script_Handle((int)udata);
}

static int lua_mysql_connect(lua_State* tolua_S)
{
	//获得参数--begin
	char* ip = (char*)tolua_tostring(tolua_S,1,0);
	if (ip == NULL){
		goto lua_failed;
	}

	int port = (int)tolua_tonumber(tolua_S,2,0);

	char* db_name = (char*)tolua_tostring(tolua_S, 3, 0);
	if (db_name == NULL){
		goto lua_failed;
	}

	char* uname = (char*)tolua_tostring(tolua_S,4,0);
	if (uname == NULL){
		goto lua_failed;
	}

	char* upwd = (char*)tolua_tostring(tolua_S ,5,0);
	if (upwd == NULL){
		goto lua_failed;
	}
	int handle = toluafix_ref_function(tolua_S,6,0);
	if (handle == 0){
		goto lua_failed;
	}
	//end

	mysql_wrapper::connect(ip, port, db_name, uname, upwd, on_open_cb, (void*)handle);

lua_failed:
	return 0;
}
static int lua_mysql_close(lua_State* tolua_S)
{
	void* Conn = tolua_touserdata(tolua_S,1,0);
	if (Conn){
		mysql_wrapper::close(Conn);
	}
	return 0;
}

//push查询结果
static void push_lua_row(MYSQL_ROW row, int num)
{
	lua_newtable(lua_wrapper::get_luaState());
	int index = 1;
	for (int i = 0; i < num;i++)//表中添加数据
	{
		if (row[i] == NULL){
			lua_pushnil(lua_wrapper::get_luaState());
		}
		else{
			lua_pushstring(lua_wrapper::get_luaState(), row[i]);
		}
		
		lua_rawseti(lua_wrapper::get_luaState(), -2, index);
		++index;
	}
}

static void on_lua_query_cb(const char* err, MYSQL_RES* result, void* udata)
{
	if (err){
		lua_pushstring(lua_wrapper::get_luaState(), err);
		lua_pushnil(lua_wrapper::get_luaState());
	}
	else{
		lua_pushnil(lua_wrapper::get_luaState());
		if (result)//将查询的结果push成一个表
		{
			lua_newtable(lua_wrapper::get_luaState());//创建lua表
			int index = 1;
			int num = mysql_num_fields(result);
			MYSQL_ROW row;
			while (row = mysql_fetch_row(result))//添加表中数据
			{
				push_lua_row(row, num);
				lua_rawseti(lua_wrapper::get_luaState(), -2, index);
				++index;
			}
		}
		else{
			lua_pushnil(lua_wrapper::get_luaState());
		}
	}

	lua_wrapper::execute_script_Handle((int)udata, 2);
	lua_wrapper::remove_script_Handle((int)udata);
}


static int lua_mysql_query(lua_State* tolua_S)
{
	void* Conn = tolua_touserdata(tolua_S, 1, 0);
	if (!Conn){
		goto lua_failed;
	}

	char* sql = (char*)tolua_tostring(tolua_S,2,0);
	if (sql == NULL){
		goto lua_failed;
	}

	int handle = toluafix_ref_function(tolua_S, 3, 0);
	if (handle == 0){
		goto lua_failed;
	}

	mysql_wrapper::query(Conn, sql, on_lua_query_cb, (void*)handle);

lua_failed:
	return 0;
}


int register_mysql_export(lua_State* tolua_S)
{
	lua_getglobal(tolua_S,"_G");//get函数

	if (lua_istable(tolua_S,-1))//是否是表
	{
		tolua_open(tolua_S);
		tolua_module(tolua_S,"mysql",0);
		tolua_beginmodule(tolua_S,"mysql");

		tolua_function(tolua_S,"connect",lua_mysql_connect);//函数入栈
		tolua_function(tolua_S, "close", lua_mysql_close);
		tolua_function(tolua_S, "query", lua_mysql_query);

		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S,1);
	return 0;
}
