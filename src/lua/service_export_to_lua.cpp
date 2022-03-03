#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lua_wrapper.h"
#include "../netbus/service.h"
#include "../netbus/service_manager.h"
#include "../netbus/netbus.h"
#include "../netbus/proto_man.h"
#include "../utils/logger.h"

#include "google/protobuf/message.h"
using namespace google::protobuf;
#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

//#include "tolua_fix.h"
#include "service_export_to_lua.h"
#pragma region lua_service_mapping 
#define SERVICE_FUNCTION_MAPPING "lua_service_function_mapping"

static unsigned int s_function_ref_id = 0;

//创建并初始化service的全局映射表
static void init_service_function_map (lua_State* L) 
{
	lua_pushstring (L, SERVICE_FUNCTION_MAPPING);
	lua_newtable (L);
	lua_rawset (L, LUA_REGISTRYINDEX);
}

//保存handler函数
static int save_service_function (lua_State* L, int lo, int def) {
	// function at lo
	if (!lua_isfunction (L, lo)) return 0;

	s_function_ref_id++;

	lua_pushstring (L, SERVICE_FUNCTION_MAPPING);
	lua_rawget (L, LUA_REGISTRYINDEX);                           /* stack: fun ... refid_fun */
	lua_pushinteger (L, s_function_ref_id);                      /* stack: fun ... refid_fun refid */
	lua_pushvalue (L, lo);                                       /* stack: fun ... refid_fun refid fun */

	lua_rawset (L, -3);                  /* refid_fun[refid] = fun, stack: fun ... refid_ptr */
	lua_pop (L, 1);                                              /* stack: fun ... */

	return s_function_ref_id;

	// lua_pushvalue(L, lo);                                           /* stack: ... func */
	// return luaL_ref(L, LUA_REGISTRYINDEX);
}

//获得service函数
static void get_service_function (lua_State* L, int refid) 
{
	lua_pushstring (L, SERVICE_FUNCTION_MAPPING);
	lua_rawget (L, LUA_REGISTRYINDEX);                           /* stack: ... refid_fun */
	lua_pushinteger (L, refid);                                  /* stack: ... refid_fun refid */
	lua_rawget (L, -2);                                          /* stack: ... refid_fun fun */
	lua_remove (L, -2);                                          /* stack: ... fun */
}

//执行函数
static int executeFunction (int numArgs) 
{
	int functionIndex = -(numArgs + 1);
	lua_State* lua_state = lua_wrapper::get_luaState();
	if (!lua_isfunction (lua_state, functionIndex))
	{
		log_error ("value at stack [%d] is not function", functionIndex);
		lua_pop (lua_state, numArgs + 1); // remove function and arguments
		return 0;
	}

	int traceback = 0;
	lua_getglobal (lua_state, "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
	if (!lua_isfunction (lua_state, -1))
	{
		lua_pop (lua_state, 1);                                            /* L: ... func arg1 arg2 ... */
	}
	else
	{
		lua_insert (lua_state, functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
		traceback = functionIndex - 1;
	}

	int error = 0;
	error = lua_pcall (lua_state, numArgs, 1, traceback);                  /* L: ... [G] ret */
	if (error)
	{
		if (traceback == 0)
		{
			log_error ("[LUA ERROR] %s", lua_tostring (lua_state, -1));        /* L: ... error */
			lua_pop (lua_state, 1); // remove error message from stack
		}
		else                                                            /* L: ... G error */
		{
			lua_pop (lua_state, 2); // remove __G__TRACKBACK__ and error message from stack
		}
		return 0;
	}

	// get return value
	int ret = 0;
	if (lua_isnumber (lua_state, -1))
	{
		ret = (int)lua_tointeger (lua_state, -1);
	}
	else if (lua_isboolean (lua_state, -1))
	{
		ret = (int)lua_toboolean (lua_state, -1);
	}
	// remove return value from stack
	lua_pop (lua_state, 1);                                                /* L: ... [G] */

	if (traceback)
	{
		lua_pop (lua_state, 1); // remove __G__TRACKBACK__ from stack      /* L: ... */
	}

	return ret;
}

//将函数压入栈中
static bool	pushFunctionByHandler (int nHandler) 
{
	lua_State* lua_state = lua_wrapper::get_luaState();
	get_service_function (lua_state, nHandler);                  /* L: ... func */
	if (!lua_isfunction (lua_state, -1))
	{
		log_error ("[LUA ERROR] function refid '%d' does not reference a Lua function", nHandler);
		lua_pop (lua_state, 1);
		return false;
	}
	return true;
}

//完整的执行函数和参数
static int execute_service_function (int nHandle, int numArgs) //参数：1函数索引，2参数个数
{
	lua_State* lua_state = lua_wrapper::get_luaState();
	int ret = 0;
	if (pushFunctionByHandler (nHandle))                                /* L: ... arg1 arg2 ... func */
	{
		if (numArgs > 0)
		{
			lua_insert (lua_state, -(numArgs + 1));                        /* L: ... func arg1 arg2 ... */
		}
		ret = executeFunction (numArgs);
	}
	lua_settop (lua_state, 0);
	return ret;
}
#pragma endregion

#pragma region lua_service

class lua_service : public service
{
public :
	unsigned int lua_recv_raw_Handler;
	unsigned int lua_recv_cmd_Handler;
	unsigned int lua_disconnect_Handler;
public :
	virtual bool on_session_recv_raw_cmd (session* s, struct raw_msg* raw) override;
	virtual bool on_session_recv_cmd (session* s, struct cmd_msg* msg) override;//recv cmd
	virtual void on_session_disconnect (session* s,int stype) override;//client disconnect
 };

//将protobuf数据转成lua表
void push_proto_message_tolua (const Message* message) 
{
	lua_State* state = lua_wrapper::get_luaState();
	if (!message) {
		// printf("PushProtobuf2LuaTable failed, message is NULL");
		return;
	}
	const Reflection* reflection = message->GetReflection ();

	// 顶层table
	lua_newtable (state);

	const Descriptor* descriptor = message->GetDescriptor ();//获取描述对象
	for (int32_t index = 0; index < descriptor->field_count (); ++index) {
		const FieldDescriptor* fd = descriptor->field (index);
		const std::string& name = fd->lowercase_name ();

		// key
		lua_pushstring (state, name.c_str ());

		bool bReapeted = fd->is_repeated ();//是否是数组

		if (bReapeted) {
			// repeated这层的table
			lua_newtable (state);
			int size = reflection->FieldSize (*message, fd);
			for (int i = 0; i < size; ++i) {
				char str[32] = { 0 };
				switch (fd->cpp_type ()) {
					case FieldDescriptor::CPPTYPE_DOUBLE:
						lua_pushnumber (state, reflection->GetRepeatedDouble (*message, fd, i));
						break;
					case FieldDescriptor::CPPTYPE_FLOAT:
						lua_pushnumber (state, (double)reflection->GetRepeatedFloat (*message, fd, i));
						break;
					case FieldDescriptor::CPPTYPE_INT64:
						sprintf (str, "%lld", (long long)reflection->GetRepeatedInt64 (*message, fd, i));
						lua_pushstring (state, str);
						break;
					case FieldDescriptor::CPPTYPE_UINT64:

						sprintf (str, "%llu", (unsigned long long)reflection->GetRepeatedUInt64 (*message, fd, i));
						lua_pushstring (state, str);
						break;
					case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
						lua_pushinteger (state, reflection->GetRepeatedEnum (*message, fd, i)->number ());
						break;
					case FieldDescriptor::CPPTYPE_INT32:
						lua_pushinteger (state, reflection->GetRepeatedInt32 (*message, fd, i));
						break;
					case FieldDescriptor::CPPTYPE_UINT32:
						lua_pushinteger (state, reflection->GetRepeatedUInt32 (*message, fd, i));
						break;
					case FieldDescriptor::CPPTYPE_STRING:
					{
						std::string value = reflection->GetRepeatedString(*message, fd, i);
						lua_pushlstring(state, value.c_str(), value.size());
					}
						break;
					case FieldDescriptor::CPPTYPE_BOOL:
						lua_pushboolean (state, reflection->GetRepeatedBool (*message, fd, i));
						break;
					case FieldDescriptor::CPPTYPE_MESSAGE:
						push_proto_message_tolua (&(reflection->GetRepeatedMessage (*message, fd, i)));
						break;
					default:
						break;
				}

				lua_rawseti (state, -2, i + 1); // lua's index start at 1
			}

		}
		else {
			char str[32] = { 0 };
			switch (fd->cpp_type ()) {

				case FieldDescriptor::CPPTYPE_DOUBLE:
					lua_pushnumber (state, reflection->GetDouble (*message, fd));
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					lua_pushnumber (state, (double)reflection->GetFloat (*message, fd));
					break;
				case FieldDescriptor::CPPTYPE_INT64:

					sprintf (str, "%lld", (long long)reflection->GetInt64 (*message, fd));
					lua_pushstring (state, str);
					break;
				case FieldDescriptor::CPPTYPE_UINT64:

					sprintf (str, "%llu", (unsigned long long)reflection->GetUInt64 (*message, fd));
					lua_pushstring (state, str);
					break;
				case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
					lua_pushinteger (state, (int)reflection->GetEnum (*message, fd)->number ());
					break;
				case FieldDescriptor::CPPTYPE_INT32:
					lua_pushinteger (state, reflection->GetInt32 (*message, fd));
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					lua_pushinteger (state, reflection->GetUInt32 (*message, fd));
					break;
				case FieldDescriptor::CPPTYPE_STRING:
				{
														std::string value = reflection->GetString (*message, fd);
														lua_pushlstring (state, value.c_str (), value.size ());
				}
					break;
				case FieldDescriptor::CPPTYPE_BOOL:
					lua_pushboolean (state, reflection->GetBool (*message, fd));
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
#pragma push_macro("GetMessage")
#undef GetMessage
					push_proto_message_tolua (&(reflection->GetMessage (*message, fd)));
#pragma pop_macro("GetMessage")
					break;
				default:
					break;
			}
		}

		lua_rawset (state, -3);
	}
}

bool lua_service::on_session_recv_raw_cmd (session* s, struct raw_msg* msg) 
{
	//int argc = lua_gettop (lua_wrapper::lua_state());
	//if (argc != 2){
	//	printf ("argc is error !!!");
	//	goto lua_failed;
	//}
	lua_State* lua_state = lua_wrapper::get_luaState();
	tolua_pushuserdata (lua_state, (void*)s);
	tolua_pushuserdata (lua_state ,(void*)msg);

	execute_service_function (this->lua_recv_raw_Handler, 2);
	return true;

lua_failed:
	return false;
}

/*
protobf:message key,value 传递成 lua table 
json:json string 传给string

表中格式
{ 1:stype,2:ctype,3:utag,4:body---table 或是 string}
*/
bool lua_service::on_session_recv_cmd (session* s, struct cmd_msg* msg) 
{
	lua_State* lua_state = lua_wrapper::get_luaState();
	tolua_pushuserdata (lua_state, (void*)s);

	lua_newtable (lua_state);
	int index = 1;

	lua_pushinteger (lua_state,msg->stype);
	lua_rawseti (lua_state, -2, index);
	++index;

	lua_pushinteger (lua_state, msg->ctype);
	lua_rawseti (lua_state, -2, index);
	++index;

	lua_pushinteger (lua_state, msg->utag);
	lua_rawseti (lua_state, -2, index);
	++index;

	if (!msg->body)
	{
		lua_pushnil (lua_state);
		lua_rawseti (lua_state, -2, index);
		++index;
	}
	else{
		if (proto_man::proto_type() == PROTO_JSON){
			lua_pushstring (lua_state, (char*)msg->body);
		}
		else{
			push_proto_message_tolua ((Message*)msg->body);
		}
		lua_rawseti (lua_state, -2, index);
		++index;
	}
	execute_service_function (this->lua_recv_cmd_Handler, 2);
	return true;
 }

void lua_service::on_session_disconnect (session* s ,int stype) 
{
	//int argc = lua_gettop (lua_state);
	//if (argc != 2){
	//	printf ("argc is error !!!");
	//	return;
	//}
	lua_State* lua_state = lua_wrapper::get_luaState();
	tolua_pushuserdata (lua_state,(void*)s);
	lua_pushinteger (lua_state, stype);
	execute_service_function (this->lua_disconnect_Handler,2);
	
 }
# pragma endregion
 
#pragma	region lua_interface

static int lua_register_raw_service (lua_State* tolua_S) {
	int stype = (int)tolua_tonumber (tolua_S, 1, 0);
	bool ret = false;
	if (!lua_istable (tolua_S, 2)){
		goto lua_failed;
	}

	unsigned int lua_recv_raw_Handler;
	unsigned int lua_disconnect_Handler;

	lua_getfield (tolua_S, 2, "on_session_recv_raw_cmd");
	lua_getfield (tolua_S, 2, "on_session_disconnect");

	lua_recv_raw_Handler = save_service_function (tolua_S, 3, 0);
	lua_disconnect_Handler = save_service_function (tolua_S, 4, 0);

	//register service
	lua_service* s = new lua_service ();
	s->using_raw_cmd = true;
	s->lua_disconnect_Handler = lua_disconnect_Handler;
	s->lua_recv_cmd_Handler = 0;
	s->lua_recv_raw_Handler = lua_recv_raw_Handler;
	ret = service_manager::register_service (stype, s);
	//end
	lua_pushboolean (tolua_S, ret ? 1 : 0);

lua_failed:
	return 1;
}

static int lua_register_service (lua_State* tolua_S) 
{
	int stype = (int)tolua_tonumber (tolua_S,1,0);//参数1
	bool ret = false;
	if (!lua_istable (tolua_S, 2))//参数2是否是table
	{
		goto lua_failed;
	}

	unsigned int lua_recv_cmd_Handler;
	unsigned int lua_disconnect_Handler;

	lua_getfield (tolua_S,2,"on_session_recv_cmd");//将表中参数2push到栈上--索引3
	lua_getfield (tolua_S,2,"on_session_disconnect");//将表中参数2push到栈上--索引4

	lua_recv_cmd_Handler = save_service_function (tolua_S, 3, 0);
	lua_disconnect_Handler = save_service_function (tolua_S, 4, 0);

	//register service
	lua_service* s = new lua_service ();
	s->using_raw_cmd = false;
	s->lua_disconnect_Handler = lua_disconnect_Handler;
	s->lua_recv_cmd_Handler = lua_recv_cmd_Handler;
	s->lua_recv_raw_Handler = 0;
	ret = service_manager::register_service (stype, s);
	//end
	//push返回值
	lua_pushboolean (tolua_S, ret ? 1 : 0);

lua_failed:
	return 1;
}

int register_service_export (lua_State* tolua_S) {
	init_service_function_map (tolua_S);//创建表

	lua_getglobal (tolua_S, "_G");
	if (lua_istable (tolua_S, -1)){
		tolua_open (tolua_S);
		tolua_module (tolua_S, "service", 0);
		tolua_beginmodule (tolua_S, "service");

		tolua_function (tolua_S, "register", lua_register_service);
		tolua_function (tolua_S, "register_with_raw", lua_register_raw_service);

		tolua_endmodule (tolua_S);
	}
	lua_pop (tolua_S, 1);
	return 0;
}
#pragma endregion