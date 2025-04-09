#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>

#include "pf_cmd_map.h"
#include "../../../netbus/proto_man.h"

void init_pf_cmd_map()
{
	std::map<int, std::string> cmd_map = {
		{0,"LoginReq"},
		{1,"LoginRes"}

	};
	proto_man::register_protobuf_cmd_map(cmd_map);
}
