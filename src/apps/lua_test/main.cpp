#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>

#include "../../netbus/netbus.h"
#include "../../netbus/proto_man.h"
#include "../../utils/logger.h"
#include "../../utils/time_list.h"
#include "../../database/mysql_wrapper.h"
#include "../../database/redis_wrapper.h"
#include "../../lua/lua_wrapper.h"


int main(int argc, char* argv[])
{
	netbus::instance()->init();
	lua_wrapper::init();

	if (argc != 3)
	{
		lua_wrapper::add_search_path("../../src/apps/lua_test/scripts/");
		lua_wrapper::do_file("../../src/apps/lua_test/scripts/main.lua");
	}
	else
	{
		std::string search_path = argv[1];
		if (*(search_path.end() - 1) != '/')
		{
			search_path += '/';
		}

		lua_wrapper::add_search_path(search_path);
		lua_wrapper::do_file(search_path + argv[2]);
	}

	netbus::instance()->run();
	lua_wrapper::exit();

	return 0;
}
