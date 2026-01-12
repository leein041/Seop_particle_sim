set(_target "seop_imgui")
add_library(${_target})
add_library(seop::imgui ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_imgui/imgui_renderer.cpp 
    seop_imgui/imgui_renderer.hpp
)

target_link_libraries(${_target}
    PUBLIC
    imgui
    GLEW::GLEW
    glfw
    OpenGL::GL

    seop::window
    seop::math
    seop::scene
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
