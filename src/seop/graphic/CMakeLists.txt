set(_target "seop_graphic")
add_library(${_target})
add_library(seop::graphic ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_graphic/device.hpp
    seop_graphic/device.cpp 
    seop_graphic/shaders/quad_shader.hpp
    seop_graphic/shaders/particle_shader.hpp
    seop_graphic/shaders/grid_shader.hpp
    seop_graphic/shaders/wire_shader.hpp
    seop_graphic/compute_shaders/gravity_cs_source.hpp 
    seop_graphic/compute_shaders/electro_cs_shader.hpp 
    seop_graphic/compute_shaders/magnetic_field_cs_shader.hpp
)

target_link_libraries(${_target}
    PUBLIC
    seop::math
    seop::primitive
    seop::item
    seop::entity
    seop::context
    seop::msg
    
    seop::scene
    seop::input
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
