#!/bin/bash

set -e

if [ $# -ne 2 ]; then
    echo "Usage:"
    echo "  $0 <OLD_PROJECT> <NEW_PROJECT>"
    echo
    echo "Example:"
    echo "  $0 UART_Interrupt UART_DMA"
    exit 1
fi

OLD=$1
NEW=$2

if [ ! -d "$OLD" ]; then
    echo "Project '$OLD' not found."
    exit 1
fi

if [ -d "$NEW" ]; then
    echo "Project '$NEW' already exists."
    exit 1
fi

echo "======================================"
echo " Cloning STM32 Project"
echo "======================================"

echo "[1/8] Copying project..."
cp -r "$OLD" "$NEW"

echo "[2/8] Removing build folders..."
rm -rf "$NEW/Debug"
rm -rf "$NEW/Release"

echo "[3/8] Removing CMake generated files..."
find "$NEW" -name CMakeCache.txt -delete
find "$NEW" -name build.ninja -delete
find "$NEW" -name rules.ninja -delete
find "$NEW" -name TargetDirectories.txt -delete
find "$NEW" -name "*.map" -delete
find "$NEW" -name ".ninja_*" -delete

echo "[4/8] Renaming launch/config files..."

if [ -f "$NEW/${OLD} Debug.launch" ]; then
    mv "$NEW/${OLD} Debug.launch" \
       "$NEW/${NEW} Debug.launch"
fi

if [ -f "$NEW/${OLD} Debug.cfg" ]; then
    mv "$NEW/${OLD} Debug.cfg" \
       "$NEW/${NEW} Debug.cfg"
fi

echo "[5/8] Removing obsolete cfg..."

find "$NEW" -maxdepth 1 -name "*${OLD,,}*.cfg" -delete

echo "[6/8] Updating project metadata..."

find "$NEW" -type f \
\( \
-name "CMakeLists.txt" -o \
-name ".project" -o \
-name ".cproject" -o \
-name "*.launch" -o \
-name "*.cfg" \
\) \
-exec sed -i \
-e "s/${OLD}/${NEW}/g" \
-e "s/${OLD,,}/${NEW,,}/g" \
{} \;

echo "[7/8] Regenerating CMake..."

cmake \
-DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake \
-S "$NEW" \
-B "$NEW/Debug" \
-G "Unix Makefiles" \
-DCMAKE_BUILD_TYPE=Debug

echo "[8/8] Building..."

cmake --build "$NEW/Debug" -j

echo
echo "======================================"
echo " Project Ready"
echo "======================================"
echo
echo "Project : $NEW"
echo
echo "ELF:"
echo "  $NEW/Debug/${NEW,,}.elf"
echo
echo "Open in STM32CubeIDE and Flash."

