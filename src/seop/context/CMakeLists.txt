set(_target "seop_context")
add_library(${_target})
add_library(seop::context ALIAS ${_target})

# 소스 파일 등록
target_sources(${_target} PRIVATE
    seop_context/context.hpp
    seop_context/context.cpp
)

# lib 링크
target_link_libraries(${_target} PRIVATE
)

# 사용 경로 설정
target_include_directories(${_target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)