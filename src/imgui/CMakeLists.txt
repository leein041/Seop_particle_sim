set(_target "imgui")
add_library(${_target})

target_sources(
    ${_target} PRIVATE
    imgui/imconfig.h
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_glfw.h
    imgui/imgui_impl_opengl3_loader.h
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_opengl3.h
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_draw.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
)

target_link_libraries(${_target} PRIVATE
    GLEW::GLEW
    glfw
    OpenGL::GL
)

target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # "imgui/imgui.h"로 접근 가능
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui # "imgui.h"로 바로 접근 가능
)
