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

# configurations
cmake_minimum_required(VERSION 3.22.1)
project(Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# files
set(SOURCES
    thirdparty/entt/entt.hpp

    thirdparty/imgui/imguizmo/imguizmo.h thirdparty/imgui/imguizmo/imguizmo.cpp
    thirdparty/imgui/imconfig.h
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_internal.h
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui.h
    thirdparty/imgui/imstb_rectpack.h
    thirdparty/imgui/imstb_textedit.h
    thirdparty/imgui/imstb_truetype.h
    thirdparty/imgui/backends/imgui_impl_sdl3.h    #thirdparty/imgui/backends/imgui_impl_sdl3.cpp
    thirdparty/imgui/backends/imgui_impl_vulkan.h  #thirdparty/imgui/backends/imgui_impl_vulkan.cpp
    #thirdparty/font/awesome.c thirdparty/font/lucide.c thirdparty/font/robotomono_medium.c

    include/core/application.h source/core/application.cpp
    include/core/input.h
    include/core/logger.h source/core/logger.cpp
    include/core/renderer.h source/core/renderer.cpp
    include/core/window.h source/core/window.cpp

    include/scene/components.h source/scene/components.cpp
    include/scene/entity.h source/scene/entity.cpp
    include/scene/prefab.h source/scene/prefab.cpp
    include/scene/world.h source/scene/world.cpp

    include/ui/gizmo.h source/ui/gizmo.cpp
    include/ui/gui.h source/ui/gui.cpp
    include/ui/icon.h
    include/ui/theme.h
    include/ui/widget.h source/ui/widget.cpp
    include/ui/wrapper_imgui.h

    include/util/datafile.h source/util/datafile.cpp
    include/util/library.h
    include/util/stack.h

    include/cosmos.h
)

# create application and setup
group_sources(${SOURCES})

add_library(Engine SHARED ${SOURCES})

set_target_properties(Engine PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Engine>")
set_target_properties(Engine PROPERTIES FOLDER "Projects")

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/entt)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/font)

# dependencies
find_package(Vulkan REQUIRED)
target_link_libraries(Engine PRIVATE CRen)
target_link_libraries(Engine PRIVATE Vulkan::Vulkan)
target_link_libraries(Engine PRIVATE SDL3::SDL3)

# commands
add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL3::SDL3>"
        "$<TARGET_FILE_DIR:Engine>"
    VERBATIM
)

if(ANDROID)
    set(ANDROID_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../project_android/app/src/main/assets/data") # set path to Android's assets directory
    file(MAKE_DIRECTORY ${ANDROID_ASSETS_DIR}) # create assets directory if it doesn't exist
    
    add_custom_target(CopyingEngineAssets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/data"
            "${ANDROID_ASSETS_DIR}"
        COMMENT "Copying assets to Android's assets/data/"
    ) # copy all files from your data/ folder to Android's assets
    add_dependencies(Engine CopyingEngineAssets) # ensure this runs before the main target
else()
    add_custom_command(
        TARGET Engine POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Engine>/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            $<TARGET_FILE_DIR:Engine>/data
    )
endif()