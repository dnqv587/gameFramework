
set(gamebus_SRC utils/cache.c
        utils/logger.cpp
        utils/time_list.c
        utils/timestamp.c
        netbus/netbus.cpp
        netbus/proto_man.cpp
        netbus/service.cpp
        netbus/service_manager.cpp
        netbus/session_udp.cpp
        netbus/session_uv.cpp
        netbus/tcp_protocol.cpp
        netbus/ws_protocol.cpp
        lua/log_export_to_lua.cpp
        lua/lua_wrapper.cpp
        lua/mysql_export_to_lua.cpp
        lua/netbus_export_to_lua.cpp
        lua/proto_man_export_to_lua.cpp
        lua/redis_export_to_lua.cpp
        lua/scheduler_export_to_lua.cpp
        lua/service_export_to_lua.cpp
        lua/session_export_to_lua.cpp
        database/mysql_wrapper.cpp
        database/redis_wrapper.cpp
        crypto/Base64.cpp
        crypto/Sha1.cpp)

if(BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${gamebus_SRC})
else()
    add_library(${PROJECT_NAME} STATIC ${gamebus_SRC})
endif ()

unset(protobuf_DIR)
unset(absl_DIR)
unset(utf8_range_DIR)
unset(GTest_DIR)

set(protobuf_DIR "/home/dai/app/protobuf/lib/cmake/protobuf")
set(absl_DIR "/home/dai/app/protobuf/lib/cmake/absl")
set(utf8_range_DIR "/home/dai/app/protobuf/lib/cmake/utf8_range")
set(GTest_DIR "/home/dai/app/protobuf/lib/cmake/GTest")

find_package(GTest CONFIG REQUIRED)

find_package(absl CONFIG REQUIRED)
if(absl_FOUND)
    include_directories(${absl_INCLUDE_DIR})
    message(STATUS "found absl [${absl_INCLUDE_DIR}]")
else(absl_FOUND)
    message(STATUS "not found absl")
endif (absl_FOUND)

find_package(protobuf CONFIG REQUIRED)
if(protobuf_FOUND)
    include_directories(${Protobuf_INCLUDE_DIR})
    message(STATUS "found protobuf [${Protobuf_INCLUDE_DIR}]")
else(protobuf_FOUND)
    message(STATUS "not found protobuf")
endif (protobuf_FOUND)

include_directories(${PROJECT_SOURCE_DIR}/3rd/http_parser)
include_directories(${PROJECT_SOURCE_DIR}/3rd/jsoncpp/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/libuv/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/lua-5.3.4/src)
include_directories(${PROJECT_SOURCE_DIR}/3rd/mysql/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/openssl/include)
#include_directories(${PROJECT_SOURCE_DIR}/3rd/protobuf/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/redis/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/tolua)

link_directories(${PROJECT_SOURCE_DIR}/3rd/lua-5.3.4/lib)
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/libuv/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/mysql/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/openssl/lib)
#link_directories(${PROJECT_SOURCE_DIR}/3rd/protobuf/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/redis/lib)

target_link_libraries(${PROJECT_NAME} PRIVATE http_parser tolua protobuf jsoncpp mysqlclient ssl protobuf )