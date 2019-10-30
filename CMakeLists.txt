cmake_minimum_required(VERSION 2.8)

PROJECT(logger)

aux_source_directory(./ SRC_LIST)
set(EXECUTABLE_OUTPUT_PATH ./)

add_definitions(-g -W -Wall -Wno-deprecated -std=c++11)

include_directories(./)

add_executable(logger ${SRC_LIST})

target_link_libraries(logger pthread)

