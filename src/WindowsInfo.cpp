#include "WindowsInfo.hpp"
#include "SystemInfo.hpp"
#include "WMIHelper.hpp"

#include <windows.h>
#include <Wbemidl.h>
#include <comdef.h>

#include <string>
#include <stdint.h>
#pragma comment(lib, "wbemuuid.lib")

static WindowsEdition GetEdition(const std::string& edition)
{
    if (edition == "Professional")
        return WindowsEdition::Pro;

    if (edition == "Core")
        return WindowsEdition::Home;

    if (edition == "Enterprise")
        return WindowsEdition::Enterprise;

    if (edition == "Education")
        return WindowsEdition::Education;

    if (edition == "ProfessionalEducation")
        return WindowsEdition::ProEducation;

    if (edition == "ProfessionalWorkstation")
        return WindowsEdition::ProWorkstation;

    return WindowsEdition::Unknown;
}

static std::string ReadRegistryString(
    HKEY root,
    const std::string& subkey,
    const std::string& value)
{
    HKEY key;

    if (RegOpenKeyExA(
            root,
            subkey.c_str(),
            0,
            KEY_READ,
            &key) != ERROR_SUCCESS)
    {
        return "";
    }

    char buffer[512];

    DWORD size = sizeof(buffer);

    if (RegQueryValueExA(
            key,
            value.c_str(),
            nullptr,
            nullptr,
            reinterpret_cast<LPBYTE>(buffer),
            &size) != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return "";
    }

    RegCloseKey(key);

    return buffer;
}

static void FillOSInfo(WindowsInfo& windows)
{
    constexpr const char* path =
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

    windows.ProductName =
        ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            path,
            "ProductName");

    std::string edition =
        ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            path,
            "EditionID");

    windows.Edition =
        GetEdition(edition);

    windows.Version =
        ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            path,
            "CurrentVersion");

    windows.Build =
        ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            path,
            "CurrentBuild");

    windows.DisplayVersion =
        ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            path,
            "DisplayVersion");

    windows.ReleaseID =
        ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            path,
            "ReleaseId");
}
static void FillInstallInfo(WindowsInfo& windows)
{
    IWbemLocator* locator = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&locator);

    if (FAILED(hr))
        return;

    IWbemServices* services = nullptr;

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

    IEnumWbemClassObject* enumerator = nullptr;

    hr = services->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT InstallDate, LastBootUpTime FROM Win32_OperatingSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        services->Release();
        locator->Release();
        return;
    }

    IWbemClassObject* object = nullptr;
    ULONG returned = 0;

    if (enumerator->Next(
            WBEM_INFINITE,
            1,
            &object,
            &returned) == WBEM_S_NO_ERROR)
    {
        VARIANT value;
        VariantInit(&value);

        if (SUCCEEDED(object->Get(
                L"InstallDate",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                windows.InstallDate =
                    _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"LastBootUpTime",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                windows.BootTime =
                    _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        object->Release();
    }

    enumerator->Release();
    services->Release();
    locator->Release();

    // Calculate uptime
    ULONGLONG ms = GetTickCount64();

    windows.UptimeSeconds =
        static_cast<uint64_t>(ms / 1000);
}

static void FillUserInfo(WindowsInfo& windows)
{
    // ------------------------------
    // Computer Name
    // ------------------------------

    char computerName[MAX_COMPUTERNAME_LENGTH + 1];

    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

    if (GetComputerNameA(computerName, &size))
    {
        windows.ComputerName = computerName;
    }

    // ------------------------------
    // Current User
    // ------------------------------

    char userName[256];

    size = sizeof(userName);

    if (GetUserNameA(userName, &size))
    {
        windows.CurrentUser = userName;
    }

    // ------------------------------
    // Registered Owner
    // ------------------------------

    windows.RegisteredOwner =
        ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            "RegisteredOwner");

    // ------------------------------
    // Organization
    // ------------------------------

    windows.Organization =
        ReadRegistryString(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            "RegisteredOrganization");
}


static void FillArchitectureInfo(WindowsInfo& windows)
{
    // ------------------------------
    // Is 64-bit Windows
    // ------------------------------

#if defined(_WIN64)

    windows.Is64Bit = true;

#else

    BOOL isWow64 = FALSE;

    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(
        HANDLE,
        PBOOL);

    LPFN_ISWOW64PROCESS fn =
        (LPFN_ISWOW64PROCESS)GetProcAddress(
            GetModuleHandleA("kernel32"),
            "IsWow64Process");

    if (fn)
    {
        fn(GetCurrentProcess(), &isWow64);
    }

    windows.Is64Bit = isWow64;

#endif

    // ------------------------------
    // Windows Directory
    // ------------------------------

    char buffer[MAX_PATH];

    UINT len =
        GetWindowsDirectoryA(
            buffer,
            MAX_PATH);

    if (len > 0)
    {
        windows.WindowsDirectory = buffer;
    }

    // ------------------------------
    // System Directory
    // ------------------------------

    len =
        GetSystemDirectoryA(
            buffer,
            MAX_PATH);

    if (len > 0)
    {
        windows.SystemDirectory = buffer;
    }
}
static DWORD ReadRegistryDWORD(
    HKEY root,
    const std::string& subkey,
    const std::string& value)
{
    HKEY key;

    if (RegOpenKeyExA(
            root,
            subkey.c_str(),
            0,
            KEY_READ,
            &key) != ERROR_SUCCESS)
    {
        return 0;
    }

    DWORD data = 0;
    DWORD size = sizeof(DWORD);

    if (RegQueryValueExA(
            key,
            value.c_str(),
            nullptr,
            nullptr,
            reinterpret_cast<LPBYTE>(&data),
            &size) != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return 0;
    }

    RegCloseKey(key);

    return data;
}
static void FillStatusInfo(WindowsInfo& windows)
{
    // ------------------------------
    // Safe Mode
    // ------------------------------

    windows.SafeMode =
        (GetSystemMetrics(SM_CLEANBOOT) != 0);

    // ------------------------------
    // Fast Startup
    // ------------------------------

    windows.FastStartupEnabled =
        ReadRegistryDWORD(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Power",
            "HiberbootEnabled") != 0;

    // ------------------------------
    // Hibernation
    // ------------------------------

    windows.HibernationEnabled =
        ReadRegistryDWORD(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Power",
            "HibernateEnabled") != 0;

    // ------------------------------
    // Remote Desktop
    // ------------------------------

    windows.RemoteDesktopEnabled =
        ReadRegistryDWORD(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Terminal Server",
            "fDenyTSConnections") == 0;
}

static void FillActivationInfo(WindowsInfo& windows)
{
    IEnumWbemClassObject* enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT LicenseStatus, Description, ID FROM SoftwareLicensingProduct WHERE PartialProductKey IS NOT NULL"),
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
        uint32_t status = 0;

        if (GetWMIProperty(object, L"LicenseStatus", status))
        {
            windows.Activation.Activated = (status == 1);

            switch (status)
            {
            case 0:
                windows.Activation.LicenseStatus = "Unlicensed";
                break;

            case 1:
                windows.Activation.LicenseStatus = "Licensed";
                break;

            case 2:
                windows.Activation.LicenseStatus = "OOB Grace";
                break;

            case 3:
                windows.Activation.LicenseStatus = "OOT Grace";
                break;

            case 4:
                windows.Activation.LicenseStatus = "Non Genuine";
                break;

            case 5:
                windows.Activation.LicenseStatus = "Notification";
                break;

            default:
                windows.Activation.LicenseStatus = "Unknown";
                break;
            }
        }

        std::string description;

        if (GetWMIProperty(object, L"Description", description))
        {
            if (description.find("OEM") != std::string::npos)
                windows.Activation.ProductKeyChannel = "OEM";
            else if (description.find("Retail") != std::string::npos)
                windows.Activation.ProductKeyChannel = "Retail";
            else if (description.find("VOLUME") != std::string::npos)
                windows.Activation.ProductKeyChannel = "Volume";
            else
                windows.Activation.ProductKeyChannel = "Unknown";
        }

        GetWMIProperty(
            object,
            L"ID",
            windows.Activation.ActivationID);

        object->Release();
    }

    enumerator->Release();
}

WindowsInfo GetWindowsInfo()
{
    WindowsInfo windows;

    FillOSInfo(windows);
    FillInstallInfo(windows);
    FillUserInfo(windows);
    FillArchitectureInfo(windows);
    FillStatusInfo(windows);
    FillActivationInfo(windows);

    return windows;
}