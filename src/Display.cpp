#include "Display.hpp"
#include "WMIHelper.hpp"

#include <cmath>
#include <algorithm>
#include <cctype>
#include <windows.h>
#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")

static void FillDisplayDevices(DisplaySystemInfo &displays);
static void FillDisplaySettings(DisplaySystemInfo &displays);
static void FillMonitorInfo(DisplaySystemInfo &displays);
static void FillBrightness(DisplaySystemInfo &displays);
static void FillAdvancedFeatures(DisplaySystemInfo &displays);
static void CalculateSummary(DisplaySystemInfo &displays);

DisplaySystemInfo GetDisplayInfo()
{
    DisplaySystemInfo displays;

    FillDisplayDevices(displays);
    FillDisplaySettings(displays);
    FillMonitorInfo(displays);
    FillBrightness(displays);
    FillAdvancedFeatures(displays);
    CalculateSummary(displays);

    return displays;
}

static void FillDisplayDevices(DisplaySystemInfo &displays)
{
    DISPLAY_DEVICEA adapter;
    adapter.cb = sizeof(DISPLAY_DEVICEA);

    for (DWORD adapterIndex = 0;
         EnumDisplayDevicesA(nullptr, adapterIndex, &adapter, 0);
         adapterIndex++)
    {
        if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;

        DISPLAY_DEVICEA monitor;
        monitor.cb = sizeof(DISPLAY_DEVICEA);

        for (DWORD monitorIndex = 0;
             EnumDisplayDevicesA(adapter.DeviceName,
                                 monitorIndex,
                                 &monitor,
                                 0);
             monitorIndex++)
        {
            DisplayInfo display;

            //----------------------------------
            // Identity
            //----------------------------------

            display.Name = adapter.DeviceName;
            display.Model = monitor.DeviceString;
            display.Model = monitor.DeviceString;

            //----------------------------------
            // Status
            //----------------------------------

            display.Connected =
                (monitor.StateFlags & DISPLAY_DEVICE_ACTIVE);

            display.PrimaryDisplay =
                (adapter.StateFlags &
                 DISPLAY_DEVICE_PRIMARY_DEVICE);

            display.Enabled = true;

            //----------------------------------
            // Internal / External
            //----------------------------------

            std::string id = monitor.DeviceID;

            display.InternalDisplay =
                id.find("LCD") != std::string::npos ||
                id.find("DISPLAY") != std::string::npos;

            display.BuiltIn = display.InternalDisplay;

            //----------------------------------
            // Connection Type (Best Guess)
            //----------------------------------

            if (id.find("DISPLAYPORT") != std::string::npos)
            {
                display.Connection =
                    DisplayConnection::DisplayPort;
            }
            else if (id.find("HDMI") != std::string::npos)
            {
                display.Connection =
                    DisplayConnection::HDMI;
            }
            else if (id.find("DVI") != std::string::npos)
            {
                display.Connection =
                    DisplayConnection::DVI;
            }
            else if (id.find("VGA") != std::string::npos)
            {
                display.Connection =
                    DisplayConnection::VGA;
            }
            else if (display.InternalDisplay)
            {
                display.Connection =
                    DisplayConnection::Internal;
            }
            else
            {
                display.Connection =
                    DisplayConnection::Unknown;
            }

            displays.Displays.push_back(display);
        }
    }
}

static void CalculateSummary(DisplaySystemInfo &displays)
{
    displays.TotalDisplays =
        static_cast<uint32_t>(displays.Displays.size());

    displays.ConnectedDisplays = 0;

    for (const auto &display : displays.Displays)
    {
        if (display.Connected)
            displays.ConnectedDisplays++;
    }
}

static void FillDisplaySettings(DisplaySystemInfo &displays)
{
    for (auto &display : displays.Displays)
    {
        DEVMODEA mode{};
        mode.dmSize = sizeof(DEVMODEA);

        if (!EnumDisplaySettingsA(
                display.Name.c_str(),
                ENUM_CURRENT_SETTINGS,
                &mode))
        {
            continue;
        }

        //----------------------------------
        // Resolution
        //----------------------------------

        display.CurrentResolution.Width =
            mode.dmPelsWidth;

        display.CurrentResolution.Height =
            mode.dmPelsHeight;

        //----------------------------------
        // Refresh Rate
        //----------------------------------

        display.Timing.CurrentRefreshRate =
            static_cast<double>(mode.dmDisplayFrequency);

        display.Timing.MaximumRefreshRate =
            display.Timing.CurrentRefreshRate;

        display.Timing.MinimumRefreshRate =
            display.Timing.CurrentRefreshRate;

        //----------------------------------
        // Color Depth
        //----------------------------------

        display.Color.BitsPerPixel =
            mode.dmBitsPerPel;

        display.Color.BitsPerChannel =
            mode.dmBitsPerPel / 3;

        //----------------------------------
        // Orientation
        //----------------------------------

        switch (mode.dmDisplayOrientation)
        {
        case DMDO_DEFAULT:
        case DMDO_180:
            display.ScreenOrientation =
                Orientation::Landscape;
            break;

        case DMDO_90:
        case DMDO_270:
            display.ScreenOrientation =
                Orientation::Portrait;
            break;

        default:
            display.ScreenOrientation =
                Orientation::Landscape;
            break;
        }
    }
}

static void FillMonitorInfo(DisplaySystemInfo &displays)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM WmiMonitorBasicDisplayParams"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
        return;

    IWbemClassObject *object = nullptr;
    ULONG returned = 0;

    size_t index = 0;

    while (enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == WBEM_S_NO_ERROR)
    {
        if (index >= displays.Displays.size())
        {
            object->Release();
            break;
        }

        DisplayInfo &display = displays.Displays[index];

        uint32_t width = 0;
        uint32_t height = 0;

        GetWMIProperty(
            object,
            L"MaxHorizontalImageSize",
            width);

        GetWMIProperty(
            object,
            L"MaxVerticalImageSize",
            height);

        display.WidthMM =
            static_cast<uint32_t>(width) * 10;

        display.HeightMM =
            static_cast<uint32_t>(height) * 10;

        //-----------------------------------------
        // Screen Size
        //-----------------------------------------

        double w =
            display.WidthMM / 25.4;

        double h =
            display.HeightMM / 25.4;

        display.SizeInches =
            std::sqrt(w * w + h * h);

        //-----------------------------------------
        // PPI
        //-----------------------------------------

        if (display.SizeInches > 0)
        {
            double pixels =
                std::sqrt(
                    display.CurrentResolution.Width *
                        display.CurrentResolution.Width +
                    display.CurrentResolution.Height *
                        display.CurrentResolution.Height);

            display.PPI =
                pixels / display.SizeInches;
        }

        object->Release();

        index++;
    }

    enumerator->Release();
    enumerator = nullptr;

    hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM WmiMonitorID"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
        return;

    index = 0;
    while (enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == WBEM_S_NO_ERROR)
    {
        if (index >= displays.Displays.size())
        {
            object->Release();
            break;
        }

        DisplayInfo &display =
            displays.Displays[index];

        auto ReadStringArray =
            [&](const wchar_t *property)
        {
            VARIANT vt;
            VariantInit(&vt);

            std::string result;

            if (SUCCEEDED(object->Get(property, 0, &vt, nullptr, nullptr)))
            {
                if ((vt.vt & VT_ARRAY) && vt.parray)
                {
                    SAFEARRAY *sa = vt.parray;

                    LONG l, u;

                    SafeArrayGetLBound(sa, 1, &l);
                    SafeArrayGetUBound(sa, 1, &u);

                    for (LONG i = l; i <= u; i++)
                    {
                        USHORT ch;

                        SafeArrayGetElement(sa, &i, &ch);

                        if (ch == 0)
                            break;

                        result.push_back(
                            static_cast<char>(ch));
                    }
                }
            }

            VariantClear(&vt);

            return result;
        };

        std::string manufacturer =
            ReadStringArray(L"ManufacturerName");

        if (!manufacturer.empty())
            display.Manufacturer = manufacturer;

        std::string model =
            ReadStringArray(L"UserFriendlyName");

        if (!model.empty())
            display.Model = model;

        std::string serial =
            ReadStringArray(L"SerialNumberID");

        if (!serial.empty())
            display.SerialNumber = serial;

        object->Release();

        index++;
    }

    enumerator->Release();
}

static void FillBrightness(DisplaySystemInfo& displays)
{
    IEnumWbemClassObject* enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM WmiMonitorBrightness"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (FAILED(hr))
        return;

    IWbemClassObject* object = nullptr;
    ULONG returned = 0;

    size_t index = 0;

    while (enumerator->Next(
               WBEM_INFINITE,
               1,
               &object,
               &returned) == WBEM_S_NO_ERROR)
    {
        if (index >= displays.Displays.size())
        {
            object->Release();
            break;
        }

        DisplayInfo& display = displays.Displays[index];

        uint32_t brightness = 0;

        if (GetWMIProperty(
                object,
                L"CurrentBrightness",
                brightness))
        {
            display.BrightnessPercent = brightness;
        }

        object->Release();

        index++;
    }

    enumerator->Release();
}


static void FillAdvancedFeatures(DisplaySystemInfo& displays)
{
    //------------------------------------------------------
    // Touch Screen
    //------------------------------------------------------

    bool touchSupported =
        (GetSystemMetrics(SM_MAXIMUMTOUCHES) > 0);

    //------------------------------------------------------
    // Pen Support
    //------------------------------------------------------

    bool penSupported =
        (GetSystemMetrics(SM_DIGITIZER) &
         NID_INTEGRATED_PEN) != 0;

    //------------------------------------------------------
    // Touch + Pen
    //------------------------------------------------------

    for (auto& display : displays.Displays)
    {
        display.TouchSupported = touchSupported;

        display.PenSupported = penSupported;
    }

    //------------------------------------------------------
    // Built-in Camera
    //------------------------------------------------------

    bool hasCamera = false;

    IEnumWbemClassObject* enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PnPEntity WHERE PNPClass='Camera'"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (SUCCEEDED(hr))
    {
        IWbemClassObject* object = nullptr;
        ULONG returned = 0;

        if (enumerator->Next(
                WBEM_INFINITE,
                1,
                &object,
                &returned) == WBEM_S_NO_ERROR)
        {
            hasCamera = true;

            object->Release();
        }

        enumerator->Release();
    }

    //------------------------------------------------------
    // Built-in Microphone
    //------------------------------------------------------

    bool hasMicrophone = false;

    enumerator = nullptr;

    hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_SoundDevice"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &enumerator);

    if (SUCCEEDED(hr))
    {
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
                L"Name",
                name);

            std::string lower = name;

            std::transform(
                lower.begin(),
                lower.end(),
                lower.begin(),
                ::tolower);

            if (lower.find("microphone") != std::string::npos ||
                lower.find("mic") != std::string::npos)
            {
                hasMicrophone = true;
            }

            object->Release();
        }

        enumerator->Release();
    }

    //------------------------------------------------------
    // Apply to Displays
    //------------------------------------------------------

    for (auto& display : displays.Displays)
    {
        display.BuiltInCamera = hasCamera;

        display.BuiltInMicrophone = hasMicrophone;
    }
}