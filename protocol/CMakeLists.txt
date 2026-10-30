add_library(cppchat-protocol INTERFACE)
add_library(cppchat::protocol ALIAS cppchat-protocol)

target_include_directories(cppchat-protocol
    INTERFACE
        include
)
