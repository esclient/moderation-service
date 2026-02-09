#!/bin/bash
set -e

echo "=== Installing C++ Development Tools ==="

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    if ! net session > /dev/null 2>&1; then
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        echo "ERROR: This script MUST be run as ADMINISTRATOR on Windows."
        echo "Please restart Git Bash/VS Code as Administrator and try again."
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        exit 1
    fi
fi

echo "=== Detecting OS ==="

# Install OS-specific dependencies (pipx, curl, build tools, LLVM, Cmake, cppcheck)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Linux detected (Ubuntu/Debian style)"
    sudo apt-get update
    sudo apt-get install -y pipx curl build-essential clang-format clang-tidy cppcheck cmake
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    echo "Windows detected (Git Bash/MSYS)"

    if ! command -v pipx &> /dev/null; then
        echo "Installing pipx via python..."
        python -m pip install --user pipx
    fi

    choco install llvm -y
    choco install cmake -y
    choco install cppcheck -y
else
    echo "Unknown OS: $OSTYPE. Please install dependencies manually."
    exit 1
fi

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    PYTHON_SCRIPTS=$(python -c "import os, sysconfig; print(sysconfig.get_path('scripts',f'{os.name}_user'))")
    export PATH="$PATH:$PYTHON_SCRIPTS:$HOME/.local/bin"
else
    export PATH="$PATH:$HOME/.local/bin"
fi

# Install Conan
if ! command -v conan &> /dev/null; then
    echo "Installing Conan..."
    pipx install conan --quiet
    export PATH="$PATH:$HOME/.local/bin"
    conan profile detect --force
fi

# Install Python-based tools via pipx
echo "Installing Python linting tools via pipx..."
pipx install cpplint --quiet || true
pipx install cmakelang --quiet || true

echo ""
echo "All development tools installed!"
echo ""
echo "Installed versions:"
conan --version | head -n1
cmake --version | head -n1
clang-format --version | head -n1
clang-tidy --version | head -n1
cppcheck --version | head -n1
cpplint --version 2>&1 | head -n1 || echo "cpplint installed via pipx"
echo "Cmake-format version:"
cmake-format --version 2>&1 | head -n1 || echo "cmake-format not installed"