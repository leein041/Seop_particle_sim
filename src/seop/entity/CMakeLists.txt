set(_target "seop_entity")
add_library(${_target})
add_library(seop::entity ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_entity/particle.hpp
    seop_entity/particle.cpp
    seop_entity/attractor.hpp
    seop_entity/attractor.cpp
)

target_link_libraries(${_target}
    seop::math
)

target_include_directories(${_target} PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)