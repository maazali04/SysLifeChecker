#pragma once

#include <windows.h>
#include <Wbemidl.h>
#include <string>
#include <comdef.h>
#include <type_traits>
#include <cstdint>
#include "SystemInfo.hpp"

extern IWbemLocator *gLocator;
extern IWbemServices *gService;

bool InitializeWMI();
void ShutdownWMI();

template <typename T>
bool GetWMIProperty(
    IWbemClassObject *object,
    const wchar_t *property,
    T &value)
{
    VARIANT vt;
    VariantInit(&vt);

    HRESULT hr = object->Get(property, 0, &vt, nullptr, nullptr);

    if (FAILED(hr))
    {
        VariantClear(&vt);
        return false;
    }

    if constexpr (std::is_same_v<T, std::string>)
    {
        if (vt.vt == VT_BSTR)
            value = _bstr_t(vt.bstrVal);
        else
        {
            VariantClear(&vt);
            return false;
        }
    }
    else if constexpr (std::is_same_v<T, uint64_t>)
    {
        if (vt.vt == VT_BSTR)
            value = _wcstoui64(vt.bstrVal, nullptr, 10);
        else if (vt.vt == VT_UI8)
            value = vt.ullVal;
        else
        {
            VariantClear(&vt);
            return false;
        }
    }
    else if constexpr (std::is_same_v<T, uint32_t>)
    {
        if (vt.vt == VT_UI4 || vt.vt == VT_I4)
            value = vt.uintVal;
        else
        {
            VariantClear(&vt);
            return false;
        }
    }
    else if constexpr (std::is_same_v<T, uint16_t>)
    {
        if (vt.vt == VT_UI2 || vt.vt == VT_I2)
            value = vt.uiVal;
        else
        {
            VariantClear(&vt);
            return false;
        }
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        if (vt.vt == VT_BOOL)
            value = (vt.boolVal == VARIANT_TRUE);
        else
        {
            VariantClear(&vt);
            return false;
        }
    }

    VariantClear(&vt);
    return true;
}

bool ReadRegistryDWORD(
    HKEY root,
    const char *subKey,
    const char *valueName,
    DWORD &value);