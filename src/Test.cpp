#include "RAM.hpp"
#include "Test.hpp"

#include <iostream>

using std::cout;
using std::endl;

void TestCPU(const CPUInfo &cpu)
{
    cout << "\n========================================\n";
    cout << "CPU INFORMATION\n";
    cout << "========================================\n\n";

    //------------------------------
    // Identity
    //------------------------------

    cout << "Name : " << cpu.Name << endl;
    cout << "Brand String : " << cpu.BrandString << endl;
    cout << "Manufacturer : " << cpu.Manufacturer << endl;
    cout << "Codename : " << cpu.Codename << endl;
    cout << "Microarchitecture : " << cpu.Microarchitecture << endl;
    cout << "Socket : " << cpu.Socket << endl;

    cout << "Processor ID : " << cpu.ProcessorID << endl;

    cout << "Family : " << cpu.Family << endl;
    cout << "Model : " << cpu.Model << endl;
    cout << "Stepping : " << cpu.Stepping << endl;

    cout << "Extended Family : " << cpu.ExtendedFamily << endl;
    cout << "Extended Model : " << cpu.ExtendedModel << endl;

    cout << "Maximum CPUID Leaf : " << cpu.MaximumCPUIDLeaf << endl;
    cout << "Maximum Extended Leaf : " << cpu.MaximumExtendedLeaf << endl;

    cout << "Initial APIC ID : " << cpu.InitialAPICID << endl;
    cout << "Logical Per Package : " << cpu.LogicalProcessorsPerPackage << endl;

    cout << "HyperThreading : " << cpu.HyperThreadingSupported << endl;
    cout << "x2APIC : " << cpu.x2APICSupported << endl;

    //------------------------------
    // Topology
    //------------------------------

    cout << "\n------ Topology ------\n";

    cout << "Packages : " << cpu.PhysicalPackages << endl;
    cout << "Physical Cores : " << cpu.PhysicalCores << endl;
    cout << "Logical Processors : " << cpu.LogicalProcessors << endl;
    cout << "Performance Cores : " << cpu.PerformanceCores << endl;
    cout << "Efficiency Cores : " << cpu.EfficiencyCores << endl;
    cout << "NUMA Nodes : " << cpu.NUMANodes << endl;
    cout << "Processor Groups : " << cpu.ProcessorGroups << endl;

    //------------------------------
    // Manufacturing
    //------------------------------

    cout << "\n------ Manufacturing ------\n";

    cout << "Generation : " << cpu.Generation << endl;
    //------------------------------
    // Frequency
    //------------------------------

    cout << "\n------ Frequency ------\n";

    cout << "Base Clock : " << cpu.Frequency.BaseClockGHz << " GHz" << endl;
    cout << "Boost Clock : " << cpu.Frequency.BoostClockGHz << " GHz" << endl;
    cout << "Current Clock : " << cpu.Frequency.CurrentClockGHz << " GHz" << endl;

    //------------------------------
    // Cache
    //------------------------------

    cout << "\n------ Cache ------\n";

    cout << "L1 Instruction : " << cpu.Cache.L1InstructionKB << " KB" << endl;
    cout << "L1 Data : " << cpu.Cache.L1DataKB << " KB" << endl;
    cout << "L2 : " << cpu.Cache.L2KB << " KB" << endl;
    cout << "L3 : " << cpu.Cache.L3KB << " KB" << endl;
    cout << "Cache Line : " << cpu.Cache.CacheLineSize << " Bytes" << endl;

    //------------------------------
    // Runtime
    //------------------------------

    cout << "\n------ Runtime ------\n";

    cout << "CPU Usage : " << cpu.Status.UsagePercent << "%" << endl;

    //------------------------------
    // Instruction Sets
    //------------------------------

    cout << "\n------ Instruction Sets ------\n";

    for (const auto &instruction : cpu.InstructionSets)
    {
        cout << instruction << endl;
    }

    //------------------------------
    // Features
    //------------------------------

    cout << "\n------ CPU Features ------\n";

#define PRINT_FEATURE(x) \
    cout << #x << " : " << cpu.Features.x << endl;

    PRINT_FEATURE(MMX)
    PRINT_FEATURE(SSE)
    PRINT_FEATURE(SSE2)
    PRINT_FEATURE(SSE3)
    PRINT_FEATURE(SSSE3)
    PRINT_FEATURE(SSE41)
    PRINT_FEATURE(SSE42)

    PRINT_FEATURE(AVX)
    PRINT_FEATURE(AVX2)
    PRINT_FEATURE(AVX512)

    PRINT_FEATURE(FMA)
    PRINT_FEATURE(AES)
    PRINT_FEATURE(SHA)

    PRINT_FEATURE(BMI1)
    PRINT_FEATURE(BMI2)

    PRINT_FEATURE(HTT)
    PRINT_FEATURE(SMT)

    PRINT_FEATURE(Virtualization)
    PRINT_FEATURE(VT_x)
    PRINT_FEATURE(AMD_V)
    PRINT_FEATURE(VT_d)

#undef PRINT_FEATURE
}




void TestRAM(const RAMInfo &ram)
{

    std::cout << "\n========================================\n";
    std::cout << "RAM INFORMATION\n";
    std::cout << "========================================\n\n";

    // ---------- Overall ----------

    std::cout << "Channel Mode : " << static_cast<int>(ram.ChannelMode) << '\n';
    std::cout << "RAM Type : ";

if (ram.Type == RAMType::Unknown)
    std::cout << "Not Reported\n";
else
    std::cout << RAMTypeToString(ram.Type) << '\n';

    std::cout << "Total Capacity : "
              << ram.TotalCapacityGB
              << " GB\n";

    std::cout << "Installed Modules : "
              << ram.InstalledModules
              << '\n';

    std::cout << "Total Slots : "
              << ram.TotalSlots
              << '\n';

    std::cout << "Occupied Slots : "
              << ram.OccupiedSlots
              << '\n';

    std::cout << "Empty Slots : "
              << ram.EmptySlots
              << '\n';

    std::cout << "Speed : "
              << ram.SpeedMHz
              << " MHz\n";

    std::cout << "ECC Supported : "
              << ram.ECCSupported
              << '\n';

    std::cout << "ECC Enabled : "
              << ram.ECCEnabled
              << '\n';

    std::cout << "Upgradeable : "
              << ram.Upgradeable
              << '\n';

    std::cout << "Maximum Capacity : "
              << ram.MaximumSupportedCapacityGB
              << " GB\n";

    std::cout << "Memory Channels : "
              << ram.MemoryChannels
              << '\n';

    std::cout << "Visible Memory : "
              << ram.TotalVisibleMemoryBytes
              << '\n';

    std::cout << "Hardware Reserved : "
              << ram.HardwareReservedBytes
              << '\n';

    std::cout << "Virtual Memory : "
              << ram.TotalVirtualMemoryBytes
              << '\n';

    std::cout << "Available Virtual : "
              << ram.AvailableVirtualMemoryBytes
              << '\n';

    std::cout << "Usage : "
              << ram.UsagePercent
              << "%\n";

    std::cout << "Used Memory : "
              << ram.UsedMemoryBytes
              << '\n';

    std::cout << "Available Memory : "
              << ram.AvailableMemoryBytes
              << '\n';

    std::cout << "Cached Memory : "
              << ram.CachedMemoryBytes
              << '\n';

    std::cout << "Free Memory : "
              << ram.FreeMemoryBytes
              << '\n';

    std::cout << "Committed Memory : "
              << ram.CommittedMemoryBytes
              << '\n';

    // ---------- Modules ----------

    for (size_t i = 0; i < ram.Modules.size(); ++i)
    {
        const RAMModule &module = ram.Modules[i];

        std::cout << "\n----------------------------------------\n";
        std::cout << "RAM MODULE "
                  << i + 1
                  << '\n';
        std::cout << "----------------------------------------\n";

        std::cout << "Manufacturer : " << module.Manufacturer << '\n';
        std::cout << "Part Number : " << module.PartNumber << '\n';
        std::cout << "Serial Number : " << module.SerialNumber << '\n';

        std::cout << "Device Locator : " << module.DeviceLocator << '\n';
        std::cout << "Bank Label : " << module.BankLabel << '\n';

        if (module.DataWidth == 0)
            std::cout << "Data Width : Not Reported\n";
        else
            std::cout << "Data Width : " << module.DataWidth << " bits\n";

        if (module.TotalWidth == 0)
            std::cout << "Total Width : Not Reported\n";
        else
            std::cout << "Total Width : " << module.TotalWidth << " bits\n";

        std::cout << "RAM Type : ";

        if (module.Type == RAMType::Unknown)
            std::cout << "Not Reported\n";
        else
            std::cout << RAMTypeToString(module.Type) << '\n';

        std::cout << "Form Factor : ";

        if (module.FormFactor == RAMFormFactor::Unknown)
            std::cout << "Not Reported\n";
        else
            std::cout << RAMFormFactorToString(module.FormFactor) << '\n';
        std::cout << std::endl;

        std::cout << "Slot Name : " << module.SlotName << '\n';
        std::cout << "Slot Number : " << module.SlotNumber << '\n';

        std::cout << "Installed : " << module.Installed << '\n';
        std::cout << "Removable : " << module.Removable << '\n';

        std::cout << "Capacity : "
                  << module.CapacityGB
                  << " GB\n";

        std::cout << "Configured Speed : "
                  << module.ConfiguredSpeedMHz
                  << " MHz\n";

        std::cout << "Maximum Speed : "
                  << module.MaximumSpeedMHz
                  << " MHz\n";

        std::cout << "Voltage : "
                  << module.Voltage
                  << " V\n";

        std::cout << "Minimum Voltage : "
                  << module.MinimumVoltage
                  << " V\n";

        std::cout << "Maximum Voltage : "
                  << module.MaximumVoltage
                  << " V\n";

        std::cout << "ECC : " << module.ECC << '\n';
        std::cout << "Registered : " << module.Registered << '\n';
        std::cout << "Buffered : " << module.Buffered << '\n';

        if (module.MemoryTechnology.empty())
            std::cout << "Memory Technology : Not Reported\n";
        else
            std::cout << "Memory Technology : " << module.MemoryTechnology << '\n';

        std::cout << "Rank : "
                  << module.Rank
                  << '\n';
    }
}