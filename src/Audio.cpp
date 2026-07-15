#define INITGUID

#include "Audio.hpp"
#include "WMIHelper.hpp"

#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>
#include <algorithm>
#include <cstdlib>
#include <audioclient.h>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Ole32.lib")

static void FillPlaybackDevices(AudioInfo &audio)
{
    IMMDeviceEnumerator *enumerator = nullptr;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void **>(&enumerator));

    if (FAILED(hr))
        return;

    //---------------------------------------
    // Default playback device
    //---------------------------------------

    IMMDevice *defaultDevice = nullptr;
    std::string defaultID;

    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(
            eRender,
            eConsole,
            &defaultDevice)))
    {
        LPWSTR id = nullptr;

        if (SUCCEEDED(defaultDevice->GetId(&id)))
        {
            defaultID = _bstr_t(id);
            CoTaskMemFree(id);
        }

        defaultDevice->Release();
    }

    //---------------------------------------
    // Enumerate playback devices
    //---------------------------------------

    IMMDeviceCollection *devices = nullptr;

    hr = enumerator->EnumAudioEndpoints(
        eRender,
        DEVICE_STATE_ACTIVE,
        &devices);

    if (FAILED(hr))
    {
        enumerator->Release();
        return;
    }

    UINT count = 0;
    devices->GetCount(&count);

    for (UINT i = 0; i < count; i++)
    {
        IMMDevice *device = nullptr;

        if (FAILED(devices->Item(i, &device)))
            continue;

        AudioDevice audioDevice;

        //---------------------------------------
        // Device ID
        //---------------------------------------

        LPWSTR id = nullptr;

        if (SUCCEEDED(device->GetId(&id)))
        {
            audioDevice.DeviceID = _bstr_t(id);

            if (audioDevice.DeviceID == defaultID)
                audioDevice.DefaultPlayback = true;

            CoTaskMemFree(id);
        }

        //---------------------------------------
        // Friendly Name
        //---------------------------------------

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

                PropVariantClear(&value);
            }

            properties->Release();
        }

        //---------------------------------------
        // Guess Device Type
        //---------------------------------------

        std::string lower = audioDevice.Name;

        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            ::tolower);

        if (lower.find("headphone") != std::string::npos)
            audioDevice.Type = AudioDeviceType::Headphones;

        else if (lower.find("speaker") != std::string::npos)
            audioDevice.Type = AudioDeviceType::Speaker;

        else if (lower.find("hdmi") != std::string::npos)
            audioDevice.Type = AudioDeviceType::HDMIAudio;

        else if (lower.find("bluetooth") != std::string::npos)
            audioDevice.Type = AudioDeviceType::BluetoothAudio;

        else if (lower.find("usb") != std::string::npos)
            audioDevice.Type = AudioDeviceType::USBAudio;

        else
            audioDevice.Type = AudioDeviceType::Unknown;

        //---------------------------------------
        // Volume
        //---------------------------------------

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

            audioDevice.VolumePercent =
                volume * 100.0;

            audioDevice.Muted =
                (muted == TRUE);

            endpoint->Release();
        }

        audio.Devices.push_back(audioDevice);

        device->Release();
    }

    devices->Release();
    enumerator->Release();
}

static void FillRecordingDevices(AudioInfo &audio)
{
    IMMDeviceEnumerator *enumerator = nullptr;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void **>(&enumerator));

    if (FAILED(hr))
        return;

    //---------------------------------------
    // Default Recording Device
    //---------------------------------------

    IMMDevice *defaultDevice = nullptr;
    std::string defaultID;

    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(
            eCapture,
            eConsole,
            &defaultDevice)))
    {
        LPWSTR id = nullptr;

        if (SUCCEEDED(defaultDevice->GetId(&id)))
        {
            defaultID = _bstr_t(id);

            CoTaskMemFree(id);
        }

        defaultDevice->Release();
    }

    //---------------------------------------
    // Enumerate Recording Devices
    //---------------------------------------

    IMMDeviceCollection *devices = nullptr;

    hr = enumerator->EnumAudioEndpoints(
        eCapture,
        DEVICE_STATE_ACTIVE,
        &devices);

    if (FAILED(hr))
    {
        enumerator->Release();
        return;
    }

    UINT count = 0;
    devices->GetCount(&count);

    for (UINT i = 0; i < count; i++)
    {
        IMMDevice *device = nullptr;

        if (FAILED(devices->Item(i, &device)))
            continue;

        AudioDevice audioDevice;

        //---------------------------------------
        // Device ID
        //---------------------------------------

        LPWSTR id = nullptr;

        if (SUCCEEDED(device->GetId(&id)))
        {
            audioDevice.DeviceID = _bstr_t(id);

            if (audioDevice.DeviceID == defaultID)
                audioDevice.DefaultRecording = true;

            CoTaskMemFree(id);
        }

        //---------------------------------------
        // Friendly Name
        //---------------------------------------

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

                PropVariantClear(&value);
            }

            properties->Release();
        }

        //---------------------------------------
        // Guess Device Type
        //---------------------------------------

        std::string lower = audioDevice.Name;

        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            ::tolower);

        if (lower.find("microphone") != std::string::npos)
            audioDevice.Type = AudioDeviceType::Microphone;

        else if (lower.find("headset") != std::string::npos)
            audioDevice.Type = AudioDeviceType::Headset;

        else if (lower.find("bluetooth") != std::string::npos)
            audioDevice.Type = AudioDeviceType::BluetoothAudio;

        else if (lower.find("usb") != std::string::npos)
            audioDevice.Type = AudioDeviceType::USBAudio;

        else
            audioDevice.Type = AudioDeviceType::Microphone;

        //---------------------------------------
        // Volume
        //---------------------------------------

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

            audioDevice.VolumePercent =
                volume * 100.0;

            audioDevice.Muted =
                (muted == TRUE);

            endpoint->Release();
        }

        audio.Devices.push_back(audioDevice);

        device->Release();
    }

    devices->Release();
    enumerator->Release();
}

static void FillDriverInfo(AudioInfo &audio)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_SoundDevice"),
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
        std::string name;
        std::string manufacturer;
        std::string deviceID;

        GetWMIProperty(
            object,
            L"Name",
            name);

        GetWMIProperty(
            object,
            L"Manufacturer",
            manufacturer);

        GetWMIProperty(
            object,
            L"DeviceID",
            deviceID);

        std::string driverVersion;

        GetWMIProperty(
            object,
            L"DriverVersion",
            driverVersion);

        //-------------------------------------
        // Match with our collected devices
        //-------------------------------------

        for (auto &device : audio.Devices)
        {
            if (device.Name.find(name) != std::string::npos ||
                name.find(device.Name) != std::string::npos)
            {
                device.Manufacturer = manufacturer;
                device.DriverVersion = driverVersion;

                if (device.DeviceID.empty())
                    device.DeviceID = deviceID;

                break;
            }
        }

        object->Release();
    }

    enumerator->Release();
}

static void FillAudioProperties(AudioInfo &audio)
{
    IMMDeviceEnumerator *enumerator = nullptr;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void **>(&enumerator));

    if (FAILED(hr))
        return;

    IMMDeviceCollection *devices = nullptr;

    hr = enumerator->EnumAudioEndpoints(
        eAll,
        DEVICE_STATE_ACTIVE,
        &devices);

    if (FAILED(hr))
    {
        enumerator->Release();
        return;
    }

    UINT count = 0;
    devices->GetCount(&count);

    for (UINT i = 0; i < count; i++)
    {
        IMMDevice *device = nullptr;

        if (FAILED(devices->Item(i, &device)))
            continue;

        LPWSTR id = nullptr;
        std::string deviceID;

        if (SUCCEEDED(device->GetId(&id)))
        {
            deviceID = _bstr_t(id);
            CoTaskMemFree(id);
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
                for (auto &audioDevice : audio.Devices)
                {
                    if (audioDevice.DeviceID == deviceID)
                    {
                        audioDevice.Channels = format->nChannels;
                        audioDevice.SampleRate = format->nSamplesPerSec;
                        audioDevice.BitDepth = format->wBitsPerSample;
                        break;
                    }
                }

                CoTaskMemFree(format);
            }

            client->Release();
        }

        device->Release();
    }

    devices->Release();
    enumerator->Release();
}

AudioInfo GetAudioInfo()
{
    AudioInfo audio;

    FillPlaybackDevices(audio);
    FillRecordingDevices(audio);
    FillDriverInfo(audio);
    FillAudioProperties(audio);

    audio.TotalDevices =
        static_cast<uint32_t>(audio.Devices.size());

    return audio;
}