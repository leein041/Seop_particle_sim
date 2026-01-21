set(_target "seop_window")
add_library(${_target})
add_library(seop::window ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_window/glf_window.cpp
    seop_window/glf_window.hpp
    seop_window/window_message.hpp
    seop_window/window_message.cpp
)

target_link_libraries(${_target}
    PUBLIC
    GLEW::GLEW
    glfw
    OpenGL::GL

    seop::math
    seop::msg
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
