#include "SystemInfo.hpp"
#include "Test.hpp"
#include "WMIHelper.hpp"

#include <iostream>

int main()
{
    if (!InitializeWMI())
    {
        std::cout << "Failed to initialize WMI\n";
        return 1;
    }

    SystemInfo pc = GetSystemInfo();

    // TestCPU(pc.CPU);
    // TestRAM(pc.RAM);
    // TestStorage(pc.Storage);
    // TestGPU(pc.GPUs);
//     for (const auto& battery : pc.Batteries)
// {
//     TestBattery(battery);
// }
    // TestNetwork(pc.Network);
    // TestDisplay(pc.Displays);
    TestWindows(pc.Windows);
    // TestSecurity(pc.Security);
    // TestPorts(pc.Ports);
    // TestAudio(pc.Audio);
    // TestCamera(pc.Cameras);
    // TestDrivers(pc.Drivers);
    // TestCharger(pc.Chargers);

    ShutdownWMI();

    return 0;
}