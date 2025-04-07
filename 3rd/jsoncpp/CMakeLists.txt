set(jsoncpp_SRC src/lib_json/json_reader.cpp
                src/lib_json/json_value.cpp
                src/lib_json/json_writer.cpp)

target_include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(jsoncpp STATIC jsoncpp_SRC)