#pragma once
#include <string>
#include <lua.hpp>

class lua_wrapper
{
public:
	//��ʼ��
	static void init();
	//�˳�
	static void exit();
	//ִ��lua�ļ�
	static bool do_file(const std::string& file);
	//���lua������·��
	static void add_search_path(const std::string& path);
	/*
	ע�ắ����lua
	������
	name:ע���ĺ�����
	func������
	*/
	static void reg_func2lua(const char* name, int (*func) (lua_State* L));
	//ִ��lua����
	/*
	* nHandle:���
	* numArgs����������
	*/
	static int execute_script_Handle(int nHandle, int numArgs);
	//ɾ��lua����
	static void remove_script_Handle(int nHandle);

	inline static lua_State* get_luaState() { return g_lua_State; }

private:
	//lua���������
	static lua_State* g_lua_State;

};
