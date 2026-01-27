set(_target "seop_editor")
add_library(${_target})
add_library(seop::editor ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_editor/editor.hpp
    seop_editor/editor.cpp
)

target_link_libraries(${_target}
    seop::scene
    seop::input
    seop::item
    seop::math
    seop::msg
    seop::window
    seop::raytrace
    seop::primitive
    seop::opengl
    seop::scene
)

target_include_directories(${_target} PUBLIC 

${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)