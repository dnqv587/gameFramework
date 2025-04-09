cmake_minimum_required(VERSION 3.18)
set(PROJECT_NAME "gamebus")
project(${PROJECT_NAME}
        VERSION 1.0.0
        LANGUAGES CXX C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#target_compile_options(${PROJECT_NAME} PUBLIC -fPIC)

add_subdirectory(3rd/http_parser)
add_subdirectory(3rd/jsoncpp)
add_subdirectory(3rd/tolua)
add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()

