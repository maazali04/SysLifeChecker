#!/usr/bin/env bash

cd "$(dirname "$0")"

g++ ui/*.cpp ui/controls/*.cpp \
    -Iui \
    -Iui/controls \
    $(wx-config --cxxflags) \
    -o SysLifeChecker.exe \
    $(wx-config --libs)

if [ $? -ne 0 ]; then
    echo
    echo "[ERROR] Build failed."
    read -p "Press Enter to continue..."
    exit 1
fi

echo
echo "[SUCCESS] Build completed."

./SysLifeChecker.exe

echo
echo "Program exited with code $?"

read -p "Press Enter to continue..."