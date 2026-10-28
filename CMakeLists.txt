cmake_minimum_required(VERSION 3.25.0)

project(cppchat
    VERSION "0.0.0"
    LANGUAGES CXX
    DESCRIPTION "A C++ desktop messaging application."
    HOMEPAGE_URL "https://github.com/Marko-Sanchez/CppChat"
)

include(CTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)

option(CHAT_BUILD_SERVER "Build the headless server" OFF)
option(CHAT_BUILD_CHAT "Build the QT client" OFF)

find_package(Git QUIET)
find_package(Threads REQUIRED)
find_package(Boost 1.83 REQUIRED)

# add_subdirectory(protocol)
# add_subdirectory(transport)

if(CHAT_BUILD_SERVER)
    add_subdirectory(server)
endif()
if(CHAT_BUILD_CHAT)
    add_subdirectory(client)
endif()
