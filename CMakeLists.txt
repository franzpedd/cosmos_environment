cmake_minimum_required(VERSION 3.19)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_PROJECTS "Build the projects as well as CRen" ON)

project(CRen VERSION 1.0 LANGUAGES C)

add_subdirectory(cren)

if(BUILD_PROJECTS)
    add_subdirectory(project_helloworld)
    add_subdirectory(project_engine)
    add_subdirectory(project_editor)
    
    add_subdirectory(project_thirdparty/glfw)
    add_subdirectory(project_thirdparty/imgui)
endif()