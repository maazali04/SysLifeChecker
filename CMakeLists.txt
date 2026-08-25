cmake_minimum_required(VERSION 3.16)
project(SysLifeChecker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files are saved as UTF-8 (they contain °, ©, etc. as literal or
# escaped characters). Without this, MSVC assumes the system codepage and
# those characters come out corrupted (mojibake) in the built app.
if (MSVC)
    add_compile_options(/utf-8)
endif()

include_directories(
    include
    src
    ui
    ui/controls
)

# Backend source files
set(BACKEND_SOURCES
    src/WMIHelper.cpp
    src/CPU.cpp
    src/RAM.cpp
    src/GPU.cpp
    src/Storage.cpp
    src/Battery.cpp
    src/Display.cpp
    src/Network.cpp
    src/Motherboard.cpp
    src/BIOS.cpp
    src/WindowsInfo.cpp
    src/Security.cpp
    src/Audio.cpp
    src/Camera.cpp
    src/Drivers.cpp
    src/Chargers.cpp
    src/Test.cpp
    src/SystemInfo.cpp
)

# GUI source files
set(GUI_SOURCES
    ui/App.cpp
    ui/MainFrame.cpp
    ui/DashboardPanel.cpp
    ui/ReportPanel.cpp
    ui/TestPanel.cpp
    ui/SettingsDialog.cpp
    ui/Sidebar.cpp
    ui/MenuBar.cpp
    ui/Theme.cpp
    ui/FontManager.cpp
    ui/SystemMonitorService.cpp
    ui/controls/CardPanel.cpp
    ui/controls/SidebarItem.cpp
)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Main GUI Executable
add_executable(SysLifeChecker WIN32
    ${BACKEND_SOURCES}
    ${GUI_SOURCES}
)

target_link_libraries(SysLifeChecker PRIVATE
    ${wxWidgets_LIBRARIES}
    ole32
    oleaut32
    wbemuuid
    psapi
    setupapi
    cfgmgr32
    mf
    mfplat
    mfreadwrite
    mfuuid
    dxgi
    pdh
    iphlpapi
    wlanapi
    bthprops
    gdi32
    wininet
    hid
    uuid
)

# CLI / Diagnostic Console Tool
add_executable(SysLifeChecker_CLI
    ${BACKEND_SOURCES}
    src/main.cpp
)

target_link_libraries(SysLifeChecker_CLI PRIVATE
    ole32
    oleaut32
    wbemuuid
    psapi
    setupapi
    cfgmgr32
    mf
    mfplat
    mfreadwrite
    mfuuid
    dxgi
    pdh
    iphlpapi
    wlanapi
    bthprops
    gdi32
    wininet
    hid
    uuid
)
