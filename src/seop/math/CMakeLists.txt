set(_target "seop_math")
add_library(${_target})
add_library(seop::math ALIAS ${_target})

target_sources(${_target} PRIVATE
    seop_math/math.hpp
    seop_math/math.cpp
)

target_link_libraries(${_target}
)

target_include_directories(${_target} PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${_target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)