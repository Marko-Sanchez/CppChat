add_library(cppchat-server-core
    server.cpp
    server.hpp
    acceptor.cpp
    acceptor.h
    service.cpp
    service.hpp
    worker.hpp
)
add_library(server::core ALIAS cppchat-server-core)

target_include_directories(cppchat-server-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(cppchat-server-core
    PUBLIC
        cppchat::protocol
        Threads::Threads
        Boost::boost
)
