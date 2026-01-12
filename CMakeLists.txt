cmake_minimum_required(VERSION 3.10)
project(ParticleSim)

add_subdirectory(src)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist" CACHE PATH "..." FORCE)
install(TARGETS sim DESTINATION bin)

if(WIN32)
    # 1. Debug 빌드 시: vcpkg debug/bin 폴더의 모든 DLL 복사
    install(
        DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/debug/bin/"
        DESTINATION bin
        CONFIGURATIONS Debug
        FILES_MATCHING PATTERN "*.dll"
    )

    # 2. Release 빌드 시: vcpkg bin 폴더의 모든 DLL 복사
    install(
        DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/bin/"
        DESTINATION bin
        CONFIGURATIONS Release
        FILES_MATCHING PATTERN "*.dll"
    )
endif()