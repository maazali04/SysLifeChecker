#include "Battery.hpp"
#include "WMIHelper.hpp"

#include <windows.h>
#include <powrprof.h>

#pragma comment(lib, "PowrProf.lib")

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

        battery.RemainingCapacityPercent =
            status.BatteryLifePercent;

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

        GetWMIProperty(
            object,
            L"DesignVoltage",
            battery.DesignedVoltagemV);

        GetWMIProperty(
            object,
            L"EstimatedChargeRemaining",
            battery.RemainingCapacityPercent);

        battery.HealthPercent = 100.0;

        battery.Healthy = true;
        battery.ReplaceRecommended = false;

        object->Release();

        ++index;
    }

    enumerator->Release();
}

std::vector<BatteryInfo> GetBatteryInfo()
{
    std::vector<BatteryInfo> batteries;

    FillBatteryIdentity(batteries);

    if (batteries.empty())
        return batteries;

    FillBatteryStatus(batteries);
    FillBatteryHealth(batteries);

    return batteries;
}