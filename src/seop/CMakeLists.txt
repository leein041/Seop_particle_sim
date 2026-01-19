set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)


add_subdirectory(math)
add_subdirectory(primitive)
add_subdirectory(item)
add_subdirectory(raytrace)
add_subdirectory(window)
add_subdirectory(command)
add_subdirectory(message)
add_subdirectory(context)
add_subdirectory(input)
add_subdirectory(entity)
add_subdirectory(graphic)
add_subdirectory(imgui)
add_subdirectory(scene)
add_subdirectory(example)

# 실행파일 생성
add_executable(sim main.cpp)

#main 실행
target_link_libraries(sim
    PRIVATE
    seop::example
)



