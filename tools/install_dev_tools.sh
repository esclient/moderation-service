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

# Check for WSL first (highest priority)
if grep -qEi "(Microsoft|WSL)" /proc/version 2>/dev/null; then
    IS_WSL=true
    echo -e "${GREEN}[OK] WSL detected - Linux running inside Windows${NC}"
# Detect Windows (Git Bash, MSYS, Cygwin)
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    IS_WINDOWS=true
elif [[ "$(uname -s 2>/dev/null)" == MINGW* || "$(uname -s 2>/dev/null)" == MSYS* || "$(uname -s 2>/dev/null)" == CYGWIN* ]]; then
    IS_WINDOWS=true
elif [[ -n "$WINDIR" || -n "$windir" ]]; then
    IS_WINDOWS=true
# Detect Linux
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    IS_LINUX=true
fi

if [[ "$IS_WSL" == "true" ]]; then
    echo -e "${GREEN}[OK] WSL environment${NC}"
elif [[ "$IS_WINDOWS" == "true" ]]; then
    echo -e "${GREEN}[OK] Windows detected (Git Bash/MSYS)${NC}"
    
    # Check for admin rights on Windows
    if ! net session > /dev/null 2>&1; then
        echo -e "${RED}!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        echo "ERROR: This script MUST be run as ADMINISTRATOR on Windows."
        echo "Please restart Git Bash as Administrator and try again."
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!${NC}"
        exit 1
    fi
    echo -e "${GREEN}[OK] Running as Administrator${NC}"
elif [[ "$IS_LINUX" == "true" ]]; then
    echo -e "${GREEN}[OK] Linux detected${NC}"
else
    echo -e "${RED}[X] Unknown OS: $OSTYPE${NC}"
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
    echo -e "${RED}[X] Python not found!${NC}"
    if [[ "$IS_WINDOWS" == "true" ]]; then
        echo "Install Python from: https://www.python.org/downloads/"
        echo "Or use: choco install python -y"
    elif [[ "$IS_LINUX" == "true" || "$IS_WSL" == "true" ]]; then
        echo "Install with: sudo apt-get install python3 python3-pip"
    elif [[ "$IS_MACOS" == "true" ]]; then
        echo "Install with: brew install python3"
    fi
    exit 1
else
    PYTHON_CMD=$(command -v python3 2>/dev/null || command -v python)
    echo -e "${GREEN}[OK] Python found: $($PYTHON_CMD --version)${NC}"
fi

# Check pip module
if ! $PYTHON_CMD -m pip --version &> /dev/null; then
    echo -e "${YELLOW}⚠ pip module not found, installing...${NC}"
    if [[ "$IS_LINUX" == "true" || "$IS_WSL" == "true" ]]; then
        if command -v apt-get &> /dev/null; then
            ${SUDO_CMD:-sudo} apt-get install -y python3-pip python3-venv
        elif command -v dnf &> /dev/null; then
            ${SUDO_CMD:-sudo} dnf install -y python3-pip
        elif command -v yum &> /dev/null; then
            ${SUDO_CMD:-sudo} yum install -y python3-pip
        elif command -v pacman &> /dev/null; then
            ${SUDO_CMD:-sudo} pacman -S --noconfirm python-pip
        elif command -v zypper &> /dev/null; then
            ${SUDO_CMD:-sudo} zypper install -y python3-pip
        fi
    fi
    
    # Verify pip is now available
    if ! $PYTHON_CMD -m pip --version &> /dev/null; then
        echo -e "${RED}[X] Failed to install pip${NC}"
        exit 1
    fi
    echo -e "${GREEN}[OK] pip installed successfully${NC}"
else
    echo -e "${GREEN}[OK] pip found${NC}"
fi

# Check Chocolatey on Windows
if [[ "$IS_WINDOWS" == "true" ]]; then
    if ! command -v choco &> /dev/null; then
        echo -e "${YELLOW}⚠ Chocolatey not found${NC}"
        echo "Installing Chocolatey..."
        powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"
        export PATH="$PATH:/c/ProgramData/chocolatey/bin"
    else
        echo -e "${GREEN}[OK] Chocolatey found${NC}"
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
        echo -e "${GREEN}[OK] LLVM already installed${NC}"
    fi

    # CMake
    if ! command -v cmake &> /dev/null; then
        echo "Installing CMake..."
        choco install cmake -y
    else
        echo -e "${GREEN}[OK] CMake already installed${NC}"
    fi

    # cppcheck
    if ! command -v cppcheck &> /dev/null; then
        echo "Installing cppcheck..."
        choco install cppcheck -y
    else
        echo -e "${GREEN}[OK] cppcheck already installed${NC}"
    fi

elif [[ "$IS_LINUX" == "true" || "$IS_WSL" == "true" ]]; then
    # Detect package manager
    if command -v apt-get &> /dev/null; then
        echo "Detected: Debian/Ubuntu (apt-get)"
        
        # Check if we need sudo
        SUDO_CMD=""
        if [ "$EUID" -ne 0 ]; then
            if command -v sudo &> /dev/null; then
                SUDO_CMD="sudo"
            else
                echo -e "${RED}[X] This script requires root privileges or sudo${NC}"
                exit 1
            fi
        fi
        
        echo "Updating package lists..."
        $SUDO_CMD apt-get update -qq
        
        PACKAGES_TO_INSTALL=()
        
        command -v curl &> /dev/null || PACKAGES_TO_INSTALL+=(curl)
        command -v g++ &> /dev/null || PACKAGES_TO_INSTALL+=(build-essential)
        command -v clang-format &> /dev/null || PACKAGES_TO_INSTALL+=(clang-format)
        command -v clang-tidy &> /dev/null || PACKAGES_TO_INSTALL+=(clang-tidy)
        command -v cppcheck &> /dev/null || PACKAGES_TO_INSTALL+=(cppcheck)
        command -v cmake &> /dev/null || PACKAGES_TO_INSTALL+=(cmake)
        command -v pipx &> /dev/null || PACKAGES_TO_INSTALL+=(pipx)
        dpkg -l | grep -q python3-full || PACKAGES_TO_INSTALL+=(python3-full)
        
        if [ ${#PACKAGES_TO_INSTALL[@]} -gt 0 ]; then
            echo "Installing: ${PACKAGES_TO_INSTALL[*]}"
            $SUDO_CMD apt-get install -y "${PACKAGES_TO_INSTALL[@]}"
        else
            echo -e "${GREEN}[OK] All system packages already installed${NC}"
        fi
        
    elif command -v dnf &> /dev/null; then
        echo "Detected: Fedora/RHEL (dnf)"
        
        SUDO_CMD=""
        if [ "$EUID" -ne 0 ]; then
            if command -v sudo &> /dev/null; then
                SUDO_CMD="sudo"
            else
                echo -e "${RED}[X] This script requires root privileges or sudo${NC}"
                exit 1
            fi
        fi
        
        PACKAGES_TO_INSTALL=()
        
        command -v curl &> /dev/null || PACKAGES_TO_INSTALL+=(curl)
        command -v g++ &> /dev/null || PACKAGES_TO_INSTALL+=(gcc-c++ make)
        command -v clang-format &> /dev/null || PACKAGES_TO_INSTALL+=(clang-tools-extra)
        command -v cppcheck &> /dev/null || PACKAGES_TO_INSTALL+=(cppcheck)
        command -v cmake &> /dev/null || PACKAGES_TO_INSTALL+=(cmake)
        
        if [ ${#PACKAGES_TO_INSTALL[@]} -gt 0 ]; then
            echo "Installing: ${PACKAGES_TO_INSTALL[*]}"
            $SUDO_CMD dnf install -y "${PACKAGES_TO_INSTALL[@]}"
        else
            echo -e "${GREEN}[OK] All system packages already installed${NC}"
        fi
        
    elif command -v yum &> /dev/null; then
        echo "Detected: CentOS/RHEL (yum)"
        
        SUDO_CMD=""
        if [ "$EUID" -ne 0 ]; then
            if command -v sudo &> /dev/null; then
                SUDO_CMD="sudo"
            else
                echo -e "${RED}[X] This script requires root privileges or sudo${NC}"
                exit 1
            fi
        fi
        
        PACKAGES_TO_INSTALL=()
        
        command -v curl &> /dev/null || PACKAGES_TO_INSTALL+=(curl)
        command -v g++ &> /dev/null || PACKAGES_TO_INSTALL+=(gcc-c++ make)
        command -v clang-format &> /dev/null || PACKAGES_TO_INSTALL+=(clang)
        command -v cppcheck &> /dev/null || PACKAGES_TO_INSTALL+=(cppcheck)
        command -v cmake &> /dev/null || PACKAGES_TO_INSTALL+=(cmake)
        
        if [ ${#PACKAGES_TO_INSTALL[@]} -gt 0 ]; then
            echo "Installing: ${PACKAGES_TO_INSTALL[*]}"
            $SUDO_CMD yum install -y "${PACKAGES_TO_INSTALL[@]}"
        else
            echo -e "${GREEN}[OK] All system packages already installed${NC}"
        fi
        
    elif command -v pacman &> /dev/null; then
        echo "Detected: Arch Linux (pacman)"
        
        SUDO_CMD=""
        if [ "$EUID" -ne 0 ]; then
            if command -v sudo &> /dev/null; then
                SUDO_CMD="sudo"
            else
                echo -e "${RED}[X] This script requires root privileges or sudo${NC}"
                exit 1
            fi
        fi
        
        PACKAGES_TO_INSTALL=()
        
        command -v curl &> /dev/null || PACKAGES_TO_INSTALL+=(curl)
        command -v g++ &> /dev/null || PACKAGES_TO_INSTALL+=(base-devel)
        command -v clang-format &> /dev/null || PACKAGES_TO_INSTALL+=(clang)
        command -v cppcheck &> /dev/null || PACKAGES_TO_INSTALL+=(cppcheck)
        command -v cmake &> /dev/null || PACKAGES_TO_INSTALL+=(cmake)
        
        if [ ${#PACKAGES_TO_INSTALL[@]} -gt 0 ]; then
            echo "Installing: ${PACKAGES_TO_INSTALL[*]}"
            $SUDO_CMD pacman -S --noconfirm "${PACKAGES_TO_INSTALL[@]}"
        else
            echo -e "${GREEN}[OK] All system packages already installed${NC}"
        fi
        
    elif command -v zypper &> /dev/null; then
        echo "Detected: openSUSE (zypper)"
        
        SUDO_CMD=""
        if [ "$EUID" -ne 0 ]; then
            if command -v sudo &> /dev/null; then
                SUDO_CMD="sudo"
            else
                echo -e "${RED}[X] This script requires root privileges or sudo${NC}"
                exit 1
            fi
        fi
        
        PACKAGES_TO_INSTALL=()
        
        command -v curl &> /dev/null || PACKAGES_TO_INSTALL+=(curl)
        command -v g++ &> /dev/null || PACKAGES_TO_INSTALL+=(-t pattern devel_C_C++)
        command -v clang-format &> /dev/null || PACKAGES_TO_INSTALL+=(clang)
        command -v cppcheck &> /dev/null || PACKAGES_TO_INSTALL+=(cppcheck)
        command -v cmake &> /dev/null || PACKAGES_TO_INSTALL+=(cmake)
        
        if [ ${#PACKAGES_TO_INSTALL[@]} -gt 0 ]; then
            echo "Installing: ${PACKAGES_TO_INSTALL[*]}"
            $SUDO_CMD zypper install -y "${PACKAGES_TO_INSTALL[@]}"
        else
            echo -e "${GREEN}[OK] All system packages already installed${NC}"
        fi
        
    else
        echo -e "${YELLOW}⚠ Unknown package manager${NC}"
        echo "Please install these packages manually:"
        echo "  - curl"
        echo "  - build-essential / gcc-c++ / base-devel"
        echo "  - clang-format"
        echo "  - clang-tidy"
        echo "  - cppcheck"
        echo "  - cmake"
    fi
fi

echo ""

# ============================================================================
# Install Compiler Cache (sccache preferred, ccache fallback)
# ============================================================================
echo -e "${CYAN}=== Installing Compiler Cache ===${NC}"

if command -v sccache &> /dev/null; then
    echo -e "${GREEN}[OK] sccache already installed${NC}"
elif command -v ccache &> /dev/null; then
    echo -e "${GREEN}[OK] ccache already installed (sccache not available)${NC}"
else
    CACHE_INSTALLED=false

    if [[ "$IS_WINDOWS" == "true" ]]; then
        echo "Installing sccache via Chocolatey..."
        choco install sccache -y && CACHE_INSTALLED=true

    elif [[ "$IS_LINUX" == "true" || "$IS_WSL" == "true" ]]; then
        # Try cargo first (most up-to-date)
        if command -v cargo &> /dev/null; then
            echo "Installing sccache via cargo..."
            cargo install sccache && CACHE_INSTALLED=true

        # Try GitHub releases binary
        else
            echo "Installing sccache from GitHub releases..."
            SCCACHE_VERSION="v0.9.1"
            ARCH=$(uname -m)
            if [[ "$ARCH" == "x86_64" ]]; then
                SCCACHE_ARCH="x86_64-unknown-linux-musl"
            elif [[ "$ARCH" == "aarch64" ]]; then
                SCCACHE_ARCH="aarch64-unknown-linux-musl"
            else
                echo -e "${YELLOW}⚠ Unsupported arch for sccache binary: $ARCH${NC}"
                SCCACHE_ARCH=""
            fi

            if [[ -n "$SCCACHE_ARCH" ]]; then
                SCCACHE_URL="https://github.com/mozilla/sccache/releases/download/${SCCACHE_VERSION}/sccache-${SCCACHE_VERSION}-${SCCACHE_ARCH}.tar.gz"
                echo "Downloading from: $SCCACHE_URL"
                curl -fsSL "$SCCACHE_URL" | tar xz -C /tmp
                ${SUDO_CMD:-sudo} mv /tmp/sccache-${SCCACHE_VERSION}-${SCCACHE_ARCH}/sccache /usr/local/bin/sccache
                ${SUDO_CMD:-sudo} chmod +x /usr/local/bin/sccache
                CACHE_INSTALLED=true
            fi
        fi

        # ccache as fallback if sccache failed
        if [[ "$CACHE_INSTALLED" == "false" ]]; then
            echo -e "${YELLOW}⚠ sccache install failed, falling back to ccache...${NC}"
            if command -v apt-get &> /dev/null; then
                ${SUDO_CMD:-sudo} apt-get install -y ccache && CACHE_INSTALLED=true
            elif command -v dnf &> /dev/null; then
                ${SUDO_CMD:-sudo} dnf install -y ccache && CACHE_INSTALLED=true
            elif command -v yum &> /dev/null; then
                ${SUDO_CMD:-sudo} yum install -y ccache && CACHE_INSTALLED=true
            elif command -v pacman &> /dev/null; then
                ${SUDO_CMD:-sudo} pacman -S --noconfirm ccache && CACHE_INSTALLED=true
            elif command -v zypper &> /dev/null; then
                ${SUDO_CMD:-sudo} zypper install -y ccache && CACHE_INSTALLED=true
            fi
        fi
    fi

    if [[ "$CACHE_INSTALLED" == "true" ]]; then
        CACHE_TOOL=$(command -v sccache 2>/dev/null || command -v ccache 2>/dev/null || echo "none")
        echo -e "${GREEN}[OK] Compiler cache installed: $CACHE_TOOL${NC}"
    else
        echo -e "${YELLOW}⚠ No compiler cache installed — builds will work but won't be cached${NC}"
    fi
fi

echo ""

# ============================================================================
# Setup Python Environment
# ============================================================================
echo -e "${CYAN}=== Setting up Python Environment ===${NC}"

# Ensure pipx is available and configured
if command -v pipx &> /dev/null; then
    echo -e "${GREEN}[OK] pipx found${NC}"
    pipx ensurepath &> /dev/null || true
    export PATH="$PATH:$HOME/.local/bin"
else
    echo -e "${YELLOW}⚠ pipx not found, will use pip with --user flag${NC}"
fi

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

# Helper function to install Python packages
install_python_tool() {
    local tool_name=$1
    local package_name=$2
    local check_cmd=$3
    
    if eval "$check_cmd" &> /dev/null; then
        echo -e "${GREEN}[OK] $tool_name already installed${NC}"
        return 0
    fi
    
    echo "Installing $tool_name..."
    
    # Try pipx first (preferred for Ubuntu 24.04+)
    if command -v pipx &> /dev/null; then
        if pipx list 2>/dev/null | grep -q "$package_name"; then
            echo -e "${GREEN}[OK] $tool_name already installed via pipx${NC}"
            return 0
        fi
        pipx install "$package_name" &> /dev/null && return 0
    fi
    
    # Fallback to pip with --user
    if $PYTHON_CMD -m pip install --user "$package_name" --quiet 2>/dev/null; then
        return 0
    fi
    
    # Last resort: try with --break-system-packages (not recommended but works)
    echo -e "${YELLOW}⚠ Using --break-system-packages flag${NC}"
    $PYTHON_CMD -m pip install --break-system-packages "$package_name" --quiet
}

# Conan
if ! command -v conan &> /dev/null; then
    echo "Installing Conan..."
    install_python_tool "Conan" "conan" "command -v conan"
    if command -v conan &> /dev/null; then
        conan profile detect --force 2>/dev/null || true
    fi
else
    echo -e "${GREEN}[OK] Conan already installed${NC}"
    conan profile detect --force 2>/dev/null || true
fi

# cpplint
if command -v cpplint &> /dev/null || $PYTHON_CMD -c 'import cpplint' 2>/dev/null; then
    echo -e "${GREEN}[OK] cpplint already installed${NC}"
else
    echo "Installing cpplint..."
    install_python_tool "cpplint" "cpplint" "command -v cpplint"
fi

# cmakelang (cmake-format, cmake-lint)
if command -v cmake-format &> /dev/null || $PYTHON_CMD -c 'import cmakelang' 2>/dev/null; then
    echo -e "${GREEN}[OK] cmakelang already installed${NC}"
else
    echo "Installing cmakelang..."
    install_python_tool "cmakelang" "cmakelang" "command -v cmake-format"
fi

echo ""

# ============================================================================
# Install Just Command Runner
# ============================================================================
echo -e "${CYAN}=== Installing Just Command Runner ===${NC}"

if ! command -v just &> /dev/null; then
    if [[ "$IS_WINDOWS" == "true" ]]; then
        echo "Installing just via Chocolatey..."
        choco install just -y
    elif [[ "$IS_LINUX" == "true" || "$IS_WSL" == "true" ]]; then
        if command -v snap &> /dev/null; then
            echo "Installing just via snap..."
            ${SUDO_CMD:-sudo} snap install just --classic
        elif command -v cargo &> /dev/null; then
            echo "Installing just via cargo..."
            cargo install just
        else
            echo -e "${YELLOW}⚠ Neither snap nor cargo found${NC}"
            echo "Installing just from GitHub releases..."
            JUST_VERSION="1.36.0"
            ARCH=$(uname -m)
            if [[ "$ARCH" == "x86_64" ]]; then
                JUST_ARCH="x86_64"
            elif [[ "$ARCH" == "aarch64" ]]; then
                JUST_ARCH="aarch64"
            else
                echo -e "${RED}[X] Unsupported architecture: $ARCH${NC}"
                echo "Please install just manually from: https://github.com/casey/just"
                JUST_VERSION=""
            fi
            
            if [[ -n "$JUST_VERSION" ]]; then
                JUST_URL="https://github.com/casey/just/releases/download/${JUST_VERSION}/just-${JUST_VERSION}-${JUST_ARCH}-unknown-linux-musl.tar.gz"
                echo "Downloading from: $JUST_URL"
                curl -L "$JUST_URL" | tar xz -C /tmp
                ${SUDO_CMD:-sudo} mv /tmp/just /usr/local/bin/just
                ${SUDO_CMD:-sudo} chmod +x /usr/local/bin/just
            fi
        fi
    fi
else
    echo -e "${GREEN}[OK] just already installed${NC}"
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
        echo -e "${GREEN}[OK]${NC} $tool"
    else
        echo -e "${RED}[X]${NC} $tool - NOT FOUND"
        FAILED=1
    fi
}

check_tool "CMake" "command -v cmake"
check_tool "Conan" "command -v conan"
check_tool "clang-format" "command -v clang-format"
check_tool "clang-tidy" "command -v clang-tidy"
check_tool "cppcheck" "command -v cppcheck"

# Check Python tools - they might be in pipx or pip --user
if command -v cpplint &> /dev/null || $PYTHON_CMD -c 'import cpplint' 2>/dev/null; then
    echo -e "${GREEN}[OK]${NC} cpplint"
else
    echo -e "${RED}[X]${NC} cpplint - NOT FOUND"
    FAILED=1
fi

if command -v cmake-format &> /dev/null || $PYTHON_CMD -c 'import cmakelang' 2>/dev/null; then
    echo -e "${GREEN}[OK]${NC} cmakelang"
else
    echo -e "${RED}[X]${NC} cmakelang - NOT FOUND"
    FAILED=1
fi

check_tool "just" "command -v just"

# Compiler cache (one of the two is enough)
if command -v sccache &> /dev/null; then
    echo -e "${GREEN}[OK]${NC} sccache (compiler cache)"
elif command -v ccache &> /dev/null; then
    echo -e "${GREEN}[OK]${NC} ccache (compiler cache fallback)"
else
    echo -e "${YELLOW}[~]${NC} compiler cache - not installed (optional but recommended)"
    # Not setting FAILED=1 since this is optional
fi

echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}=== [OK] All tools installed successfully! ===${NC}"
    echo ""
    echo "Installed versions:"
    cmake --version 2>/dev/null | head -n1 || echo "cmake: version check failed"
    conan --version 2>/dev/null | head -n1 || echo "conan: version check failed"
    clang-format --version 2>/dev/null | head -n1 || echo "clang-format: version check failed"
    cppcheck --version 2>/dev/null | head -n1 || echo "cppcheck: version check failed"
    echo ""
    echo -e "${YELLOW}Note: You may need to restart your terminal for PATH changes to take effect.${NC}"
else
    echo -e "${RED}=== [X] Some tools failed to install ===${NC}"
    echo "Please check the errors above and try again."
    exit 1
fi