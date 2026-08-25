@echo off
setlocal
cd /d "%~dp0"

echo [BUILD] Setting up environment and compiling SysLifeChecker...

if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%"
)

g++ ui/*.cpp ui/controls/*.cpp ^
    src/WMIHelper.cpp ^
    src/CPU.cpp ^
    src/RAM.cpp ^
    src/GPU.cpp ^
    src/Storage.cpp ^
    src/Battery.cpp ^
    src/Display.cpp ^
    src/Network.cpp ^
    src/Motherboard.cpp ^
    src/BIOS.cpp ^
    src/WindowsInfo.cpp ^
    src/Security.cpp ^
    src/Audio.cpp ^
    src/Camera.cpp ^
    src/Drivers.cpp ^
    src/Chargers.cpp ^
    src/Test.cpp ^
    src/SystemInfo.cpp ^
    -Iinclude ^
    -Isrc ^
    -Iui ^
    -Iui/controls ^
    -IC:/msys64/ucrt64/lib/wx/include/msw-unicode-3.2 ^
    -IC:/msys64/ucrt64/include/wx-3.2 ^
    -DWXUSINGDLL ^
    -D__WXMSW__ ^
    -LC:/msys64/ucrt64/lib ^
    -lwx_mswu_xrc-3.2 ^
    -lwx_mswu_html-3.2 ^
    -lwx_mswu_qa-3.2 ^
    -lwx_mswu_core-3.2 ^
    -lwx_baseu_xml-3.2 ^
    -lwx_baseu_net-3.2 ^
    -lwx_baseu-3.2 ^
    -lole32 ^
    -loleaut32 ^
    -lwbemuuid ^
    -lpsapi ^
    -lsetupapi ^
    -lcfgmgr32 ^
    -lmf ^
    -lmfplat ^
    -lmfreadwrite ^
    -lmfuuid ^
    -ldxgi ^
    -lpdh ^
    -liphlpapi ^
    -lwlanapi ^
    -lbthprops ^
    -lgdi32 ^
    -lwininet ^
    -lhid ^
    -luuid ^
    -mwindows ^
    -o SysLifeChecker.exe

if %errorlevel% equ 0 (
    echo [SUCCESS] SysLifeChecker.exe built successfully!
) else (
    echo [ERROR] Compilation failed!
)

pause