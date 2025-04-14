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
project(Engine LANGUAGES CXX )

# settings
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# files
set(SOURCES
    thirdparty/entt/entt.hpp

    include/core/application.h source/core/application.cpp
    include/core/input.h
    include/core/logger.h source/core/logger.cpp
    include/core/renderer.h source/core/renderer.cpp
    include/core/window.h source/core/window.cpp

    include/scene/components.h source/scene/components.cpp
    include/scene/entity.h source/scene/entity.cpp
    include/scene/prefab.h source/scene/prefab.cpp
    include/scene/world.h source/scene/world.cpp

    include/ui/gui.h source/ui/gui.cpp
    include/ui/icon.h
    include/ui/theme.h
    include/ui/widget.h source/ui/widget.cpp

    include/util/datafile.h source/util/datafile.cpp
    include/util/library.h
    include/util/stack.h

    include/cosmos.h
)
group_sources(${SOURCES})

add_library(Engine STATIC ${SOURCES})

target_include_directories(Engine PUBLIC include)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/entt)

set_target_properties(Engine PROPERTIES FOLDER "Projects")
add_custom_command(
    TARGET Engine POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data
)

# dependencies
target_include_directories(Engine PRIVATE ${CMAKE_SOURCE_DIR}/cren)
target_link_libraries(Engine PRIVATE CRen)

target_include_directories(Engine PRIVATE ${CMAKE_SOURCE_DIR}/project_thirdparty/glfw)
target_link_libraries(Engine PRIVATE GLFW)

target_include_directories(Engine PRIVATE ${CMAKE_SOURCE_DIR}/project_thirdparty/imgui)
target_link_libraries(Engine PRIVATE IMGUI)