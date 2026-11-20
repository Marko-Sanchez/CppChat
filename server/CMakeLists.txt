project(ChatServer)

add_subdirectory(utility)
add_subdirectory(core)

add_executable(cppchat-server main.cpp)

target_include_directories(cppchat-server
    PRIVATE
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(cppchat-server
    PRIVATE
        server::utility
        server::core
)
