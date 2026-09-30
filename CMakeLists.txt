cmake_minimum_required(VERSION 3.25)
project(CupertinoUi LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Dear ImGui's features/shadows branch: a checkout in imgui/, or the pinned commit from GitHub
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
if (NOT EXISTS ${IMGUI_DIR}/imgui.h)
    include(FetchContent)
    FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG 332bf51ee9ba61a68f2c66124bf5342fb490104c)
    FetchContent_MakeAvailable(imgui)
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
endif()

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_compile_definitions(imgui PUBLIC IMGUI_USE_WCHAR32 IMGUI_DEFINE_MATH_OPERATORS)
target_compile_features(imgui PUBLIC cxx_std_20)
if (MSVC)
    target_compile_options(imgui PUBLIC /utf-8 /permissive- /Zc:__cplusplus)
endif()

file(GLOB_RECURSE CUPERTINO_SOURCES CONFIGURE_DEPENDS src/CupertinoUi/*.cpp src/CupertinoUi/*.h)
add_library(CupertinoUi STATIC ${CUPERTINO_SOURCES})
target_include_directories(CupertinoUi PUBLIC src/CupertinoUi PRIVATE third_party/stb)
target_link_libraries(CupertinoUi PUBLIC imgui)

file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS examples/*.cpp examples/*.h)
add_library(Examples STATIC ${EXAMPLE_SOURCES})
target_include_directories(Examples PUBLIC examples)
target_link_libraries(Examples PUBLIC CupertinoUi)

add_executable(showcase WIN32 showcase/main.cpp)
target_link_libraries(showcase PRIVATE Examples d3d11 dxgi)
