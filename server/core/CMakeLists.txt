add_library(cppchat-server-core INTERFACE)
add_library(server::core ALIAS cppchat-server-core)

target_include_directories(cppchat-server-core
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(cppchat-server-core
    INTERFACE
        cppchat::protocol
        Threads::Threads
        Boost::boost
)
