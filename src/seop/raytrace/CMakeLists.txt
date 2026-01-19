set(_target "seop_raytrace")
add_library(${_target})
add_library(seop::raytrace ALIAS ${_target})

target_sources(${_target} PRIVATE 
    seop_raytrace/ray.hpp
    seop_raytrace/ray.cpp
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
