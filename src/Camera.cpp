#define INITGUID

#include "Camera.hpp"

#include <windows.h>

#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <devpkey.h>


#include <comdef.h>

#include <algorithm>

#pragma comment(lib,"Setupapi.lib")
#pragma comment(lib,"Cfgmgr32.lib")
#pragma comment(lib,"Ole32.lib")

static void FillCameraHardwareInformation(CameraSystemInfo& cameras)
{
    HDEVINFO deviceInfo = SetupDiGetClassDevs(
        &GUID_DEVCLASS_IMAGE,
        nullptr,
        nullptr,
        DIGCF_PRESENT);

    if (deviceInfo == INVALID_HANDLE_VALUE)
        return;

    SP_DEVINFO_DATA deviceData{};
    deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD index = 0;
         SetupDiEnumDeviceInfo(deviceInfo, index, &deviceData);
         ++index)
    {
        CameraInfo camera;

        char buffer[512];

        //-------------------------------------
        // Name
        //-------------------------------------

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

        //-------------------------------------
        // Manufacturer
        //-------------------------------------

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

        //-------------------------------------
        // Driver Version
        //-------------------------------------

        DEVPROPTYPE propertyType;
        WCHAR versionBuffer[256];

        if (SetupDiGetDevicePropertyW(
                deviceInfo,
                &deviceData,
                &DEVPKEY_Device_DriverVersion,
                &propertyType,
                reinterpret_cast<PBYTE>(versionBuffer),
                sizeof(versionBuffer),
                nullptr,
                0))
        {
            camera.DriverVersion = _bstr_t(versionBuffer);
        }

        //-------------------------------------
        // Device ID
        //-------------------------------------

        char deviceID[MAX_DEVICE_ID_LEN];

        if (CM_Get_Device_IDA(
                deviceData.DevInst,
                deviceID,
                MAX_DEVICE_ID_LEN,
                0) == CR_SUCCESS)
        {
            camera.DeviceID = deviceID;
        }

        //-------------------------------------
        // Connected / Enabled
        //-------------------------------------

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

CameraSystemInfo GetCameraInfo()
{
    CameraSystemInfo cameras;

    FillCameraHardwareInformation(cameras);

    cameras.TotalCameras =
        static_cast<uint32_t>(cameras.Cameras.size());

    return cameras;
}