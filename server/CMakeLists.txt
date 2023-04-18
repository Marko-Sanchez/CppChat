cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ChatServer)

add_executable(ChatServer main.cpp)
target_include_directories(ChatServer PUBLIC ${BOOST_ROOT})
