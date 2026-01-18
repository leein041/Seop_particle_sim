set(_target "seop_primitive")
add_library(${_target})
add_library(seop::primitive ALIAS ${_target})

target_sources(${_target} PRIVATE 
    seop_primitive/vertex.hpp
    seop_primitive/vertex.cpp
)

target_link_libraries(${_target} PUBLIC
    seop::math
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
