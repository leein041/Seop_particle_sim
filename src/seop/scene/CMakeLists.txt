set(_target "seop_scene")
add_library(${_target})
add_library(seop::scene ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_scene/scene_command.hpp
    seop_scene/scene_message.hpp
    seop_scene/scene_message.cpp
    seop_scene/scene.cpp
    seop_scene/scene.hpp
    seop_scene/camera.cpp
    seop_scene/camera.hpp
)

target_link_libraries(${_target}
    seop::command
    seop::msg
    seop::math
    seop::entity
    seop::input
    seop::window
    seop::context
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
