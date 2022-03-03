#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lua_wrapper.h"
#include "../database/redis_wrapper.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

#include "tolua_fix.h"
#include "redis_export_to_lua.h"

static void on_open_cb(const char* err, void* context, void* udata)
{
	if (err){
		lua_pushstring(lua_wrapper::get_luaState(),err);
		lua_pushnil(lua_wrapper::get_luaState());
	}
	else{
		lua_pushnil(lua_wrapper::get_luaState());
		tolua_pushuserdata(lua_wrapper::get_luaState(), context);
	}

	lua_wrapper::execute_script_Handle((int)udata, 2);
	lua_wrapper::remove_script_Handle((int)udata);
}

static int lua_redis_connect(lua_State* tolua_S)
{
	char* ip = (char*)tolua_tostring(tolua_S, 1, 0);
	if (ip == NULL){
		goto lua_failed;
	}

	int port = (int)tolua_tonumber(tolua_S, 2, 0);

	int handle = toluafix_ref_function(tolua_S, 3, 0);
	if (handle == NULL){
		goto lua_failed;
	}
	redis_wrapper::connect(ip, port, on_open_cb, (void*)handle);

lua_failed:
	return 0;
}

static int lua_redis_close(lua_State* tolua_S)
{
	void* Conn = tolua_touserdata(tolua_S, 1, 0);
	if (Conn){
		redis_wrapper::close(Conn);
	}
	return 0;
}

static void push_result_to_lua(redisReply* result)
{
	switch (result->type){
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_STATUS:
			lua_pushstring(lua_wrapper::get_luaState(), result->str);
			break;
		case REDIS_REPLY_INTEGER:
			lua_pushinteger(lua_wrapper::get_luaState(), result->integer);
			break;
		case REDIS_REPLY_NIL:
			lua_pushnil(lua_wrapper::get_luaState());
			break;
		case REDIS_REPLY_ARRAY:
			lua_newtable(lua_wrapper::get_luaState());
			int index = 1;
			for (int i = 0; i < result->elements; i++)
			{
				push_result_to_lua(result->element[i]);
				lua_rawseti(lua_wrapper::get_luaState(), -2, index);
				++index;
			}
		
			break;
	}
}

static void on_query_cb(const char* err, redisReply* result, void* udata)
{
	if (err){
		lua_pushstring(lua_wrapper::get_luaState() ,err);
		lua_pushnil(lua_wrapper::get_luaState());
	}
	else{
		lua_pushnil(lua_wrapper::get_luaState());
		if (result){
			push_result_to_lua(result);
		}
		else{
			lua_pushnil(lua_wrapper::get_luaState());
		}
		
	}

	lua_wrapper::execute_script_Handle((int)udata, 2);
	lua_wrapper::remove_script_Handle((int)udata);
}

static int lua_redis_query(lua_State* tolua_S)
{
	void* context = tolua_touserdata(tolua_S, 1, 0);
	if (!context){
		goto lua_failed;
	}

	char* sql = (char*)tolua_tostring(tolua_S, 2, 0);
	if (sql == NULL){
		goto lua_failed;
	}

	int handle = toluafix_ref_function(tolua_S, 3, 0);
	if (handle == 0){
		goto lua_failed;
	}

	redis_wrapper::query(context, sql, on_query_cb, (void*)handle);

lua_failed:
	return 0;
}


int register_redis_export(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)){
		tolua_open(tolua_S);
		tolua_module(tolua_S, "redis", 0);
		tolua_beginmodule(tolua_S, "redis");

		tolua_function(tolua_S, "connect", lua_redis_connect);
		tolua_function(tolua_S, "close", lua_redis_close);
		tolua_function(tolua_S, "query", lua_redis_query);

		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}
