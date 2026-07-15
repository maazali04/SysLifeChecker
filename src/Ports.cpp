#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>

#include <iphlpapi.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <comdef.h>

#include <mmdeviceapi.h>
#undef HDMI
#include <functiondiscoverykeys_devpkey.h>

#include <algorithm>
#include <cctype>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")



#include "Ports.hpp"
#include "WMIHelper.hpp"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Setupapi.lib")

static void FillUSBPorts(PortSystemInfo &ports);
static void FillVideoPorts(PortSystemInfo &ports);
static void FillNetworkPorts(PortSystemInfo &ports);
static void FillAudioPorts(PortSystemInfo &ports);
static void FillCardReaders(PortSystemInfo &ports);


PortSystemInfo GetPortInfo()
{
    PortSystemInfo ports;

    FillUSBPorts(ports);
    FillVideoPorts(ports);
    FillNetworkPorts(ports);
    FillAudioPorts(ports);
    FillCardReaders(ports);

    return ports;
}

static void FillUSBPorts(PortSystemInfo &ports)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_USBHub"),
        WBEM_FLAG_FORWARD_ONLY |
            WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
        return;

    IWbemClassObject *object = nullptr;
    ULONG returned = 0;

    while (enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == WBEM_S_NO_ERROR)
    {
        PortInfo port;

        port.Type = PortType::USB2;
        port.Status = PortStatus::Connected;
        port.Occupied = true;

        GetWMIProperty(
            object,
            L"Name",
            port.Name);

        GetWMIProperty(
            object,
            L"Manufacturer",
            port.ConnectedDevice.Manufacturer);

        GetWMIProperty(
            object,
            L"DeviceID",
            port.ConnectedDevice.SerialNumber);

        GetWMIProperty(
            object,
            L"Description",
            port.ConnectedDevice.DeviceClass);

        port.ConnectedDevice.Name = port.Name;

        //----------------------------------------
        // Guess USB Version
        //----------------------------------------

        std::string lower = port.Name;

        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            ::tolower);

        if (lower.find("usb 3") != std::string::npos ||
            lower.find("xhci") != std::string::npos ||
            lower.find("superspeed") != std::string::npos)
        {
            port.Type = PortType::USB3;
            port.Version = "USB 3.x";
            port.MaximumSpeedMbps = 5000;

            ports.USB3Ports++;
        }
        else
        {
            port.Type = PortType::USB2;
            port.Version = "USB 2.0";
            port.MaximumSpeedMbps = 480;

            ports.USB2Ports++;
        }

        //----------------------------------------
        // USB-C (Best Guess)
        //----------------------------------------

        if (lower.find("type-c") != std::string::npos ||
            lower.find("usb-c") != std::string::npos)
        {
            port.Type = PortType::USBTypeC;
            ports.USBCPorts++;
        }

        //----------------------------------------
        // Safe Removal
        //----------------------------------------

        port.ConnectedDevice.SafelyRemovable = true;

        ports.Ports.push_back(port);

        object->Release();
    }

    enumerator->Release();

    ports.TotalUSBPorts =
        ports.USB2Ports +
        ports.USB3Ports +
        ports.USBCPorts;
}

static void FillVideoPorts(PortSystemInfo &ports)
{
    DISPLAY_DEVICEA adapter{};
    adapter.cb = sizeof(adapter);

    for (DWORD adapterIndex = 0;
         EnumDisplayDevicesA(nullptr,
                             adapterIndex,
                             &adapter,
                             0);
         ++adapterIndex)
    {
        if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;

        DISPLAY_DEVICEA monitor{};
        monitor.cb = sizeof(monitor);

        for (DWORD monitorIndex = 0;
             EnumDisplayDevicesA(
                 adapter.DeviceName,
                 monitorIndex,
                 &monitor,
                 0);
             ++monitorIndex)
        {
            PortInfo port;

            port.Name = monitor.DeviceString;

            port.Status = PortStatus::Connected;
            port.Occupied = true;

            port.DataTransferSupported = false;
            port.DisplayOutputSupported = true;

            std::string id =
                monitor.DeviceID;

            std::transform(
                id.begin(),
                id.end(),
                id.begin(),
                ::toupper);

            //----------------------------------------------------
            // Best effort connection detection
            //----------------------------------------------------

            //----------------------------------------------------
            // Best effort connection detection
            //----------------------------------------------------

            if (id.find("HDMI") != std::string::npos)
            {
                port.Type = PortType::HDMI;
                port.Version = "HDMI";

                ports.HDMIPorts++;
            }
            else if (id.find("DISPLAYPORT") != std::string::npos ||
                     id.find("DP") != std::string::npos)
            {
                port.Type = PortType::DisplayPort;
                port.Version = "DisplayPort";

                ports.DisplayPorts++;
            }
            else if (id.find("DVI") != std::string::npos)
            {
                port.Type = PortType::DVI;
                port.Version = "DVI";

                ports.DVIPorts++;
            }
            else if (id.find("VGA") != std::string::npos)
            {
                port.Type = PortType::VGA;
                port.Version = "VGA";

                ports.VGAPorts++;
            }
            else
            {
                port.Type = PortType::Unknown;
                port.Version = "Unknown";
            }

            ports.Ports.push_back(port);
        }
    }
}

static void FillNetworkPorts(PortSystemInfo &ports)
{
    ULONG size = 0;

    GetAdaptersAddresses(
        AF_UNSPEC,
        GAA_FLAG_INCLUDE_PREFIX,
        nullptr,
        nullptr,
        &size);

    std::vector<BYTE> buffer(size);

    IP_ADAPTER_ADDRESSES *adapters =
        reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

    if (GetAdaptersAddresses(
            AF_UNSPEC,
            GAA_FLAG_INCLUDE_PREFIX,
            nullptr,
            adapters,
            &size) != NO_ERROR)
    {
        return;
    }

    for (IP_ADAPTER_ADDRESSES *adapter = adapters;
         adapter;
         adapter = adapter->Next)
    {
        // Ignore loopback/tunnel adapters
        if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
            adapter->IfType == IF_TYPE_TUNNEL)
            continue;

        // Ethernet only
        if (adapter->IfType != IF_TYPE_ETHERNET_CSMACD)
            continue;

        PortInfo port;

        port.Name =
            adapter->FriendlyName ? _bstr_t(adapter->FriendlyName) : "";

        port.Type = PortType::Ethernet;

        port.Version = "Ethernet";

        port.DataTransferSupported = true;

        port.DisplayOutputSupported = false;

        port.MaximumSpeedMbps = 1000; // Placeholder

        port.Occupied =
            adapter->OperStatus ==
            IfOperStatusUp;

        port.Status =
            port.Occupied ? PortStatus::Connected : PortStatus::Available;

        ports.EthernetPorts++;

        ports.Ports.push_back(port);
    }
}

static void FillAudioPorts(PortSystemInfo &ports)
{
    IMMDeviceEnumerator *enumerator = nullptr;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void **>(&enumerator));

    if (FAILED(hr))
        return;

    IMMDeviceCollection *devices = nullptr;

    hr = enumerator->EnumAudioEndpoints(
        eAll,
        DEVICE_STATE_ACTIVE,
        &devices);

    if (FAILED(hr))
    {
        enumerator->Release();
        return;
    }

    UINT count = 0;
    devices->GetCount(&count);

    for (UINT i = 0; i < count; i++)
    {
        IMMDevice *device = nullptr;

        if (FAILED(devices->Item(i, &device)))
            continue;

        IPropertyStore *props = nullptr;

        if (FAILED(device->OpenPropertyStore(
                STGM_READ,
                &props)))
        {
            device->Release();
            continue;
        }

        PortInfo port;

        port.Type = PortType::Audio35mm;
        port.Status = PortStatus::Connected;
        port.Occupied = true;

        port.DataTransferSupported = false;
        port.DisplayOutputSupported = false;

        PROPVARIANT var;
        PropVariantInit(&var);

        if (SUCCEEDED(props->GetValue(
                PKEY_Device_FriendlyName,
                &var)))
        {
            port.Name = _bstr_t(var.pwszVal);

            PropVariantClear(&var);
        }

        ports.AudioPorts++;

        ports.Ports.push_back(port);

        props->Release();
        device->Release();
    }

    devices->Release();
    enumerator->Release();
}

static void FillCardReaders(PortSystemInfo& ports)
{
    HDEVINFO deviceInfo = SetupDiGetClassDevsA(
        nullptr,
        "USB", 
        nullptr,
        DIGCF_ALLCLASSES | DIGCF_PRESENT);

    if (deviceInfo == INVALID_HANDLE_VALUE)
        return;

    SP_DEVINFO_DATA deviceData{};
    deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD index = 0;
         SetupDiEnumDeviceInfo(deviceInfo, index, &deviceData);
         ++index)
    {
        char buffer[512]{};

        if (!SetupDiGetDeviceRegistryPropertyA(
                deviceInfo,
                &deviceData,
                SPDRP_DEVICEDESC,
                nullptr,
                reinterpret_cast<PBYTE>(buffer),
                sizeof(buffer),
                nullptr))
        {
            continue;
        }

        std::string name = buffer;

        std::string lower = name;

        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            ::tolower);

        PortInfo port;

        port.Name = name;
        port.Status = PortStatus::Available;
        port.Occupied = false;
        port.DataTransferSupported = true;

        bool found = false;

        if (lower.find("sd") != std::string::npos &&
            lower.find("micro") == std::string::npos)
        {
            port.Type = PortType::SDCard;
            port.Version = "SD";

            ports.SDCardSlots++;
            found = true;
        }
        else if (lower.find("microsd") != std::string::npos ||
                 lower.find("micro sd") != std::string::npos)
        {
            port.Type = PortType::MicroSD;
            port.Version = "MicroSD";

            ports.MicroSDSlots++;
            found = true;
        }

        if (found)
            ports.Ports.push_back(port);
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
}