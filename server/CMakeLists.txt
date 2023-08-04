cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ChatServer)

add_executable(ChatServer main.cpp)
# environment variable BOOST_ROOT must be set, simply run "export BOOST_ROOT=/path/to/boost" in terminal or .bashrc
target_include_directories(ChatServer PUBLIC  $ENV{BOOST_ROOT})
