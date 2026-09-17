cmake_minimum_required(VERSION 3.10)
project (hello_window VERSION 1.0.0)

find_package(OpenGL REQUIRED)

add_executable(hello_window source/config.h source/main.cpp source/glad.c)

add_library(matrices STATIC
    source/mathobjects.h
    source/mathobjects.cpp
)

add_library(imgui STATIC
    dependencies/imgui/imgui.cpp
    dependencies/imgui/imgui.h
    dependencies/imgui/imgui_draw.cpp
    dependencies/imgui/imgui_tables.cpp
    dependencies/imgui/imgui_widgets.cpp
    dependencies/imgui/imgui_demo.cpp
    dependencies/imgui/backends/imgui_impl_glfw.cpp
    dependencies/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC 
    dependencies/imgui
    dependencies/imgui/backends
    dependencies
)

target_link_libraries(hello_window PRIVATE matrices imgui ../dependencies/GLFW/glfw3 OpenGL::GL)
target_include_directories(hello_window PRIVATE dependencies)