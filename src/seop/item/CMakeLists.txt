set(_target "seop_item")
add_library(${_target})
add_library(seop::item ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_item/item.hpp
    seop_item/item.cpp
    seop_item/wire.hpp
    seop_item/wire.cpp
)

target_link_libraries(${_target} PUBLIC
    OpenGL::GL
    GLEW::GLEW
    
    seop::primitive
    seop::opengl
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
