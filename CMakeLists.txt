cmake_minimum_required(VERSION 4.2)
project(nightglow_engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "This project targets Windows (Win32 + DirectX 12) only.")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)

add_executable(nightglow_engine WIN32
        main.cpp
        Windows/BaseWindow.cpp
        Windows/BaseWindow.h
        Utils/Temp/DXSampleHelper.h
        Application/NightglowApplication.cpp
        Application/NightglowApplication.h
        Windows/PrimaryWindow.cpp
        Windows/PrimaryWindow.h
        Rendering/Vertices/VertexTypes.h
        Rendering/Utils/d3dUtil.cpp
        Rendering/Utils/d3dUtil.h
        Rendering/Utils/d3dUtil.h)

target_link_libraries(nightglow_engine PRIVATE
        fmt::fmt
        d3d12.lib
        Microsoft::DirectXMath)

target_compile_definitions(nightglow_engine PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET nightglow_engine PROPERTY CXX_STANDARD 23)
endif()