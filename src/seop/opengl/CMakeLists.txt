set(_target "seop_opengl")
add_library(${_target})
add_library(seop::opengl ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_opengl/vertex_array.hpp
    seop_opengl/vertex_array.cpp
    seop_opengl/buffer.hpp
    seop_opengl/buffer.cpp
    seop_opengl/shader.hpp
    seop_opengl/shader.cpp
    seop_opengl/program.hpp
    seop_opengl/program.cpp
    seop_opengl/texture.hpp
    seop_opengl/texture.cpp

)

target_link_libraries(${_target} PUBLIC
    OpenGL::GL
    GLEW::GLEW

    seop::math
    seop::util
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
