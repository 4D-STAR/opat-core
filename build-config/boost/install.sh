#!/usr/bin/env bash
#
# install.sh - Interactive installation script for Boost.
#
# This script checks if Boost is installed (by looking for boost/version.hpp in common locations).
# If not found, it prompts the user to install Boost using the native package manager for:
#   - FreeBSD, Ubuntu, Debian, Fedora, Arch, Mint, Manjaro, macOS, OpenSuse, and Nix.
#
# All output is colorized for clarity and logged to meson-log.txt.

set -e

# ANSI color codes.
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
NC="\033[0m"  # No Color

# Log file.
LOGFILE="4DSSE-install-log.txt"
touch "$LOGFILE"


# Log function: prints to stdout and appends to logfile.
log() {
  local message="$1"
  # Print the colored message to stdout.
  echo -e "$message"
  # Strip ANSI escape sequences and append the cleaned message to the log file.
  echo -e "$message" | sed -r 's/\x1B\[[0-9;]*[mK]//g' >> "$LOGFILE"
}



check_boost_installed() {
  log "${BLUE}[Info] Checking for Boost::numeric and Boost::phoenix using Meson...${NC}"
  local tmpDir
  if [[ -d mesonTest ]]; then
    rm -rf mesonTest
  fi
  tmpDir=$(mkdir mesonTest)
  local sourceFile="mesonTest/test.cpp"
  local mesonFile="mesonTest/meson.build"

  # Write test.cpp
  cat > "$sourceFile" <<EOF
#include <boost/numeric/ublas/vector.hpp>
#include <boost/phoenix/phoenix.hpp>

int main() {
  boost::numeric::ublas::vector<double> v(3);
  v[0] = 1.0;
  return v[0];
}
EOF

  # Write meson.build
  cat > "$mesonFile" <<EOF
project('boost-check', 'cpp', default_options: ['cpp_std=c++17'])

boost_dep = dependency('boost', required: true)

executable('boostAvaliTest', 'test.cpp', dependencies: [boost_dep])
EOF

  # Try configuring and compiling
  if meson setup "mesonTest/build" "mesonTest"  && \
     meson compile -C "mesonTest/build"; then
    log "${GREEN}[Success] Boost::numeric and Boost::phoenix are available.${NC}"
    rm -rf "mesonTest"
    return 0
  else
    log "${RED}[Error] Boost could not be found or required components are missing.${NC}"
    rm -rf "mesonTest"
    return 1
  fi
}



# Prompt the user for a yes/no answer.
prompt_yes_no() {
  local promptMsg="$1"
  read -p "$(echo -e ${YELLOW}$promptMsg${NC}) " answer
  if [[ "$answer" =~ ^[Yy]$ ]]; then
    return 0
  else
    return 1
  fi
}

# Detect OS.
OS=$(uname -s)
DISTRO="unknown"
if [ -f /etc/os-release ]; then
  # shellcheck disable=SC1091
  . /etc/os-release
  DISTRO=$ID
fi

if [[ "$OS" == "Darwin" ]]; then
  OS="macOS"
fi

if [[ $OS == "macOS" ]]; then
    log "${BLUE}[Info] Detected OS: ${OS}${NC}"
else
    log "${BLUE}[Info] Detected OS: ${OS}  Distribution: ${DISTRO}${NC}"
fi

# Check if Boost is already installed.
if check_boost_installed; then
  log "${GREEN}[Success] Boost is already installed on your system.${NC}"
  log "${GREEN}[Success] 4DSSE installation can now continue.${NC}"
  exit 0
else
  log "${YELLOW}[Info] Boost was not detected on your system.${NC}"
  if prompt_yes_no "[Query] Would you like to install Boost now? (y/n): "; then

    case "$OS" in
      "macOS")
        # On macOS, check if Homebrew is available.
        if ! command -v brew &> /dev/null; then
          log "${YELLOW}[Warning] Homebrew is not installed. It is recommended for installing Boost on macOS.${NC}"
          if prompt_yes_no "[Query] Would you like to install Homebrew now? (y/n): "; then
            log "${BLUE}[Info] Installing Homebrew...${NC}"
            /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" | tee -a "$LOGFILE"
          else
            log "${RED}[Error] Homebrew is required for an easy Boost installation on macOS. Aborting.${NC}"
            exit 1
          fi
        fi
        log "${BLUE}[Info] Running: brew install boost${NC}"
        brew install boost | tee -a "$LOGFILE"
        ;;
      "Linux")
        case "$DISTRO" in
          "ubuntu"|"debian"|"linuxmint")
            log "${BLUE}[Info] Running: sudo apt-get update && sudo apt-get install -y libboost-all-dev${NC}"
            sudo apt-get update | tee -a "$LOGFILE"
            sudo apt-get install -y libboost-all-dev | tee -a "$LOGFILE"
            ;;
          "fedora")
            log "${BLUE}[Info] Running: sudo dnf install boost-devel${NC}"
            sudo dnf install -y boost-devel | tee -a "$LOGFILE"
            ;;
          "arch"|"manjaro")
            log "${BLUE}[Info] Running: sudo pacman -S boost${NC}"
            sudo pacman -S --noconfirm boost | tee -a "$LOGFILE"
            ;;
          "opensuse")
            log "${BLUE}[Info] Running: sudo zypper install libboost-devel${NC}"
            sudo zypper install -y libboost-devel | tee -a "$LOGFILE"
            ;;
          "nixos"|"nix")
            log "${BLUE}[Info] Running: nix-env -iA nixpkgs.boost${NC}"
            nix-env -iA nixpkgs.boost | tee -a "$LOGFILE"
            ;;
          *)
            log "${RED}[Error] Your Linux distribution is not recognized. Please install Boost manually.${NC}"
            exit 1
            ;;
        esac
        ;;
      "FreeBSD")
        log "${BLUE}[Info] Running: sudo pkg install boost-all${NC}"
        sudo pkg install -y boost-all | tee -a "$LOGFILE"
        ;;
      *)
        log "${RED}[Error] Automatic Boost installation is not supported on OS: ${OS}. Please install Boost manually.${NC}"
        exit 1
        ;;
    esac

    # Verify Boost installation.
    if check_boost_installed; then
      log "${GREEN}[Success] Boost installation succeeded.${NC}"
    else
      log "${RED}[Error] Boost installation appears to have failed. Please install Boost manually.${NC}"
      exit 1
    fi

  else
    log "${RED}[Error] Boost is required. Please install it using the appropriate command for your system:${NC}"
    case "$OS" in
      "macOS")
        log "${RED}[INFO] brew install boost (Install Homebrew from https://brew.sh if not present)${NC}"
        ;;
      "Linux")
        case "$DISTRO" in
          "ubuntu"|"debian"|"linuxmint")
            log "${RED}[INFO] sudo apt-get install libboost-all-dev${NC}"
            ;;
          "fedora")
            log "${RED}[INFO] sudo dnf install boost-devel${NC}"
            ;;
          "arch"|"manjaro")
            log "${RED}[INFO] sudo pacman -S boost${NC}"
            ;;
          "opensuse")
            log "${RED}[INFO] sudo zypper install libboost-devel${NC}"
            ;;
          "nixos"|"nix")
            log "${RED}[INFO] nix-env -iA nixpkgs.boost${NC}"
            ;;
          *)
            log "${RED}[INFO] Please consult your distribution's documentation for installing Boost.${NC}"
            ;;
        esac
        ;;
      "FreeBSD")
        log "${RED}[INFO] sudo pkg install boost-all${NC}"
        ;;
      *)
        log "${RED}[INFO] Please consult your operating system's documentation for installing Boost.${NC}"
        ;;
    esac
    exit 1
  fi
fi

check_boost_installed

log "${GREEN}[Success] Boost installed successfully!${NC}"
log "${GREEN}[Success] 4DSSE installation can now continue.${NC}"
