# group files within same directory-tree inside visual studio
function(group_sources)
    foreach(file IN LISTS ARGN)
        get_filename_component(dir "${file}" DIRECTORY) # get the directory of the file relative to CMakeLists.txt

        if(dir STREQUAL "")
            set(group "root")  # group root files under "root"
        else()
            string(REPLACE "/" "\\" group "${dir}")  # replace "/" with "\\" for visual vtudio
        endif()

        source_group("${group}" FILES "${file}")
    endforeach()
endfunction()

cmake_minimum_required(VERSION 3.10)
project(CRen LANGUAGES C)

set(SOURCES
    thirdparty/volk/volk.h thirdparty/volk/volk.c
    thirdparty/stb/stb_image.h thirdparty/stb/stb_image_write.h thirdparty/stb/stb_defs.c

    include/cren_callback.h source/cren_callback.c
    include/cren_camera.h source/cren_camera.c
    include/cren_context.h source/cren_context.c
    include/cren_math.h source/cren_math.c
    include/cren_platform.h source/cren_platform.c
    include/cren_utils.h source/cren_utils.c
    include/cren_vulkan.h source/cren_vulkan.c
    include/cren.h
)

group_sources(${SOURCES})

add_library(CRen STATIC ${SOURCES})
set_property(TARGET CRen PROPERTY C_STANDARD 11)
set_target_properties(CRen PROPERTIES FOLDER "CRen")

target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb)
target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/volk)
target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vulkan)

add_custom_command(
    TARGET CRen POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data
)