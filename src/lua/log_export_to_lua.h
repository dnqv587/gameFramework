#pragma once

//struct lua_State;

int lua_logger_init (lua_State* L);

int register_logger_export (lua_State* L);

int lua_panic (lua_State* g_lua_state);
int  lua_log_error (lua_State* luastate);

