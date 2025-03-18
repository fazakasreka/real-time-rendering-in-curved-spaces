cmake_minimum_required(VERSION 3.10)
project(real-time-rendering-in-curved-spaces)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")


find_package(OpenGL REQUIRED)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_WEBGL2=1")
    add_executable(${PROJECT_NAME} 
        src/main.cpp
        src/framework/geometry.cpp
        src/framework/gpuProgram.cpp
        src/framework/shader.cpp
        src/framework/texture.cpp
        src/non-euclidean/curvature.cpp
        src/non-euclidean/geomCamera.cpp
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
        src
        src/framework
        src/non-euclidean
    ) 
else()
    add_subdirectory(external/glfw)
    add_executable(${PROJECT_NAME} 
        external/glad/src/glad.c
        src/main.cpp
        src/framework/geometry.cpp
        src/framework/gpuProgram.cpp
        src/framework/shader.cpp
        src/framework/texture.cpp
        src/non-euclidean/curvature.cpp
        src/non-euclidean/geomCamera.cpp
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenGL::GL
        glfw
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
        src
        src/framework
        src/non-euclidean
        external/glad/include
        external/glfw/include
    ) 
endif()
