add_library(tolua STATIC tolua_event.c
                         tolua_fix.cc
                         tolua_is.c
                         tolua_map.c
                         tolua_push.c
                         tolua_to.c)

target_include_directories(tolua PRIVATE ${CMAKE_SOURCE_DIR}/3rd/lua-5.3.4/src)