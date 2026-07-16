#pragma once

#include "SystemInfo.hpp"

RAMInfo GetRAMInfo();

const char* RAMTypeToString(RAMType type);
const char* RAMFormFactorToString(RAMFormFactor form);