set(_target "seop_example")
add_library(${_target})
add_library(seop::example ALIAS ${_target})

# 소스 파일 등록
target_sources(${_target} PRIVATE
    seop_example/example.hpp
    seop_example/example.cpp
)

# lib 링크
target_link_libraries(${_target} PRIVATE
    seop::context
    seop::command
    seop::msg
    seop::math
    seop::window
    seop::input
    seop::entity
    seop::graphic
    seop::scene
    seop::imgui
    seop::editor
    
    GLEW::GLEW
    glfw
    OpenGL::GL
)

# 사용 경로 설정
target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)