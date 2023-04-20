cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ChatClient)

add_executable(ChatClient main.cpp)
target_include_directories(ChatClient PUBLIC ${BOOST_ROOT})
