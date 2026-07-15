#include "WMIHelper.hpp"

IWbemLocator* gLocator = nullptr;
IWbemServices* gService = nullptr;
// --------------------------------- COM

bool InitializeWMI()
{
    HRESULT hr;

    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return false;

    hr = CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE,
        nullptr);

    if (FAILED(hr) && hr != RPC_E_TOO_LATE)
    {
        CoUninitialize();
        return false;
    }

    hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<LPVOID *>(&gLocator));

    if (FAILED(hr))
    {
        CoUninitialize();
        return false;
    }

    hr = gLocator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &gService);

    if (FAILED(hr))
    {
        gLocator->Release();
        gLocator = nullptr;

        CoUninitialize();
        return false;
    }

    hr = CoSetProxyBlanket(
        gService,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);

    if (FAILED(hr))
    {
        gService->Release();
        gLocator->Release();

        gService = nullptr;
        gLocator = nullptr;

        CoUninitialize();

        return false;
    }

    return true;
}
void ShutdownWMI()
{
    if (gService)
    {
        gService->Release();
        gService = nullptr;
    }

    if (gLocator)
    {
        gLocator->Release();
        gLocator = nullptr;
    }

    CoUninitialize();
}
bool ReadRegistryDWORD(
    HKEY root,
    const char* subKey,
    const char* valueName,
    DWORD& value)
{
    HKEY hKey;

    if (RegOpenKeyExA(
            root,
            subKey,
            0,
            KEY_READ,
            &hKey) != ERROR_SUCCESS)
    {
        return false;
    }

    DWORD type = REG_DWORD;
    DWORD size = sizeof(DWORD);

    LONG result = RegQueryValueExA(
        hKey,
        valueName,
        nullptr,
        &type,
        reinterpret_cast<LPBYTE>(&value),
        &size);

    RegCloseKey(hKey);

    return result == ERROR_SUCCESS;
}