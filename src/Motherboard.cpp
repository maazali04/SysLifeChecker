#include "Motherboard.hpp"


#include <windows.h>
#include <Wbemidl.h>
#include <comdef.h>

#pragma comment(lib,"wbemuuid.lib")

static void FillBaseBoard(MotherboardInfo& board)
{
    HRESULT hr;

    hr = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED);

    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return;

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

    IWbemLocator* locator = nullptr;

    hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&locator);

    if (FAILED(hr))
    {
        CoUninitialize();
        return;
    }

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
        CoUninitialize();
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
        bstr_t("SELECT * FROM Win32_BaseBoard"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        services->Release();
        locator->Release();
        CoUninitialize();
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
                L"Manufacturer",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.Manufacturer =
                    _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"Product",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
            {
                board.ProductName =
                    _bstr_t(value.bstrVal);

                board.Model =
                    board.ProductName;
            }

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"Version",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.Version =
                    _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"SerialNumber",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.SerialNumber =
                    _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"Tag",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.AssetTag =
                    _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        object->Release();
    }

    enumerator->Release();
    services->Release();
    locator->Release();

    CoUninitialize();
}
static void FillComputerSystemProduct(MotherboardInfo& board)
{
    HRESULT hr;

    hr = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED);

    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return;

    IWbemLocator* locator = nullptr;

    hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&locator);

    if (FAILED(hr))
    {
        CoUninitialize();
        return;
    }

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
        CoUninitialize();
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
        bstr_t("SELECT * FROM Win32_ComputerSystemProduct"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        services->Release();
        locator->Release();
        CoUninitialize();
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
                L"UUID",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.UUID =
                    _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"SKUNumber",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.SKU =
                    _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        object->Release();
    }

    enumerator->Release();
    services->Release();
    locator->Release();

    CoUninitialize();
}

static void FillBIOS(MotherboardInfo& board)
{
    HRESULT hr;

    hr = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED);

    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return;

    IWbemLocator* locator = nullptr;

    hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&locator);

    if (FAILED(hr))
    {
        CoUninitialize();
        return;
    }

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
        CoUninitialize();
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
        bstr_t("SELECT * FROM Win32_BIOS"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        services->Release();
        locator->Release();
        CoUninitialize();
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
                L"Manufacturer",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.BIOSVendor = _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"SMBIOSBIOSVersion",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.BIOSVersion = _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"ReleaseDate",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_BSTR)
                board.BIOSReleaseDate = _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        object->Release();
    }

    enumerator->Release();
    services->Release();
    locator->Release();

    CoUninitialize();
}

static void FillMemoryArray(MotherboardInfo& board)
{
    HRESULT hr;

    hr = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED);

    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return;

    IWbemLocator* locator = nullptr;

    hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&locator);

    if (FAILED(hr))
    {
        CoUninitialize();
        return;
    }

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
        CoUninitialize();
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
        bstr_t("SELECT * FROM Win32_PhysicalMemoryArray"),
        WBEM_FLAG_FORWARD_ONLY |
        WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {
        services->Release();
        locator->Release();
        CoUninitialize();
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
                L"MemoryDevices",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_I4 ||
                value.vt == VT_UI4)
            {
                board.RAMSlots =
                    value.uintVal;
            }

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"MaxCapacity",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_I4 ||
                value.vt == VT_UI4)
            {
                board.MaximumRAMBytes =
                    static_cast<uint64_t>(value.uintVal) * 1024;
            }

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(
                L"MemoryErrorCorrection",
                0,
                &value,
                nullptr,
                nullptr)))
        {
            if (value.vt == VT_I4 ||
                value.vt == VT_UI4)
            {
                uint32_t ecc =
                    value.uintVal;

                board.ECCSupported =
                    (ecc != 3);
            }

            VariantClear(&value);
        }

        object->Release();
    }

    enumerator->Release();
    services->Release();
    locator->Release();

    CoUninitialize();
}


MotherboardInfo GetMotherboardInfo()
{
    MotherboardInfo board;

    FillBaseBoard(board);
    FillComputerSystemProduct(board);
    FillBIOS(board);
    FillMemoryArray(board);

    return board;
}