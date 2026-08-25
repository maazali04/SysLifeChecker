#!/usr/bin/env bash

cd "$(dirname "$0")"

echo "[BUILD] Compiling SysLifeChecker (GUI & Hardware Engine)..."

g++ ui/*.cpp ui/controls/*.cpp \
    src/WMIHelper.cpp \
    src/CPU.cpp \
    src/RAM.cpp \
    src/GPU.cpp \
    src/Storage.cpp \
    src/Battery.cpp \
    src/Display.cpp \
    src/Network.cpp \
    src/Motherboard.cpp \
    src/BIOS.cpp \
    src/WindowsInfo.cpp \
    src/Security.cpp \
    src/Audio.cpp \
    src/Camera.cpp \
    src/Drivers.cpp \
    src/Chargers.cpp \
    src/Test.cpp \
    src/SystemInfo.cpp \
    -Iinclude \
    -Isrc \
    -Iui \
    -Iui/controls \
    $(wx-config --cxxflags) \
    -lole32 \
    -loleaut32 \
    -lwbemuuid \
    -lpsapi \
    -lsetupapi \
    -lcfgmgr32 \
    -lmf \
    -lmfplat \
    -lmfreadwrite \
    -lmfuuid \
    -ldxgi \
    -lpdh \
    -liphlpapi \
    -lwlanapi \
    -lbthprops \
    -lgdi32 \
    -lwininet \
    -lhid \
    -luuid \
    -o SysLifeChecker.exe \
    $(wx-config --libs)

if [ $? -ne 0 ]; then
    echo
    echo "[ERROR] Build failed."
    exit 1
fi

echo
echo "[SUCCESS] Build completed successfully: SysLifeChecker.exe"