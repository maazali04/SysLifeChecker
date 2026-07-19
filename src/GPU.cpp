#include "GPU.hpp"
#include "SystemInfo.hpp"

#include <windows.h>
#include <Wbemidl.h>
#include <comdef.h>

#include <dxgi.h>


#include <pdh.h>
#include <pdhmsg.h>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib, "wbemuuid.lib")

const char *GPUVendorToString(GPUVendor vendor)
{
    switch (vendor)
    {
    case GPUVendor::NVIDIA:
        return "NVIDIA";
    case GPUVendor::AMD:
        return "AMD";
    case GPUVendor::Intel:
        return "Intel";
    case GPUVendor::Qualcomm:
        return "Qualcomm";
    case GPUVendor::Apple:
        return "Apple";
    case GPUVendor::ARM:
        return "ARM";
    case GPUVendor::Microsoft:
        return "Microsoft";
    case GPUVendor::VMware:
        return "VMware";
    case GPUVendor::VirtualBox:
        return "VirtualBox";

    default:
        return "Unknown";
    }
}

const char *GPUTypeToString(GPUType type)
{
    switch (type)
    {
    case GPUType::Integrated:
        return "Integrated";

    case GPUType::Dedicated:
        return "Dedicated";

    case GPUType::External:
        return "External";

    case GPUType::Virtual:
        return "Virtual";

    default:
        return "Unknown";
    }
}

static GPUVendor GetVendor(const std::string &manufacturer)
{
    if (manufacturer.find("NVIDIA") != std::string::npos)
        return GPUVendor::NVIDIA;

    if (manufacturer.find("AMD") != std::string::npos)
        return GPUVendor::AMD;

    if (manufacturer.find("ATI") != std::string::npos)
        return GPUVendor::AMD;

    if (manufacturer.find("Intel") != std::string::npos)
        return GPUVendor::Intel;

    if (manufacturer.find("Qualcomm") != std::string::npos)
        return GPUVendor::Qualcomm;

    if (manufacturer.find("Microsoft") != std::string::npos)
        return GPUVendor::Microsoft;

    if (manufacturer.find("VMware") != std::string::npos)
        return GPUVendor::VMware;

    if (manufacturer.find("VirtualBox") != std::string::npos)
        return GPUVendor::VirtualBox;

    return GPUVendor::Unknown;
}

static void FillVideoControllers(std::vector<GPUInfo> &gpus)
{
    IWbemLocator *locator = nullptr;
    IWbemServices *services = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID *)&locator);

    if (FAILED(hr))
        return;

    hr = locator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &services);

    if (FAILED(hr))
    {
        locator->Release();
        return;
    }

    hr = CoSetProxyBlanket(
        services,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);

    if (FAILED(hr))
    {
        services->Release();
        locator->Release();
        return;
    }

    IEnumWbemClassObject *enumerator = nullptr;

    hr = services->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        services->Release();
        locator->Release();
        return;
    }

    IWbemClassObject *object = nullptr;
    ULONG returned = 0;

    while (enumerator)
    {
        hr = enumerator->Next(
            WBEM_INFINITE,
            1,
            &object,
            &returned);

        if (returned == 0)
            break;

        GPUInfo gpu;

        VARIANT value;
        VariantInit(&value);

        // -------------------------------------------------
        // Name
        // -------------------------------------------------

        if (SUCCEEDED(object->Get(L"Name", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                gpu.Name = _bstr_t(value.bstrVal);
        }

        VariantClear(&value);

        // -------------------------------------------------
        // Manufacturer
        // -------------------------------------------------

        VariantInit(&value);

        if (SUCCEEDED(object->Get(L"AdapterCompatibility", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
            {
                gpu.Manufacturer = _bstr_t(value.bstrVal);
                gpu.Vendor = GetVendor(gpu.Manufacturer);
            }
        }

        VariantClear(&value);

        // -------------------------------------------------
        // DeviceID
        // -------------------------------------------------

        VariantInit(&value);

        if (SUCCEEDED(object->Get(L"DeviceID", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                gpu.DeviceID = _bstr_t(value.bstrVal);
        }

        VariantClear(&value);

        // -------------------------------------------------
        // Driver Version
        // -------------------------------------------------

        VariantInit(&value);

        if (SUCCEEDED(object->Get(L"DriverVersion", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                gpu.Driver.DriverVersion = _bstr_t(value.bstrVal);
        }

        VariantClear(&value);

        // -------------------------------------------------
        // Driver Date
        // -------------------------------------------------

        VariantInit(&value);

        if (SUCCEEDED(object->Get(L"DriverDate", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
            {
                std::string date = (char *)_bstr_t(value.bstrVal);

                if (date.length() >= 8)
                {
                    gpu.Driver.DriverDate =
                        date.substr(0, 4) + "-" +
                        date.substr(4, 2) + "-" +
                        date.substr(6, 2);
                }
                else
                {
                    gpu.Driver.DriverDate = date;
                }
            }
        }

        VariantClear(&value);

        // -------------------------------------------------
        // Adapter RAM
        // -------------------------------------------------

        VariantInit(&value);

        if (SUCCEEDED(object->Get(L"AdapterRAM", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_I4 ||
                value.vt == VT_UI4)
            {
                gpu.DedicatedVRAMBytes =
                    static_cast<uint64_t>(value.ulVal);

                gpu.DedicatedVRAMGB =
                    gpu.DedicatedVRAMBytes /
                    1024.0 /
                    1024.0 /
                    1024.0;
            }
        }

        VariantClear(&value);

        // -------------------------------------------------
        // Video Processor
        // -------------------------------------------------

        VariantInit(&value);

        if (SUCCEEDED(object->Get(L"VideoProcessor", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                gpu.Codename = _bstr_t(value.bstrVal);
        }

        VariantClear(&value);

        // -------------------------------------------------
        // Primary GPU
        // ------------------------------------------------

        gpus.push_back(gpu);

        object->Release();
    }

    enumerator->Release();
    services->Release();
    locator->Release();
}

static void FillPhase1Info(std::vector<GPUInfo> &gpus)
{
    IWbemLocator *locator = nullptr;
    IWbemServices *services = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID *)&locator);

    if (FAILED(hr))
        return;

    hr = locator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &services);

    if (FAILED(hr))
    {
        locator->Release();
        return;
    }

    CoSetProxyBlanket(
        services,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);

    IEnumWbemClassObject *enumerator = nullptr;

    services->ExecQuery(
        bstr_t("WQL"),
        bstr_t(
            "SELECT * FROM Win32_PnPSignedDriver "
            "WHERE DeviceClass='DISPLAY'"),
        WBEM_FLAG_FORWARD_ONLY |
            WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    IWbemClassObject *object = nullptr;
    ULONG returned = 0;

    while (enumerator &&
           enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == S_OK)
    {

        VARIANT value;
        VariantInit(&value);

        std::string description;

        if (SUCCEEDED(object->Get(
                L"Description",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                description = _bstr_t(value.bstrVal);
        }

        VariantClear(&value);

        for (auto &gpu : gpus)
        {

            if (description.find(gpu.Name) == std::string::npos &&
                gpu.Name.find(description) == std::string::npos)
                continue;

            VariantInit(&value);

            if (SUCCEEDED(object->Get(
                    L"DriverProviderName",
                    0,
                    &value,
                    nullptr,
                    nullptr)))
            {
                if (value.vt == VT_BSTR)
                    gpu.Driver.DriverProvider =
                        _bstr_t(value.bstrVal);
            }

            VariantClear(&value);

            VariantInit(&value);

            if (SUCCEEDED(object->Get(
                    L"InfName",
                    0,
                    &value,
                    nullptr,
                    nullptr)))
            {
                if (value.vt == VT_BSTR)
                    gpu.Driver.INFFile =
                        _bstr_t(value.bstrVal);
            }

            VariantClear(&value);

            VariantInit(&value);

            if (SUCCEEDED(object->Get(
                    L"DriverVersion",
                    0,
                    &value,
                    nullptr,
                    nullptr)))
            {
                if (value.vt == VT_BSTR)
                    gpu.Driver.DriverVersion =
                        _bstr_t(value.bstrVal);
            }

            VariantClear(&value);

            VariantInit(&value);

            if (SUCCEEDED(object->Get(
                    L"DriverDate",
                    0,
                    &value,
                    nullptr,
                    nullptr)))
            {
                if (value.vt == VT_BSTR)
                {
                    std::string date =
                        (char *)_bstr_t(value.bstrVal);

                    if (date.length() >= 8)
                    {
                        gpu.Driver.DriverDate =
                            date.substr(0, 4) + "-" +
                            date.substr(4, 2) + "-" +
                            date.substr(6, 2);
                    }
                    else
                    {
                        gpu.Driver.DriverDate = date;
                    }
                }
            }

            VariantClear(&value);

            VariantInit(&value);

            if (SUCCEEDED(object->Get(
                    L"IsSigned",
                    0,
                    &value,
                    nullptr,
                    nullptr)))
            {
                if (value.vt == VT_BOOL)
                    gpu.Driver.WHQLCertified =
                        value.boolVal == VARIANT_TRUE;
            }

            VariantClear(&value);

            break;
        }

        object->Release();
    }

    if (enumerator)
        enumerator->Release();

    services->Release();
    locator->Release();
}

static void DetectGPUType(std::vector<GPUInfo> &gpus)
{
    for (auto &gpu : gpus)
    {
        if (gpu.Vendor == GPUVendor::Microsoft ||
            gpu.Vendor == GPUVendor::VMware ||
            gpu.Vendor == GPUVendor::VirtualBox)
        {
            gpu.Type = GPUType::Virtual;
            continue;
        }

        if (gpu.DedicatedVRAMBytes > 1024ULL * 1024ULL * 512ULL)
        {
            gpu.Type = GPUType::Dedicated;
        }
        else
        {
            gpu.Type = GPUType::Integrated;
        }
    }
}

static void FillDXGIInfo(std::vector<GPUInfo> &gpus)
{
    IDXGIFactory *factory = nullptr;

    if (FAILED(CreateDXGIFactory(
            __uuidof(IDXGIFactory),
            (void **)&factory)))
    {
        return;
    }

    UINT index = 0;
    IDXGIAdapter *adapter = nullptr;

    while (factory->EnumAdapters(index, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;

        if (FAILED(adapter->GetDesc(&desc)))
        {
            adapter->Release();
            ++index;
            continue;
        }

        std::wstring ws(desc.Description);
        std::string name(ws.begin(), ws.end());

        for (auto &gpu : gpus)
        {
            if (gpu.Name != name)
                continue;

            gpu.DedicatedVRAMBytes =
                desc.DedicatedVideoMemory;

            gpu.DedicatedVRAMGB =
                desc.DedicatedVideoMemory /
                1024.0 / 1024.0 / 1024.0;

            gpu.SharedMemoryBytes =
                desc.SharedSystemMemory;

            gpu.SharedMemoryGB =
                desc.SharedSystemMemory /
                1024.0 / 1024.0 / 1024.0;

            gpu.TotalGraphicsMemoryBytes =
                desc.DedicatedVideoMemory +
                desc.SharedSystemMemory;

            gpu.TotalGraphicsMemoryGB =
                gpu.TotalGraphicsMemoryBytes /
                1024.0 / 1024.0 / 1024.0;

            gpu.DeviceID =
                std::to_string(desc.DeviceId);

            if (desc.VendorId == 0x10DE)
                gpu.Vendor = GPUVendor::NVIDIA;

            else if (desc.VendorId == 0x1002)
                gpu.Vendor = GPUVendor::AMD;

            else if (desc.VendorId == 0x8086)
                gpu.Vendor = GPUVendor::Intel;

            break;
        }

        adapter->Release();
        ++index;
    }

    factory->Release();
}
static void FillDisplayOutputs(std::vector<GPUInfo> &gpus)
{
    IDXGIFactory *factory = nullptr;

    if (FAILED(CreateDXGIFactory(
            __uuidof(IDXGIFactory),
            (void **)&factory)))
    {
        return;
    }

    UINT adapterIndex = 0;
    IDXGIAdapter *adapter = nullptr;

    while (factory->EnumAdapters(
               adapterIndex,
               &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC adapterDesc;

        adapter->GetDesc(&adapterDesc);

        std::wstring ws(adapterDesc.Description);
        std::string adapterName(ws.begin(), ws.end());

        GPUInfo *gpu = nullptr;

        for (auto &g : gpus)
        {
            if (g.Name == adapterName)
            {
                gpu = &g;
                break;
            }
        }

        if (gpu)
        {
            UINT outputIndex = 0;
            IDXGIOutput *output = nullptr;

            while (adapter->EnumOutputs(
                       outputIndex,
                       &output) != DXGI_ERROR_NOT_FOUND)
            {
                DXGI_OUTPUT_DESC desc;

                output->GetDesc(&desc);

                GPUDisplayOutput display;

                std::wstring monitor(desc.DeviceName);

                display.Name =
                    std::string(
                        monitor.begin(),
                        monitor.end());

                display.Connected =
                    desc.AttachedToDesktop;

                gpu->Outputs.push_back(display);

                output->Release();

                ++outputIndex;
            }
        }

        adapter->Release();

        ++adapterIndex;
    }

    factory->Release();
}

static double GetTotalGPUUsage()
{
    PDH_HQUERY query = nullptr;
    PDH_HCOUNTER counter = nullptr;

    if (PdhOpenQuery(nullptr, 0, &query) != ERROR_SUCCESS)
        return 0.0;

    if (PdhAddEnglishCounterW(
            query,
            L"\\GPU Engine(*)\\Utilization Percentage",
            0,
            &counter) != ERROR_SUCCESS)
    {
        PdhCloseQuery(query);
        return 0.0;
    }

    PdhCollectQueryData(query);

    Sleep(250);

    PdhCollectQueryData(query);

    DWORD bufferSize = 0;
    DWORD itemCount = 0;

    PdhGetFormattedCounterArray(
        counter,
        PDH_FMT_DOUBLE,
        &bufferSize,
        &itemCount,
        nullptr);

    std::vector<BYTE> buffer(bufferSize);

    auto *items =
        reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM>(
            buffer.data());

    if (PdhGetFormattedCounterArray(
            counter,
            PDH_FMT_DOUBLE,
            &bufferSize,
            &itemCount,
            items) != ERROR_SUCCESS)
    {
        PdhCloseQuery(query);
        return 0.0;
    }

    double total = 0.0;

    for (DWORD i = 0; i < itemCount; ++i)
    {
        total += items[i].FmtValue.doubleValue;
    }

    PdhCloseQuery(query);

    if (total > 100.0)
        total = 100.0;

    return total;
}
static void FillRuntimeInfo(std::vector<GPUInfo> &gpus)
{
    double usage = GetTotalGPUUsage();

    for (auto &gpu : gpus)
    {
        gpu.Status.UsagePercent = usage;
    }
}

std::vector<GPUInfo> GetGPUInfo()
{
    std::vector<GPUInfo> gpus;

    FillVideoControllers(gpus);

    FillDXGIInfo(gpus);

    FillPhase1Info(gpus);

    DetectGPUType(gpus);


    FillDisplayOutputs(gpus);

    FillRuntimeInfo(gpus);

    return gpus;
}