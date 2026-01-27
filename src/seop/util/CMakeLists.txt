set(_target "seop_util")
add_library(${_target})
add_library(seop::util ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_util/util.hpp
    seop_util/util.cpp
)

target_link_libraries(${_target} PUBLIC

)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
