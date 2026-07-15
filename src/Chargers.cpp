#include "Chargers.hpp"
#include "WMIHelper.hpp"

#include <windows.h>

static void FillBasicChargerInfo(std::vector<ChargerInfo>& chargers);

std::vector<ChargerInfo> GetChargerInfo()
{
    std::vector<ChargerInfo> chargers;

    FillBasicChargerInfo(chargers);

    return chargers;
}

static void FillBasicChargerInfo(std::vector<ChargerInfo>& chargers)
{
    SYSTEM_POWER_STATUS status;

    if (!GetSystemPowerStatus(&status))
        return;

    // Desktop PC
    if (status.BatteryFlag == 128)
        return;

    ChargerInfo charger;

    charger.Connected =
        (status.ACLineStatus == 1);

    chargers.push_back(charger);
}