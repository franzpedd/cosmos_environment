cmake_minimum_required(VERSION 3.10)
project(IMGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

add_library(IMGUI STATIC)

set(IMGUI_SOURCES
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h

    imgui/backends/imgui_impl_glfw.h    # imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_vulkan.h  # imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(IMGUI PUBLIC imgui)
target_sources(IMGUI PRIVATE ${IMGUI_SOURCES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_target_properties(IMGUI PROPERTIES FOLDER "Thirdparty")