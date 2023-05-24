#!/bin/bash

# cat /etc/os-release - outputs information about the distribution
# grep - filter strings that contains '^ID='
# awk - divide value into 2 fields before = and after =
# {print $2} select second value from awk
# tr -d '"' remove quotation marks, if exists
distro=$(cat /etc/os-release | grep '^ID=' | awk -F'=' '{print $2}' | tr -d '"')

# The -y flag indicates tha packages are installed automatically,
# withut confirmation
if [[ $distro == "ubuntu" || $distro == "debian" || $distro == "linuxmint" ]]; then
    sudo apt-get update
    sudo apt-get install -y libgmp-dev g++ cmake
    
elif [[ $distro == "centos" || $distro == "rhel" ]]; then
    # epel-release - The EPEL (Extra Packages for Enterprise Linux),
    # which provides additional packages for these distributions.
    sudo yum install -y epel-release
    sudo yum install -y gmp-devel gcc-c++ cmake

elif [[ $distro == "fedora" ]]; then
    sudo dnf install -y gmp-devel gcc-c++ cmake
    
elif [[ $distro == "arch" ]]; then
    # -S - install the package
    # --noconfirm - No need to confirm each package
    sudo pacman -Sy --noconfirm gmp gcc cmake

else
    echo "Unsupported Linux distribution: $distro"
    exit 1
fi

echo "Installation of GMP, g++ and CMake completed successfully."

cd ..
# if directory exists, do not create it
mkdir -p build
cd build

echo "Build the project."
cmake ..
make
