#include "CPU.hpp"
#include "WMIHelper.hpp"

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <thread>
#include <intrin.h>
#include <vector>
#include <algorithm>

static void FillBasicCPUInfo(CPUInfo &cpu)
{
    // ==========================
    // Architecture & Threads
    // ==========================

    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    cpu.LogicalProcessors = sysInfo.dwNumberOfProcessors;

    switch (sysInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        cpu.Architecture = CPUArchitecture::x64;
        cpu.Is64BitOS = true;
        break;

    case PROCESSOR_ARCHITECTURE_INTEL:
        cpu.Architecture = CPUArchitecture::x86;
        cpu.Is64BitOS = false;
        break;

    case PROCESSOR_ARCHITECTURE_ARM:
        cpu.Architecture = CPUArchitecture::ARM;
        break;

    case PROCESSOR_ARCHITECTURE_ARM64:
        cpu.Architecture = CPUArchitecture::ARM64;
        cpu.Is64BitOS = true;
        break;

    default:
        cpu.Architecture = CPUArchitecture::Unknown;
        break;
    }

    // ==========================
    // Physical Core Count
    // ==========================

    DWORD length = 0;

    GetLogicalProcessorInformationEx(
        RelationProcessorCore,
        nullptr,
        &length);

    if (length > 0)
    {
        std::vector<BYTE> buffer(length);

        if (GetLogicalProcessorInformationEx(
                RelationProcessorCore,
                reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),
                &length))
        {
            char *ptr = reinterpret_cast<char *>(buffer.data());
            char *end = ptr + length;

            while (ptr < end)
            {
                auto info =
                    reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(ptr);

                cpu.PhysicalCores++;

                ptr += info->Size;
            }
        }
    }

    // ==========================
    // Registry Information
    // ==========================

    HKEY hKey;

    if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            0,
            KEY_READ,
            &hKey) == ERROR_SUCCESS)
    {
        char value[256];
        DWORD size;

        // CPU Name

        size = sizeof(value);

        if (RegQueryValueExA(
                hKey,
                "ProcessorNameString",
                nullptr,
                nullptr,
                reinterpret_cast<LPBYTE>(value),
                &size) == ERROR_SUCCESS)
        {
            cpu.Name = value;
            cpu.BrandString = value;
        }

        // Vendor

        size = sizeof(value);

        if (RegQueryValueExA(
                hKey,
                "VendorIdentifier",
                nullptr,
                nullptr,
                reinterpret_cast<LPBYTE>(value),
                &size) == ERROR_SUCCESS)
        {
            cpu.Manufacturer = value;

            std::string vendor = value;

            if (vendor == "GenuineIntel")
                cpu.Vendor = CPUVendor::Intel;
            else if (vendor == "AuthenticAMD")
                cpu.Vendor = CPUVendor::AMD;
            else
                cpu.Vendor = CPUVendor::Unknown;
        }

        // Current Frequency

        DWORD mhz;
        size = sizeof(mhz);

        if (RegQueryValueExA(
                hKey,
                "~MHz",
                nullptr,
                nullptr,
                reinterpret_cast<LPBYTE>(&mhz),
                &size) == ERROR_SUCCESS)
        {
            cpu.Frequency.CurrentClockGHz = mhz / 1000.0;
        }

        RegCloseKey(hKey);
    }
}

static void FillCPUIDInfo(CPUInfo &cpu)
{
    int cpuInfo[4] = {};

    // ==========================================
    // CPUID Leaf 0
    // ==========================================

    __cpuid(cpuInfo, 0);

    int maximumLeaf = cpuInfo[0];

    char vendor[13];

    memcpy(vendor + 0, &cpuInfo[1], 4); // EBX
    memcpy(vendor + 4, &cpuInfo[3], 4); // EDX
    memcpy(vendor + 8, &cpuInfo[2], 4); // ECX

    vendor[12] = '\0';

    cpu.Manufacturer = vendor;

    if (strcmp(vendor, "GenuineIntel") == 0)
        cpu.Vendor = CPUVendor::Intel;
    else if (strcmp(vendor, "AuthenticAMD") == 0)
        cpu.Vendor = CPUVendor::AMD;
    else
        cpu.Vendor = CPUVendor::Unknown;

    cpu.MaximumCPUIDLeaf = maximumLeaf;

    // ==========================================
    // CPUID Leaf 1
    // ==========================================

    __cpuid(cpuInfo, 1);

    int eax = cpuInfo[0];

    cpu.Stepping = eax & 0xF;

    cpu.Model = (eax >> 4) & 0xF;

    cpu.Family = (eax >> 8) & 0xF;

    cpu.ProcessorType = (eax >> 12) & 0x3;

    cpu.ExtendedModel = (eax >> 16) & 0xF;

    cpu.ExtendedFamily = (eax >> 20) & 0xFF;

    // Intel decoding

    if (cpu.Family == 6 || cpu.Family == 15)
    {
        cpu.Model += (cpu.ExtendedModel << 4);
    }

    if (cpu.Family == 15)
    {
        cpu.Family += cpu.ExtendedFamily;
    }

    // ==========================================
    // Brand String
    // ==========================================

    char brand[49] = {};

    if (maximumLeaf >= 0x80000004)
    {
        int data[4];

        __cpuid(data, 0x80000002);
        memcpy(brand + 0, data, 16);

        __cpuid(data, 0x80000003);
        memcpy(brand + 16, data, 16);

        __cpuid(data, 0x80000004);
        memcpy(brand + 32, data, 16);

        brand[48] = '\0';

        cpu.BrandString = brand;

        // Use CPUID brand string as primary CPU name
        cpu.Name = cpu.BrandString;
    }

    __cpuid(cpuInfo, 0x80000000);

    cpu.MaximumExtendedLeaf = cpuInfo[0];

    // ==========================================
    // Processor Features (Leaf 1)
    // ==========================================

    __cpuid(cpuInfo, 1);

    int ebx = cpuInfo[1];
    int ecx = cpuInfo[2];
    int edx = cpuInfo[3];

    // Initial APIC ID
    cpu.InitialAPICID = (ebx >> 24) & 0xFF;

    // Logical processors in this package
    cpu.LogicalProcessorsPerPackage = (ebx >> 16) & 0xFF;

    // Hyper-Threading
    cpu.HyperThreadingSupported = ((edx >> 28) & 1);

    // x2APIC
    cpu.x2APICSupported = ((ecx >> 21) & 1);
}

static void FillCacheInfo(CPUInfo &cpu)
{
    int cpuInfo[4];

    cpu.Cache.L1DataKB = 0;
    cpu.Cache.L1InstructionKB = 0;
    cpu.Cache.L2KB = 0;
    cpu.Cache.L3KB = 0;
    cpu.Cache.CacheLineSize = 0;

    for (int index = 0;; index++)
    {
        __cpuidex(cpuInfo, 4, index);

        int cacheType = cpuInfo[0] & 0x1F;

        // No more caches
        if (cacheType == 0)
            break;

        int cacheLevel = (cpuInfo[0] >> 5) & 0x7;

        int lineSize = (cpuInfo[1] & 0xFFF) + 1;
        int partitions = ((cpuInfo[1] >> 12) & 0x3FF) + 1;
        int ways = ((cpuInfo[1] >> 22) & 0x3FF) + 1;
        int sets = cpuInfo[2] + 1;

        uint32_t cacheSize =
            static_cast<uint32_t>(ways) *
            partitions *
            lineSize *
            sets;

        cpu.Cache.CacheLineSize = lineSize;

        switch (cacheLevel)
        {
        case 1:
            if (cacheType == 1) // Data Cache
                cpu.Cache.L1DataKB = cacheSize / 1024;

            else if (cacheType == 2) // Instruction Cache
                cpu.Cache.L1InstructionKB = cacheSize / 1024;

            break;

        case 2:
            cpu.Cache.L2KB = cacheSize / 1024;
            break;

        case 3:
            cpu.Cache.L3KB = cacheSize / 1024;
            break;
        }
    }
}
static void DecodeLeaf1(CPUInfo &cpu)
{
    int info[4];

    __cpuid(info, 1);

    int ecx = info[2];
    int edx = info[3];

    cpu.Features.MMX = (edx & (1 << 23)) != 0;
    cpu.Features.SSE = (edx & (1 << 25)) != 0;
    cpu.Features.SSE2 = (edx & (1 << 26)) != 0;

    cpu.Features.SSE3 = (ecx & (1 << 0)) != 0;
    cpu.Features.PCLMULQDQ = (ecx & (1 << 1)) != 0;
    cpu.Features.MONITOR = (ecx & (1 << 3)) != 0;
    cpu.Features.VMx = (ecx & (1 << 5)) != 0;
    cpu.Features.VT_x = cpu.Features.VMx;
    cpu.Features.Virtualization = cpu.Features.VMx;
    cpu.Features.SSSE3 = (ecx & (1 << 9)) != 0;
    cpu.Features.FMA = (ecx & (1 << 12)) != 0;
    cpu.Features.CMPXCHG16B = (ecx & (1 << 13)) != 0;
    cpu.Features.SSE41 = (ecx & (1 << 19)) != 0;
    cpu.Features.SSE42 = (ecx & (1 << 20)) != 0;
    cpu.Features.MOVBE = (ecx & (1 << 22)) != 0;
    cpu.Features.POPCNT = (ecx & (1 << 23)) != 0;
    cpu.Features.AES = (ecx & (1 << 25)) != 0;
    cpu.Features.XSAVE = (ecx & (1 << 26)) != 0;
    cpu.Features.OSXSAVE = (ecx & (1 << 27)) != 0;
    cpu.Features.AVX = (ecx & (1 << 28)) != 0;
    cpu.Features.F16C = (ecx & (1 << 29)) != 0;
    cpu.Features.RDRAND = (ecx & (1 << 30)) != 0;

    cpu.Features.HTT = (edx & (1 << 28)) != 0;
}

static void DecodeLeaf7(CPUInfo &cpu)
{
    int info[4];

    __cpuidex(info, 7, 0);

    int ebx = info[1];
    int ecx = info[2];
    int edx = info[3];

    cpu.Features.FSGSBASE = (ebx & (1 << 0)) != 0;
    cpu.Features.BMI1 = (ebx & (1 << 3)) != 0;
    cpu.Features.HLE = (ebx & (1 << 4)) != 0;
    cpu.Features.AVX2 = (ebx & (1 << 5)) != 0;
    cpu.Features.SMEP = (ebx & (1 << 7)) != 0;
    cpu.Features.BMI2 = (ebx & (1 << 8)) != 0;
    cpu.Features.ERMS = (ebx & (1 << 9)) != 0;
    cpu.Features.INVPCID = (ebx & (1 << 10)) != 0;
    cpu.Features.RTM = (ebx & (1 << 11)) != 0;
    cpu.Features.MPX = (ebx & (1 << 14)) != 0;
    cpu.Features.AVX512F = (ebx & (1 << 16)) != 0;
    cpu.Features.AVX512DQ = (ebx & (1 << 17)) != 0;
    cpu.Features.RDSEED = (ebx & (1 << 18)) != 0;
    cpu.Features.ADX = (ebx & (1 << 19)) != 0;
    cpu.Features.SMAP = (ebx & (1 << 20)) != 0;
    cpu.Features.AVX512IFMA = (ebx & (1 << 21)) != 0;
    cpu.Features.CLFLUSHOPT = (ebx & (1 << 23)) != 0;
    cpu.Features.CLWB = (ebx & (1 << 24)) != 0;
    cpu.Features.SHA = (ebx & (1 << 29)) != 0;
    cpu.Features.AVX512BW = (ebx & (1 << 30)) != 0;
    cpu.Features.AVX512VL = (ebx & (1 << 31)) != 0;
}

static void DecodeExtendedLeaf(CPUInfo &cpu)
{
    int info[4];

    __cpuid(info, 0x80000000);

    // CPU doesn't support extended leaves
    if ((unsigned int)info[0] < 0x80000001)
        return;

    __cpuid(info, 0x80000001);

    int ecx = info[2];
    int edx = info[3];

    // EDX
    cpu.Features.SYSCALL = (edx & (1 << 11)) != 0;
    cpu.Features.NX = (edx & (1 << 20)) != 0;
    cpu.Features.RDTSCP = (edx & (1 << 27)) != 0;
    cpu.Features.x64 = (edx & (1 << 29)) != 0;

    // ECX
    cpu.Features.LAHFSAHF = (ecx & (1 << 0)) != 0;
    cpu.Features.SSE4A = (ecx & (1 << 6)) != 0;
    cpu.Features.ABM = (ecx & (1 << 5)) != 0;

    cpu.Features.DEP = cpu.Features.NX;
    cpu.Features.SMT = cpu.Features.HTT;

    // LZCNT shares the ABM bit
    cpu.Features.LZCNT = cpu.Features.ABM;

    // PREFETCHW shares bit 8
    cpu.Features.PREFETCHW = (ecx & (1 << 8)) != 0;

    // AMD Virtualization
    cpu.Features.AMDV = (ecx & (1 << 2)) != 0;
    cpu.Features.AMD_V = cpu.Features.AMDV;

    if (cpu.Features.AMDV)
        cpu.Features.Virtualization = true;
}

static void FillInstructionSetInfo(CPUInfo &cpu)
{
    DecodeLeaf1(cpu);
    DecodeLeaf7(cpu);
    cpu.Features.AVX512 = cpu.Features.AVX512F;
    DecodeExtendedLeaf(cpu);
    cpu.InstructionSets.clear();

    if (cpu.Features.MMX)
        cpu.InstructionSets.push_back("MMX");

    if (cpu.Features.SSE)
        cpu.InstructionSets.push_back("SSE");
    if (cpu.Features.SSE2)
        cpu.InstructionSets.push_back("SSE2");
    if (cpu.Features.SSE3)
        cpu.InstructionSets.push_back("SSE3");
    if (cpu.Features.SSSE3)
        cpu.InstructionSets.push_back("SSSE3");
    if (cpu.Features.SSE41)
        cpu.InstructionSets.push_back("SSE4.1");
    if (cpu.Features.SSE42)
        cpu.InstructionSets.push_back("SSE4.2");

    if (cpu.Features.AVX)
        cpu.InstructionSets.push_back("AVX");
    if (cpu.Features.AVX2)
        cpu.InstructionSets.push_back("AVX2");
    if (cpu.Features.AVX512F)
        cpu.InstructionSets.push_back("AVX-512F");

    if (cpu.Features.AES)
        cpu.InstructionSets.push_back("AES");
    if (cpu.Features.SHA)
        cpu.InstructionSets.push_back("SHA");

    if (cpu.Features.FMA)
        cpu.InstructionSets.push_back("FMA");

    if (cpu.Features.BMI1)
        cpu.InstructionSets.push_back("BMI1");
    if (cpu.Features.BMI2)
        cpu.InstructionSets.push_back("BMI2");

    if (cpu.Features.PCLMULQDQ)
        cpu.InstructionSets.push_back("PCLMULQDQ");
    if (cpu.Features.RDRAND)
        cpu.InstructionSets.push_back("RDRAND");
    if (cpu.Features.RDSEED)
        cpu.InstructionSets.push_back("RDSEED");
}

static double GetCPUUsage()
{
    static ULARGE_INTEGER lastIdle = {};
    static ULARGE_INTEGER lastKernel = {};
    static ULARGE_INTEGER lastUser = {};

    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
        return 0.0;

    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idleTime.dwLowDateTime;
    idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;

    if (lastIdle.QuadPart == 0)
    {
        lastIdle = idle;
        lastKernel = kernel;
        lastUser = user;
        Sleep(50);
        if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
            return 0.0;
        idle.LowPart = idleTime.dwLowDateTime;
        idle.HighPart = idleTime.dwHighDateTime;
        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;
    }

    ULONGLONG idleDiff = idle.QuadPart - lastIdle.QuadPart;
    ULONGLONG kernelDiff = kernel.QuadPart - lastKernel.QuadPart;
    ULONGLONG userDiff = user.QuadPart - lastUser.QuadPart;

    lastIdle = idle;
    lastKernel = kernel;
    lastUser = user;

    ULONGLONG total = kernelDiff + userDiff;
    if (total == 0)
        return 0.0;

    double usage = ((double)(total - idleDiff) / (double)total) * 100.0;
    return std::clamp(usage, 0.0, 100.0);
}

// Note: Make it complete
static void FillCPUPerformanceInfo(CPUInfo &cpu)
{
    //--------------------------------------
    // CPU Usage
    //--------------------------------------

    cpu.Status.UsagePercent =
        GetCPUUsage();

    //--------------------------------------
    // Current Frequency
    //--------------------------------------

    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr =
        gService->ExecQuery(
            bstr_t("WQL"),
            bstr_t(
                "SELECT CurrentClockSpeed, MaxClockSpeed "
                "FROM Win32_Processor"),
            WBEM_FLAG_FORWARD_ONLY |
                WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &enumerator);

        if (FAILED(hr))
        return;

    IWbemClassObject *object = nullptr;

    ULONG returned = 0;

    if (enumerator->Next(
            WBEM_INFINITE,
            1,
            &object,
            &returned) == WBEM_S_NO_ERROR)
    {

        uint32_t speedMHz = 0;
        uint32_t maxSpeedMHz = 0;

        GetWMIProperty(
            object,
            L"CurrentClockSpeed",
            speedMHz);
        GetWMIProperty(
            object,
            L"MaxClockSpeed",
            maxSpeedMHz);

        cpu.Frequency.BaseClockGHz =
            maxSpeedMHz / 1000.0;

        cpu.Frequency.CurrentClockGHz =
            speedMHz / 1000.0;

        object->Release();
    }

    enumerator->Release();
}

static void FillCPUVirtualizationInfo(CPUInfo &cpu)
{
    //----------------------------------------
    // CPU Support
    //----------------------------------------

    cpu.Features.Virtualization =
        cpu.Features.VT_x || cpu.Features.AMD_V;

    //----------------------------------------
    // Hypervisor Present
    //----------------------------------------

    int info[4] = {};

    __cpuid(info, 1);

    cpu.SupportsHypervisor =
        (info[2] & (1 << 31)) != 0;

    //----------------------------------------
    // Virtual Machine Detection
    //----------------------------------------

    cpu.IsVirtualMachine =
        cpu.SupportsHypervisor;

    //----------------------------------------
    // BIOS Enabled
    //----------------------------------------

    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr =
        gService->ExecQuery(
            bstr_t("WQL"),
            bstr_t(
                "SELECT VirtualizationFirmwareEnabled "
                "FROM Win32_Processor"),
            WBEM_FLAG_FORWARD_ONLY |
                WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &enumerator);

    if (SUCCEEDED(hr))
    {
        IWbemClassObject *object = nullptr;
        ULONG returned = 0;

        if (enumerator->Next(
                WBEM_INFINITE,
                1,
                &object,
                &returned) == WBEM_S_NO_ERROR)
        {
            VARIANT vt;

            VariantInit(&vt);

            if (SUCCEEDED(object->Get(
                    L"VirtualizationFirmwareEnabled",
                    0,
                    &vt,
                    nullptr,
                    nullptr)))
            {
                if (vt.vt == VT_BOOL)
                {
                    cpu.Features.VT_x =
                        (vt.boolVal == VARIANT_TRUE);
                }

                VariantClear(&vt);
            }

            object->Release();
        }

        enumerator->Release();
    }
}

static void FillCPUTopologyInfo(CPUInfo &cpu)
{
    //------------------------------------
    // Processor Packages
    //------------------------------------

    DWORD length = 0;

    GetLogicalProcessorInformationEx(
        RelationProcessorPackage,
        nullptr,
        &length);

    if (length > 0)
    {
        std::vector<BYTE> buffer(length);

        if (GetLogicalProcessorInformationEx(
                RelationProcessorPackage,
                reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),
                &length))
        {
            BYTE *ptr = buffer.data();

            while (ptr < buffer.data() + length)
            {
                auto *info =
                    reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(ptr);

                cpu.PhysicalPackages++;

                ptr += info->Size;
            }
        }
    }

    //------------------------------------
    // NUMA Nodes
    //------------------------------------

    ULONG highestNode = 0;

    if (GetNumaHighestNodeNumber(&highestNode))
    {
        cpu.NUMANodes = highestNode + 1;
    }

    //------------------------------------
    // Processor Groups
    //------------------------------------

    cpu.ProcessorGroups = GetActiveProcessorGroupCount();

     //------------------------------------
    // CPU Core Type Detection
    //------------------------------------

    // Default for non-hybrid CPUs
    if (cpu.PerformanceCores == 0 &&
        cpu.EfficiencyCores == 0)
    {
        cpu.PerformanceCores = cpu.PhysicalCores;
        cpu.EfficiencyCores = 0;
    }
}

static void FillCPUIdentityInfo(CPUInfo &cpu)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr =
        gService->ExecQuery(
            bstr_t("WQL"),
            bstr_t(
                "SELECT "
                "ProcessorId, "
                "SocketDesignation "
                "FROM Win32_Processor"),
            WBEM_FLAG_FORWARD_ONLY |
                WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &enumerator);

    if (FAILED(hr))
        return;

    IWbemClassObject *object = nullptr;

    ULONG returned = 0;

    if (enumerator->Next(
            WBEM_INFINITE,
            1,
            &object,
            &returned) == WBEM_S_NO_ERROR)
    {
        //----------------------------------
        // Processor ID
        //----------------------------------

        GetWMIProperty(
            object,
            L"ProcessorId",
            cpu.ProcessorID);

        //----------------------------------
        // Socket
        //----------------------------------

        GetWMIProperty(
            object,
            L"SocketDesignation",
            cpu.Socket);

        object->Release();
    }

    enumerator->Release();
}
static void FillCPUModelDatabase(CPUInfo &cpu)
{
    //-------------------------------------------------
    // Intel CPUs
    //-------------------------------------------------

    if (cpu.Vendor == CPUVendor::Intel &&
        cpu.Family == 6)
    {
        switch (cpu.Model)
        {
            //---------------------------------
            // 10th Gen Comet Lake
            //---------------------------------

        case 165:

            cpu.Generation = 10;

            cpu.Codename = "Comet Lake";

            cpu.Microarchitecture = "Skylake";

            cpu.Frequency.BoostClockGHz = 4.80;

            break;

            //---------------------------------
            // 8th / 9th Gen Coffee Lake
            //---------------------------------

        case 158:

            cpu.Generation = 9;

            cpu.Codename = "Coffee Lake";

            cpu.Microarchitecture = "Skylake";

            break;

            //---------------------------------
            // 11th Gen Rocket Lake
            //---------------------------------

        case 167:

            cpu.Generation = 11;

            cpu.Codename = "Rocket Lake";

            cpu.Microarchitecture = "Cypress Cove";

            break;

            //---------------------------------
            // 12th Gen Alder Lake
            //---------------------------------

        case 151:

            cpu.Generation = 12;

            cpu.Codename = "Alder Lake";

            cpu.Microarchitecture = "Golden Cove / Gracemont";

            break;

            //---------------------------------
            // 13th Gen Raptor Lake
            //---------------------------------

        case 183:

            cpu.Generation = 13;

            cpu.Codename = "Raptor Lake";

            cpu.Microarchitecture = "Raptor Cove / Gracemont";

            break;

        default:
            break;
        }
    }

    //-------------------------------------------------
    // AMD CPUs
    //-------------------------------------------------

    if (cpu.Vendor == CPUVendor::AMD)
    {
        switch (cpu.Family)
        {
            //---------------------------------
            // Zen
            //---------------------------------

        case 23:

            cpu.Codename = "Zen";

            cpu.Microarchitecture = "Zen";

            break;

            //---------------------------------
            // Zen 3 / Zen 4
            //---------------------------------

        case 25:

            cpu.Codename = "Zen";

            cpu.Microarchitecture = "Zen 3 / Zen 4";

            break;

        default:
            break;
        }
    }
}

CPUInfo GetCPUInfo()
{
    CPUInfo cpu;

    FillBasicCPUInfo(cpu);

    FillCPUIDInfo(cpu);

    FillCacheInfo(cpu);

    FillInstructionSetInfo(cpu);

    FillCPUIdentityInfo(cpu);

    FillCPUVirtualizationInfo(cpu);

    FillCPUPerformanceInfo(cpu);

    FillCPUTopologyInfo(cpu);

    FillCPUModelDatabase(cpu);

    return cpu;
}