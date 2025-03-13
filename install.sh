#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Installing real-time-rendering-in-curved-spaces...${NC}"

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo -e "${RED}Git is not installed. Installing git...${NC}"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        brew install git
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        sudo apt-get update && sudo apt-get install -y git
    elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
        winget install Git.Git
    fi
fi

# Check if cmake is installed
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}CMake is not installed. Installing CMake...${NC}"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        brew install cmake
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        sudo apt-get update && sudo apt-get install -y cmake
    elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
        winget install Kitware.CMake
    fi
fi

# Clone the repository if not already in it
if [ ! -d ".git" ]; then
    echo -e "${BLUE}Cloning repository...${NC}"
    git clone https://github.com/fazakasreka/real-time-rendering-in-curved-spaces.git
    cd real-time-rendering-in-curved-spaces
fi

# Install submodules
echo -e "${BLUE}Installing submodules...${NC}"
git submodule update --init --recursive

# Build the project
echo -e "${BLUE}Building project...${NC}"
mkdir -p build
cd build
cmake ..
cmake --build . -j$(nproc)

# Run the application
echo -e "${GREEN}Starting real-time-rendering-in-curved-spaces...${NC}"
./real-time-rendering-in-curved-spaces