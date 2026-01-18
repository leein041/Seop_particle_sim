set(_target "seop_imgui")
add_library(${_target})
add_library(seop::imgui ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_imgui/imgui_renderer.cpp 
    seop_imgui/imgui_renderer.hpp
    seop_imgui/imgui_core.cpp
    seop_imgui/imgui_core.hpp 
    seop_imgui/imgui_dockspace.hpp
    seop_imgui/imgui_dockspace.cpp

)

target_link_libraries(${_target}
    PUBLIC
    imgui
    GLEW::GLEW
    glfw
    OpenGL::GL

    seop::command
    seop::item
    seop::primitive
    seop::math

    seop::window
    seop::scene
    seop::graphic
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
