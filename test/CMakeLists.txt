
include_directories(${PROJECT_SOURCE_DIR}/3rd/http_parser)
include_directories(${PROJECT_SOURCE_DIR}/3rd/jsoncpp/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/libuv/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/lua-5.3.4/src)
include_directories(${PROJECT_SOURCE_DIR}/3rd/mysql/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/openssl/include)
#include_directories(${PROJECT_SOURCE_DIR}/3rd/protobuf/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/redis/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/tolua)

link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/libuv/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/mysql/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/openssl/lib)
#link_directories(${PROJECT_SOURCE_DIR}/3rd/protobuf/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/redis/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/lua-5.3.4/lib)

add_executable(luatest apps/lua_test/main.cpp)

target_include_directories(luatest PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(luatest PRIVATE ${PROJECT_NAME} lua http_parser tolua protobuf jsoncpp mysqlclient ssl protobuf uv hiredis)

