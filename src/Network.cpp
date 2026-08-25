#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>

#include <iphlpapi.h>
#include <netioapi.h>
#include <wininet.h>
#include <wlanapi.h>
#include <bluetoothapis.h>

#include <algorithm>
#include <cctype>

#include "Network.hpp"
#include "WMIHelper.hpp"

#pragma comment(lib, "Bthprops.lib")
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

const char *NetworkAdapterTypeToString(NetworkAdapterType type)
{
    switch (type)
    {
    case NetworkAdapterType::Ethernet:
        return "Ethernet";

    case NetworkAdapterType::WiFi:
        return "Wi-Fi";

    case NetworkAdapterType::Bluetooth:
        return "Bluetooth";

    case NetworkAdapterType::Cellular:
        return "Cellular";

    case NetworkAdapterType::Virtual:
        return "Virtual";

    case NetworkAdapterType::VPN:
        return "VPN";

    case NetworkAdapterType::Loopback:
        return "Loopback";

    default:
        return "Unknown";
    }
}

const char *NetworkStatusToString(NetworkStatus status)
{
    switch (status)
    {
    case NetworkStatus::Connected:
        return "Connected";

    case NetworkStatus::Disconnected:
        return "Disconnected";

    case NetworkStatus::Disabled:
        return "Disabled";

    case NetworkStatus::Connecting:
        return "Connecting";

    default:
        return "Unknown";
    }
}

//------------------------------------------------------------
// Internal Functions
//------------------------------------------------------------

static void FillAdapters(NetworkInfo &network);
static void FillAdapterConfiguration(NetworkInfo &network);
static void FillNetworkStatistics(NetworkInfo &network);
static void FillWiFiInfo(NetworkInfo &network);
static void FillBluetoothInfo(NetworkInfo &network);
static void FillDriverInformation(NetworkInfo &network);
static void FillInternetStatus(NetworkInfo &network);
static void CalculateSummary(NetworkInfo &network);

//------------------------------------------------------------
// Public Functions
//------------------------------------------------------------

std::vector<NetworkAdapter> GetNetworkAdapters()
{
    return GetNetworkInfo().Adapters;
}

NetworkInfo GetNetworkInfo()
{
    NetworkInfo network;

    FillAdapters(network);
    FillAdapterConfiguration(network);
    FillNetworkStatistics(network);
    FillWiFiInfo(network);
    FillBluetoothInfo(network);
    FillDriverInformation(network);
    FillInternetStatus(network);
    CalculateSummary(network);

    return network;
}

//------------------------------------------------------------
// Fill Adapters
//------------------------------------------------------------

static void FillAdapters(NetworkInfo &network)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_NetworkAdapter"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
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
        NetworkAdapter adapter;

        //----------------------------------
        // Identity
        //----------------------------------

        GetWMIProperty(object, L"Name", adapter.Name);
        GetWMIProperty(object, L"Description", adapter.Description);
        GetWMIProperty(object, L"MACAddress", adapter.MACAddress);
        GetWMIProperty(object, L"DeviceID", adapter.DeviceID);


        adapter.Model = adapter.Description;

        //----------------------------------
        // Enabled
        //----------------------------------

        bool enabled = true;

        if (GetWMIProperty(
                object,
                L"NetEnabled",
                enabled))
        {
            adapter.Enabled = enabled;
        }

        //----------------------------------
        // Adapter Type
        //----------------------------------

        std::string lower = adapter.Name;

        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            ::tolower);

        if (lower.find("wi-fi direct") != std::string::npos ||
            lower.find("miniport") != std::string::npos ||
            lower.find("virtual") != std::string::npos)
        {
            adapter.Type = NetworkAdapterType::Virtual;
        }
        else if (lower.find("wireless") != std::string::npos ||
                 lower.find("wifi") != std::string::npos ||
                 lower.find("wi-fi") != std::string::npos)
        {
            adapter.Type = NetworkAdapterType::WiFi;
        }
        else if (lower.find("bluetooth") != std::string::npos)
        {
            adapter.Type = NetworkAdapterType::Bluetooth;
        }
        else if (lower.find("vpn") != std::string::npos)
        {
            adapter.Type = NetworkAdapterType::VPN;
        }
        else if (lower.find("virtual") != std::string::npos ||
                 lower.find("vmware") != std::string::npos ||
                 lower.find("hyper-v") != std::string::npos ||
                 lower.find("virtualbox") != std::string::npos)
        {
            adapter.Type = NetworkAdapterType::Virtual;
        }
        else if (lower.find("ethernet") != std::string::npos ||
                 lower.find("gbe") != std::string::npos ||
                 lower.find("gigabit") != std::string::npos)
        {
            adapter.Type = NetworkAdapterType::Ethernet;
        }
        else
        {
            adapter.Type = NetworkAdapterType::Unknown;
        }

        //----------------------------------
        // Physical Adapter
        //----------------------------------

        GetWMIProperty(
            object,
            L"PhysicalAdapter",
            adapter.PhysicalAdapter);


        //----------------------------------
        // Connection Status
        //----------------------------------

        uint16_t status = 0;

        if (GetWMIProperty(
                object,
                L"NetConnectionStatus",
                status))
        {
            switch (status)
            {
            case 2:
                adapter.Status = NetworkStatus::Connected;
                break;

            case 0:
            case 7:
                adapter.Status = NetworkStatus::Disconnected;
                break;

            default:
                if (adapter.Enabled)
                    adapter.Status = NetworkStatus::Disconnected;
                else
                    adapter.Status = NetworkStatus::Disabled;
                break;
            }
        }
        else
        {
            // NetConnectionStatus is NULL for some adapters -- most
            // commonly a physical Ethernet NIC with no cable plugged in.
            // Previously this left adapter.Status at its default
            // (Unknown) even though the adapter clearly has a MAC
            // address; treat it as Disconnected instead.
            adapter.Status = adapter.Enabled
                ? NetworkStatus::Disconnected
                : NetworkStatus::Disabled;
        }

        //----------------------------------

        network.Adapters.push_back(adapter);

        object->Release();
    }

    enumerator->Release();
}

//------------------------------------------------------------
// Fill Adapter Configuration
//------------------------------------------------------------

static void FillAdapterConfiguration(NetworkInfo &network)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled=True"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
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
        std::string description;

        GetWMIProperty(
            object,
            L"Description",
            description);

        NetworkAdapter *adapter = nullptr;

        for (auto &a : network.Adapters)
        {
            if (a.Description == description)
            {
                adapter = &a;
                break;
            }
        }

        if (!adapter)
        {
            object->Release();
            continue;
        }

        //--------------------------------------------------
        // DHCP
        //--------------------------------------------------

        GetWMIProperty(
            object,
            L"DHCPEnabled",
            adapter->DHCPEnabled);

        //--------------------------------------------------
        // IP Addresses
        //--------------------------------------------------

        VARIANT vt;
        VariantInit(&vt);

        if (SUCCEEDED(object->Get(L"IPAddress", 0, &vt, nullptr, nullptr)))
        {
            if ((vt.vt & VT_ARRAY) && vt.parray)
            {
                SAFEARRAY *sa = vt.parray;

                LONG lBound, uBound;

                SafeArrayGetLBound(sa, 1, &lBound);
                SafeArrayGetUBound(sa, 1, &uBound);

                for (LONG i = lBound; i <= uBound; i++)
                {
                    BSTR value;

                    SafeArrayGetElement(sa, &i, &value);

                    std::wstring ws(value);
                    std::string ip(ws.begin(), ws.end());

                    if (ip.find(':') != std::string::npos)
                    {
                        IPv6Address ipv6;
                        ipv6.Address = ip;
                        adapter->IPv6.push_back(ipv6);
                    }
                    else
                    {
                        IPv4Address ipv4;
                        ipv4.Address = ip;
                        adapter->IPv4.push_back(ipv4);
                    }

                    SysFreeString(value);
                }
            }
        }

        VariantClear(&vt);
        //--------------------------------------------------
        // Subnet Mask
        //--------------------------------------------------

        VariantInit(&vt);

        if (SUCCEEDED(object->Get(L"IPSubnet", 0, &vt, nullptr, nullptr)))
        {
            if ((vt.vt & VT_ARRAY) && vt.parray)
            {
                SAFEARRAY *sa = vt.parray;

                LONG lBound, uBound;

                SafeArrayGetLBound(sa, 1, &lBound);
                SafeArrayGetUBound(sa, 1, &uBound);

                LONG ipv4Index = 0;

                for (LONG i = lBound; i <= uBound; i++)
                {
                    BSTR value;

                    SafeArrayGetElement(sa, &i, &value);

                    std::wstring ws(value);

                    std::string subnet(ws.begin(), ws.end());

                    if (subnet.find(':') == std::string::npos)
                    {
                        if (ipv4Index < adapter->IPv4.size())
                            adapter->IPv4[ipv4Index++].SubnetMask = subnet;
                    }

                    SysFreeString(value);
                }
            }
        }

        VariantClear(&vt);

        //--------------------------------------------------
        // Gateway
        //--------------------------------------------------

        VariantInit(&vt);

        if (SUCCEEDED(object->Get(L"DefaultIPGateway", 0, &vt, nullptr, nullptr)))
        {
            if ((vt.vt & VT_ARRAY) && vt.parray)
            {
                SAFEARRAY *sa = vt.parray;

                LONG lBound, uBound;

                SafeArrayGetLBound(sa, 1, &lBound);
                SafeArrayGetUBound(sa, 1, &uBound);

                LONG ipv4Index = 0;
                LONG ipv6Index = 0;

                for (LONG i = lBound; i <= uBound; i++)
                {
                    BSTR value;

                    SafeArrayGetElement(sa, &i, &value);

                    std::wstring ws(value);
                    std::string gateway(ws.begin(), ws.end());

                    if (gateway.find(':') != std::string::npos)
                    {
                        if (ipv6Index < adapter->IPv6.size())
                            adapter->IPv6[ipv6Index++].Gateway = gateway;
                    }
                    else
                    {
                        if (ipv4Index < adapter->IPv4.size())
                            adapter->IPv4[ipv4Index++].Gateway = gateway;
                    }

                    SysFreeString(value);
                }
            }
        }

        VariantClear(&vt);

        //--------------------------------------------------
        // DNS Servers
        //--------------------------------------------------

        VariantInit(&vt);

        if (SUCCEEDED(object->Get(L"DNSServerSearchOrder", 0, &vt, nullptr, nullptr)))
        {
            if ((vt.vt & VT_ARRAY) && vt.parray)
            {
                SAFEARRAY *sa = vt.parray;

                LONG lBound, uBound;

                SafeArrayGetLBound(sa, 1, &lBound);
                SafeArrayGetUBound(sa, 1, &uBound);

                for (LONG i = lBound; i <= uBound; i++)
                {
                    BSTR value;

                    SafeArrayGetElement(sa, &i, &value);

                    std::wstring ws(value);

                    adapter->DNSServers.emplace_back(
                        ws.begin(),
                        ws.end());

                    SysFreeString(value);
                }
            }
        }

        VariantClear(&vt);

        object->Release();
    }

    enumerator->Release();
}
//------------------------------------------------------------
// Fill Statistics
//------------------------------------------------------------

static void FillNetworkStatistics(NetworkInfo &network)
{
    PMIB_IF_TABLE2 table = nullptr;

    if (GetIfTable2(&table) != NO_ERROR)
        return;

    for (ULONG i = 0; i < table->NumEntries; i++)
    {
        const MIB_IF_ROW2 &row = table->Table[i];

        std::wstring ws(row.Description);
        std::string description(ws.begin(), ws.end());

        for (auto &adapter : network.Adapters)
        {
            if (adapter.Description != description)
                continue;

            adapter.Statistics.BytesSent =
                row.OutOctets;

            adapter.Statistics.BytesReceived =
                row.InOctets;

            adapter.Statistics.PacketsSent =
                row.OutUcastPkts +
                row.OutNUcastPkts;

            adapter.Statistics.PacketsReceived =
                row.InUcastPkts +
                row.InNUcastPkts;

            adapter.Statistics.Errors =
                row.InErrors +
                row.OutErrors;

            adapter.Statistics.DroppedPackets =
                row.InDiscards +
                row.OutDiscards;



            break;
        }
    }

    FreeMibTable(table);
}

//------------------------------------------------------------
// Wi-Fi
//------------------------------------------------------------

static void FillWiFiInfo(NetworkInfo &network)
{
    HANDLE client = nullptr;

    DWORD negotiatedVersion = 0;

    if (WlanOpenHandle(
            2,
            nullptr,
            &negotiatedVersion,
            &client) != ERROR_SUCCESS)
    {
        return;
    }

    PWLAN_INTERFACE_INFO_LIST interfaces = nullptr;

    if (WlanEnumInterfaces(
            client,
            nullptr,
            &interfaces) != ERROR_SUCCESS)
    {
        WlanCloseHandle(client, nullptr);
        return;
    }

    for (DWORD i = 0; i < interfaces->dwNumberOfItems; i++)
    {
        WLAN_INTERFACE_INFO &iface =
            interfaces->InterfaceInfo[i];

        std::wstring ws(iface.strInterfaceDescription);

        std::string description(
            ws.begin(),
            ws.end());

        NetworkAdapter *adapter = nullptr;

        for (auto &a : network.Adapters)
        {
            if (a.Description == description)
            {
                adapter = &a;
                break;
            }
        }

        if (!adapter)
            continue;
        PWLAN_CONNECTION_ATTRIBUTES attributes = nullptr;

        DWORD dataSize = 0;

        WLAN_OPCODE_VALUE_TYPE opcode;

        if (WlanQueryInterface(
                client,
                &iface.InterfaceGuid,
                wlan_intf_opcode_current_connection,
                nullptr,
                &dataSize,
                reinterpret_cast<PVOID *>(&attributes),
                &opcode) != ERROR_SUCCESS)
        {
            continue;
        }
        adapter->WiFi.Connected = true;

        DOT11_SSID ssid =
            attributes->wlanAssociationAttributes.dot11Ssid;

        adapter->WiFi.SSID.assign(
            reinterpret_cast<char *>(ssid.ucSSID),
            ssid.uSSIDLength);
        adapter->WiFi.SignalStrength =
            attributes->wlanAssociationAttributes.wlanSignalQuality;
        adapter->WiFi.RSSIdBm =
            -100 + (adapter->WiFi.SignalStrength / 2);
        char buffer[32];

        sprintf_s(
            buffer,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            attributes->wlanAssociationAttributes.dot11Bssid[0],
            attributes->wlanAssociationAttributes.dot11Bssid[1],
            attributes->wlanAssociationAttributes.dot11Bssid[2],
            attributes->wlanAssociationAttributes.dot11Bssid[3],
            attributes->wlanAssociationAttributes.dot11Bssid[4],
            attributes->wlanAssociationAttributes.dot11Bssid[5]);

        adapter->WiFi.BSSID = buffer;
        DWORD dataSize2 = 0;
        ULONG *channel = nullptr;

        if (WlanQueryInterface(
                client,
                &iface.InterfaceGuid,
                wlan_intf_opcode_channel_number,
                nullptr,
                &dataSize2,
                reinterpret_cast<PVOID *>(&channel),
                &opcode) == ERROR_SUCCESS)
        {
            adapter->WiFi.Channel = *channel;

            WlanFreeMemory(channel);
        }

        if (adapter->WiFi.Channel >= 1 &&
            adapter->WiFi.Channel <= 14)
        {
            adapter->WiFi.Band = "2.4 GHz";
        }
        else if (adapter->WiFi.Channel > 14)
        {
            adapter->WiFi.Band = "5 GHz";
        }

        switch (attributes->wlanAssociationAttributes.dot11PhyType)
        {
        case dot11_phy_type_fhss:
            adapter->WiFi.Standard = "802.11";
            break;

        case dot11_phy_type_dsss:
            adapter->WiFi.Standard = "802.11b";
            break;

        case dot11_phy_type_ofdm:
            adapter->WiFi.Standard = "802.11a";
            break;

        case dot11_phy_type_erp:
            adapter->WiFi.Standard = "802.11g";
            break;

        case dot11_phy_type_ht:
            adapter->WiFi.Standard = "802.11n";
            break;

        case dot11_phy_type_vht:
            adapter->WiFi.Standard = "802.11ac";
            break;

        case dot11_phy_type_he:
            adapter->WiFi.Standard = "802.11ax";
            break;

        default:
            adapter->WiFi.Standard = "Unknown";
            break;
        }

        switch (attributes->wlanSecurityAttributes.dot11AuthAlgorithm)
        {
        case DOT11_AUTH_ALGO_80211_OPEN:
            adapter->WiFi.Security = "Open";
            break;

        case DOT11_AUTH_ALGO_80211_SHARED_KEY:
            adapter->WiFi.Security = "WEP";
            break;

        case DOT11_AUTH_ALGO_WPA:
        case DOT11_AUTH_ALGO_WPA_PSK:
            adapter->WiFi.Security = "WPA";
            break;

        case DOT11_AUTH_ALGO_RSNA:
        case DOT11_AUTH_ALGO_RSNA_PSK:
            adapter->WiFi.Security = "WPA2";
            break;

        case DOT11_AUTH_ALGO_WPA3:
        case DOT11_AUTH_ALGO_WPA3_SAE:
            adapter->WiFi.Security = "WPA3";
            break;

        default:
            adapter->WiFi.Security = "Unknown";
            break;
        }

        WlanFreeMemory(attributes);
    }

    WlanFreeMemory(interfaces);

    WlanCloseHandle(client, nullptr);
}

//------------------------------------------------------------
// Bluetooth
//------------------------------------------------------------

static void FillBluetoothInfo(NetworkInfo &network)
{
    BLUETOOTH_FIND_RADIO_PARAMS params{};
    params.dwSize = sizeof(params);

    HANDLE radio = nullptr;

    HBLUETOOTH_RADIO_FIND find =
        BluetoothFindFirstRadio(&params, &radio);

    if (!find)
        return;

    do
    {
        BLUETOOTH_RADIO_INFO info{};
        info.dwSize = sizeof(info);

        if (BluetoothGetRadioInfo(radio, &info) == ERROR_SUCCESS)
        {
            std::wstring ws(info.szName);
            std::string radioName(ws.begin(), ws.end());

            for (auto &adapter : network.Adapters)
            {
                if (adapter.Type == NetworkAdapterType::Bluetooth)
                {
                    adapter.Bluetooth.Supported = true;
                    adapter.Bluetooth.Enabled = true;
                    adapter.Bluetooth.Version = "Detected";
                }
            }
        }

        CloseHandle(radio);

    } while (BluetoothFindNextRadio(find, &radio));

    BluetoothFindRadioClose(find);
}
//------------------------------------------------------------
// Internet
//------------------------------------------------------------

static void FillDriverInformation(NetworkInfo& network)
{
    IEnumWbemClassObject* enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT DeviceName, DriverVersion, DriverDate, DriverProviderName FROM Win32_PnPSignedDriver"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
        return;

    IWbemClassObject* object = nullptr;
    ULONG returned = 0;

    while (enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == WBEM_S_NO_ERROR)
    {
        std::string deviceName;
        std::string version;
        std::string provider;
        std::string date;

        GetWMIProperty(object, L"DeviceName", deviceName);
        GetWMIProperty(object, L"DriverVersion", version);
        GetWMIProperty(object, L"DriverProviderName", provider);
        GetWMIProperty(object, L"DriverDate", date);

        for (auto& adapter : network.Adapters)
        {
            if (adapter.Name != deviceName &&
                adapter.Description != deviceName)
            {
                continue;
            }

            if (!version.empty())
                adapter.DriverVersion = version;

            if (!provider.empty())
                adapter.Manufacturer = provider;

            if (date.length() >= 8)
            {
                adapter.DriverDate =
                    date.substr(0,4) + "-" +
                    date.substr(4,2) + "-" +
                    date.substr(6,2);
            }

            break;
        }

        object->Release();
    }

    enumerator->Release();
}

static void FillInternetStatus(NetworkInfo &network)
{
    DWORD flags = 0;

    network.InternetAvailable =
        InternetGetConnectedState(&flags, 0);

    if (!network.InternetAvailable)
        return;

    for (auto &adapter : network.Adapters)
    {
        if (adapter.Status == NetworkStatus::Connected)
        {
            // We have at least one active adapter.
            break;
        }
    }
}

//------------------------------------------------------------
// Summary
//------------------------------------------------------------

static void CalculateSummary(NetworkInfo &network)
{
    network.TotalAdapters =
        static_cast<uint32_t>(network.Adapters.size());

    network.ConnectedAdapters = 0;

    for (auto &adapter : network.Adapters)
    {
        if (!adapter.IPv4.empty() || !adapter.IPv6.empty())
        {
            network.ConnectedAdapters++;

            if (adapter.Status == NetworkStatus::Unknown)
                adapter.Status = NetworkStatus::Connected;
        }
    }
}