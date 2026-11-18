add_library(cppchat-server-utility
    commandlineparser.cpp
    commandlineparser.h
)
add_library(server::utility ALIAS cppchat-server-utility)

target_include_directories(cppchat-server-utility
    PUBLIC
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cppchat-server-utility
    PUBLIC
)
