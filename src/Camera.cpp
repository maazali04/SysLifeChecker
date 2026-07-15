#define INITGUID
#include "Camera.hpp"
#include "WMIHelper.hpp"

#include <windows.h>

#include <SetupAPI.h>
#include <devguid.h>

#include <cfgmgr32.h>
#include <WbemIdl.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <cstdlib>
#include <algorithm>

#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfreadwrite.lib")

#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "wbemuuid.lib")

static void FillCameraDevices(CameraSystemInfo &cameras)
{
    HDEVINFO deviceInfo =
        SetupDiGetClassDevs(
            &GUID_DEVCLASS_IMAGE,
            nullptr,
            nullptr,
            DIGCF_PRESENT);

    if (deviceInfo == INVALID_HANDLE_VALUE)
        return;

    SP_DEVINFO_DATA deviceData{};
    deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD index = 0;
         SetupDiEnumDeviceInfo(
             deviceInfo,
             index,
             &deviceData);
         ++index)
    {
        CameraInfo camera;

        char buffer[512];

        //---------------------------------------
        // Name
        //---------------------------------------

        if (SetupDiGetDeviceRegistryPropertyA(
                deviceInfo,
                &deviceData,
                SPDRP_FRIENDLYNAME,
                nullptr,
                reinterpret_cast<PBYTE>(buffer),
                sizeof(buffer),
                nullptr))
        {
            camera.Name = buffer;
        }
        else if (SetupDiGetDeviceRegistryPropertyA(
                     deviceInfo,
                     &deviceData,
                     SPDRP_DEVICEDESC,
                     nullptr,
                     reinterpret_cast<PBYTE>(buffer),
                     sizeof(buffer),
                     nullptr))
        {
            camera.Name = buffer;
        }

        //---------------------------------------
        // Manufacturer
        //---------------------------------------

        if (SetupDiGetDeviceRegistryPropertyA(
                deviceInfo,
                &deviceData,
                SPDRP_MFG,
                nullptr,
                reinterpret_cast<PBYTE>(buffer),
                sizeof(buffer),
                nullptr))
        {
            camera.Manufacturer = buffer;
        }

        //---------------------------------------
        // Device ID
        //---------------------------------------

        char instanceID[MAX_DEVICE_ID_LEN];

        if (CM_Get_Device_IDA(
                deviceData.DevInst,
                instanceID,
                MAX_DEVICE_ID_LEN,
                0) == CR_SUCCESS)
        {
            camera.DeviceID = instanceID;
        }

        //---------------------------------------
        // Connected / Enabled
        //---------------------------------------

        ULONG status;
        ULONG problem;

        if (CM_Get_DevNode_Status(
                &status,
                &problem,
                deviceData.DevInst,
                0) == CR_SUCCESS)
        {
            camera.Connected =
                !(status & DN_DEVICE_DISCONNECTED);

            camera.Enabled =
                !(status & DN_HAS_PROBLEM);
        }

        cameras.Cameras.push_back(camera);
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
}

static void FillDriverInfo(CameraSystemInfo &cameras)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PnPSignedDriver"),
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
        std::string deviceName;
        std::string deviceID;
        std::string driverVersion;

        GetWMIProperty(
            object,
            L"DeviceName",
            deviceName);

        GetWMIProperty(
            object,
            L"DeviceID",
            deviceID);

        GetWMIProperty(
            object,
            L"DriverVersion",
            driverVersion);

        for (auto &camera : cameras.Cameras)
        {
            if ((!camera.DeviceID.empty() &&
                 camera.DeviceID == deviceID) ||
                (!camera.Name.empty() &&
                 (camera.Name.find(deviceName) != std::string::npos ||
                  deviceName.find(camera.Name) != std::string::npos)))
            {
                camera.DriverVersion = driverVersion;
                break;
            }
        }

        object->Release();
    }

    enumerator->Release();
}

static void FillCapabilities(CameraSystemInfo &cameras)
{
    HRESULT hr = MFStartup(MF_VERSION);

    if (FAILED(hr))
        return;

    IMFAttributes *attributes = nullptr;

    hr = MFCreateAttributes(&attributes, 1);

    if (FAILED(hr))
    {
        MFShutdown();
        return;
    }

    attributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

    IMFActivate **devices = nullptr;

    UINT32 count = 0;

    hr = MFEnumDeviceSources(
        attributes,
        &devices,
        &count);

    if (FAILED(hr))
    {
        attributes->Release();
        MFShutdown();
        return;
    }

    for (UINT32 i = 0;
         i < count && i < cameras.Cameras.size();
         i++)
    {
        IMFMediaSource *source = nullptr;

        if (FAILED(devices[i]->ActivateObject(
                IID_PPV_ARGS(&source))))
            continue;

        IMFSourceReader *reader = nullptr;

        if (FAILED(MFCreateSourceReaderFromMediaSource(
                source,
                nullptr,
                &reader)))
        {
            source->Release();
            continue;
        }

        DWORD index = 0;

        while (true)
        {
            IMFMediaType *mediaType = nullptr;

            hr = reader->GetNativeMediaType(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                index,
                &mediaType);

            if (FAILED(hr))
                break;

            UINT32 width = 0;
            UINT32 height = 0;

            MFGetAttributeSize(
                mediaType,
                MF_MT_FRAME_SIZE,
                &width,
                &height);

            UINT32 num = 0;
            UINT32 den = 1;

            MFGetAttributeRatio(
                mediaType,
                MF_MT_FRAME_RATE,
                &num,
                &den);

            CameraInfo &camera =
                cameras.Cameras[i];

            if (width > camera.MaxWidth)
                camera.MaxWidth = width;

            if (height > camera.MaxHeight)
                camera.MaxHeight = height;

            UINT32 fps =
                den ? num / den : 0;

            if (fps > camera.MaxFPS)
                camera.MaxFPS = fps;

            mediaType->Release();

            index++;
        }

        reader->Release();
        source->Release();
    }

    for (UINT32 i = 0; i < count; i++)
        devices[i]->Release();

    CoTaskMemFree(devices);

    attributes->Release();

    MFShutdown();
}

static void FillFeatures(CameraSystemInfo &cameras)
{
    for (auto &camera : cameras.Cameras)
    {
        std::string lower = camera.Name;

        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            ::tolower);

        //---------------------------------------
        // Built-in Camera
        //---------------------------------------

        if (lower.find("integrated") != std::string::npos ||
            lower.find("internal") != std::string::npos ||
            lower.find("builtin") != std::string::npos ||
            lower.find("built-in") != std::string::npos)
        {
            camera.BuiltIn = true;
        }

        //---------------------------------------
        // Infrared Camera
        //---------------------------------------

        if (lower.find("infrared") != std::string::npos ||
            lower.find("ir") != std::string::npos)
        {
            camera.InfraredCamera = true;
        }

        //---------------------------------------
        // Windows Hello (Best Effort)
        //---------------------------------------

        if (camera.InfraredCamera)
        {
            camera.SupportsWindowsHello = true;
        }

        //---------------------------------------
        // Privacy Shutter
        //---------------------------------------

        // Windows has no standard API to detect a physical
        // privacy shutter, so leave it false unless you later
        // add vendor-specific support.

        camera.PrivacyShutter = false;
    }
}

CameraSystemInfo GetCameraInfo()
{
    CameraSystemInfo cameras;

    FillCameraDevices(cameras);
    FillDriverInfo(cameras);
    FillCapabilities(cameras);
    FillFeatures(cameras);

    cameras.TotalCameras =
        static_cast<uint32_t>(cameras.Cameras.size());

    return cameras;
}
