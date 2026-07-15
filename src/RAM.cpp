#include "RAM.hpp"
#include "WMIHelper.hpp"
#include <windows.h>
#include <psapi.h>  
#pragma comment(lib, "psapi.lib")

static void FillOverallRAMInfo(RAMInfo &ram)
{
    MEMORYSTATUSEX memoryStatus{};
    memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (!GlobalMemoryStatusEx(&memoryStatus))
        return;

    // Total Installed / Visible RAM
    ram.TotalCapacityBytes = memoryStatus.ullTotalPhys;
    ram.TotalCapacityGB =
        static_cast<double>(ram.TotalCapacityBytes) /
        (1024.0 * 1024.0 * 1024.0);

    // Available & Used
    ram.AvailableMemoryBytes = memoryStatus.ullAvailPhys;
    ram.UsedMemoryBytes =
        ram.TotalCapacityBytes - ram.AvailableMemoryBytes;

    // Usage %
    ram.UsagePercent =
        (static_cast<double>(ram.UsedMemoryBytes) * 100.0) /
        static_cast<double>(ram.TotalCapacityBytes);

    // Virtual Memory
    ram.TotalVirtualMemoryBytes = memoryStatus.ullTotalVirtual;
    ram.AvailableVirtualMemoryBytes = memoryStatus.ullAvailVirtual;

    // Hardware Reserved (will be updated later if available)
    ram.HardwareReservedBytes = 0;
}

static void FillMemoryModuleInfo(RAMInfo &ram)
{
    if (gService == nullptr)
        return;

    IEnumWbemClassObject *pEnumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(L"SELECT * FROM Win32_PhysicalMemory"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator);

    if (FAILED(hr) || pEnumerator == nullptr)
        return;

    IWbemClassObject *pObject = nullptr;
    ULONG returned = 0;

    while (true)
    {
        hr = pEnumerator->Next(
            WBEM_INFINITE,
            1,
            &pObject,
            &returned);

        if (FAILED(hr) || returned == 0)
            break;

        RAMModule module;

        GetWMIProperty(pObject, L"Manufacturer", module.Manufacturer);
        GetWMIProperty(pObject, L"PartNumber", module.PartNumber);
        GetWMIProperty(pObject, L"SerialNumber", module.SerialNumber);

        GetWMIProperty(pObject, L"DeviceLocator", module.DeviceLocator);
        module.SlotName = module.DeviceLocator;

        GetWMIProperty(pObject, L"BankLabel", module.BankLabel);

        GetWMIProperty(pObject, L"Capacity", module.CapacityBytes);
        module.CapacityGB =
            static_cast<double>(module.CapacityBytes) /
            (1024.0 * 1024.0 * 1024.0);

        GetWMIProperty(pObject, L"ConfiguredClockSpeed", module.ConfiguredSpeedMHz);
        GetWMIProperty(pObject, L"Speed", module.MaximumSpeedMHz);

        uint16_t memoryType = 0;
        if (GetWMIProperty(pObject, L"SMBIOSMemoryType", memoryType))
            module.Type = static_cast<RAMType>(memoryType);

        uint16_t formFactor = 0;
        if (GetWMIProperty(pObject, L"FormFactor", formFactor))
            module.FormFactor = static_cast<RAMFormFactor>(formFactor);

        uint32_t voltage = 0;
        if (GetWMIProperty(pObject, L"ConfiguredVoltage", voltage))
            module.Voltage = voltage / 1000.0;

        GetWMIProperty(pObject, L"Attributes", module.Rank);

        module.Installed = true;

        ram.Modules.push_back(module);

        pObject->Release();
    }

    pEnumerator->Release();
}

static void FillMemoryArrayInfo(RAMInfo &ram)
{
    if (gService == nullptr)
        return;

    IEnumWbemClassObject *pEnumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(L"SELECT * FROM Win32_PhysicalMemoryArray"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator);

    if (FAILED(hr) || pEnumerator == nullptr)
        return;

    IWbemClassObject *pObject = nullptr;
    ULONG returned = 0;

    if (SUCCEEDED(pEnumerator->Next(
            WBEM_INFINITE,
            1,
            &pObject,
            &returned)) &&
        returned > 0)
    {
        // Number of slots
        GetWMIProperty(pObject,
                       L"MemoryDevices",
                       ram.TotalSlots);

        // Maximum supported capacity (bytes)
        // Maximum supported capacity

        uint64_t maxCapacityEx = 0;

        if (GetWMIProperty(pObject,
                           L"MaxCapacityEx",
                           maxCapacityEx))
        {
            ram.MaximumSupportedCapacityBytes = maxCapacityEx;
        }
        else
        {
            uint32_t maxCapacityKB = 0;

            if (GetWMIProperty(pObject,
                               L"MaxCapacity",
                               maxCapacityKB))
            {
                // MaxCapacity is reported in KB
                ram.MaximumSupportedCapacityBytes =
                    static_cast<uint64_t>(maxCapacityKB) * 1024ULL;
            }
        }

        ram.MaximumSupportedCapacityGB =
            static_cast<double>(ram.MaximumSupportedCapacityBytes) /
            (1024.0 * 1024.0 * 1024.0);

        // ECC Support
        uint16_t errorCorrection = 0;

        if (GetWMIProperty(pObject,
                           L"MemoryErrorCorrection",
                           errorCorrection))
        {
            switch (errorCorrection)
            {
            case 4: // Parity
            case 5: // Single-bit ECC
            case 6: // Multi-bit ECC
            case 7: // CRC
                ram.ECCSupported = true;
                break;

            default:
                ram.ECCSupported = false;
                break;
            }
        }

        pObject->Release();
    }

    pEnumerator->Release();

    ram.OccupiedSlots =
        static_cast<uint32_t>(ram.Modules.size());

    if (ram.TotalSlots >= ram.OccupiedSlots)
        ram.EmptySlots =
            ram.TotalSlots - ram.OccupiedSlots;

    ram.Upgradeable =
        (ram.EmptySlots > 0);
    if (ram.MaximumSupportedCapacityBytes != 0 &&
        ram.MaximumSupportedCapacityBytes < ram.TotalCapacityBytes)
    {
        ram.MaximumSupportedCapacityBytes = 0;
        ram.MaximumSupportedCapacityGB = 0.0;
    }
}

static void FillRuntimeMemoryInfo(RAMInfo &ram)
{
    MEMORYSTATUSEX memStatus{};
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (!GlobalMemoryStatusEx(&memStatus))
        return;

    ram.UsedMemoryBytes =
        memStatus.ullTotalPhys - memStatus.ullAvailPhys;

    ram.AvailableMemoryBytes =
        memStatus.ullAvailPhys;

    ram.FreeMemoryBytes =
        memStatus.ullAvailPhys;

    ram.UsagePercent =
        (static_cast<double>(ram.UsedMemoryBytes) * 100.0) /
        static_cast<double>(memStatus.ullTotalPhys);

    PERFORMANCE_INFORMATION perfInfo{};

    if (GetPerformanceInfo(&perfInfo, sizeof(perfInfo)))
    {
        SIZE_T pageSize = perfInfo.PageSize;

        ram.CommittedMemoryBytes =
            static_cast<uint64_t>(perfInfo.CommitTotal) * pageSize;

        ram.CachedMemoryBytes =
            static_cast<uint64_t>(perfInfo.SystemCache) * pageSize;
    }
}

static void FillChannelInfo(RAMInfo &ram)
{
    ram.ChannelMode = MemoryChannelMode::Unknown;

    const size_t modules = ram.Modules.size();

    if (modules == 0)
        return;

    if (modules == 1)
    {
        ram.ChannelMode = MemoryChannelMode::Single;
        return;
    }

    if (modules == 2)
    {
        const RAMModule &a = ram.Modules[0];
        const RAMModule &b = ram.Modules[1];

        if (a.CapacityBytes == b.CapacityBytes &&
            a.ConfiguredSpeedMHz == b.ConfiguredSpeedMHz &&
            a.Type == b.Type)
        {
            ram.ChannelMode = MemoryChannelMode::Dual;
        }

        return;
    }

    if (modules == 3)
    {
        ram.ChannelMode = MemoryChannelMode::Triple;
        return;
    }

    if (modules == 4)
    {
        // Most consumer platforms (Intel LGA115x/1200/1700,
        // AMD AM4/AM5) are still Dual Channel with four DIMMs.
        // Without reading the memory controller directly, we
        // cannot reliably distinguish Dual vs Quad.

        ram.ChannelMode = MemoryChannelMode::Unknown;
    }
}

RAMInfo GetRAMInfo()
{
    RAMInfo ram;

    FillOverallRAMInfo(ram);
    FillMemoryModuleInfo(ram);
    FillMemoryArrayInfo(ram);
    FillRuntimeMemoryInfo(ram);
    FillChannelInfo(ram);

    return ram;
}
