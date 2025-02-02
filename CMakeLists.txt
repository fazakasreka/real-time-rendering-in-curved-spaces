cmake_minimum_required(VERSION 3.10)
project(non-euclidean-space)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

find_package(OpenGL REQUIRED)

add_subdirectory(external/glfw)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/framework/globalConstants.cpp
    src/framework/gpuProgram.cpp
    src/framework/texture.cpp
    src/framework/shader.cpp
    external/glad/src/glad.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/framework
    external/glad/include
    external/glfw/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
) 