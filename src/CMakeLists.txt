# 최상위(루트) CMakeLists.txt
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(imgui)
add_subdirectory(seop)