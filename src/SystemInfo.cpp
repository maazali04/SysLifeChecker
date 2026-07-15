#include "SystemInfo.hpp"

#include "CPU.hpp"
#include "RAM.hpp"
#include "GPU.hpp"
#include "Storage.hpp"
#include "Battery.hpp"  
#include "Display.hpp"  
#include "Network.hpp"   
#include "Motherboard.hpp"
#include "WindowsInfo.hpp" 
#include "Security.hpp"  
#include "Ports.hpp"     
#include "Audio.hpp"     
#include "Camera.hpp"   
#include "Drivers.hpp"   
#include "Chargers.hpp"  
 
SystemInfo GetSystemInfo()
{
    SystemInfo info;
    info.CPU = GetCPUInfo();
    info.RAM = GetRAMInfo();
    info.GPUs = GetGPUInfo();
    info.Storage = GetStorageInfo();
    info.Batteries = GetBatteryInfo();
    info.Displays = GetDisplayInfo();
    info.Network = GetNetworkInfo();
    info.Motherboard = GetMotherboardInfo();
    info.Windows = GetWindowsInfo();
    info.Security = GetSecurityInfo();
    info.Ports = GetPortInfo();
    info.Audio = GetAudioInfo();
    info.Cameras = GetCameraInfo();
    info.Drivers = GetDriverInfo();
    info.Chargers = GetChargerInfo();
    return info;
}


