#include "Motherboard.hpp"
#include "WMIHelper.hpp"

#include <windows.h>
#include <Wbemidl.h>
#include <comdef.h>

#pragma comment(lib, "wbemuuid.lib")

std::string FormFactorToString(MotherboardFormFactor form)
{
    switch (form)
    {
    case MotherboardFormFactor::ATX:
        return "ATX";
    case MotherboardFormFactor::MicroATX:
        return "MicroATX";
    case MotherboardFormFactor::MiniITX:
        return "MiniITX";
    case MotherboardFormFactor::EATX:
        return "EATX";
    case MotherboardFormFactor::XLATX:
        return "XLATX";
    case MotherboardFormFactor::NanoITX:
        return "NanoITX";
    case MotherboardFormFactor::PicoITX:
        return "PicoITX";
    case MotherboardFormFactor::Proprietary:
        return "Proprietary";
    default:
        return "Unknown";
    }
}

static void FillBaseBoard(MotherboardInfo &board)
{
    if (!gService)
        return;
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_BaseBoard"),
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

        if (object)
        {
            object->Release();
            object = nullptr;
        }
    }

    if (enumerator)
        enumerator->Release();
}
static void FillComputerSystemProduct(MotherboardInfo &board)
{
    if (!gService)
        return;

    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_ComputerSystemProduct"),
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

        
        if (object)
        {
            object->Release();
            object = nullptr;
        }
    }

    if (enumerator)
        enumerator->Release();
}

static void FillMemoryArray(MotherboardInfo &board)
{
    if (!gService)
        return;
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PhysicalMemoryArray"),
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

                switch (ecc)
                {
                case 4:
                case 5:
                case 6:
                case 7:
                    board.ECCSupported = true;
                    break;

                default:
                    board.ECCSupported = false;
                    break;
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

MotherboardInfo GetMotherboardInfo()
{
    MotherboardInfo board;

    FillBaseBoard(board);
    FillComputerSystemProduct(board);
    FillMemoryArray(board);

    return board;
}