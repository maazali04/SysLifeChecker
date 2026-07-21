#pragma once

#include "SystemInfo.hpp"

const char* BatteryChemistryToString(BatteryChemistry chemistry);
const char* BatteryStatusToString(BatteryStatus status);
std::vector<BatteryInfo> GetBatteryInfo();