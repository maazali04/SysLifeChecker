#include "Drivers.hpp"
#include "WMIHelper.hpp"
#include <unordered_set>

static void RemoveDuplicateDrivers(DriverSystemInfo& drivers)
{
    std::unordered_set<std::string> seen;

    std::vector<DriverInfo> uniqueDrivers;

    for (const auto& driver : drivers.Drivers)
    {
        std::string key =
            driver.DeviceName + "|" +
            driver.Provider + "|" +
            driver.Version + "|" +
            driver.INFFile;

        if (seen.insert(key).second)
        {
            uniqueDrivers.push_back(driver);
        }
    }

    drivers.Drivers = std::move(uniqueDrivers);
}

static void FillInstalledDrivers(DriverSystemInfo &drivers);

DriverSystemInfo GetDriverInfo()
{
    DriverSystemInfo drivers;

    FillInstalledDrivers(drivers);
    RemoveDuplicateDrivers(drivers);


    drivers.TotalDrivers =
        static_cast<uint32_t>(drivers.Drivers.size());

    return drivers;
}

static void FillInstalledDrivers(DriverSystemInfo &drivers)
{
    IEnumWbemClassObject *enumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PnPSignedDriver"),
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
        DriverInfo driver;

        //---------------------------------------
        // Device Name
        //---------------------------------------

        GetWMIProperty(
            object,
            L"DeviceName",
            driver.DeviceName);

        //---------------------------------------
        // Provider
        //---------------------------------------

        GetWMIProperty(
            object,
            L"DriverProviderName",
            driver.Provider);

        //---------------------------------------
        // Version
        //---------------------------------------

        GetWMIProperty(
            object,
            L"DriverVersion",
            driver.Version);

        //---------------------------------------
        // Date
        //---------------------------------------

        std::string date;

        if (GetWMIProperty(
                object,
                L"DriverDate",
                date))
        {
            if (date.length() >= 8)
            {
                driver.Date =
                    date.substr(0, 4) + "-" +
                    date.substr(4, 2) + "-" +
                    date.substr(6, 2);
            }
            else
            {
                driver.Date = date;
            }
        }

        //---------------------------------------
        // INF File
        //---------------------------------------

        GetWMIProperty(
            object,
            L"InfName",
            driver.INFFile);

        //---------------------------------------
        // Digital Signature
        //---------------------------------------

        GetWMIProperty(
            object,
            L"IsSigned",
            driver.DigitallySigned);

        driver.DriverPresent = !driver.DeviceName.empty();

        if (!driver.DeviceName.empty())
        {
            drivers.Drivers.push_back(driver);
        }

        object->Release();
    }

    enumerator->Release();
}