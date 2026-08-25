@echo off
setlocal
cd /d "%~dp0"

echo [BUILD] Compiling SysLifeChecker CLI Diagnostic Tool...

if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%"
)

g++ src/*.cpp ^
    -Iinclude ^
    -Isrc ^
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
    -o SysLifeChecker_CLI.exe

if %errorlevel% equ 0 (
    echo [SUCCESS] SysLifeChecker_CLI.exe built successfully!
    .\SysLifeChecker_CLI.exe
) else (
    echo [ERROR] Compilation failed!
)

pause
