set(jsoncpp_SRC src/lib_json/json_reader.cpp
                src/lib_json/json_value.cpp
                src/lib_json/json_writer.cpp)

include_directories(${CMAKE_SOURCE_DIR}/3rd/jsoncpp/include)

add_library(jsoncpp STATIC ${jsoncpp_SRC})