#pragma once

#include "SystemInfo.hpp"

const char* TPMVersionToString(TPMVersion version);
const char* DriveEncryptionTypeToString(DriveEncryptionType type);
SecurityInfo GetSecurityInfo();