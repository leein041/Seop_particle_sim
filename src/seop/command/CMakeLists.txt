set(_target "seop_command")
add_library(${_target})
add_library(seop::command ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_command/command.cpp
    seop_command/command.hpp
)

target_link_libraries(${_target}
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
