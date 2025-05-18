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

# options
option(CREN_BUILD_AS_DLL "Build CRen as a DLL" OFF)

# configurations
cmake_minimum_required(VERSION 3.22.1)
project(CRen LANGUAGES C)

set(CMAKE_C_STANDARD 11) 
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES
    thirdparty/stb/stb_image.h thirdparty/stb/stb_image_write.h thirdparty/stb/stb_defs.c

    include/cren_callback.h source/cren_callback.c
    include/cren_camera.h source/cren_camera.c
    include/cren_context.h source/cren_context.c
    include/cren_error.h source/cren_error.c
    include/cren_defines.h
    include/cren_math.h source/cren_math.c
    include/cren_platform.h source/cren_platform.c
    include/cren_utils.h source/cren_utils.c
    include/cren_vulkan.h source/cren_vulkan.c
    include/cren.h
)

# create library and setup
group_sources(${SOURCES})

if(CREN_BUILD_AS_DLL)
    add_library(CRen SHARED ${SOURCES})
    target_compile_definitions(CRen PRIVATE CREN_SHARED_LIBRARY)
    target_compile_definitions(CRen PRIVATE CREN_BUILDING_DLL)
else()
    add_library(CRen STATIC ${SOURCES})
endif(CREN_BUILD_AS_DLL)

set_target_properties(CRen PROPERTIES FOLDER "CRen")
set_target_properties(CRen PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CRen>")

target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb)
target_include_directories(CRen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vulkan)

#dependencies
find_package(Vulkan REQUIRED)
target_link_libraries(CRen PRIVATE Vulkan::Vulkan)

# android dependencies, if on android
if(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)
    target_link_libraries(CRen PUBLIC ${log-lib} ${android-lib})
endif(ANDROID)

# paste shaders on output path
if(NOT ANDROID)
    
endif()

if(ANDROID)
    set(ANDROID_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../project_android/app/src/main/assets/data") # set path to Android's assets directory
    file(MAKE_DIRECTORY ${ANDROID_ASSETS_DIR}) # create assets directory if it doesn't exist
    
    add_custom_target(CopyingAssets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/data"
            "${ANDROID_ASSETS_DIR}"
        COMMENT "Copying assets to Android's assets/data/"
    ) # copy all files from your data/ folder to Android's assets
    add_dependencies(CRen CopyingAssets) # ensure this runs before the main target
else()
    add_custom_command(
        TARGET CRen POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:CRen>/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            $<TARGET_FILE_DIR:CRen>/data
    )
endif()