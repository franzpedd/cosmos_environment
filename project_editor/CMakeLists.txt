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
project(Editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES
    include/core/application.h source/core/application.cpp

    include/ui/console.h source/ui/console.cpp
    include/ui/demo.h source/ui/demo.cpp
    include/ui/dockspace.h source/ui/dockspace.cpp
    include/ui/viewport.h source/ui/viewport.cpp

    source/main.cpp
)

# create application and setup
group_sources(${SOURCES})

add_executable(Editor ${SOURCES})
target_include_directories(Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# dependencies
target_link_libraries(Editor PRIVATE CRen)
target_link_libraries(Editor PRIVATE Engine)

set_target_properties(Editor PROPERTIES FOLDER "Projects")
set_target_properties(Editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Editor>")