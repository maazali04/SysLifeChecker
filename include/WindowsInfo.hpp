#pragma once

#include "SystemInfo.hpp"

const char* WindowsEditionToString(WindowsEdition edition);
 std::string FormatUptime(uint64_t seconds);
MotherboardInfo GetMotherboardInfo();