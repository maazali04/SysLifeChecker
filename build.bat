@echo off
:: Navigate to the source folder
cd src

:: Compile all source files with the correct include paths
g++ *.cpp ^
-I ..\include ^
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
-lwininet ^
-o SysLifeChecker.exe

:: Check if the compilation succeeded before running
if %errorlevel% equ 0 (
    echo [SUCCESS] Compilation finished. Starting program...
    .\SysLifeChecker.exe
) else (
    echo [ERROR] Compilation failed! Please check the errors above.
)

:: Pause the window so it doesn't close instantly if there is an error
pause
