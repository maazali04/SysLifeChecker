#pragma once

#include <string>
#include <vector>
#include <cstdint>

bool InitializeWMI();
void ShutdownWMI();

// ------------------------------------
// CPU
// ------------------------------------
enum class CPUVendor
{
    Unknown,
    Intel,
    AMD,
    Qualcomm,
    Apple,
    VIA,
    IBM,
    ARM
};

enum class CPUArchitecture
{
    Unknown,
    x86,
    x64,
    ARM,
    ARM64
};

struct CPUCacheInfo
{
    uint32_t L1InstructionKB = 0;
    uint32_t L1DataKB = 0;
    uint32_t L2KB = 0;
    uint32_t L3KB = 0;
    int32_t CacheLineSize = 0;
};

struct CPUFrequencyInfo
{
    double BaseClockGHz = 0.0;
    double BoostClockGHz = 0.0;
    double CurrentClockGHz = 0.0;
};

struct CPUFeatureInfo
{
    bool MMX = false;

    bool SSE = false;
    bool SSE2 = false;
    bool SSE3 = false;
    bool SSSE3 = false;
    bool SSE41 = false;
    bool SSE42 = false;

    bool AVX = false;
    bool AVX2 = false;
    bool AVX512 = false;

    bool FMA = false;
    bool AES = false;
    bool SHA = false;

    bool BMI1 = false;
    bool BMI2 = false;

    bool HTT = false;
    bool SMT = false;

    bool Virtualization = false;
    bool VT_x = false;
    bool AMD_V = false;
    bool VT_d = false;

    bool NX = false;
    bool DEP = false;

    bool SGX = false;
    bool SMEP = false;
    bool SMAP = false;

    bool TXT = false;
    bool TPM = false;

    bool x64 = false;

    bool PCLMULQDQ = false;
    bool MONITOR = false;
    bool VMx = false;
    bool CMPXCHG16B = false;
    bool MOVBE = false;
    bool POPCNT = false;
    bool XSAVE = false;
    bool OSXSAVE = false;
    bool F16C = false;
    bool RDRAND = false;

    bool FSGSBASE = false;
    bool HLE = false;
    bool ERMS = false;
    bool INVPCID = false;
    bool RTM = false;
    bool MPX = false;
    bool AVX512F = false;
    bool AVX512DQ = false;
    bool RDSEED = false;
    bool ADX = false;
    bool AVX512IFMA = false;
    bool CLFLUSHOPT = false;
    bool CLWB = false;
    bool AVX512BW = false;
    bool AVX512VL = false;

    bool LAHFSAHF = false;
    bool LZCNT = false;
    bool PREFETCHW = false;
    bool SYSCALL = false;
    bool RDTSCP = false;
    bool AMDV = false;
    bool ABM = false;
    bool SSE4A = false;
};

struct CPUStatusInfo
{
    double UsagePercent = 0.0;
};

struct CPUInfo
{
    // ---------- Identity ----------

    std::string Name;

    std::string BrandString;

    CPUVendor Vendor = CPUVendor::Unknown;

    CPUArchitecture Architecture = CPUArchitecture::Unknown;

    std::string Manufacturer;

    std::string Codename;

    std::string Microarchitecture;

    std::string Socket;

    int MaximumCPUIDLeaf = 0;

    int Family = 0;

    int Model = 0;

    int Stepping = 0;

    int ExtendedFamily = 0;

    int ExtendedModel = 0;

    int ProcessorType = 0;

    std::string ProcessorID;

    uint32_t InitialAPICID = 0;

    uint32_t LogicalProcessorsPerPackage = 0;

    bool HyperThreadingSupported = false;

    bool x2APICSupported = false;

    uint32_t MaximumExtendedLeaf = 0;

    // ---------- Topology ----------

    uint32_t PhysicalPackages = 0;

    uint32_t PhysicalCores = 0;

    uint32_t LogicalProcessors = 0;

    uint32_t EfficiencyCores = 0;

    uint32_t PerformanceCores = 0;

    uint32_t NUMANodes = 0;

    uint32_t ProcessorGroups = 0;

    // ---------- Manufacturing ----------

    int Generation = 0;

    // ---------- Frequency ----------

    CPUFrequencyInfo Frequency;

    // ---------- Cache ----------

    CPUCacheInfo Cache;

    // ---------- Features ----------

    CPUFeatureInfo Features;

    // ---------- Supported Instruction Sets ----------

    std::vector<std::string> InstructionSets;

    // ---------- Operating System ----------

    bool Is64BitOS = false;

    bool IsVirtualMachine = false;

    bool SupportsHypervisor = false;

    // ---------- Runtime Status ----------

    CPUStatusInfo Status;
};

// ------------------------------------
// RAM
// ------------------------------------

enum class RAMType
{
    Unknown,
    DDR,
    DDR2,
    DDR3,
    DDR3L,
    DDR4,
    DDR4X,
    DDR5,
    DDR5X,
    LPDDR3,
    LPDDR4,
    LPDDR4X,
    LPDDR5,
    LPDDR5X,
    HBM,
    HBM2,
    HBM3
};

enum class RAMFormFactor
{
    Unknown,
    DIMM,
    SO_DIMM,
    MicroDIMM,
    MiniDIMM,
    OnBoard
};

struct RAMModule
{
    // ---------- Identity ----------

    std::string Manufacturer;
    std::string PartNumber;
    std::string SerialNumber;

    // ---------- SMBIOS ----------

    std::string DeviceLocator;
    std::string BankLabel;
    // ---------- Bus ----------

    uint16_t DataWidth = 0;
    uint16_t TotalWidth = 0;
    // ---------- Physical ----------

    RAMType Type = RAMType::Unknown;
    RAMFormFactor FormFactor = RAMFormFactor::Unknown;

    std::string SlotName;
    uint32_t SlotNumber = 0;

    bool Installed = false;
    bool Removable = true;

    // ---------- Capacity ----------

    uint64_t CapacityBytes = 0;
    double CapacityGB = 0.0;

    // ---------- Speed ----------

    uint32_t ConfiguredSpeedMHz = 0;
    uint32_t MaximumSpeedMHz = 0;

    // ---------- Electrical ----------

    double Voltage = 0.0;
    double MinimumVoltage = 0.0;
    double MaximumVoltage = 0.0;

    bool ECC = false;
    bool Registered = false;
    bool Buffered = false;
    // ---------- JEDEC ----------

    std::string MemoryTechnology;

    // ---------- Rank ----------

    uint32_t Rank = 0;
};
enum class MemoryChannelMode
{
    Unknown,
    Single,
    Dual,
    Triple,
    Quad,
    Octa
};

struct RAMInfo
{
    // ---------- Overall ----------

    MemoryChannelMode ChannelMode = MemoryChannelMode::Unknown;
    RAMType Type = RAMType::Unknown;

    uint64_t TotalCapacityBytes = 0;
    double TotalCapacityGB = 0.0;

    uint32_t InstalledModules = 0;

    uint32_t TotalSlots = 0;
    uint32_t OccupiedSlots = 0;
    uint32_t EmptySlots = 0;

    uint32_t SpeedMHz = 0;

    bool ECCSupported = false;
    bool ECCEnabled = false;

    bool Upgradeable = false;

    uint64_t MaximumSupportedCapacityBytes = 0;
    double MaximumSupportedCapacityGB = 0.0;
    // ---------- Memory Controller ----------

    uint32_t MemoryChannels = 0;

    // ---------- OS ----------

    uint64_t TotalVisibleMemoryBytes = 0;
    uint64_t HardwareReservedBytes = 0;

    // ---------- Page File ----------

    uint64_t TotalVirtualMemoryBytes = 0;
    uint64_t AvailableVirtualMemoryBytes = 0;

    // ---------- Runtime ----------

    double UsagePercent = 0.0;
    uint64_t UsedMemoryBytes = 0;
    uint64_t AvailableMemoryBytes = 0;
    uint64_t CachedMemoryBytes = 0;
    uint64_t FreeMemoryBytes = 0;
    uint64_t CommittedMemoryBytes = 0;

    // ---------- Modules ----------

    std::vector<RAMModule> Modules;
};

// ------------------------------------
// GPU
// ------------------------------------

enum class GPUVendor
{
    Unknown,
    NVIDIA,
    AMD,
    Intel,
    Qualcomm,
    Apple,
    ARM,
    Microsoft,
    VMware,
    VirtualBox
};

enum class GPUType
{
    Unknown,
    Integrated,
    Dedicated,
    External,
    Virtual
};



struct GPUDriverInfo
{
    std::string DriverVersion;

    std::string DriverDate;

    std::string DriverProvider;

    std::string INFFile;


    bool WHQLCertified = false;
};

struct GPUDisplayOutput
{
    std::string Name;
    bool Connected = false;
};

struct GPUStatusInfo
{
    double UsagePercent = 0.0;
};

struct GPUInfo
{
    // ---------- Identity ----------

    std::string Name;

    GPUVendor Vendor = GPUVendor::Unknown;

    GPUType Type = GPUType::Unknown;

    std::string Manufacturer;

    std::string Codename;

    std::string DeviceID;

    std::string PCIBus;

    // ---------- Memory ----------

    uint64_t DedicatedVRAMBytes = 0;
    double DedicatedVRAMGB = 0.0;

    uint64_t SharedMemoryBytes = 0;
    double SharedMemoryGB = 0.0;

    uint64_t TotalGraphicsMemoryBytes = 0;
    double TotalGraphicsMemoryGB = 0.0;

    // ---------- Driver ----------

    GPUDriverInfo Driver;

    // ---------- Connected Displays ----------

    std::vector<GPUDisplayOutput> Outputs;

    // ---------- Runtime ----------

    GPUStatusInfo Status;

};

// ------------------------------------
// Storage Devices
// ------------------------------------
enum class StorageDeviceType
{
    Unknown,
    HDD,
    SSD,
    NVMe,
    SSHD,
    USB,
    SDCard,
    Optical
};

enum class StorageBusType
{
    Unknown,
    SATA,
    SAS,
    NVMe,
    PCIe,
    USB,
    IDE,
    eMMC,
    UFS
};

enum class PartitionStyle
{
    Unknown,
    MBR,
    GPT
};

struct SMARTInfo
{
    bool Supported = false;
    bool Enabled = false;

    bool PredictFailure = false;

    int HealthPercent = 0;

    double Temperature = 0.0;
    double MaxTemperature = 0.0;

    uint64_t PowerOnHours = 0;
    uint64_t PowerCycles = 0;

    double EstimatedAgeYears = 0.0;
    double EstimatedRemainingYears = 0.0;

    uint64_t PercentageUsed = 0;
    uint64_t PercentageRemaining = 0;

    uint64_t AvailableSpare = 0;
    uint64_t SpareThreshold = 0;

    uint64_t UnsafeShutdowns = 0;

    uint64_t ControllerBusyMinutes = 0;

    uint64_t HostReadsGB = 0;
    uint64_t HostWritesGB = 0;

    uint64_t DataReadGB = 0;
    uint64_t DataWrittenGB = 0;

    uint64_t MediaErrors = 0;
    uint64_t ErrorLogEntries = 0;

    uint64_t ReallocatedSectors = 0;
    uint64_t PendingSectors = 0;
    uint64_t UncorrectableErrors = 0;

    uint64_t CRCErrors = 0;

    uint64_t SeekErrorRate = 0;
    uint64_t SpinRetryCount = 0;

    uint64_t StartStopCount = 0;
    uint64_t LoadUnloadCycles = 0;
};

struct PartitionInfo
{
    std::string DriveLetter;

    std::string VolumeName;

    std::string FileSystem;

    PartitionStyle Style = PartitionStyle::Unknown;

    uint64_t TotalBytes = 0;

    uint64_t UsedBytes = 0;

    uint64_t FreeBytes = 0;

    bool BootPartition = false;

    bool SystemPartition = false;

    bool Hidden = false;

    bool BitLockerEncrypted = false;
};

struct StorageDevice
{
    // ---------- Identity ----------

    std::string Model;

    std::string Manufacturer;

    std::string SerialNumber;

    std::string FirmwareVersion;

    std::string DeviceID;
    std::string MediaType;
    std::string PNPDeviceID;
    std::string Status;
    std::string InstallDate;
    std::string Name;
    std::string Description;
    uint32_t BytesPerSector = 0;
    bool IsFailing = false;
    std::string HealthStatus;
    std::string Recommendation;

    bool MediaLoaded = false;
    uint32_t LogicalSectorSize = 0;
    uint32_t PhysicalSectorSize = 0;

    std::string DriveLetter;

    std::string VolumeName;

    std::string FileSystem;

    bool GPT = false;
    bool MBR = false;

    // ---------- Type ----------

    StorageDeviceType Type = StorageDeviceType::Unknown;

    StorageBusType Bus = StorageBusType::Unknown;

    std::string Interface;

    std::string Protocol;

    // ---------- Capacity ----------

    uint64_t CapacityBytes = 0;

    double CapacityGB = 0.0;

    // ---------- Performance ----------

    uint32_t RPM = 0;

    bool TRIMSupported = false;

    bool TRIMEnabled = false;

    bool NCQSupported = false;

    bool Removable = false;

    bool BootDrive = false;

    bool SystemDrive = false;

    // ---------- SMART ----------

    SMARTInfo SMART;

    // ---------- Partitions ----------

    std::vector<PartitionInfo> Partitions;
};

struct StorageInfo
{
    uint32_t TotalDrives = 0;

    uint32_t HDDCount = 0;

    uint32_t SSDCount = 0;

    uint32_t NVMeCount = 0;

    uint64_t TotalCapacityBytes = 0;
    uint64_t FreeSpaceBytes = 0;
    uint64_t UsedSpaceBytes = 0;

    double TotalCapacityGB = 0.0;
    double FreeSpaceGB = 0.0;
    double UsedSpaceGB = 0.0;
    double UsagePercent = 0.0;

    std::vector<StorageDevice> Drives;
};

// ------------------------------------
// Battery
// ------------------------------------

enum class BatteryChemistry
{
    Unknown,
    LiIon,
    LiPolymer,
    NiMH,
    NiCd,
    LeadAcid
};

enum class BatteryStatus
{
    Unknown,
    Charging,
    Discharging,
    FullyCharged,
    NotCharging,
    ACConnected,
    Critical
};

struct BatteryInfo
{
    // ---------- Identity ----------

    std::string Name;

    std::string Manufacturer;

    std::string Model;

    std::string SerialNumber;

    std::string DeviceID;

    BatteryChemistry Chemistry = BatteryChemistry::Unknown;

    // ---------- Capacity ----------

    uint64_t DesignCapacitymWh = 0;

    uint64_t FullChargeCapacitymWh = 0;

    uint64_t RemainingCapacitymWh = 0;

    uint64_t RemainingCapacityPercent = 0;

    double HealthPercent = 0.0;

    // ---------- Charging ----------

    BatteryStatus Status = BatteryStatus::Unknown;

    bool ACConnected = false;

    bool IsCharging = false;

    bool IsBatteryPresent = false;

    uint32_t ChargeRatemW = 0;

    uint32_t DischargeRatemW = 0;

    // ---------- Life ----------

    uint32_t CycleCount = 0;

    uint32_t DesignedVoltagemV = 0;

    uint32_t CurrentVoltagemV = 0;

    // ---------- Runtime ----------

    int EstimatedRemainingMinutes = 0;

    int EstimatedChargeMinutes = 0;

    // ---------- Temperature ----------

    double TemperatureC = 0.0;

    // ---------- Manufacturing ----------

    std::string ManufactureDate;

    std::string FirstUseDate;

    // ---------- Flags ----------

    bool Healthy = true;

    bool ReplaceRecommended = false;

    bool FastChargingSupported = false;

    bool FastChargingActive = false;
};

// ------------------------------------
// Display Info
// ------------------------------------
enum class DisplayTechnology
{
    Unknown,
    LCD,
    LED,
    IPS,
    TN,
    VA,
    OLED,
    MiniLED,
    MicroLED
};

enum class DisplayConnection
{
    Unknown,
    Internal,
    HDMI,
    DisplayPort,
    MiniDisplayPort,
    USB_C,
    VGA,
    DVI
};

enum class Orientation
{
    Landscape,
    Portrait
};

struct DisplayResolution
{
    uint32_t Width = 0;
    uint32_t Height = 0;
};

struct DisplayColorInfo
{
    uint32_t BitsPerPixel = 0;
    uint32_t BitsPerChannel = 0;

    bool HDRSupported = false;
    bool HDREnabled = false;

    bool WideColorGamut = false;

    std::string ColorSpace;
};

struct DisplayTimingInfo
{
    double CurrentRefreshRate = 0.0;
    double MaximumRefreshRate = 0.0;
    double MinimumRefreshRate = 0.0;

    bool VariableRefreshRate = false;
};

struct DisplayInfo
{
    // ---------- Identity ----------

    std::string Name;
    std::string Manufacturer;
    std::string Model;
    std::string SerialNumber;

    // ---------- Hardware ----------

    DisplayTechnology Technology = DisplayTechnology::Unknown;
    DisplayConnection Connection = DisplayConnection::Unknown;

    bool InternalDisplay = false;
    bool PrimaryDisplay = false;
    bool BuiltIn = false;

    // ---------- Resolution ----------

    DisplayResolution NativeResolution;
    DisplayResolution CurrentResolution;

    // ---------- Physical ----------

    double SizeInches = 0.0;

    uint32_t WidthMM = 0;
    uint32_t HeightMM = 0;

    double PPI = 0.0;

    Orientation ScreenOrientation = Orientation::Landscape;

    // ---------- Timing ----------

    DisplayTimingInfo Timing;

    // ---------- Color ----------

    DisplayColorInfo Color;

    // ---------- Features ----------

    bool TouchSupported = false;
    bool PenSupported = false;

    bool AdaptiveSync = false;
    bool GSync = false;
    bool FreeSync = false;

    bool BuiltInCamera = false;
    bool BuiltInMicrophone = false;

    // ---------- Status ----------

    bool Connected = true;
    bool Enabled = true;
    bool Sleeping = false;

    uint32_t BrightnessPercent = 0;
};

struct DisplaySystemInfo
{
    uint32_t TotalDisplays = 0;
    uint32_t ConnectedDisplays = 0;

    std::vector<DisplayInfo> Displays;
};

// ------------------------------------
// Network Adapter
// ------------------------------------
enum class NetworkAdapterType
{
    Unknown,
    Ethernet,
    WiFi,
    Bluetooth,
    Cellular,
    Virtual,
    VPN,
    Loopback
};

enum class NetworkStatus
{
    Unknown,
    Connected,
    Disconnected,
    Disabled,
    Connecting
};

struct IPv4Address
{
    std::string Address;
    std::string SubnetMask;
    std::string Gateway;
};

struct IPv6Address
{
    std::string Address;
    uint32_t PrefixLength = 0;
    std::string Gateway;
};

struct WiFiInfo
{
    std::string SSID;
    std::string BSSID;

    std::string Standard; // Wi-Fi 4, 5, 6, 6E, 7

    uint32_t Channel = 0;

    uint32_t ChannelWidthMHz = 0;

    int SignalStrength = 0; // %

    int RSSIdBm = 0;

    std::string Band; // 2.4 / 5 / 6 GHz

    std::string Security; // WPA2, WPA3...

    bool Connected = false;
};

struct BluetoothInfo
{
    bool Supported = false;

    bool Enabled = false;

    std::string Version;

    uint32_t ConnectedDevices = 0;
};

struct NetworkStatistics
{
    uint64_t BytesSent = 0;

    uint64_t BytesReceived = 0;

    uint64_t PacketsSent = 0;

    uint64_t PacketsReceived = 0;

    uint64_t Errors = 0;

    uint64_t DroppedPackets = 0;

    double UploadSpeedMbps = 0.0;

    double DownloadSpeedMbps = 0.0;
};

struct NetworkAdapter
{
    // ---------- Identity ----------

    std::string Name;

    std::string Description;

    std::string Manufacturer;

    std::string Model;

    std::string DriverVersion;

    std::string DriverDate;

    std::string MACAddress;

    std::string DeviceID;

    // ---------- Type ----------

    NetworkAdapterType Type = NetworkAdapterType::Unknown;

    NetworkStatus Status = NetworkStatus::Unknown;

    bool PhysicalAdapter = true;

    bool Enabled = true;

    // ---------- Link ----------

    uint64_t LinkSpeedMbps = 0;

    uint64_t MaxLinkSpeedMbps = 0;

    bool DHCPEnabled = false;

    // ---------- IP ----------

    std::vector<IPv4Address> IPv4;

    std::vector<IPv6Address> IPv6;

    std::vector<std::string> DNSServers;

    // ---------- Wireless ----------

    WiFiInfo WiFi;

    BluetoothInfo Bluetooth;

    // ---------- Statistics ----------

    NetworkStatistics Statistics;
};

struct NetworkInfo
{
    uint32_t TotalAdapters = 0;

    uint32_t ConnectedAdapters = 0;

    bool InternetAvailable = false;

    std::vector<NetworkAdapter> Adapters;
};

// ------------------------------------
// BIOS
// ------------------------------------
enum class BIOSMode
{
    Unknown,
    Legacy,
    UEFI
};

struct BIOSInfo
{
    // ---------- Identity ----------

    std::string Vendor;
    std::string Manufacturer;
    std::string Version;
    std::string ReleaseDate;
    std::string Description;

    std::string SerialNumber;
    std::string SMBIOSVersion;

    // ---------- Firmware ----------

    BIOSMode Mode = BIOSMode::Unknown;

    uint64_t ROMSizeBytes = 0;

    bool UEFISupported = false;
    bool UEFIEnabled = false;

    bool SecureBootSupported = false;
    bool SecureBootEnabled = false;

    bool TPMAvailable = false;
    bool TPMEnabled = false;

    std::string TPMVersion;

    // ---------- Features ----------

    bool VirtualizationSupported = false;
    bool VirtualizationEnabled = false;

    bool FastBootSupported = false;
    bool FastBootEnabled = false;

    bool PXEBootSupported = false;

    bool WakeOnLANSupported = false;

    bool WakeOnUSBSupported = false;

    bool BIOSPasswordEnabled = false;

    bool AdminPasswordEnabled = false;

    // ---------- Misc ----------

    std::string CurrentLanguage;

    std::vector<std::string> SupportedLanguages;

    std::string LastUpdateDate;
};

// ------------------------------------
// Motherboard Info
// ------------------------------------
enum class MotherboardFormFactor
{
    Unknown,
    ATX,
    MicroATX,
    MiniITX,
    EATX,
    XLATX,
    NanoITX,
    PicoITX,
    Proprietary
};

struct PCIeSlotInfo
{
    std::string Name; // PCIEX16_1

    std::string Version; // PCIe 4.0

    uint32_t Lanes = 0; // x16

    bool Occupied = false;

    std::string DeviceInstalled; // RTX 4070
};

struct M2SlotInfo
{
    std::string Name; // M2_1

    bool Occupied = false;

    std::string DeviceInstalled;

    std::string Interface; // NVMe / SATA

    std::string PCIeVersion; // Gen4 x4
};

struct SATAPortInfo
{
    std::string Name; // SATA0

    bool Connected = false;

    std::string DeviceInstalled;
};

struct MotherboardInfo
{
    // ---------- Identity ----------

    std::string Manufacturer;

    std::string ProductName;

    std::string Model;

    std::string Version;

    std::string SerialNumber;

    std::string AssetTag;

    std::string SKU;

    std::string UUID;

    // ---------- Hardware ----------

    MotherboardFormFactor FormFactor = MotherboardFormFactor::Unknown;

    std::string Chipset;

    std::string Socket;

    // ---------- Memory ----------

    uint32_t RAMSlots = 0;

    uint64_t MaximumRAMBytes = 0;

    bool ECCSupported = false;

    // ---------- Expansion ----------

    std::vector<PCIeSlotInfo> PCIeSlots;

    std::vector<M2SlotInfo> M2Slots;

    std::vector<SATAPortInfo> SATAPorts;

    // ---------- Features ----------

    bool TPMHeaderPresent = false;

    bool RGBHeaderPresent = false;

    bool ARGBHeaderPresent = false;

    bool WiFiIntegrated = false;

    bool BluetoothIntegrated = false;

    bool RAIDSupported = false;

    // ---------- Firmware ----------

    std::string BIOSVendor;

    std::string BIOSVersion;

    std::string BIOSReleaseDate;
};

// ------------------------------------
// Windows Info
// ------------------------------------
enum class WindowsEdition
{
    Unknown,
    Home,
    HomeSingleLanguage,
    HomeN,
    Pro,
    ProN,
    ProEducation,
    ProWorkstation,
    Enterprise,
    EnterpriseN,
    Education,
    IoT,
    Server
};

struct WindowsActivationInfo
{
    bool Activated = false;

    std::string ProductKeyChannel; // OEM / Retail / Volume

    std::string LicenseStatus;

    std::string ActivationID;
};

struct WindowsUpdateInfo
{
    bool AutomaticUpdatesEnabled = false;

    bool PendingRestart = false;

    std::string LastUpdateDate;

    std::string LastCheckedDate;
};

struct WindowsInfo
{
    // ---------- Identity ----------

    std::string ProductName;

    WindowsEdition Edition = WindowsEdition::Unknown;

    std::string Version;

    std::string Build;

    std::string DisplayVersion; // 23H2, 24H2...

    std::string ReleaseID;

    // ---------- Installation ----------

    std::string InstallDate;

    std::string BootTime;

    uint64_t UptimeSeconds = 0;

    // ---------- User ----------

    std::string ComputerName;

    std::string CurrentUser;

    std::string RegisteredOwner;

    std::string Organization;

    // ---------- Architecture ----------

    bool Is64Bit = false;

    std::string SystemDirectory;

    std::string WindowsDirectory;

    // ---------- Status ----------

    bool SafeMode = false;

    bool FastStartupEnabled = false;

    bool HibernationEnabled = false;

    bool RemoteDesktopEnabled = false;

    // ---------- Activation ----------

    WindowsActivationInfo Activation;

    // ---------- Windows Update ----------

    WindowsUpdateInfo Updates;
};

// ------------------------------------
// Security
// ------------------------------------
enum class TPMVersion
{
    Unknown,
    TPM12,
    TPM20
};

enum class DriveEncryptionType
{
    None,
    BitLocker,
    DeviceEncryption,
    ThirdParty
};

struct AntivirusInfo
{
    std::string Name;

    std::string Version;

    bool Enabled = false;

    bool UpToDate = false;

    bool RealTimeProtection = false;
};

struct FirewallInfo
{
    bool Enabled = false;

    bool DomainProfile = false;

    bool PrivateProfile = false;

    bool PublicProfile = false;
};

struct DriveEncryptionInfo
{
    std::string DriveLetter;

    DriveEncryptionType Type = DriveEncryptionType::None;

    bool Encrypted = false;

    double EncryptionPercent = 0.0;
};

struct UserAccountInfo
{
    std::string Username;

    std::string FullName;

    bool Administrator = false;

    bool PasswordRequired = true;

    bool PasswordExpires = false;

    bool AccountDisabled = false;

    bool AccountLocked = false;
};

struct SecurityInfo
{
    // ---------- Secure Boot ----------

    bool SecureBootSupported = false;

    bool SecureBootEnabled = false;

    // ---------- TPM ----------

    bool TPMPresent = false;

    bool TPMReady = false;

    bool TPMEnabled = false;

    TPMVersion TPM = TPMVersion::Unknown;

    std::string TPMManufacturer;

    std::string TPMFirmwareVersion;

    // ---------- Microsoft Defender ----------

    AntivirusInfo Defender;

    // ---------- Other Antivirus ----------

    std::vector<AntivirusInfo> InstalledAntivirus;

    // ---------- Firewall ----------

    FirewallInfo Firewall;

    // ---------- Encryption ----------

    std::vector<DriveEncryptionInfo> EncryptedDrives;

    // ---------- Windows Security ----------

    bool MemoryIntegrityEnabled = false;

    bool CoreIsolationEnabled = false;

    bool SmartScreenEnabled = false;

    bool UserAccountControlEnabled = false;

    bool VirtualizationBasedSecurity = false;

    bool CredentialGuardEnabled = false;

    bool DeviceGuardEnabled = false;

    // ---------- Accounts ----------

    std::vector<UserAccountInfo> Users;
};

// ------------------------------------
// USB
// ------------------------------------
enum class PortType
{
    Unknown,

    USB2,
    USB3,
    USB31,
    USB32,
    USB4,

    USBTypeA,
    USBTypeC,

    Thunderbolt3,
    Thunderbolt4,
    Thunderbolt5,

    HDMI,
    DisplayPort,
    MiniDisplayPort,
    VGA,
    DVI,

    Ethernet,

    Audio35mm,
    OpticalAudio,

    SDCard,
    MicroSD,

    PS2,

    Serial,
    Parallel
};

enum class PortStatus
{
    Unknown,
    Available,
    Connected,
    Disabled
};

struct USBDeviceInfo
{
    std::string Name;

    std::string Manufacturer;

    std::string SerialNumber;

    std::string DeviceClass;

    std::string DriverVersion;

    bool SafelyRemovable = false;
};

struct PortInfo
{
    // ---------- Identity ----------

    std::string Name;

    PortType Type = PortType::Unknown;

    PortStatus Status = PortStatus::Unknown;

    // ---------- Hardware ----------

    std::string Version; // USB 3.2 Gen2, HDMI 2.1...

    uint32_t MaximumSpeedMbps = 0;

    bool PowerDeliverySupported = false;

    bool DisplayOutputSupported = false;

    bool DataTransferSupported = true;

    // ---------- Runtime ----------

    bool Occupied = false;

    USBDeviceInfo ConnectedDevice;
};

struct PortSystemInfo
{
    // ---------- USB ----------

    uint32_t TotalUSBPorts = 0;

    uint32_t USB2Ports = 0;

    uint32_t USB3Ports = 0;

    uint32_t USBCPorts = 0;

    uint32_t ThunderboltPorts = 0;

    // ---------- Video ----------

    uint32_t HDMIPorts = 0;

    uint32_t DisplayPorts = 0;

    uint32_t VGAPorts = 0;

    uint32_t DVIPorts = 0;

    // ---------- Network ----------

    uint32_t EthernetPorts = 0;

    // ---------- Audio ----------

    uint32_t AudioPorts = 0;

    // ---------- Card Readers ----------

    uint32_t SDCardSlots = 0;

    uint32_t MicroSDSlots = 0;

    // ---------- Complete List ----------

    std::vector<PortInfo> Ports;
};
// ------------------------------------
// AudioInfo
// ------------------------------------
enum class AudioDeviceType
{
    Unknown,
    Speaker,
    Headphones,
    Microphone,
    Headset,
    HDMIAudio,
    BluetoothAudio,
    USBAudio,
    Virtual
};

struct AudioDevice
{
    std::string Name;
    std::string Manufacturer;
    std::string DriverVersion;
    std::string DeviceID;

    AudioDeviceType Type = AudioDeviceType::Unknown;

    bool DefaultPlayback = false;
    bool DefaultRecording = false;

    bool Enabled = true;
    bool Connected = true;

    uint32_t Channels = 0;
    uint32_t SampleRate = 0;
    uint32_t BitDepth = 0;

    double VolumePercent = 0.0;

    bool Muted = false;
};

struct AudioInfo
{
    uint32_t TotalDevices = 0;

    std::vector<AudioDevice> Devices;
};

// ------------------------------------
// CameraInfo
// ------------------------------------
struct CameraInfo
{
    std::string Name;
    std::string Manufacturer;
    std::string DriverVersion;
    std::string DeviceID;

    uint32_t MaxWidth = 0;
    uint32_t MaxHeight = 0;

    uint32_t MaxFPS = 0;

    bool BuiltIn = false;

    bool Connected = true;

    bool Enabled = true;

    bool PrivacyShutter = false;

    bool InfraredCamera = false;

    bool SupportsWindowsHello = false;
};

struct CameraSystemInfo
{
    uint32_t TotalCameras = 0;

    std::vector<CameraInfo> Cameras;
};

// ------------------------------------
// DriverInfo
// ------------------------------------
struct DriverInfo
{
    std::string DeviceName;

    std::string Provider;

    std::string Version;

    std::string Date;

    std::string INFFile;

    bool DigitallySigned = false;
};

struct DriverSystemInfo
{
    uint32_t TotalDrivers = 0;

    std::vector<DriverInfo> Drivers;
};
// ------------------------------------
// ChargerInfo
// ------------------------------------
enum class ChargerType
{
    Unknown,
    Barrel,
    USB_PD,
    MagSafe,
    Wireless
};

struct ChargerInfo
{
    std::string Manufacturer;

    std::string Model;

    ChargerType Type = ChargerType::Unknown;

    uint32_t RatedPowerW = 0;

    uint32_t CurrentPowerW = 0;

    uint32_t Voltage = 0;

    uint32_t Current = 0;

    bool Connected = false;

    bool OriginalCharger = false;

    bool FastChargingSupported = false;

    bool FastChargingActive = false;
};

// ------------------------------------
// SystemInfo
// ------------------------------------

struct SystemInfo
{
    CPUInfo CPU;
    RAMInfo RAM;
    std::vector<GPUInfo> GPUs;
    StorageInfo Storage;
    std::vector<BatteryInfo> Batteries;
    DisplaySystemInfo Displays;
    NetworkInfo Network;
    MotherboardInfo Motherboard;
    WindowsInfo Windows;
    SecurityInfo Security;
    PortSystemInfo Ports;
    AudioInfo Audio;
    CameraSystemInfo Cameras;
    DriverSystemInfo Drivers;
    std::vector<ChargerInfo> Chargers;
};

SystemInfo GetSystemInfo();

CPUInfo GetCPUInfo();
RAMInfo GetRAMInfo();
std::vector<GPUInfo> GetGPUInfo();
StorageInfo GetStorageInfo();
std::vector<BatteryInfo> GetBatteryInfo();
DisplaySystemInfo GetDisplayInfo();
NetworkInfo GetNetworkInfo();
MotherboardInfo GetMotherboardInfo();
WindowsInfo GetWindowsInfo();
SecurityInfo GetSecurityInfo();
PortSystemInfo GetPortInfo();
AudioInfo GetAudioInfo();
CameraSystemInfo GetCameraInfo();
DriverSystemInfo GetDriverInfo();
std::vector<ChargerInfo> GetChargerInfo();