#include "Security.hpp"
#include "WMIHelper.hpp"
#include <netfw.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

static void FillTPMInfo(SecurityInfo& security);
static void FillSecureBootInfo(SecurityInfo& security);
static void FillDefenderInfo(SecurityInfo& security);
static void FillInstalledAntivirus(SecurityInfo& security);
static void FillFirewallInfo(SecurityInfo& security);
static void FillEncryptionInfo(SecurityInfo& security);
static void FillWindowsSecurity(SecurityInfo& security);
static void FillUsers(SecurityInfo& security);

SecurityInfo GetSecurityInfo()
{
    SecurityInfo security;

    FillTPMInfo(security);
    FillSecureBootInfo(security);
    FillDefenderInfo(security);
    FillInstalledAntivirus(security);
    FillFirewallInfo(security);
    FillEncryptionInfo(security);
    FillWindowsSecurity(security);
    FillUsers(security);

    return security;
}

static void FillTPMInfo(SecurityInfo& security)
{
    IWbemLocator* locator = nullptr;
    IWbemServices* service = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<void**>(&locator));

    if (FAILED(hr))
        return;

    hr = locator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2\\Security\\MicrosoftTpm"),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &service);

    if (FAILED(hr))
    {
        locator->Release();
        return;
    }

    CoSetProxyBlanket(
        service,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;

    hr = service->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_Tpm"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        service->Release();
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
        security.TPMPresent = true;

        GetWMIProperty(
            object,
            L"ManufacturerVersion",
            security.TPMFirmwareVersion);

        GetWMIProperty(
            object,
            L"ManufacturerIdTxt",
            security.TPMManufacturer);

        bool enabled = false;

        GetWMIProperty(
            object,
            L"IsEnabled_InitialValue",
            enabled);

        security.TPMEnabled = enabled;

        bool activated = false;

        GetWMIProperty(
            object,
            L"IsActivated_InitialValue",
            activated);

        security.TPMReady =
            enabled && activated;

        std::string spec;

        GetWMIProperty(
            object,
            L"SpecVersion",
            spec);

        if (spec.find("2.0") != std::string::npos)
            security.TPM = TPMVersion::TPM20;
        else if (spec.find("1.2") != std::string::npos)
            security.TPM = TPMVersion::TPM12;
        else
            security.TPM = TPMVersion::Unknown;

        object->Release();
    }

    enumerator->Release();
    service->Release();
    locator->Release();
}

static void FillSecureBootInfo(SecurityInfo& security)
{
    HKEY hKey;

    if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State",
            0,
            KEY_READ,
            &hKey) == ERROR_SUCCESS)
    {
        DWORD value = 0;
        DWORD size = sizeof(DWORD);

        if (RegQueryValueExA(
                hKey,
                "UEFISecureBootEnabled",
                nullptr,
                nullptr,
                reinterpret_cast<LPBYTE>(&value),
                &size) == ERROR_SUCCESS)
        {
            security.SecureBootSupported = true;
            security.SecureBootEnabled = (value != 0);
        }

        RegCloseKey(hKey);
    }
    else
    {
        security.SecureBootSupported = false;
        security.SecureBootEnabled = false;
    }
}
static void FillDefenderInfo(SecurityInfo& security)
{
    IWbemLocator* locator = nullptr;
    IWbemServices* service = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<void**>(&locator));

    if (FAILED(hr))
        return;

    hr = locator->ConnectServer(
        _bstr_t(L"ROOT\\SecurityCenter2"),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &service);

    if (FAILED(hr))
    {
        locator->Release();
        return;
    }

    CoSetProxyBlanket(
        service,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;

    hr = service->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM AntiVirusProduct"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        service->Release();
        locator->Release();
        return;
    }

    IWbemClassObject* object = nullptr;
    ULONG returned = 0;

    while (enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == WBEM_S_NO_ERROR)
    {
        std::string name;

        GetWMIProperty(
            object,
            L"displayName",
            name);

        if (name.find("Defender") != std::string::npos)
        {
            security.Defender.Name = name;

            uint32_t state = 0;

            GetWMIProperty(
                object,
                L"productState",
                state);

            // Basic decoding
            security.Defender.Enabled =
                (state & 0x1000) != 0;

            security.Defender.RealTimeProtection =
                (state & 0x0010) != 0;

            security.Defender.UpToDate =
                (state & 0x000010) == 0;

            object->Release();
            break;
        }

        object->Release();
    }

    enumerator->Release();
    service->Release();
    locator->Release();
}

static void FillInstalledAntivirus(SecurityInfo& security)
{
    IWbemLocator* locator = nullptr;
    IWbemServices* service = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<void**>(&locator));

    if (FAILED(hr))
        return;

    hr = locator->ConnectServer(
        _bstr_t(L"ROOT\\SecurityCenter2"),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &service);

    if (FAILED(hr))
    {
        locator->Release();
        return;
    }

    CoSetProxyBlanket(
        service,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;

    hr = service->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM AntiVirusProduct"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        service->Release();
        locator->Release();
        return;
    }

    IWbemClassObject* object = nullptr;
    ULONG returned = 0;

    while (enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == WBEM_S_NO_ERROR)
    {
        AntivirusInfo antivirus;

        GetWMIProperty(
            object,
            L"displayName",
            antivirus.Name);

        uint32_t state = 0;

        GetWMIProperty(
            object,
            L"productState",
            state);

        //---------------------------------
        // Decode state (basic)
        //---------------------------------

        antivirus.Enabled =
            (state & 0x1000) != 0;

        antivirus.RealTimeProtection =
            (state & 0x0010) != 0;

        antivirus.UpToDate =
            (state & 0x000010) == 0;

        security.InstalledAntivirus.push_back(antivirus);

        object->Release();
    }

    enumerator->Release();
    service->Release();
    locator->Release();
}



static void FillFirewallInfo(SecurityInfo& security)
{
    INetFwPolicy2* policy = nullptr;

    HRESULT hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        reinterpret_cast<void**>(&policy));

    if (FAILED(hr))
        return;

    VARIANT_BOOL enabled = VARIANT_FALSE;

    //-----------------------------
    // Domain
    //-----------------------------

    if (SUCCEEDED(policy->get_FirewallEnabled(
        NET_FW_PROFILE2_DOMAIN,
        &enabled)))
    {
        security.Firewall.DomainProfile =
            (enabled == VARIANT_TRUE);
    }

    //-----------------------------
    // Private
    //-----------------------------

    if (SUCCEEDED(policy->get_FirewallEnabled(
        NET_FW_PROFILE2_PRIVATE,
        &enabled)))
    {
        security.Firewall.PrivateProfile =
            (enabled == VARIANT_TRUE);
    }

    //-----------------------------
    // Public
    //-----------------------------

    if (SUCCEEDED(policy->get_FirewallEnabled(
        NET_FW_PROFILE2_PUBLIC,
        &enabled)))
    {
        security.Firewall.PublicProfile =
            (enabled == VARIANT_TRUE);
    }

    //-----------------------------
    // Overall
    //-----------------------------

    security.Firewall.Enabled =
        security.Firewall.DomainProfile ||
        security.Firewall.PrivateProfile ||
        security.Firewall.PublicProfile;

    policy->Release();
}

static void FillEncryptionInfo(SecurityInfo& security)
{
    IWbemLocator* locator = nullptr;
    IWbemServices* service = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<void**>(&locator));

    if (FAILED(hr))
        return;

    hr = locator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2\\Security\\MicrosoftVolumeEncryption"),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &service);

    if (FAILED(hr))
    {
        locator->Release();
        return;
    }

    CoSetProxyBlanket(
        service,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;

    hr = service->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_EncryptableVolume"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        service->Release();
        locator->Release();
        return;
    }

    IWbemClassObject* object = nullptr;
    ULONG returned = 0;

    while (enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == WBEM_S_NO_ERROR)
    {
        DriveEncryptionInfo drive;

        GetWMIProperty(
            object,
            L"DriveLetter",
            drive.DriveLetter);

        uint32_t protectionStatus = 0;

        GetWMIProperty(
            object,
            L"ProtectionStatus",
            protectionStatus);

        drive.Encrypted = (protectionStatus == 1);

        if (drive.Encrypted)
            drive.Type = DriveEncryptionType::BitLocker;
        else
            drive.Type = DriveEncryptionType::None;

        //----------------------------------------
        // Encryption Percentage
        //----------------------------------------


        drive.EncryptionPercent =
            drive.Encrypted ? 100.0 : 0.0;

        security.EncryptedDrives.push_back(drive);

        object->Release();
    }

    enumerator->Release();
    service->Release();
    locator->Release();
}

static void FillWindowsSecurity(SecurityInfo& security)
{
    DWORD value = 0;

    //----------------------------------------
    // UAC
    //----------------------------------------

    if (ReadRegistryDWORD(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
            "EnableLUA",
            value))
    {
        security.UserAccountControlEnabled = (value != 0);
    }

    //----------------------------------------
    // SmartScreen
    //----------------------------------------

    if (ReadRegistryDWORD(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer",
            "SmartScreenEnabled",
            value))
    {
        security.SmartScreenEnabled = (value != 0);
    }

    //----------------------------------------
    // Memory Integrity (HVCI)
    //----------------------------------------

    if (ReadRegistryDWORD(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\DeviceGuard\\Scenarios\\HypervisorEnforcedCodeIntegrity",
            "Enabled",
            value))
    {
        security.MemoryIntegrityEnabled = (value != 0);
    }

    //----------------------------------------
    // Device Guard
    //----------------------------------------

    if (ReadRegistryDWORD(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\DeviceGuard",
            "EnableVirtualizationBasedSecurity",
            value))
    {
        security.DeviceGuardEnabled = (value != 0);
        security.VirtualizationBasedSecurity = (value != 0);
    }

    //----------------------------------------
    // Credential Guard
    //----------------------------------------

    if (ReadRegistryDWORD(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Lsa",
            "LsaCfgFlags",
            value))
    {
        security.CredentialGuardEnabled = (value != 0);
    }

    //----------------------------------------
    // Core Isolation
    //----------------------------------------

    security.CoreIsolationEnabled =
        security.MemoryIntegrityEnabled;
}

static void FillUsers(SecurityInfo& security)
{
    IEnumWbemClassObject* enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_UserAccount WHERE LocalAccount=True"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
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
        UserAccountInfo user;

        GetWMIProperty(
            object,
            L"Name",
            user.Username);

        GetWMIProperty(
            object,
            L"FullName",
            user.FullName);

        GetWMIProperty(
            object,
            L"Disabled",
            user.AccountDisabled);

        GetWMIProperty(
            object,
            L"Lockout",
            user.AccountLocked);

        GetWMIProperty(
            object,
            L"PasswordRequired",
            user.PasswordRequired);

        GetWMIProperty(
            object,
            L"PasswordExpires",
            user.PasswordExpires);

        bool admin = false;

        VARIANT vt;
        VariantInit(&vt);

        if (SUCCEEDED(object->Get(
                L"SIDType",
                0,
                &vt,
                nullptr,
                nullptr)))
        {
            // Basic approximation.
            // Proper administrator detection would require
            // checking local group membership.

            admin = (vt.uintVal == 1);
        }

        VariantClear(&vt);

        user.Administrator = admin;

        security.Users.push_back(user);

        object->Release();
    }

    enumerator->Release();
}