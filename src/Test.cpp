#include "RAM.hpp"
#include "Test.hpp"
#include "Storage.hpp"

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

void TestStorage(const StorageInfo &storage)
{
    std::cout << "\n========================================\n";
    std::cout << "STORAGE INFORMATION\n";
    std::cout << "========================================\n\n";

    std::cout << "Total Drives : " << storage.TotalDrives << '\n';
    std::cout << "HDD Count : " << storage.HDDCount << '\n';
    std::cout << "SSD Count : " << storage.SSDCount << '\n';
    std::cout << "NVMe Count : " << storage.NVMeCount << '\n';

    std::cout << "Total Capacity : " << storage.TotalCapacityGB << " GB\n";
    std::cout << "Used Space : " << storage.UsedSpaceGB << " GB\n";
    std::cout << "Free Space : " << storage.FreeSpaceGB << " GB\n";
    std::cout << "Usage : " << storage.UsagePercent << "%\n";

    for (size_t i = 0; i < storage.Drives.size(); i++)
    {
        const StorageDevice &drive = storage.Drives[i];

        std::cout << "\n----------------------------------------\n";
        std::cout << "DRIVE " << i + 1 << '\n';
        std::cout << "----------------------------------------\n";

        std::cout << "Model : " << drive.Model << '\n';
        std::cout << "Manufacturer : " << drive.Manufacturer << '\n';
        std::cout << "Serial Number : " << drive.SerialNumber << '\n';
        std::cout << "Firmware Version : " << drive.FirmwareVersion << '\n';
        std::cout << "Device ID : " << drive.DeviceID << '\n';

        std::cout << "Media Type : " << drive.MediaType << '\n';
        std::cout << "PNP Device ID : " << drive.PNPDeviceID << '\n';
        std::cout << "Status : " << drive.Status << '\n';
        std::cout << "Install Date : " << drive.InstallDate << '\n';
        std::cout << "Name : " << drive.Name << '\n';
        std::cout << "Description : " << drive.Description << '\n';

        std::cout << "Bytes Per Sector : " << drive.BytesPerSector << '\n';

        std::cout << "Health Status : " << drive.HealthStatus << '\n';
        std::cout << "Recommendation : " << drive.Recommendation << '\n';
        std::cout << "Is Failing : " << drive.IsFailing << '\n';

        std::cout << "Media Loaded : " << drive.MediaLoaded << '\n';
        std::cout << "Logical Sector Size : " << drive.LogicalSectorSize << '\n';
        std::cout << "Physical Sector Size : " << drive.PhysicalSectorSize << '\n';

        std::cout << "Drive Letter : " << drive.DriveLetter << '\n';
        std::cout << "Volume Name : " << drive.VolumeName << '\n';
        std::cout << "File System : " << drive.FileSystem << '\n';

        std::cout << "GPT : " << drive.GPT << '\n';
        std::cout << "MBR : " << drive.MBR << '\n';

        std::cout << "Type : "
                  << StorageDeviceTypeToString(drive.Type)
                  << '\n';

        std::cout << "Bus : "
                  << StorageBusTypeToString(drive.Bus)
                  << '\n';

        std::cout << "Interface : "
                  << drive.Interface
                  << '\n';
        std::cout << "Protocol : " << drive.Protocol << '\n';

        std::cout << "Capacity : " << drive.CapacityGB << " GB\n";

        std::cout << "RPM : " << drive.RPM << '\n';

        std::cout << "TRIM Supported : " << drive.TRIMSupported << '\n';
        std::cout << "TRIM Enabled : " << drive.TRIMEnabled << '\n';
        std::cout << "NCQ Supported : " << drive.NCQSupported << '\n';

        std::cout << "Removable : " << drive.Removable << '\n';
        std::cout << "Boot Drive : " << drive.BootDrive << '\n';
        std::cout << "System Drive : " << drive.SystemDrive << '\n';

        std::cout << "\n------ SMART ------\n";

        const SMARTInfo &smart = drive.SMART;

        std::cout << "Supported : " << smart.Supported << '\n';
        std::cout << "Enabled : " << smart.Enabled << '\n';
        std::cout << "Predict Failure : " << smart.PredictFailure << '\n';

        std::cout << "Health : " << smart.HealthPercent << "%\n";

        std::cout << "Temperature : " << smart.Temperature << " C\n";
        std::cout << "Maximum Temperature : " << smart.MaxTemperature << " C\n";

        std::cout << "Power On Hours : " << smart.PowerOnHours << '\n';
        std::cout << "Power Cycles : " << smart.PowerCycles << '\n';

        std::cout << "Estimated Age : " << smart.EstimatedAgeYears << " Years\n";
        std::cout << "Estimated Remaining : " << smart.EstimatedRemainingYears << " Years\n";

        std::cout << "Percentage Used : " << smart.PercentageUsed << '\n';
        std::cout << "Percentage Remaining : " << smart.PercentageRemaining << '\n';

        std::cout << "Available Spare : " << smart.AvailableSpare << '\n';
        std::cout << "Spare Threshold : " << smart.SpareThreshold << '\n';

        std::cout << "Unsafe Shutdowns : " << smart.UnsafeShutdowns << '\n';
        std::cout << "Controller Busy Minutes : " << smart.ControllerBusyMinutes << '\n';

        std::cout << "Host Reads : " << smart.HostReadsGB << " GB\n";
        std::cout << "Host Writes : " << smart.HostWritesGB << " GB\n";

        std::cout << "Data Read : " << smart.DataReadGB << " GB\n";
        std::cout << "Data Written : " << smart.DataWrittenGB << " GB\n";

        std::cout << "Media Errors : " << smart.MediaErrors << '\n';
        std::cout << "Error Log Entries : " << smart.ErrorLogEntries << '\n';

        std::cout << "Reallocated Sectors : " << smart.ReallocatedSectors << '\n';
        std::cout << "Pending Sectors : " << smart.PendingSectors << '\n';
        std::cout << "Uncorrectable Errors : " << smart.UncorrectableErrors << '\n';

        std::cout << "CRC Errors : " << smart.CRCErrors << '\n';
        std::cout << "Seek Error Rate : " << smart.SeekErrorRate << '\n';
        std::cout << "Spin Retry Count : " << smart.SpinRetryCount << '\n';
        std::cout << "Start Stop Count : " << smart.StartStopCount << '\n';
        std::cout << "Load Unload Cycles : " << smart.LoadUnloadCycles << '\n';

        for (size_t j = 0; j < drive.Partitions.size(); j++)
        {
            const PartitionInfo &part = drive.Partitions[j];

            std::cout << "\n------ Partition " << j + 1 << " ------\n";

            std::cout << "Drive Letter : " << part.DriveLetter << '\n';
            std::cout << "Volume Name : " << part.VolumeName << '\n';
            std::cout << "File System : " << part.FileSystem << '\n';

            std::cout << "Partition Style : "
          << PartitionStyleToString(part.Style)
          << '\n';

            std::cout << "Total : "
                      << part.TotalBytes / (1024.0 * 1024 * 1024)
                      << " GB\n";

            std::cout << "Used : "
                      << part.UsedBytes / (1024.0 * 1024 * 1024)
                      << " GB\n";

            std::cout << "Free : "
                      << part.FreeBytes / (1024.0 * 1024 * 1024)
                      << " GB\n";

            std::cout << "Boot Partition : " << part.BootPartition << '\n';
            std::cout << "System Partition : " << part.SystemPartition << '\n';
            std::cout << "Hidden : " << part.Hidden << '\n';
            std::cout << "BitLocker : " << part.BitLockerEncrypted << '\n';
        }
    }
}
