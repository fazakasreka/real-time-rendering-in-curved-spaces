# non-euclidean-space

Float around in curved spaces with C++ and OpenGL.

Based on the paper [Adapting Game Engines to Curved Spaces](https://link.springer.com/article/10.1007/s00371-021-02303-2) by László Szirmay-Kalos and Milán Magdics. Many thanks to them for everything they taught me.

# How to run

0. Install CMake (if you don't have it already)
    - Windows: ```winget install Kitware.CMake```
    - Linux: ```sudo apt install cmake```
    - macOS: ```brew install cmake```
1. Install submodules (glfw)
    ```
    git submodule update --init --recursive
    ```
2. Run CMake
    ```
    mkdir build
    cmake -B build -S . #run when you first make the project or make changes to the CMakeLists.txt
    cmake --build build #run when you make changes to the code
    ```
3. Run the executable
    ```
    ./build/non-euclidean-space
    ```


# Common issues and solutions:
1. If CMake can't find OpenGL:
    On macOS, it should be included with Xcode
    On Windows, it should be included with Visual Studio
    - On Ubuntu/Debian:
        ```
        sudo apt install libgl1-mesa-dev
        ```
    - If you're using Visual Studio on Windows:
        - The CMake commands will generate a Visual Studio solution
        - You can open the solution and build from there
        - Or use the command line with:
            ```
            cmake --build . --config Release
            ```
    - For macOS users:
        - Make sure you have Xcode Command Line Tools:
            ```
            xcode-select --install
            ```
2. MacOS can't find c++ libraries:
    - Make sure you have Xcode Command Line Tools:
        ```
        xcode-select --install
        ```
    - Make sure you have the c++ library:
        ```
        c++ --version
        ```
        - If you don't have it, install it:
            ```
            brew install c++
            ```
    - If none of this works, uninstall and reinstall:
        ```
        sudo rm -rf /Library/Developer/CommandLineTools
        xcode-select --install
        ```

# What is GLAD and GLFW for?
  Non-Euclidean Space program
     ↑
    GLFW  ←→  Operating System (windows, input, events)
     ↑
    GLAD  ←→  Graphics Driver (OpenGL functions)
     ↑
  Graphics Hardware