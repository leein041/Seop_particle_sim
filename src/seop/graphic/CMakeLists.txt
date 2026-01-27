set(_target "seop_graphic")
add_library(${_target})
add_library(seop::graphic ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_graphic/device.hpp
    seop_graphic/device.cpp 
    seop_graphic/shader_program.hpp
    seop_graphic/shader_program.cpp
)

target_link_libraries(${_target}
    PUBLIC
    glfw
    GLEW::GLEW
    OpenGL::GL

    seop::math
    seop::primitive
    seop::opengl
    seop::util
    seop::item
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
