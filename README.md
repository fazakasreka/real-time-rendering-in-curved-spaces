# Real-time rendering in curved spaces

Float around in curved spaces with C++ and OpenGL.

Based on the paper [Adapting Game Engines to Curved Spaces](https://link.springer.com/article/10.1007/s00371-021-02303-2) by László Szirmay-Kalos and Milán Magdics. Many thanks to them for everything they taught me.

## Quick Start

Run this command in your terminal:

```bash
curl -s https://raw.githubusercontent.com/fazakasreka/real-time-rendering-in-curved-spaces/main/install.sh | bash
```

This will:
1. Install required dependencies (git, cmake)
2. Clone the repository
3. Build the project
4. Run the application


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


## Run downloaded repo with CMake

##### 0. Install CMake (if you don't have it already)

    Windows: winget install Kitware.CMake
    Linux: sudo apt install cmake
    macOS: brew install cmake

##### 1. Install submodules (glfw)

    git submodule update --init --recursive

##### 2. Run

    mkdir build
    cmake -B build -S . #re-run when you make changes to the CMakeLists.txt
    cmake --build build #re-run when you make changes to the code
    ./build/real-time-rendering-in-curved-spaces

##### +. Build & run (if you make changes to the code)

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
