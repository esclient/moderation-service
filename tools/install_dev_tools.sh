#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== C++ Development Tools Installer ===${NC}"
echo ""

# ============================================================================
# OS Detection
# ============================================================================
echo -e "${CYAN}=== Detecting Operating System ===${NC}"
echo "OSTYPE: $OSTYPE"
echo "uname: $(uname -s 2>/dev/null || echo 'N/A')"

IS_WINDOWS=false
IS_WSL=false
IS_LINUX=false
IS_MACOS=false

# Check for WSL first (highest priority)
if grep -qEi "(Microsoft|WSL)" /proc/version 2>/dev/null; then
    IS_WSL=true
    echo -e "${RED}WSL detected - Linux running inside Windows${NC}"
    echo -e "${RED}ERROR: This script is not designed for WSL!${NC}"
    echo -e "${YELLOW}Please use one of these instead:${NC}"
    echo "  1. PowerShell script: tools/install_dev_tools.ps1"
    echo "  2. Git Bash (from Git for Windows)"
    echo ""
    echo "Download Git for Windows: https://git-scm.com/download/win"
    exit 1
fi

# Detect Windows (Git Bash, MSYS, Cygwin)
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    IS_WINDOWS=true
elif [[ "$(uname -s 2>/dev/null)" == MINGW* || "$(uname -s 2>/dev/null)" == MSYS* || "$(uname -s 2>/dev/null)" == CYGWIN* ]]; then
    IS_WINDOWS=true
elif [[ -n "$WINDIR" || -n "$windir" ]]; then
    IS_WINDOWS=true
# Detect Linux
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    IS_LINUX=true
# Detect macOS
elif [[ "$OSTYPE" == "darwin"* ]]; then
    IS_MACOS=true
fi

if [[ "$IS_WINDOWS" == "true" ]]; then
    echo -e "${GREEN}✓ Windows detected (Git Bash/MSYS)${NC}"
    
    # Check for admin rights on Windows
    if ! net session > /dev/null 2>&1; then
        echo -e "${RED}!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        echo "ERROR: This script MUST be run as ADMINISTRATOR on Windows."
        echo "Please restart Git Bash as Administrator and try again."
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Running as Administrator${NC}"
elif [[ "$IS_LINUX" == "true" ]]; then
    echo -e "${GREEN}✓ Linux detected${NC}"
elif [[ "$IS_MACOS" == "true" ]]; then
    echo -e "${GREEN}✓ macOS detected${NC}"
else
    echo -e "${RED}✗ Unknown OS: $OSTYPE${NC}"
    echo "Please install dependencies manually."
    exit 1
fi

echo ""

# ============================================================================
# Check Prerequisites
# ============================================================================
echo -e "${CYAN}=== Checking Prerequisites ===${NC}"

# Check Python
if ! command -v python &> /dev/null && ! command -v python3 &> /dev/null; then
    echo -e "${RED}✗ Python not found!${NC}"
    if [[ "$IS_WINDOWS" == "true" ]]; then
        echo "Install Python from: https://www.python.org/downloads/"
        echo "Or use: choco install python -y"
    elif [[ "$IS_LINUX" == "true" ]]; then
        echo "Install with: sudo apt-get install python3 python3-pip"
    elif [[ "$IS_MACOS" == "true" ]]; then
        echo "Install with: brew install python3"
    fi
    exit 1
else
    PYTHON_CMD=$(command -v python3 2>/dev/null || command -v python)
    echo -e "${GREEN}✓ Python found: $($PYTHON_CMD --version)${NC}"
fi

# Check Chocolatey on Windows
if [[ "$IS_WINDOWS" == "true" ]]; then
    if ! command -v choco &> /dev/null; then
        echo -e "${YELLOW}⚠ Chocolatey not found${NC}"
        echo "Installing Chocolatey..."
        powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"
        export PATH="$PATH:/c/ProgramData/chocolatey/bin"
    else
        echo -e "${GREEN}✓ Chocolatey found${NC}"
    fi
fi

# Check Homebrew on macOS
if [[ "$IS_MACOS" == "true" ]]; then
    if ! command -v brew &> /dev/null; then
        echo -e "${YELLOW}⚠ Homebrew not found${NC}"
        echo "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    else
        echo -e "${GREEN}✓ Homebrew found${NC}"
    fi
fi

echo ""

# ============================================================================
# Install System Dependencies
# ============================================================================
echo -e "${CYAN}=== Installing System Dependencies ===${NC}"

if [[ "$IS_WINDOWS" == "true" ]]; then
    # LLVM (clang-format, clang-tidy)
    if ! command -v clang-format &> /dev/null; then
        echo "Installing LLVM..."
        choco install llvm -y
    else
        echo -e "${GREEN}✓ LLVM already installed${NC}"
    fi

    # CMake
    if ! command -v cmake &> /dev/null; then
        echo "Installing CMake..."
        choco install cmake -y
    else
        echo -e "${GREEN}✓ CMake already installed${NC}"
    fi

    # cppcheck
    if ! command -v cppcheck &> /dev/null; then
        echo "Installing cppcheck..."
        choco install cppcheck -y
    else
        echo -e "${GREEN}✓ cppcheck already installed${NC}"
    fi

elif [[ "$IS_LINUX" == "true" ]]; then
    echo "Updating package lists..."
    sudo apt-get update -qq
    
    PACKAGES_TO_INSTALL=()
    
    command -v curl &> /dev/null || PACKAGES_TO_INSTALL+=(curl)
    command -v g++ &> /dev/null || PACKAGES_TO_INSTALL+=(build-essential)
    command -v clang-format &> /dev/null || PACKAGES_TO_INSTALL+=(clang-format)
    command -v clang-tidy &> /dev/null || PACKAGES_TO_INSTALL+=(clang-tidy)
    command -v cppcheck &> /dev/null || PACKAGES_TO_INSTALL+=(cppcheck)
    command -v cmake &> /dev/null || PACKAGES_TO_INSTALL+=(cmake)
    
    if [ ${#PACKAGES_TO_INSTALL[@]} -gt 0 ]; then
        echo "Installing: ${PACKAGES_TO_INSTALL[*]}"
        sudo apt-get install -y "${PACKAGES_TO_INSTALL[@]}"
    else
        echo -e "${GREEN}✓ All system packages already installed${NC}"
    fi

elif [[ "$IS_MACOS" == "true" ]]; then
    command -v clang-format &> /dev/null || brew install clang-format
    command -v cmake &> /dev/null || brew install cmake
    command -v cppcheck &> /dev/null || brew install cppcheck
fi

echo ""

# ============================================================================
# Setup Python Environment
# ============================================================================
echo -e "${CYAN}=== Setting up Python Environment ===${NC}"

# Upgrade pip
echo "Upgrading pip..."
$PYTHON_CMD -m pip install --upgrade pip --quiet --user

# Update PATH for user-installed Python packages
if [[ "$IS_WINDOWS" == "true" ]]; then
    PYTHON_SCRIPTS=$($PYTHON_CMD -c "import os, sysconfig; print(sysconfig.get_path('scripts', f'{os.name}_user'))" 2>/dev/null || echo "$HOME/AppData/Roaming/Python/Scripts")
    export PATH="$PATH:$PYTHON_SCRIPTS:$HOME/.local/bin"
else
    export PATH="$PATH:$HOME/.local/bin"
fi

echo ""

# ============================================================================
# Install Python Tools
# ============================================================================
echo -e "${CYAN}=== Installing Python-based Tools ===${NC}"

# Conan
if ! command -v conan &> /dev/null; then
    echo "Installing Conan..."
    $PYTHON_CMD -m pip install --user conan --quiet
    export PATH="$PATH:$HOME/.local/bin"
    
    # Detect Conan profile
    if command -v conan &> /dev/null; then
        conan profile detect --force 2>/dev/null || true
    fi
else
    echo -e "${GREEN}✓ Conan already installed${NC}"
fi

# cpplint
if ! $PYTHON_CMD -c "import cpplint" 2>/dev/null; then
    echo "Installing cpplint..."
    $PYTHON_CMD -m pip install --user cpplint --quiet
else
    echo -e "${GREEN}✓ cpplint already installed${NC}"
fi

# cmakelang (cmake-format, cmake-lint)
if ! $PYTHON_CMD -c "import cmakelang" 2>/dev/null; then
    echo "Installing cmakelang..."
    $PYTHON_CMD -m pip install --user cmakelang --quiet
else
    echo -e "${GREEN}✓ cmakelang already installed${NC}"
fi

echo ""

# ============================================================================
# Verify Installation
# ============================================================================
echo -e "${CYAN}=== Verifying Installation ===${NC}"
echo ""

FAILED=0

check_tool() {
    local tool=$1
    local check_cmd=$2
    
    if eval "$check_cmd" &> /dev/null; then
        echo -e "${GREEN}✓${NC} $tool"
    else
        echo -e "${RED}✗${NC} $tool - NOT FOUND"
        FAILED=1
    fi
}

check_tool "CMake" "command -v cmake"
check_tool "Conan" "command -v conan"
check_tool "clang-format" "command -v clang-format"
check_tool "clang-tidy" "command -v clang-tidy"
check_tool "cppcheck" "command -v cppcheck"
check_tool "cpplint" "$PYTHON_CMD -c 'import cpplint'"
check_tool "cmakelang" "$PYTHON_CMD -c 'import cmakelang'"

echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}=== ✓ All tools installed successfully! ===${NC}"
    echo ""
    echo "Installed versions:"
    cmake --version 2>/dev/null | head -n1 || echo "cmake: version check failed"
    conan --version 2>/dev/null | head -n1 || echo "conan: version check failed"
    clang-format --version 2>/dev/null | head -n1 || echo "clang-format: version check failed"
    cppcheck --version 2>/dev/null | head -n1 || echo "cppcheck: version check failed"
    echo ""
    echo -e "${YELLOW}Note: You may need to restart your terminal for PATH changes to take effect.${NC}"
else
    echo -e "${RED}=== ✗ Some tools failed to install ===${NC}"
    echo "Please check the errors above and try again."
    exit 1
fi