
#include "Battery.hpp"
#include "WMIHelper.hpp"
#include <windows.h>
#include <powrprof.h>
#include <setupapi.h>

#include <iostream>
#include <memory> // Required for std::unique_ptr
#include <algorithm>

#include <winioctl.h>
#include <initguid.h>
#include <batclass.h>
#include <devguid.h>

#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "Setupapi.lib")

static HANDLE OpenBatteryDevice()
{
    HDEVINFO deviceInfo = SetupDiGetClassDevs(
        &GUID_DEVICE_BATTERY,
        nullptr,
        nullptr,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (deviceInfo == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    SP_DEVICE_INTERFACE_DATA interfaceData{};
    interfaceData.cbSize = sizeof(interfaceData);

    if (!SetupDiEnumDeviceInterfaces(deviceInfo, nullptr, &GUID_DEVICE_BATTERY, 0, &interfaceData))
    {
        SetupDiDestroyDeviceInfoList(deviceInfo);
        return INVALID_HANDLE_VALUE;
    }

    DWORD detailSize = 0;
    SetupDiGetDeviceInterfaceDetail(deviceInfo, &interfaceData, nullptr, 0, &detailSize, nullptr);

    // Allocate memory dynamically as a byte array to fit the flexible structural size safely
    auto buffer = std::make_unique<BYTE[]>(detailSize);

    // Explicitly cast the raw buffer address to the Win32 detail data structure pointer
    auto *detailData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA *>(buffer.get());
    detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    if (!SetupDiGetDeviceInterfaceDetail(deviceInfo, &interfaceData, detailData, detailSize, nullptr, nullptr))
    {
        SetupDiDestroyDeviceInfoList(deviceInfo);
        return INVALID_HANDLE_VALUE;
    }

    HANDLE hBattery = CreateFile(
        detailData->DevicePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    SetupDiDestroyDeviceInfoList(deviceInfo);
    return hBattery;
}

static ULONG GetBatteryTag(HANDLE hBattery)
{
    ULONG batteryTag = 0;
    DWORD bytesReturned = 0;

    if (!DeviceIoControl(
            hBattery,
            IOCTL_BATTERY_QUERY_TAG,
            &batteryTag,
            sizeof(batteryTag),
            &batteryTag,
            sizeof(batteryTag),
            &bytesReturned,
            nullptr))
    {
        return 0;
    }

    return batteryTag;
}
static bool QueryBatteryInformation(
    HANDLE hBattery,
    ULONG batteryTag,
    BATTERY_INFORMATION &info)
{
    BATTERY_QUERY_INFORMATION query{};
    query.BatteryTag = batteryTag;
    query.InformationLevel = BatteryInformation;

    DWORD bytesReturned = 0;

    return DeviceIoControl(
        hBattery,
        IOCTL_BATTERY_QUERY_INFORMATION,
        &query,
        sizeof(query),
        &info,
        sizeof(info),
        &bytesReturned,
        nullptr);
}

static bool QueryBatteryStatus(
    HANDLE hBattery,
    ULONG batteryTag,
    BATTERY_STATUS &status)
{
    BATTERY_WAIT_STATUS waitStatus{};
    waitStatus.BatteryTag = batteryTag;

    DWORD bytesReturned = 0;

    return DeviceIoControl(
        hBattery,
        IOCTL_BATTERY_QUERY_STATUS,
        &waitStatus,
        sizeof(waitStatus),
        &status,
        sizeof(status),
        &bytesReturned,
        nullptr);
}

static void FillBatteryAPIInformation(
    HANDLE hBattery,
    std::vector<BatteryInfo> &batteries)
{
    if (batteries.empty())
        return;

    ULONG tag = GetBatteryTag(hBattery);

    if (tag == 0)
        return;

    BATTERY_INFORMATION info{};

    if (!QueryBatteryInformation(hBattery, tag, info))
        return;
        

    BatteryInfo &battery = batteries.front();

    battery.DesignCapacitymWh = info.DesignedCapacity;

    battery.FullChargeCapacitymWh = info.FullChargedCapacity;

    battery.CycleCount = info.CycleCount;

    if (memcmp(info.Chemistry, "LION", 4) == 0)
    {
        battery.Chemistry = BatteryChemistry::LiIon;
    }
    else if (memcmp(info.Chemistry, "LIPO", 4) == 0)
    {
        battery.Chemistry = BatteryChemistry::LiPolymer;
    }
    else if (memcmp(info.Chemistry, "NICD", 4) == 0)
    {
        battery.Chemistry = BatteryChemistry::NiCd;
    }
    else if (memcmp(info.Chemistry, "NIMH", 4) == 0)
    {
        battery.Chemistry = BatteryChemistry::NiMH;
    }
    else if (memcmp(info.Chemistry, "PBAC", 4) == 0)
    {
        battery.Chemistry = BatteryChemistry::LeadAcid;
    }
    else
    {
        battery.Chemistry = BatteryChemistry::Unknown;
    }

    BATTERY_STATUS status{};

if (QueryBatteryStatus(hBattery, tag, status))
{
    // NOTE: this comes from the IOCTL, not SYSTEM_POWER_STATUS, so it
    // does not carry the 255-"unknown" sentinel that FillBatteryStatus()
    // has to guard against below.
    battery.RemainingCapacitymWh =
        status.Capacity;

    battery.CurrentVoltagemV =
        status.Voltage;

    LONG rate = status.Rate;

    if (rate > 0)
    {
        battery.ChargeRatemW = rate;
        battery.DischargeRatemW = 0;
    }
    else
    {
        battery.DischargeRatemW = -rate;
        battery.ChargeRatemW = 0;
    }

    if (status.PowerState & BATTERY_CHARGING)
    {
        battery.IsCharging = true;
        battery.Status = BatteryStatus::Charging;
    }
    else if (status.PowerState & BATTERY_DISCHARGING)
    {
        battery.IsCharging = false;
        battery.Status = BatteryStatus::Discharging;
    }
    else if (status.PowerState & BATTERY_POWER_ON_LINE)
    {
        battery.Status = BatteryStatus::FullyCharged;
    }
}
// Design Capacity
battery.DesignCapacitymWh =
    info.DesignedCapacity;

// Full Charge Capacity
battery.FullChargeCapacitymWh =
    info.FullChargedCapacity;

// Cycle Count
battery.CycleCount =
    info.CycleCount;
    if (battery.DesignCapacitymWh > 0)
{
    battery.HealthPercent =
        battery.FullChargeCapacitymWh *
        100.0 /
        battery.DesignCapacitymWh;
}
else
{
    battery.HealthPercent = 0.0;
}
battery.Healthy =
    battery.HealthPercent >= 80.0;
    battery.ReplaceRecommended =
(
    battery.HealthPercent < 60.0 ||
    battery.CycleCount > 800
);
if (battery.DesignCapacitymWh)
{
    battery.RemainingCapacityPercent =
        static_cast<uint64_t>(
            battery.RemainingCapacitymWh *
            100 /
            battery.FullChargeCapacitymWh);
}
if (battery.DischargeRatemW > 0)
{
    battery.EstimatedRemainingMinutes =
        static_cast<int>(
            battery.RemainingCapacitymWh
            * 60
            / battery.DischargeRatemW);
}
if (battery.ChargeRatemW > 0)
{
    battery.EstimatedChargeMinutes =
        static_cast<int>(
            (battery.FullChargeCapacitymWh -
             battery.RemainingCapacitymWh)
            * 60
            / battery.ChargeRatemW);
}
battery.FastChargingSupported = false;
battery.FastChargingActive = false;

if (battery.ChargeRatemW >= 30000)
{
    battery.FastChargingSupported = true;
    battery.FastChargingActive = battery.IsCharging;
}
}

const char *BatteryChemistryToString(BatteryChemistry chemistry)
{
    switch (chemistry)
    {
    case BatteryChemistry::LiIon:
        return "Li-Ion";
    case BatteryChemistry::LiPolymer:
        return "Li-Polymer";
    case BatteryChemistry::NiMH:
        return "NiMH";
    case BatteryChemistry::NiCd:
        return "NiCd";
    case BatteryChemistry::LeadAcid:
        return "Lead Acid";
    default:
        return "Unknown";
    }
}

const char *BatteryStatusToString(BatteryStatus status)
{
    switch (status)
    {
    case BatteryStatus::Charging:
        return "Charging";
    case BatteryStatus::Discharging:
        return "Discharging";
    case BatteryStatus::FullyCharged:
        return "Fully Charged";
    case BatteryStatus::NotCharging:
        return "Not Charging";
    case BatteryStatus::ACConnected:
        return "AC Connected";
    case BatteryStatus::Critical:
        return "Critical";
    default:
        return "Unknown";
    }
}

static void FillBatteryIdentity(std::vector<BatteryInfo> &batteries)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_Battery"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
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
        BatteryInfo battery;

        GetWMIProperty(object, L"Name", battery.Name);
        GetWMIProperty(object, L"Manufacturer", battery.Manufacturer);
        GetWMIProperty(object, L"DeviceID", battery.DeviceID);
        GetWMIProperty(object, L"SerialNumber", battery.SerialNumber);

        battery.Model = battery.Name;
        battery.IsBatteryPresent = true;

        uint16_t chemistry = 0;

        if (GetWMIProperty(object, L"Chemistry", chemistry))
        {
            switch (chemistry)
            {
            case 6:
                battery.Chemistry = BatteryChemistry::LiIon;
                break;

            case 7:
                battery.Chemistry = BatteryChemistry::LiPolymer;
                break;

            case 3:
                battery.Chemistry = BatteryChemistry::LeadAcid;
                break;

            case 8:
                battery.Chemistry = BatteryChemistry::NiMH;
                break;

            case 9:
                battery.Chemistry = BatteryChemistry::NiCd;
                break;

            default:
                battery.Chemistry = BatteryChemistry::Unknown;
                break;
            }
        }

        batteries.push_back(battery);

        object->Release();
    }

    enumerator->Release();
}

static void FillBatteryStatus(std::vector<BatteryInfo> &batteries)
{
    SYSTEM_POWER_STATUS status;

    if (!GetSystemPowerStatus(&status))
        return;

    for (auto &battery : batteries)
    {
        battery.ACConnected =
            (status.ACLineStatus == 1);

        battery.IsCharging =
            (status.BatteryFlag & 8);

        // BatteryLifePercent is 255 when Windows doesn't know the value
        // (common right after boot, or on some desktops with a phantom
        // ACPI battery device). Displaying 255 as "255%" is the bug --
        // skip the overwrite and keep whatever FillBatteryAPIInformation()
        // already computed from the real capacity values.
        if (status.BatteryLifePercent != 255)
        {
            battery.RemainingCapacityPercent =
                status.BatteryLifePercent;
        }

        battery.EstimatedRemainingMinutes =
            (status.BatteryLifeTime == (DWORD)-1)
                ? -1
                : status.BatteryLifeTime / 60;

        if (battery.IsCharging)
            battery.Status = BatteryStatus::Charging;
        else if (battery.ACConnected)
            battery.Status = BatteryStatus::FullyCharged;
        else
            battery.Status = BatteryStatus::Discharging;
    }
}

static void FillBatteryHealth(std::vector<BatteryInfo> &batteries)
{
    if (!gService)
        return;

    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_Battery"),
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
               &returned) == WBEM_S_NO_ERROR &&
           index < batteries.size())
    {
        BatteryInfo &battery = batteries[index];

        if (battery.DesignedVoltagemV == 0)
        {
            GetWMIProperty(
                object,
                L"DesignVoltage",
                battery.DesignedVoltagemV);
        }

        if (battery.RemainingCapacityPercent == 0)
        {
            GetWMIProperty(
                object,
                L"EstimatedChargeRemaining",
                battery.RemainingCapacityPercent);
        }

        object->Release();
        ++index;
    }

    enumerator->Release();
}

std::vector<BatteryInfo> GetBatteryInfo()
{
    std::vector<BatteryInfo> batteries;

    FillBatteryIdentity(batteries);

    if (!batteries.empty())
    {
        HANDLE hBattery = OpenBatteryDevice();
        if (hBattery != INVALID_HANDLE_VALUE)
        {
            FillBatteryAPIInformation(hBattery, batteries);
            CloseHandle(hBattery);
        }

        FillBatteryStatus(batteries);
        FillBatteryHealth(batteries);

        for (auto &b : batteries)
        {
            if (b.HealthPercent <= 0.0)
            {
                if (b.DesignCapacitymWh > 0 && b.FullChargeCapacitymWh > 0)
                    b.HealthPercent = (b.FullChargeCapacitymWh * 100.0) / b.DesignCapacitymWh;
                else
                    b.HealthPercent = 100.0;
            }

            // Health thresholds: >=50% Healthy (green), 30-49% Attention
            // (amber, the "else" case in the UI), <30% Replace (red).
            b.Healthy = (b.HealthPercent >= 50.0);
            b.ReplaceRecommended = (b.HealthPercent < 30.0 || b.CycleCount > 800);
        }

        // Some desktops (and a few laptops with ACPI control-method
        // batteries) report a phantom Win32_Battery entry with no real
        // capacity or charge data at all. Treat those as "no battery"
        // instead of showing a bogus percentage for a PC that has none.
        batteries.erase(
            std::remove_if(batteries.begin(), batteries.end(),
                [](const BatteryInfo &b)
                {
                    return b.DesignCapacitymWh == 0 &&
                           b.FullChargeCapacitymWh == 0 &&
                           b.RemainingCapacityPercent == 0;
                }),
            batteries.end());
    }

    if (batteries.empty())
    {
        // Fallback: no WMI battery at all, or it was a phantom entry
        // filtered out above. Ask Windows directly whether this machine
        // is running on AC (desktop) or has a real battery.
        SYSTEM_POWER_STATUS powerStatus;
        if (GetSystemPowerStatus(&powerStatus))
        {
            BatteryInfo battery;
            battery.ACConnected = (powerStatus.ACLineStatus == 1);

            if (powerStatus.BatteryFlag != 128 && powerStatus.BatteryFlag != 255)
            {
                // Battery is physically present
                battery.IsBatteryPresent = true;
                battery.Name = "System Battery";
                battery.RemainingCapacityPercent =
                    (powerStatus.BatteryLifePercent == 255) ? 0 : powerStatus.BatteryLifePercent;
                battery.IsCharging = (powerStatus.BatteryFlag & 8) != 0;
                battery.Status = battery.IsCharging ? BatteryStatus::Charging :
                                 (battery.ACConnected ? BatteryStatus::FullyCharged : BatteryStatus::Discharging);
                battery.HealthPercent = 100.0;
                battery.Healthy = true;
                batteries.push_back(battery);
            }
            else
            {
                // Desktop PC: no battery at all -- report it plainly as
                // "none" rather than inventing a fake percentage.
                battery.IsBatteryPresent = false;
                battery.Name = "Desktop AC Power";
                battery.Status = BatteryStatus::ACConnected;
                battery.HealthPercent = 100.0;
                battery.Healthy = true;
                batteries.push_back(battery);
            }
        }
    }

    return batteries;
}