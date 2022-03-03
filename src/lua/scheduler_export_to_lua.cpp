#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>

#include "../utils/time_list.h"
#include "lua_wrapper.h"

#include "../utils/logger.h"


#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

#include "scheduler_export_to_lua.h"
#include "tolua_fix.h"

//#include "../utils/small_alloc.h"
#define my_malloc malloc
#define my_free free

struct repeat_data {
	int handler;
	int repeat_count;
};


static	void on_lua_repeat_timer (void* udata) {
	struct repeat_data* rd = (struct repeat_data*)udata;
	lua_wrapper::execute_script_Handle (rd->handler,0);

	if (rd->repeat_count == -1)
		return;

	rd->repeat_count--;
	if (rd->repeat_count <= 0){
		lua_wrapper::remove_script_Handle (rd->handler);
		my_free (rd);
	}
}

static int lua_schedule_repeat (lua_State* tolua_S) {
	int handler = toluafix_ref_function (tolua_S,1,0);
	if (NULL == handler){
		goto lua_failed;
	}

	int after_msec = lua_tointeger (tolua_S,2);
	if (after_msec <= 0){
		goto lua_failed;
	}

	int repeat_count = lua_tointeger (tolua_S, 3);
	if (repeat_count == 0){
		goto lua_failed;
	}
	if (repeat_count < 0){
		repeat_count = -1;
	}

	int repeat_mesc = lua_tointeger (tolua_S, 4);
	if (repeat_mesc <= 0){
		repeat_mesc = after_msec;
	}

	struct repeat_data* rd = (struct repeat_data*)my_malloc (sizeof(struct repeat_data));
	rd->handler = handler;
	rd->repeat_count = repeat_count;

	struct timer* time_t = schedule_repeat (on_lua_repeat_timer, rd, after_msec, repeat_count, repeat_mesc);

	tolua_pushuserdata (tolua_S, time_t);
	return 1;
lua_failed:
	if (NULL != handler){
		lua_wrapper::remove_script_Handle (handler);
	}
	lua_pushnil (tolua_S);
	return 1;
}


static int lua_schedule_once (lua_State* tolua_S) {

	int handler = toluafix_ref_function (tolua_S, 1, 0);
	if (NULL == handler){
		goto lua_failed;
	}

	int after_msec = lua_tointeger (tolua_S, 2);
	if (after_msec <= 0){
		goto lua_failed;
	}
	struct repeat_data* rd = (struct repeat_data*)my_malloc (sizeof(struct repeat_data));
	rd->handler = handler;
	rd->repeat_count = 1;

	struct timer* time_t = schedule_once (on_lua_repeat_timer, rd, after_msec);
	tolua_pushuserdata (tolua_S, time_t);

	return 1;

lua_failed:
	lua_pushnil (tolua_S);
	return 1;
}



static int lua_cancel_timer (lua_State* tolua_S) {
	if (!lua_istable (tolua_S, 1)){
		goto lua_failed;
	}

	struct timer* t = (struct timer*)lua_touserdata (tolua_S, 1);
	struct repeat_data* rd = (struct repeat_data*)get_timer_udata (t);
	lua_wrapper::remove_script_Handle (rd->handler);
	my_free (rd);
	cancel_timer (t);
lua_failed:
	return 0;
}

int register_scheduler_export (lua_State* tolua_S) {
	lua_getglobal (tolua_S, "_G");
	if (lua_istable (tolua_S, -1)){
		tolua_open (tolua_S);
		tolua_module (tolua_S, "scheduler", 0);
		tolua_beginmodule (tolua_S, "scheduler");

		tolua_function (tolua_S, "schedule", lua_schedule_repeat);
		tolua_function (tolua_S, "once", lua_schedule_once);
		tolua_function (tolua_S, "cancel", lua_cancel_timer);

		tolua_endmodule (tolua_S);
	}
	lua_pop (tolua_S, 1);
	return 0;
}