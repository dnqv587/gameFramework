
set(SRC utils/cache.c
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
        netbus/ws_protocol.cpp)

if(BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED SRC)
else()
    add_library(${PROJECT_NAME} STATIC SRC)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE http_parser tolua protobuf  jsoncpp mysqlclient ssl protobuf hiredis )