#include "Storage.hpp"
#include "WMIHelper.hpp"
#include <windows.h>
#include <array>
#include <algorithm>
#include <winioctl.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

const char *PartitionStyleToString(PartitionStyle style)
{
    switch (style)
    {
    case PartitionStyle::MBR:
        return "MBR";

    case PartitionStyle::GPT:
        return "GPT";

    default:
        return "Unknown";
    }
}

const char *StorageDeviceTypeToString(StorageDeviceType type)
{
    switch (type)
    {
    case StorageDeviceType::HDD:
        return "HDD";
    case StorageDeviceType::SSD:
        return "SSD";
    case StorageDeviceType::NVMe:
        return "NVMe";
    case StorageDeviceType::SSHD:
        return "SSHD";
    case StorageDeviceType::USB:
        return "USB";
    case StorageDeviceType::SDCard:
        return "SD Card";
    case StorageDeviceType::Optical:
        return "Optical";
    default:
        return "Unknown";
    }
}

const char *StorageBusTypeToString(StorageBusType bus)
{
    switch (bus)
    {
    case StorageBusType::SATA:
        return "SATA";
    case StorageBusType::SAS:
        return "SAS";
    case StorageBusType::NVMe:
        return "NVMe";
    case StorageBusType::PCIe:
        return "PCIe";
    case StorageBusType::USB:
        return "USB";
    case StorageBusType::IDE:
        return "IDE";
    case StorageBusType::eMMC:
        return "eMMC";
    case StorageBusType::UFS:
        return "UFS";
    default:
        return "Unknown";
    }
}

static void FillOverallStorageInfo(StorageInfo &storage)
{
    DWORD drives = GetLogicalDrives();

    char root[] = "A:\\";

    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalBytes;
    ULARGE_INTEGER totalFreeBytes;

    for (char drive = 'A'; drive <= 'Z'; ++drive)
    {
        if (!(drives & (1 << (drive - 'A'))))
            continue;

        root[0] = drive;

        if (GetDriveTypeA(root) != DRIVE_FIXED)
            continue;

        if (!GetDiskFreeSpaceExA(
                root,
                &freeBytesAvailable,
                &totalBytes,
                &totalFreeBytes))
        {
            continue;
        }

        storage.TotalCapacityBytes += totalBytes.QuadPart;
        storage.FreeSpaceBytes += totalFreeBytes.QuadPart;
    }

    storage.UsedSpaceBytes =
        storage.TotalCapacityBytes - storage.FreeSpaceBytes;

    storage.TotalCapacityGB =
        static_cast<double>(storage.TotalCapacityBytes) /
        (1024.0 * 1024.0 * 1024.0);

    storage.FreeSpaceGB =
        static_cast<double>(storage.FreeSpaceBytes) /
        (1024.0 * 1024.0 * 1024.0);

    storage.UsedSpaceGB =
        static_cast<double>(storage.UsedSpaceBytes) /
        (1024.0 * 1024.0 * 1024.0);

    if (storage.TotalCapacityBytes != 0)
    {
        storage.UsagePercent =
            (static_cast<double>(storage.UsedSpaceBytes) * 100.0) /
            storage.TotalCapacityBytes;
    }
}

static void FillPhysicalDriveInfo(StorageInfo &storage)
{
    if (gService == nullptr)
        return;

    IEnumWbemClassObject *pEnumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(L"SELECT * FROM Win32_DiskDrive"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator);

    if (FAILED(hr) || pEnumerator == nullptr)
        return;

    IWbemClassObject *pObject = nullptr;
    ULONG returned = 0;

    while (SUCCEEDED(pEnumerator->Next(
               WBEM_INFINITE,
               1,
               &pObject,
               &returned)) &&
           returned)
    {
        StorageDevice drive;

        GetWMIProperty(pObject, L"Model", drive.Model);
        GetWMIProperty(
            pObject,
            L"Manufacturer",
            drive.Manufacturer);

        if (drive.Manufacturer.empty() ||
            drive.Manufacturer == "(Standard disk drives)")
        {
            if (drive.Model.find("SAMSUNG") != std::string::npos)
                drive.Manufacturer = "Samsung";

            else if (drive.Model.find("WDC") != std::string::npos ||
                     drive.Model.find("WD") != std::string::npos)
                drive.Manufacturer = "Western Digital";

            else if (drive.Model.find("Seagate") != std::string::npos ||
                     drive.Model.find("ST") == 0)
                drive.Manufacturer = "Seagate";

            else if (drive.Model.find("TOSHIBA") != std::string::npos)
                drive.Manufacturer = "Toshiba";

            else if (drive.Model.find("KINGSTON") != std::string::npos)
                drive.Manufacturer = "Kingston";

            else if (drive.Model.find("CRUCIAL") != std::string::npos)
                drive.Manufacturer = "Crucial";

            else if (drive.Model.find("SanDisk") != std::string::npos)
                drive.Manufacturer = "SanDisk";

            else if (drive.Model.find("Intel") != std::string::npos)
                drive.Manufacturer = "Intel";

            else
                drive.Manufacturer = "Unknown";
        }

        GetWMIProperty(pObject, L"SerialNumber", drive.SerialNumber);
        GetWMIProperty(pObject, L"FirmwareRevision", drive.FirmwareVersion);

        GetWMIProperty(pObject, L"DeviceID", drive.DeviceID);
        GetWMIProperty(pObject, L"InterfaceType", drive.Interface);

        if (drive.Protocol == "NVMe")
        {
            drive.Interface = "NVMe";
        }
        else if (drive.Protocol == "AHCI")
        {
            drive.Interface = "SATA";
        }

        GetWMIProperty(pObject, L"Size", drive.CapacityBytes);
        // ---------- Bus / Connection ----------

        GetWMIProperty(pObject,
                       L"PNPDeviceID",
                       drive.PNPDeviceID);

        GetWMIProperty(pObject,
                       L"Caption",
                       drive.Name);

        GetWMIProperty(pObject,
                       L"Description",
                       drive.Description);

        // ---------- Geometry ----------

        GetWMIProperty(pObject,
                       L"BytesPerSector",
                       drive.BytesPerSector);

        // ---------- Status ----------

        GetWMIProperty(pObject,
                       L"Status",
                       drive.Status);

        GetWMIProperty(pObject,
                       L"InstallDate",
                       drive.InstallDate);

        // ---------- Media ----------

        std::string mediaType;

        if (GetWMIProperty(pObject,
                           L"MediaType",
                           mediaType))
        {
            drive.MediaType = mediaType;
        }

        std::string mediaLoaded;

        if (GetWMIProperty(pObject,
                           L"MediaLoaded",
                           mediaLoaded))
        {
            drive.MediaLoaded =
                (mediaLoaded == "TRUE");
        }

        drive.CapacityGB =
            static_cast<double>(drive.CapacityBytes) /
            (1024.0 * 1024.0 * 1024.0);

        storage.Drives.push_back(std::move(drive));

        pObject->Release();
    }

    pEnumerator->Release();

    storage.TotalDrives =
        static_cast<uint32_t>(storage.Drives.size());
}

static StorageDevice *FindStorageDevice(
    StorageInfo &storage,
    const std::string &deviceID)
{
    for (auto &drive : storage.Drives)
    {
        if (drive.DeviceID == deviceID)
            return &drive;
    }

    return nullptr;
}

static void FillLogicalDriveInfo(StorageInfo &storage)
{
    DWORD drives = GetLogicalDrives();

    char root[] = "A:\\";
    char volumeName[MAX_PATH] = {};
    char fileSystem[MAX_PATH] = {};

    for (char driveLetter = 'A'; driveLetter <= 'Z'; ++driveLetter)
    {
        if (!(drives & (1 << (driveLetter - 'A'))))
            continue;

        root[0] = driveLetter;

        UINT driveType = GetDriveTypeA(root);

        if (driveType == DRIVE_NO_ROOT_DIR)
            continue;

        PartitionInfo partition;

        partition.DriveLetter = std::string(1, driveLetter) + ":";

        DWORD serial = 0;
        DWORD maxComponent = 0;
        DWORD flags = 0;

        GetVolumeInformationA(
            root,
            volumeName,
            MAX_PATH,
            &serial,
            &maxComponent,
            &flags,
            fileSystem,
            MAX_PATH);

        partition.VolumeName = volumeName;
        partition.FileSystem = fileSystem;

        ULARGE_INTEGER freeBytes;
        ULARGE_INTEGER totalBytes;
        ULARGE_INTEGER totalFreeBytes;

        if (GetDiskFreeSpaceExA(
                root,
                &freeBytes,
                &totalBytes,
                &totalFreeBytes))
        {
            partition.TotalBytes = totalBytes.QuadPart;
            partition.FreeBytes = totalFreeBytes.QuadPart;
            partition.UsedBytes =
                partition.TotalBytes - partition.FreeBytes;
        }

        partition.BootPartition =
            (_stricmp(root, "C:\\") == 0);

        partition.SystemPartition =
            (_stricmp(root, "C:\\") == 0);

        // Associate with physical drive
        for (auto &device : storage.Drives)
        {
            if (partition.TotalBytes <= device.CapacityBytes)
            {
                device.Partitions.push_back(partition);

                if (device.DriveLetter.empty())
                {
                    device.DriveLetter = partition.DriveLetter;
                }

                if (partition.BootPartition)
                {
                    device.BootDrive = true;
                }

                if (partition.SystemPartition)
                {
                    device.SystemDrive = true;
                }
                break;
            }
        }
    }
}

static void FillPartitionStyle(StorageDevice &drive)
{
    HANDLE hDrive = CreateFileA(
        drive.DeviceID.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (hDrive == INVALID_HANDLE_VALUE)
        return;

    BYTE buffer[4096]{};
    DWORD bytesReturned = 0;

    if (DeviceIoControl(
            hDrive,
            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
            nullptr,
            0,
            buffer,
            sizeof(buffer),
            &bytesReturned,
            nullptr))
    {
        auto *layout =
            reinterpret_cast<DRIVE_LAYOUT_INFORMATION_EX *>(buffer);

        PartitionStyle style = PartitionStyle::Unknown;

        if (layout->PartitionStyle == PARTITION_STYLE_GPT)
        {
            drive.GPT = true;
            drive.MBR = false;
            style = PartitionStyle::GPT;
        }
        else if (layout->PartitionStyle == PARTITION_STYLE_MBR)
        {
            drive.GPT = false;
            drive.MBR = true;
            style = PartitionStyle::MBR;
        }
        for (auto &partition : drive.Partitions)
        {
            partition.Style = style;
        }
    }

    CloseHandle(hDrive);
}

static void FillPartitionInfo(StorageInfo &storage)
{
    if (gService == nullptr)
        return;

    IEnumWbemClassObject *pEnumerator = nullptr;

    HRESULT hr = gService->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(L"SELECT * FROM Win32_DiskDrive"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator);

    if (FAILED(hr) || pEnumerator == nullptr)
        return;

    IWbemClassObject *pObject = nullptr;
    ULONG returned = 0;

    while (SUCCEEDED(pEnumerator->Next(
               WBEM_INFINITE,
               1,
               &pObject,
               &returned)) &&
           returned)
    {
        std::string deviceID;

        GetWMIProperty(
            pObject,
            L"DeviceID",
            deviceID);

        StorageDevice *drive =
            FindStorageDevice(storage, deviceID);

        if (drive == nullptr)
        {
            pObject->Release();
            continue;
        }

        std::string escapedID = deviceID;

        size_t pos = 0;
        while ((pos = escapedID.find('\\', pos)) != std::string::npos)
        {
            escapedID.replace(pos, 1, "\\\\");
            pos += 2;
        }

        std::string query =
            "ASSOCIATORS OF {Win32_DiskDrive.DeviceID=\"" +
            escapedID +
            "\"} WHERE AssocClass=Win32_DiskDriveToDiskPartition";

        IEnumWbemClassObject *pPartitionEnum = nullptr;

        hr = gService->ExecQuery(
            bstr_t(L"WQL"),
            bstr_t(_bstr_t(query.c_str())),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &pPartitionEnum);

        if (SUCCEEDED(hr) && pPartitionEnum)
        {
            IWbemClassObject *pPartition = nullptr;
            ULONG partReturned = 0;

            while (SUCCEEDED(pPartitionEnum->Next(
                       WBEM_INFINITE,
                       1,
                       &pPartition,
                       &partReturned)) &&
                   partReturned)
            {
                PartitionInfo partition;

                GetWMIProperty(
                    pPartition,
                    L"Name",
                    partition.VolumeName);

                uint64_t size = 0;

                if (GetWMIProperty(
                        pPartition,
                        L"Size",
                        size))
                {
                    partition.TotalBytes = size;
                }

                drive->Partitions.push_back(partition);

                pPartition->Release();
            }

            pPartitionEnum->Release();
        }

        pObject->Release();
    }

    pEnumerator->Release();
}

static void FillStorageFeatures(StorageInfo &storage)
{
    for (auto &drive : storage.Drives)
    {
        FillPartitionStyle(drive);
        HANDLE hDrive = CreateFileA(
            drive.DeviceID.c_str(),
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);

        if (hDrive == INVALID_HANDLE_VALUE)
            continue;

        STORAGE_PROPERTY_QUERY query{};
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        BYTE buffer[1024]{};

        DWORD bytesReturned = 0;

        if (DeviceIoControl(
                hDrive,
                IOCTL_STORAGE_QUERY_PROPERTY,
                &query,
                sizeof(query),
                buffer,
                sizeof(buffer),
                &bytesReturned,
                nullptr))
        {
            auto *descriptor =
                reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR *>(buffer);

            // ---------- Bus Type ----------

            switch (descriptor->BusType)
            {
            case BusTypeNvme:
                drive.Bus = StorageBusType::NVMe;
                drive.Protocol = "NVMe";
                break;

            case BusTypeSata:
            case BusTypeAta:
                drive.Bus = StorageBusType::SATA;
                drive.Protocol = "AHCI";
                break;

            case BusTypeUsb:
                drive.Bus = StorageBusType::USB;
                drive.Protocol = "USB";
                break;

            case BusTypeScsi:
                drive.Bus = StorageBusType::SAS;
                drive.Protocol = "SCSI";
                break;

            default:
                drive.Bus = StorageBusType::Unknown;
                break;
            }

            // ---------- Drive Type ----------

            if (descriptor->BusType == BusTypeNvme)
            {
                drive.Type = StorageDeviceType::NVMe;

                storage.NVMeCount++;
                storage.SSDCount++;
            }
            else
            {
                std::string media = drive.MediaType;

                if (media.find("SSD") != std::string::npos)
                {
                    drive.Type = StorageDeviceType::SSD;
                    storage.SSDCount++;
                }
                else if (media.find("Fixed") != std::string::npos ||
                         media.find("Hard") != std::string::npos)
                {
                    drive.Type = StorageDeviceType::HDD;
                    storage.HDDCount++;
                }
            }

            drive.Removable = descriptor->RemovableMedia;
        }
        // ---------- TRIM Support ----------

        STORAGE_PROPERTY_QUERY trimQuery{};
        trimQuery.PropertyId = StorageDeviceTrimProperty;
        trimQuery.QueryType = PropertyStandardQuery;

        DEVICE_TRIM_DESCRIPTOR trimDesc{};

        if (DeviceIoControl(
                hDrive,
                IOCTL_STORAGE_QUERY_PROPERTY,
                &trimQuery,
                sizeof(trimQuery),
                &trimDesc,
                sizeof(trimDesc),
                &bytesReturned,
                nullptr))
        {
            drive.TRIMSupported = trimDesc.TrimEnabled;
            drive.TRIMEnabled = trimDesc.TrimEnabled;
        } // ---------- Sector Sizes ----------

        STORAGE_PROPERTY_QUERY sectorQuery{};
        sectorQuery.PropertyId = StorageAccessAlignmentProperty;
        sectorQuery.QueryType = PropertyStandardQuery;

        STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR alignment{};

        if (DeviceIoControl(
                hDrive,
                IOCTL_STORAGE_QUERY_PROPERTY,
                &sectorQuery,
                sizeof(sectorQuery),
                &alignment,
                sizeof(alignment),
                &bytesReturned,
                nullptr))
        {
            drive.LogicalSectorSize =
                alignment.BytesPerLogicalSector;

            drive.PhysicalSectorSize =
                alignment.BytesPerPhysicalSector;

            drive.BytesPerSector =
                alignment.BytesPerLogicalSector;
        }

        CloseHandle(hDrive);
    }
}

static std::string ExecuteCommand(const std::string &command)
{
    std::array<char, 4096> buffer{};
    std::string result;

    FILE *pipe = _popen(command.c_str(), "r");

    if (!pipe)
        return result;

    while (fgets(buffer.data(), buffer.size(), pipe))
    {
        result += buffer.data();
    }

    _pclose(pipe);

    return result;
}
static bool ReadDriveSMART(
    const std::string &device,
    json &output)
{
    const std::string smartctl =
        "..\\third_party\\smartmontools\\smartctl.exe";

    std::string command =
        "\"" + smartctl + "\" -a -j " + device;

    std::string jsonText =
        ExecuteCommand(command);

    if (jsonText.empty())
        return false;

    try
    {
        output = json::parse(jsonText);
    }
    catch (...)
    {
        return false;
    }

    if (output.contains("smartctl"))
    {
        if (output["smartctl"].contains("exit_status"))
        {
            int status =
                output["smartctl"]["exit_status"];

            // Bit 1 = Device open failed
            if (status & 0x02)
                return false;
        }
    }

    return true;
}

static void FillSMARTInfo(StorageInfo &storage)
{
    std::string scanText =
        ExecuteCommand(
            "\"..\\third_party\\smartmontools\\smartctl.exe\" --scan-open -j");

    if (scanText.empty())
        return;

    json scan = json::parse(scanText);

    for (const auto &dev : scan["devices"])
    {
        json smart;

        if (!ReadDriveSMART(
                dev["name"].get<std::string>(),
                smart))
        {
            continue;
        }

        std::string serial;

        if (smart.contains("serial_number"))
            serial = smart["serial_number"].get<std::string>();

        StorageDevice *drive = nullptr;

        // First try matching by serial number
        if (!serial.empty())
        {
            for (auto &d : storage.Drives)
            {
                if (d.SerialNumber == serial)
                {
                    drive = &d;
                    break;
                }
            }
        }

        // If serial wasn't available, match by model
        if (drive == nullptr && smart.contains("model_name"))
        {
            std::string model =
                smart["model_name"].get<std::string>();

            for (auto &d : storage.Drives)
            {
                if (d.Model == model)
                {
                    drive = &d;
                    break;
                }
            }
        }

        if (!drive)
            continue;
        drive->SMART.Supported = true;
        drive->SMART.Enabled = true;

        if (smart.contains("smart_status"))
        {
            if (smart["smart_status"].contains("passed"))
            {
                drive->SMART.PredictFailure =
                    !smart["smart_status"]["passed"].get<bool>();
            }
        }
        if (smart.contains("temperature"))
        {
            if (smart["temperature"].contains("current"))
            {
                drive->SMART.Temperature =
                    smart["temperature"]["current"];
            }
        }
        if (smart.contains("power_on_time"))
        {
            if (smart["power_on_time"].contains("hours"))
            {
                drive->SMART.PowerOnHours =
                    smart["power_on_time"]["hours"];
            }
            drive->SMART.EstimatedAgeYears =
                drive->SMART.PowerOnHours / 24.0 / 365.25;
        }
        if (smart.contains("ata_smart_attributes"))
        {
            auto table =
                smart["ata_smart_attributes"]["table"];

            for (auto &attr : table)
            {
                int id = attr["id"];

                uint64_t raw = 0;

                if (attr.contains("raw") && attr["raw"].contains("value"))
                    raw = attr["raw"]["value"].get<uint64_t>();

                switch (id)
                {
                case 4:
                    drive->SMART.PowerCycles = raw;
                    break;

                case 5:
                    drive->SMART.ReallocatedSectors = raw;
                    break;

                case 9:
                    drive->SMART.PowerOnHours = raw;
                    break;

                case 10:
                    drive->SMART.SpinRetryCount = raw;
                    break;

                case 12:
                    drive->SMART.StartStopCount = raw;
                    break;

                case 193:
                    drive->SMART.LoadUnloadCycles = raw;
                    break;

                case 197:
                    drive->SMART.PendingSectors = raw;
                    break;

                case 198:
                    drive->SMART.UncorrectableErrors = raw;
                    break;

                case 199:
                    drive->SMART.CRCErrors = raw;
                    break;
                }
            }
        }
        if (smart.contains("nvme_smart_health_information_log"))
        {
            auto &nvme =
                smart["nvme_smart_health_information_log"];

            drive->SMART.PercentageUsed =
                nvme.value("percentage_used", 0ULL);

            drive->SMART.PercentageRemaining =
                100 - drive->SMART.PercentageUsed;

            drive->SMART.AvailableSpare =
                nvme.value("available_spare", 0ULL);

            drive->SMART.SpareThreshold =
                nvme.value("available_spare_threshold", 0ULL);

            drive->SMART.MediaErrors =
                nvme.value("media_errors", 0ULL);

            drive->SMART.ErrorLogEntries =
                nvme.value("num_err_log_entries", 0ULL);

            drive->SMART.UnsafeShutdowns =
                nvme.value("unsafe_shutdowns", 0ULL);

            drive->SMART.ControllerBusyMinutes =
                nvme.value("controller_busy_time", 0ULL);

            drive->SMART.PowerCycles =
                nvme.value("power_cycles", 0ULL);

            drive->SMART.PowerOnHours =
                nvme.value("power_on_hours", 0ULL);

            uint64_t dataRead =
                nvme.value("data_units_read", 0ULL);

            uint64_t dataWritten =
                nvme.value("data_units_written", 0ULL);

            // NVMe data units are 512,000 bytes each
            drive->SMART.DataReadGB =
                (dataRead * 512000ULL) / (1024ULL * 1024ULL * 1024ULL);

            drive->SMART.DataWrittenGB =
                (dataWritten * 512000ULL) / (1024ULL * 1024ULL * 1024ULL);

            // Host reads/writes are controller-specific.
            // Leave them as raw counts unless documented.
            drive->SMART.HostReadsGB =
                drive->SMART.DataReadGB;

            drive->SMART.HostWritesGB =
                drive->SMART.DataWrittenGB;
        }
        int health = 100;
        drive->SMART.EstimatedAgeYears =
            drive->SMART.PowerOnHours /
            24.0 /
            365.25;

        if (drive->SMART.PercentageUsed > 0)
        {
            drive->SMART.EstimatedRemainingYears =
                (drive->SMART.EstimatedAgeYears /
                 drive->SMART.PercentageUsed) *
                (100 - drive->SMART.PercentageUsed);
        }

        if (drive->Type == StorageDeviceType::NVMe ||
            drive->Type == StorageDeviceType::SSD)
        {
            health = 100 - drive->SMART.PercentageUsed;

            if (drive->SMART.MediaErrors > 0)
                health -= 20;

            if (drive->SMART.PredictFailure)
                health = 0;
        }
        else
        {
            if (drive->SMART.ReallocatedSectors > 0)
                health -= 20;

            if (drive->SMART.PendingSectors > 0)
                health -= 30;

            if (drive->SMART.UncorrectableErrors > 0)
                health -= 40;

            if (drive->SMART.PredictFailure)
                health = 0;
        }

        health = std::clamp(health, 0, 100);

        drive->SMART.HealthPercent = health;
        if (drive->SMART.HealthPercent < 0)
            drive->SMART.HealthPercent = 0;

        if (drive->SMART.HealthPercent > 100)
            drive->SMART.HealthPercent = 100;

        drive->IsFailing =
            drive->SMART.PredictFailure ||
            drive->SMART.UncorrectableErrors > 0;
        if (drive->SMART.PredictFailure)
        {
            drive->HealthStatus = "Critical";

            drive->Recommendation =
                "SMART predicts imminent drive failure. Replace immediately.";
        }
        else if (health >= 80)
        {
            drive->HealthStatus = "Healthy";

            drive->Recommendation =
                "Drive is operating normally.";
        }
        else if (health >= 50)
        {
            drive->HealthStatus = "Good";

            drive->Recommendation =
                "Drive is healthy but showing normal wear.";
        }
        else if (health >= 40)
        {
            drive->HealthStatus = "Warning";

            drive->Recommendation =
                "Back up important data and monitor drive health.";
        }
        else
        {
            drive->HealthStatus = "Poor";

            drive->Recommendation =
                "Drive health is significantly degraded. Replacement is recommended.";
        }
    }
};

StorageInfo GetStorageInfo()
{
    StorageInfo storage;

    FillOverallStorageInfo(storage);
    FillPhysicalDriveInfo(storage);
    FillLogicalDriveInfo(storage);
    // FillPartitionInfo(storage);
    FillStorageFeatures(storage);
    FillSMARTInfo(storage);
    return storage;
}
