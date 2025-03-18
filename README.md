# Real-time rendering in curved spaces

Float around in curved spaces with C++ and OpenGL.

Based on the paper [Adapting Game Engines to Curved Spaces](https://link.springer.com/article/10.1007/s00371-021-02303-2) by László Szirmay-Kalos and Milán Magdics. Many thanks to them for everything they taught me.

## Quick Run with Docker
#### Run the docker image
    docker run --rm -p 8080:8080 fazakasreka/real-time-rendering-in-curved-spaces:latest
#### Go to the url
    http://localhost:8080/real-time-rendering-in-curved-spaces.html

*This docker image runs with webGL.*

## Controls
##### Change geometry
    1 - Hyperbolic space
    2 - Euclidean space (default)
    3 - Spherical space
##### Move
    W - forward
    S - backward
    A - left
    D - right
    E - up
    Q - down
##### Pan
    left click + drag
##### Teleport
    SPACE - If you get lost press space to teleport to the origin.


## Run cloned repo with CMake

##### 0. Install CMake (if you don't have it already)

    Windows: winget install Kitware.CMake
    Linux: sudo apt install cmake
    macOS: brew install cmake

##### 1. Install submodules (glfw)

    git submodule update --init --recursive

##### 2. Build

    mkdir build
    cmake -B build -S . #re-run when you make changes to the CMakeLists.txt
    cmake --build build #re-run when you make changes to the code

#### 3. Run 
MacOS/Linux:
```
./build/real-time-rendering-in-curved-spaces
```
Windows:
```
find where your cpp compiler put the exe and run it from the project's root directory
```

##### +. Build & run for Linux/MacOS

    cmake --build build && ./build/real-time-rendering-in-curved-spaces


## Common issues and solutions

### CMake can't find OpenGL

_On macOS OpenGL should be included with Xcode. On Windows with Visual Studio._ 
- Ubuntu/Debian:
    Install the OpenGL library:
    `sudo apt install libgl1-mesa-dev`
- Windows:
    The CMake commands will generate a Visual Studio solution. You can open the solution and build from there.
    Or use the command line with:
    `cmake --build . --config Release`
- macOS:
    Make sure you have Xcode Command Line Tools:
    `xcode-select --install`

### MacOS can't find c++ libraries
- Make sure you have Xcode Command Line Tools:
    `xcode-select --install`
- Make sure you have the c++ library:
    `c++ --version`
- If you don't have it, install it:
    `brew install c++`
- If none of this works, uninstall and reinstall:
    `sudo rm -rf /Library/Developer/CommandLineTools`
    `xcode-select --install`

#### Publish docker
    docker build -t fazakasreka/real-time-rendering-in-curved-spaces .  

    docker run --rm -p 8080:8080 fazakasreka/real-time-rendering-in-curved-spaces

    docker tag fazakasreka/real-time-rendering-in-curved-spaces fazakasreka/real-time-rendering-in-curved-spaces:latest

    docker push fazakasreka/real-time-rendering-in-curved-spaces:latest                                   