cmake_minimum_required(VERSION 3.25.0)

project(cppchat
    VERSION "1.1.0"
    LANGUAGES CXX
    DESCRIPTION "A C++ desktop messaging application."
    HOMEPAGE_URL "https://github.com/Marko-Sanchez/CppChat"
)

include(CTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)

option(BUILD_SERVER "Build the headless server" OFF)
option(BUILD_CHAT "Build the QT client" OFF)

find_package(Git QUIET)
find_package(Threads REQUIRED)
find_package(Boost 1.83 REQUIRED)

if (GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE CPPCHAT_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(CPPCHAT_GIT_HASH "unkown")
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/cppchat.h.in
    ${CMAKE_BINARY_DIR}/cppchat.h
)


add_subdirectory(protocol)
# add_subdirectory(transport)

if(BUILD_SERVER)
    add_subdirectory(server)
endif()
if(BUILD_CHAT)
    add_subdirectory(client)
endif()
