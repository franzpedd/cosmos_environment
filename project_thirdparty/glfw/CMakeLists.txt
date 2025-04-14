cmake_minimum_required(VERSION 3.10)
project(GLFW LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(GLFW STATIC)

set(GLFW_SOURCES
    glfw/src/context.c
    glfw/src/init.c
    glfw/src/input.c
    glfw/src/monitor.c
    glfw/src/platform.c
    glfw/src/vulkan.c
    glfw/src/window.c
    
    glfw/src/null_init.c
    glfw/src/null_joystick.c
    glfw/src/null_monitor.c
    glfw/src/null_window.c
)

target_include_directories(GLFW PUBLIC glfw/include)

if(WIN32)
    list(APPEND GLFW_SOURCES
        glfw/src/egl_context.c
        glfw/src/wgl_context.c
        glfw/src/osmesa_context.c
        
        glfw/src/win32_init.c
        glfw/src/win32_joystick.c
        glfw/src/win32_module.c
        glfw/src/win32_monitor.c
        glfw/src/win32_thread.c
        glfw/src/win32_time.c
        glfw/src/win32_window.c
    )
    
    target_compile_definitions(GLFW PRIVATE _GLFW_WIN32 _CRT_SECURE_NO_WARNINGS)
    target_compile_options(GLFW PRIVATE /wd4996 /wd5105)

elseif(APPLE)
    list(APPEND GLFW_SOURCES
        glfw/src/egl_context.c
        glfw/src/nsgl_context.m
        
        glfw/src/cocoa_init.m
        glfw/src/cocoa_joystick.m
        glfw/src/posix_module.c
        glfw/src/cocoa_monitor.m
        glfw/src/posix_thread.c
        glfw/src/posix_time.c
        glfw/src/cocoa_window.m
    )
    
    target_compile_definitions(GLFW PRIVATE _GLFW_COCOA)
    set_source_files_properties(${GLFW_SOURCES} PROPERTIES LANGUAGE OBJC)

elseif(UNIX AND NOT APPLE)
    list(APPEND GLFW_SOURCES
        glfw/src/egl_context.c
        glfw/src/glx_context.c
        glfw/src/osmesa_context.c
        
        glfw/src/x11_init.c
        glfw/src/linux_joystick.c
        glfw/src/posix_module.c
        glfw/src/x11_monitor.c
        glfw/src/posix_thread.c
        glfw/src/posix_time.c
        glfw/src/x11_window.c
        
        glfw/src/posix_poll.c
        glfw/src/xkb_unicode.c
    )
    
    target_compile_definitions(GLFW PRIVATE _GLFW_X11)
    target_link_libraries(GLFW PRIVATE X11)
endif()

target_sources(GLFW PRIVATE ${GLFW_SOURCES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_target_properties(GLFW PROPERTIES FOLDER "Thirdparty")