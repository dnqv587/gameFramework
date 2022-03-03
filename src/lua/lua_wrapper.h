#pragma once
#include <string>
#include <lua.hpp>

class lua_wrapper
{
public:
	//初始化
	static void init();
	//退出
	static void exit();
	//执行lua文件
	static bool do_file(const std::string& file);
	//添加lua的搜索路径
	static void add_search_path(const std::string& path);
	/*
	注册函数给lua
	参数：
	name:注册后的函数名
	func：函数
	*/
	static void reg_func2lua(const char* name, int (*func) (lua_State* L));
	//执行lua函数
	/*
	* nHandle:句柄
	* numArgs：参数个数
	*/
	static int execute_script_Handle(int nHandle, int numArgs);
	//删除lua函数
	static void remove_script_Handle(int nHandle);

	inline static lua_State* get_luaState() { return g_lua_State; }

private:
	//lua虚拟机对象
	static lua_State* g_lua_State;

};
