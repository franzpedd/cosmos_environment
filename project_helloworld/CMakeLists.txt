add_executable(HelloWorld example_glfw.c)

target_include_directories(HelloWorld PRIVATE ${CMAKE_SOURCE_DIR}/cren)
target_link_libraries(HelloWorld PRIVATE CRen)

target_include_directories(HelloWorld PRIVATE ${CMAKE_SOURCE_DIR}/project_thirdparty/glfw)
target_link_libraries(HelloWorld PRIVATE GLFW)

set_target_properties(HelloWorld PROPERTIES FOLDER "Projects")