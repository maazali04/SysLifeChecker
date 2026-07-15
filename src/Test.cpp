#include "Test.hpp"

#include <iostream>

using std::cout;
using std::endl;

void TestCPU(const CPUInfo& cpu)
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

    for (const auto& instruction : cpu.InstructionSets)
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