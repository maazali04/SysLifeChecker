#include "BIOS.hpp"
#include "WMIHelper.hpp"

#include <windows.h>
#include <Wbemidl.h>
#include <comdef.h>

#pragma comment(lib, "wbemuuid.lib")

static void FillBIOS(BIOSInfo &bios)
{
    if (!gService)
        return;

    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_BIOS"),
        WBEM_FLAG_FORWARD_ONLY |
            WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
    {

        return;
    }

    IWbemClassObject *object = nullptr;
    ULONG returned = 0;

    if (enumerator->Next(
            WBEM_INFINITE,
            1,
            &object,
            &returned) == WBEM_S_NO_ERROR &&
        returned > 0)
    {
        VARIANT value;
        VariantInit(&value);

        if (SUCCEEDED(object->Get(L"Manufacturer", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                bios.Vendor = _bstr_t(value.bstrVal);
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
                bios.SMBIOSVersion = _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(L"SMBIOSBIOSVersion", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                bios.Version = _bstr_t(value.bstrVal);
            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(L"ReleaseDate", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                bios.ReleaseDate = _bstr_t(value.bstrVal);
            VariantClear(&value);
        }
        if (SUCCEEDED(object->Get(L"SerialNumber", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                bios.SerialNumber = _bstr_t(value.bstrVal);

            VariantClear(&value);
        }

        if (SUCCEEDED(object->Get(L"Description", 0, &value, nullptr, nullptr)))
        {
            if (value.vt == VT_BSTR)
                bios.Description = _bstr_t(value.bstrVal);

            VariantClear(&value);
        }


        if (SUCCEEDED(object->Get(L"BIOSVersion", 0, &value, nullptr, nullptr)))
{
    if ((value.vt & VT_ARRAY) && (value.vt & VT_BSTR))
    {
        SAFEARRAY* sa = value.parray;
        LONG lBound, uBound;

        SafeArrayGetLBound(sa, 1, &lBound);
        SafeArrayGetUBound(sa, 1, &uBound);

        if (lBound <= uBound)
        {
            BSTR str;
            SafeArrayGetElement(sa, &lBound, &str);

            bios.Version = _bstr_t(str);

            SysFreeString(str);
        }
    }

    VariantClear(&value);
}

        if (object)
        {
            object->Release();
            object = nullptr;
        }
    }

    if (enumerator)
        enumerator->Release();
}

BIOSInfo GetBIOSInfo()
{
    BIOSInfo bios;

    FillBIOS(bios);

    return bios;
}