#define INITGUID

#include "Audio.hpp"

#include <string>
#include <vector>
#include <cstdint>

#include <windows.h>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>

#include <propvarutil.h>
#include <comdef.h>

#include <newdev.h>
#include <devpkey.h>

#include <algorithm>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Newdev.lib")

//==========================================================
// Playback Endpoints
//==========================================================

static void FillPlaybackDevices(AudioInfo &audio)
{
    IMMDeviceEnumerator *enumerator = nullptr;
    IMMDevice *defaultDevice = nullptr;
    std::string defaultEndpointID;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void **>(&enumerator));

    if (FAILED(hr))
        return;

    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(
            eRender,
            eConsole,
            &defaultDevice)))
    {
        LPWSTR id = nullptr;

        if (SUCCEEDED(defaultDevice->GetId(&id)))
        {
            defaultEndpointID = _bstr_t(id);
            CoTaskMemFree(id);
        }

        defaultDevice->Release();
    }
    IMMDeviceCollection *collection = nullptr;

    hr = enumerator->EnumAudioEndpoints(
        eRender,
        DEVICE_STATE_ACTIVE,
        &collection);

    if (FAILED(hr))
    {
        enumerator->Release();
        return;
    }

    UINT count = 0;
    collection->GetCount(&count);

    for (UINT i = 0; i < count; i++)
    {
        IMMDevice *device = nullptr;

        if (FAILED(collection->Item(i, &device)))
            continue;

        AudioDevice audioDevice;

        //-----------------------------------------
        // Endpoint ID
        //-----------------------------------------

        LPWSTR endpointID = nullptr;

        if (SUCCEEDED(device->GetId(&endpointID)))
        {
            audioDevice.EndpointID = _bstr_t(endpointID);
            if (audioDevice.EndpointID == defaultEndpointID)
                audioDevice.DefaultPlayback = true;
            CoTaskMemFree(endpointID);
        }

        //-----------------------------------------
        // Friendly Name
        //-----------------------------------------

        IPropertyStore *properties = nullptr;

        if (SUCCEEDED(device->OpenPropertyStore(
                STGM_READ,
                &properties)))
        {
            PROPVARIANT value;
            PropVariantInit(&value);

            if (SUCCEEDED(properties->GetValue(
                    PKEY_Device_FriendlyName,
                    &value)))
            {
                audioDevice.Name = _bstr_t(value.pwszVal);
            }

            PropVariantClear(&value);

            properties->Release();
        }
        std::string lower = audioDevice.Name;

        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            ::tolower);

        if (lower.find("speaker") != std::string::npos)
            audioDevice.Type = AudioDeviceType::Speaker;

        else if (lower.find("headphone") != std::string::npos)
            audioDevice.Type = AudioDeviceType::Headphones;

        else if (lower.find("hdmi") != std::string::npos)
            audioDevice.Type = AudioDeviceType::HDMIAudio;

        else if (lower.find("bluetooth") != std::string::npos)
            audioDevice.Type = AudioDeviceType::BluetoothAudio;

        else if (lower.find("usb") != std::string::npos)
            audioDevice.Type = AudioDeviceType::USBAudio;

        IAudioEndpointVolume *endpoint = nullptr;

        if (SUCCEEDED(device->Activate(
                __uuidof(IAudioEndpointVolume),
                CLSCTX_ALL,
                nullptr,
                reinterpret_cast<void **>(&endpoint))))
        {
            float volume = 0.0f;
            BOOL muted = FALSE;

            endpoint->GetMasterVolumeLevelScalar(&volume);
            endpoint->GetMute(&muted);

            audioDevice.VolumePercent = volume * 100.0;
            audioDevice.Muted = muted == TRUE;

            endpoint->Release();
        }
        IAudioClient *client = nullptr;

        if (SUCCEEDED(device->Activate(
                __uuidof(IAudioClient),
                CLSCTX_ALL,
                nullptr,
                reinterpret_cast<void **>(&client))))
        {
            WAVEFORMATEX *format = nullptr;

            if (SUCCEEDED(client->GetMixFormat(&format)))
            {
                audioDevice.Channels = format->nChannels;
                audioDevice.SampleRate = format->nSamplesPerSec;
                audioDevice.BitDepth = format->wBitsPerSample;

                CoTaskMemFree(format);
            }

            client->Release();
        }
        audio.Devices.push_back(audioDevice);

        device->Release();
    }

    collection->Release();
    enumerator->Release();
}

//==========================================================
// Hardware Information (Device Manager)
//==========================================================

static void FillHardwareInformation(AudioInfo &audio)
{
    HDEVINFO deviceInfo = SetupDiGetClassDevs(
        &GUID_DEVCLASS_MEDIA,
        nullptr,
        nullptr,
        DIGCF_PRESENT);

    if (deviceInfo == INVALID_HANDLE_VALUE)
        return;

    SP_DEVINFO_DATA deviceData;
    deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

    DWORD index = 0;

    while (SetupDiEnumDeviceInfo(
        deviceInfo,
        index,
        &deviceData))
    {
        index++;

        char buffer[1024];
        DWORD size = 0;

        std::string friendlyName;
        std::string manufacturer;
        std::string driverVersion;

        //-------------------------------------
        // Friendly Name
        //-------------------------------------

        if (SetupDiGetDeviceRegistryPropertyA(
                deviceInfo,
                &deviceData,
                SPDRP_FRIENDLYNAME,
                nullptr,
                reinterpret_cast<PBYTE>(buffer),
                sizeof(buffer),
                &size))
        {
            friendlyName = buffer;
        }
        else if (SetupDiGetDeviceRegistryPropertyA(
                     deviceInfo,
                     &deviceData,
                     SPDRP_DEVICEDESC,
                     nullptr,
                     reinterpret_cast<PBYTE>(buffer),
                     sizeof(buffer),
                     &size))
        {
            friendlyName = buffer;
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
                &size))
        {
            manufacturer = buffer;
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
            driverVersion = _bstr_t(versionBuffer);
        }

        std::string lowerFriendly = friendlyName;

        std::transform(
            lowerFriendly.begin(),
            lowerFriendly.end(),
            lowerFriendly.begin(),
            ::tolower);



        for (auto &audioDevice : audio.Devices)
        {
            std::string lowerEndpoint = audioDevice.Name;

            std::transform(
                lowerEndpoint.begin(),
                lowerEndpoint.end(),
                lowerEndpoint.begin(),
                ::tolower);

            bool match = false;

            // Exact name contains the other
            if (lowerEndpoint.find(lowerFriendly) != std::string::npos ||
                lowerFriendly.find(lowerEndpoint) != std::string::npos)
            {
                match = true;
            }

            // Realtek devices
            else if (lowerFriendly.find("realtek") != std::string::npos &&
                     lowerEndpoint.find("realtek") != std::string::npos)
            {
                match = true;
            }

            // NVIDIA High Definition Audio only
            else if (lowerFriendly.find("nvidia high definition audio") != std::string::npos &&
                     lowerEndpoint.find("high definition audio") != std::string::npos)
            {
                match = true;
            }

            // AMD Streaming Audio only
            else if (lowerFriendly.find("amd streaming audio") != std::string::npos &&
                     lowerEndpoint.find("amd") != std::string::npos)
            {
                match = true;
            }

            if (match)
            {
                audioDevice.Manufacturer = manufacturer;
                audioDevice.DriverVersion = driverVersion;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
}



//==========================================================
// Public Function
//==========================================================

AudioInfo GetAudioInfo()
{
    AudioInfo audio;

    FillPlaybackDevices(audio);

    FillHardwareInformation(audio);

    audio.TotalDevices =
        static_cast<uint32_t>(audio.Devices.size());

    return audio;
}