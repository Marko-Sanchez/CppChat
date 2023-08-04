cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ChatClient)

add_executable(ChatClient main.cpp)
# environment variable BOOST_ROOT must be set, simply run "export BOOST_ROOT=/path/to/boost" in terminal or .bashrc
target_include_directories(ChatClient PUBLIC $ENV{BOOST_ROOT})
