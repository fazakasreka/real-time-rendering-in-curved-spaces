#!/bin/bash

set -e  # Exit on error
trap 'echo -e "\033[0;31mError occurred. Exiting.\033[0m"' ERR

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Installing real-time-rendering-in-curved-spaces...${NC}"

# Function to check and install dependencies
install_package() {
    if ! command -v "$1" &> /dev/null; then
        echo -e "${RED}$1 is not installed. Installing...${NC}"
        if [[ "$OSTYPE" == "darwin"* ]]; then
            brew install "$1"
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            sudo apt-get install -y "$2"
        elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
            winget install "$3"
        fi
    fi
}

# Install required tools
install_package "git" "git" "Git.Git"
install_package "cmake" "cmake" "Kitware.CMake"

# OpenGL & C++ dependencies
if [[ "$OSTYPE" == "darwin"* ]]; then
    if ! xcode-select -p &> /dev/null; then
        echo -e "${RED}Xcode Command Line Tools not found. Installing...${NC}"
        xcode-select --install
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    install_package "pkg-config" "pkg-config" ""
    install_package "libgl1-mesa-dev" "libgl1-mesa-dev" ""
fi

# Remove existing directory if it exists
if [ -d "real-time-rendering-in-curved-spaces" ]; then
    echo -e "${RED}Removing existing 'real-time-rendering-in-curved-spaces' folder...${NC}"
    rm -rf real-time-rendering-in-curved-spaces
fi

# Clone the repository
echo -e "${BLUE}Cloning repository...${NC}"
git clone https://github.com/fazakasreka/real-time-rendering-in-curved-spaces.git
cd real-time-rendering-in-curved-spaces

# Submodule update
echo -e "${BLUE}Installing submodules...${NC}"
git submodule sync
git submodule update --init --recursive --checkout --force

# Build
echo -e "${BLUE}Building project...${NC}"
mkdir -p build
cd build

cmake ..

if [[ "$OSTYPE" == "darwin"* ]]; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=$(nproc)
fi
cmake --build . -j$CORES

# Run
echo -e "${GREEN}Starting real-time-rendering-in-curved-spaces...${NC}"
exec ./real-time-rendering-in-curved-spaces