#include "DashboardPanel.hpp"
#include "controls/CardPanel.hpp"
#include "FontManager.hpp"
#include "Battery.hpp"
#include "RAM.hpp"
#include <wx/statline.h>
#include <ctime>
#include <sstream>
#include <iomanip>

DashboardPanel::DashboardPanel(wxWindow *parent)
    : wxPanel(parent)
{
    SetBackgroundColour(wxColour(245, 247, 250));
    BuildUI();
}

void DashboardPanel::BuildUI()
{
    wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);

    wxScrolledWindow *scroll =
        new wxScrolledWindow(
            this,
            wxID_ANY,
            wxDefaultPosition,
            wxDefaultSize,
            wxVSCROLL);

    scroll->SetScrollRate(0, 20);
    scroll->SetBackgroundColour(wxColour(245, 247, 250));

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    //--------------------------------------------------
    // Header
    //--------------------------------------------------
    wxBoxSizer *header = new wxBoxSizer(wxHORIZONTAL);

    auto *title = new wxStaticText(scroll, wxID_ANY, "Dashboard");
    title->SetFont(FontManager::SemiBold(20));
    header->Add(title, 0, wxALIGN_CENTER_VERTICAL);

    header->AddStretchSpacer();

    m_LastScan = new wxStaticText(scroll, wxID_ANY, "Scanning system...");
    m_LastScan->SetFont(FontManager::Regular(10));
    header->Add(m_LastScan, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

    CardPanel *status = new CardPanel(scroll);
    status->SetMinSize(wxSize(180, 40));

    wxBoxSizer *statusSizer = new wxBoxSizer(wxHORIZONTAL);
    statusSizer->AddSpacer(15);

    m_StatusDot = new wxPanel(status);
    m_StatusDot->SetMinSize(wxSize(10, 10));
    m_StatusDot->SetMaxSize(wxSize(10, 10));
    m_StatusDot->SetBackgroundColour(wxColour(40, 190, 80));

    statusSizer->Add(m_StatusDot, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    m_StatusText = new wxStaticText(status, wxID_ANY, "HEALTHY");
    m_StatusText->SetFont(FontManager::Medium(10));
    statusSizer->Add(m_StatusText, 0, wxALIGN_CENTER_VERTICAL);
    statusSizer->AddSpacer(15);

    status->SetSizer(statusSizer);
    header->Add(status, 0, wxALIGN_CENTER_VERTICAL);

    mainSizer->Add(header, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 24);
    mainSizer->AddSpacer(24);

    //--------------------------------------------------
    // Row 1 (Battery, Specs, Validation)
    //--------------------------------------------------
    wxBoxSizer *row1 = new wxBoxSizer(wxHORIZONTAL);

    m_BatteryCard = new CardPanel(scroll);
    m_SpecsCard = new CardPanel(scroll);
    m_ValidCard = new CardPanel(scroll);

    m_BatteryCard->SetMinSize(wxSize(0, 270));
    m_SpecsCard->SetMinSize(wxSize(0, 270));
    m_ValidCard->SetMinSize(wxSize(0, 270));

    row1->Add(m_BatteryCard, 1, wxEXPAND | wxRIGHT, 18);
    row1->Add(m_SpecsCard, 1, wxEXPAND | wxRIGHT, 18);
    row1->Add(m_ValidCard, 1, wxEXPAND);

    // 🔋 Battery Card
    m_BatterySizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText *batteryTitle = new wxStaticText(m_BatteryCard, wxID_ANY, "Battery & Power");
    batteryTitle->SetFont(FontManager::Medium(12));
    batteryTitle->SetForegroundColour(wxColour(30, 41, 59));
    m_BatterySizer->Add(batteryTitle, 0, wxLEFT | wxTOP, 20);

    m_BatteryPercent = new wxStaticText(m_BatteryCard, wxID_ANY, "--%");
    m_BatteryPercent->SetFont(FontManager::Bold(26));
    m_BatteryPercent->SetForegroundColour(wxColour(35, 180, 70));

    m_BatterySizer->AddSpacer(8);
    m_BatterySizer->Add(m_BatteryPercent, 0, wxALIGN_CENTER_HORIZONTAL);

    m_BatteryGauge = new wxGauge(m_BatteryCard, wxID_ANY, 100, wxDefaultPosition, wxSize(180, 8));
    m_BatteryGauge->SetValue(100);

    m_BatterySizer->AddSpacer(8);
    m_BatterySizer->Add(m_BatteryGauge, 0, wxALIGN_CENTER_HORIZONTAL);

    m_BatteryBadge = new CardPanel(m_BatteryCard);
    m_BatteryBadge->SetBackgroundColour(wxColour(235, 248, 238));

    wxBoxSizer* bBadgeSizer = new wxBoxSizer(wxHORIZONTAL);
    bBadgeSizer->AddSpacer(12);
    m_BatteryBadgeText = new wxStaticText(m_BatteryBadge, wxID_ANY, "Checking...");
    m_BatteryBadgeText->SetFont(FontManager::SemiBold(9));
    m_BatteryBadgeText->SetForegroundColour(wxColour(35, 180, 70));
    bBadgeSizer->Add(m_BatteryBadgeText, 0, wxALIGN_CENTER_VERTICAL);
    bBadgeSizer->AddSpacer(12);
    m_BatteryBadge->SetSizerAndFit(bBadgeSizer);

    m_BatterySizer->AddSpacer(8);
    m_BatterySizer->Add(m_BatteryBadge, 0, wxALIGN_CENTER_HORIZONTAL);

    wxStaticLine* line1 = new wxStaticLine(m_BatteryCard);
    m_BatterySizer->AddSpacer(10);
    m_BatterySizer->Add(line1, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
    m_BatterySizer->AddSpacer(10);

    m_BatteryDetailsSizer = new wxBoxSizer(wxVERTICAL);
    m_BatterySizer->Add(m_BatteryDetailsSizer, 1, wxEXPAND);

    m_BatteryCard->SetSizer(m_BatterySizer);

    // 🧩 Specs Card
    m_SpecsSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* specsTitle = new wxStaticText(m_SpecsCard, wxID_ANY, "Hardware Specs");
    specsTitle->SetFont(FontManager::Medium(12));
    specsTitle->SetForegroundColour(wxColour(30, 41, 59));
    m_SpecsSizer->Add(specsTitle, 0, wxLEFT | wxTOP, 20);
    m_SpecsSizer->AddSpacer(14);

    m_SpecsDetailsSizer = new wxBoxSizer(wxVERTICAL);
    m_SpecsSizer->Add(m_SpecsDetailsSizer, 1, wxEXPAND);
    m_SpecsCard->SetSizer(m_SpecsSizer);

    // ✅ Validation Card
    m_ValidSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* validTitle = new wxStaticText(m_ValidCard, wxID_ANY, "System Validation");
    validTitle->SetFont(FontManager::Medium(12));
    validTitle->SetForegroundColour(wxColour(30, 41, 59));
    m_ValidSizer->Add(validTitle, 0, wxLEFT | wxTOP, 20);
    m_ValidSizer->AddSpacer(14);

    m_ValidDetailsSizer = new wxBoxSizer(wxVERTICAL);
    m_ValidSizer->Add(m_ValidDetailsSizer, 1, wxEXPAND);
    m_ValidCard->SetSizer(m_ValidSizer);

    mainSizer->Add(row1, 0, wxEXPAND | wxLEFT | wxRIGHT, 24);
    mainSizer->AddSpacer(24);

    //--------------------------------------------------
    // Row 2 (Storage & Summary)
    //--------------------------------------------------
    wxBoxSizer *row2 = new wxBoxSizer(wxHORIZONTAL);

    m_StorageCard = new CardPanel(scroll);
    m_SummaryCard = new CardPanel(scroll);

    m_StorageCard->SetMinSize(wxSize(0, 310));
    m_SummaryCard->SetMinSize(wxSize(0, 310));

    row2->Add(m_StorageCard, 3, wxEXPAND | wxRIGHT, 18);
    row2->Add(m_SummaryCard, 2, wxEXPAND);

    // 💾 Storage Card
    m_StorageSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* storageTitle = new wxStaticText(m_StorageCard, wxID_ANY, "Storage & Drive Health");
    storageTitle->SetFont(FontManager::Medium(12));
    storageTitle->SetForegroundColour(wxColour(30, 41, 59));
    m_StorageSizer->Add(storageTitle, 0, wxLEFT | wxTOP, 20);
    m_StorageSizer->AddSpacer(14);

    m_StorageDrivesSizer = new wxBoxSizer(wxVERTICAL);
    m_StorageSizer->Add(m_StorageDrivesSizer, 1, wxEXPAND);
    m_StorageCard->SetSizer(m_StorageSizer);

    // 📋 Overall Summary Card
    m_SummarySizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* summaryTitle = new wxStaticText(m_SummaryCard, wxID_ANY, "Overall Summary");
    summaryTitle->SetFont(FontManager::Medium(12));
    summaryTitle->SetForegroundColour(wxColour(30, 41, 59));
    m_SummarySizer->Add(summaryTitle, 0, wxLEFT | wxTOP, 20);
    m_SummarySizer->AddSpacer(14);

    m_SummaryBadge = new CardPanel(m_SummaryCard);
    m_SummaryBadge->SetBackgroundColour(wxColour(235, 248, 238));

    wxBoxSizer* sumBadgeSizer = new wxBoxSizer(wxHORIZONTAL);
    sumBadgeSizer->AddSpacer(12);
    m_SummaryBadgeText = new wxStaticText(m_SummaryBadge, wxID_ANY, "Analyzing...");
    m_SummaryBadgeText->SetFont(FontManager::SemiBold(9));
    m_SummaryBadgeText->SetForegroundColour(wxColour(35, 180, 70));
    sumBadgeSizer->Add(m_SummaryBadgeText, 0, wxALIGN_CENTER_VERTICAL);
    sumBadgeSizer->AddSpacer(12);
    m_SummaryBadge->SetSizerAndFit(sumBadgeSizer);

    m_SummarySizer->Add(m_SummaryBadge, 0, wxLEFT | wxRIGHT, 20);
    m_SummarySizer->AddSpacer(14);

    m_SummaryBody = new wxStaticText(
        m_SummaryCard, wxID_ANY,
        "Scanning your hardware components and health diagnostics...");
    m_SummaryBody->SetFont(FontManager::Regular(10));
    m_SummaryBody->SetForegroundColour(wxColour(80, 80, 80));
    m_SummaryBody->Wrap(280);

    m_SummarySizer->Add(m_SummaryBody, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
    m_SummaryCard->SetSizer(m_SummarySizer);

    mainSizer->Add(row2, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 24);

    scroll->SetSizer(mainSizer);
    rootSizer->Add(scroll, 1, wxEXPAND);
    SetSizer(rootSizer);
}

void DashboardPanel::UpdateData(const SystemInfo &info)
{
    // 1. Timestamp
    std::time_t now = std::time(nullptr);
    std::tm* t = std::localtime(&now);
    char timeBuffer[64];
    if (t)
    {
        std::strftime(timeBuffer, sizeof(timeBuffer), "Last Scan : %b %d, %Y  %I:%M %p", t);
        m_LastScan->SetLabel(timeBuffer);
    }

    // 2. Battery & Power
    m_BatteryDetailsSizer->Clear(true);

    auto AddBatteryRow = [&](const wxString& label, const wxString& val) {
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        auto* l = new wxStaticText(m_BatteryCard, wxID_ANY, label);
        l->SetFont(FontManager::Regular(10));
        l->SetForegroundColour(wxColour(120, 120, 120));

        auto* r = new wxStaticText(m_BatteryCard, wxID_ANY, val);
        r->SetFont(FontManager::SemiBold(10));
        r->SetForegroundColour(wxColour(55, 55, 55));

        row->Add(l, 0, wxALIGN_CENTER_VERTICAL);
        row->AddStretchSpacer();
        row->Add(r, 0, wxALIGN_CENTER_VERTICAL);
        m_BatteryDetailsSizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    };

    bool hasBattery = !info.Batteries.empty() && info.Batteries[0].IsBatteryPresent;

    if (hasBattery)
    {
        const auto& bat = info.Batteries[0];
        m_BatteryPercent->SetLabel(wxString::Format("%llu%%", bat.RemainingCapacityPercent));
        m_BatteryGauge->SetValue(static_cast<int>(bat.RemainingCapacityPercent));

        if (bat.Healthy)
        {
            m_BatteryBadgeText->SetLabel("Healthy");
            m_BatteryBadgeText->SetForegroundColour(wxColour(35, 180, 70));
            m_BatteryBadge->SetBackgroundColour(wxColour(235, 248, 238));
        }
        else if (bat.ReplaceRecommended)
        {
            m_BatteryBadgeText->SetLabel("Service Recommended");
            m_BatteryBadgeText->SetForegroundColour(wxColour(220, 60, 60));
            m_BatteryBadge->SetBackgroundColour(wxColour(255, 235, 238));
        }
        else
        {
            m_BatteryBadgeText->SetLabel("Fair Condition");
            m_BatteryBadgeText->SetForegroundColour(wxColour(240, 173, 0));
            m_BatteryBadge->SetBackgroundColour(wxColour(255, 248, 220));
        }

        if (bat.FullChargeCapacitymWh > 0 && bat.DesignCapacitymWh > 0)
        {
            AddBatteryRow("Capacity", wxString::Format("%.1f Wh / %.1f Wh",
                bat.FullChargeCapacitymWh / 1000.0, bat.DesignCapacitymWh / 1000.0));
        }
        else
        {
            AddBatteryRow("Status", BatteryStatusToString(bat.Status));
        }

        AddBatteryRow("Cycle Count", wxString::Format("%u", bat.CycleCount));
        AddBatteryRow("Health", wxString::Format("%.0f%%", bat.HealthPercent));
        AddBatteryRow("Power Source", bat.ACConnected ? "AC Connected" : "On Battery");
    }
    else
    {
        m_BatteryPercent->SetLabel("AC Power");
        m_BatteryGauge->SetValue(100);
        m_BatteryBadgeText->SetLabel("Desktop / AC");
        m_BatteryBadgeText->SetForegroundColour(wxColour(0, 120, 215));
        m_BatteryBadge->SetBackgroundColour(wxColour(230, 243, 255));

        AddBatteryRow("Power State", "Plugged in (AC)");
        AddBatteryRow("Device Type", "Desktop PC");
        AddBatteryRow("Battery", "No internal battery");
        AddBatteryRow("Status", "Connected & Stable");
    }

    m_BatteryCard->Layout();

    // 3. Hardware Specs
    m_SpecsDetailsSizer->Clear(true);

    auto AddSpecRow = [&](const wxString& name, const wxString& value) {
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        auto* left = new wxStaticText(m_SpecsCard, wxID_ANY, name);
        left->SetFont(FontManager::Regular(10));
        left->SetForegroundColour(wxColour(120, 120, 120));

        auto* right = new wxStaticText(m_SpecsCard, wxID_ANY, value);
        right->SetFont(FontManager::SemiBold(10));
        right->SetForegroundColour(wxColour(55, 55, 55));

        row->Add(left, 0, wxALIGN_CENTER_VERTICAL);
        row->AddStretchSpacer();
        row->Add(right, 0, wxALIGN_CENTER_VERTICAL);
        m_SpecsDetailsSizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    };

    // Processor
    wxString cpuName = info.CPU.Name.empty() ? "Standard Processor" : info.CPU.Name;
    AddSpecRow("Processor", cpuName.length() > 24 ? cpuName.substr(0, 24) + "..." : cpuName);

    // Cores / Threads
    AddSpecRow("Cores / Threads", wxString::Format("%u Cores / %u Threads",
        info.CPU.PhysicalCores ? info.CPU.PhysicalCores : info.CPU.LogicalProcessors,
        info.CPU.LogicalProcessors));

    // Memory
    wxString ramStr = wxString::Format("%.1f GB", info.RAM.TotalCapacityGB);
    if (info.RAM.Type != RAMType::Unknown)
        ramStr += " " + wxString(RAMTypeToString(info.RAM.Type));
    if (info.RAM.SpeedMHz > 0)
        ramStr += wxString::Format(" (%u MHz)", info.RAM.SpeedMHz);
    AddSpecRow("Memory", ramStr);

    // GPU
    wxString gpuName = "Generic Graphics";
    if (!info.GPUs.empty())
    {
        gpuName = info.GPUs[0].Name;
        if (info.GPUs[0].DedicatedVRAMGB > 0.1)
            gpuName += wxString::Format(" (%.1f GB)", info.GPUs[0].DedicatedVRAMGB);
    }
    AddSpecRow("Graphics", gpuName.length() > 24 ? gpuName.substr(0, 24) + "..." : gpuName);

    // Storage
    wxString storageStr = wxString::Format("%.0f GB Total", info.Storage.TotalCapacityGB);
    if (info.Storage.NVMeCount > 0 && info.Storage.HDDCount > 0)
        storageStr += " (NVMe + HDD)";
    else if (info.Storage.NVMeCount > 0)
        storageStr += " (NVMe SSD)";
    else if (info.Storage.SSDCount > 0)
        storageStr += " (SSD)";
    AddSpecRow("Storage", storageStr);

    // OS
    wxString osStr = info.Windows.ProductName;
    if (!info.Windows.DisplayVersion.empty())
        osStr += " " + info.Windows.DisplayVersion;
    AddSpecRow("OS", osStr.empty() ? "Windows 11 / 10" : osStr);

    m_SpecsCard->Layout();

    // 4. System Validation
    m_ValidDetailsSizer->Clear(true);

    auto AddCheckRow = [&](const wxString& name, bool ok, const wxString& value) {
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        wxPanel* dot = new wxPanel(m_ValidCard);
        dot->SetMinSize(wxSize(8, 8));
        dot->SetMaxSize(wxSize(8, 8));
        dot->SetBackgroundColour(ok ? wxColour(35, 180, 70) : wxColour(240, 173, 0));

        row->Add(dot, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

        auto* left = new wxStaticText(m_ValidCard, wxID_ANY, name);
        left->SetFont(FontManager::Regular(10));
        left->SetForegroundColour(wxColour(90, 90, 90));

        auto* right = new wxStaticText(m_ValidCard, wxID_ANY, value);
        right->SetFont(FontManager::SemiBold(10));
        right->SetForegroundColour(ok ? wxColour(35, 180, 70) : wxColour(200, 100, 0));

        row->Add(left, 0, wxALIGN_CENTER_VERTICAL);
        row->AddStretchSpacer();
        row->Add(right, 0, wxALIGN_CENTER_VERTICAL);

        m_ValidDetailsSizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    };

    AddCheckRow("Secure Boot", info.Security.SecureBootEnabled,
        info.Security.SecureBootEnabled ? "Enabled" : (info.Security.SecureBootSupported ? "Supported (Off)" : "Disabled"));

    AddCheckRow("TPM Security", info.Security.TPMPresent,
        info.Security.TPM == TPMVersion::TPM20 ? "TPM 2.0 (Ready)" :
        (info.Security.TPMPresent ? "TPM Detected" : "Not Present"));

    bool avActive = info.Security.Defender.Enabled || !info.Security.InstalledAntivirus.empty();
    AddCheckRow("Antivirus Protection", avActive, avActive ? "Active" : "Disabled");

    AddCheckRow("Windows Firewall", info.Security.Firewall.Enabled,
        info.Security.Firewall.Enabled ? "Protected" : "Disabled");

    AddCheckRow("Windows License", info.Windows.Activation.Activated,
        info.Windows.Activation.Activated ? "Activated" : "Unlicensed");

    bool encrypted = false;
    for (const auto& d : info.Security.EncryptedDrives)
        if (d.Encrypted) { encrypted = true; break; }
    AddCheckRow("Drive Encryption", encrypted, encrypted ? "BitLocker Active" : "Off / Optional");

    m_ValidCard->Layout();

    // 5. Storage Drives
    m_StorageDrivesSizer->Clear(true);

    int failingDrives = 0;
    int warningDrives = 0;

    for (size_t i = 0; i < info.Storage.Drives.size(); ++i)
    {
        const auto& drive = info.Storage.Drives[i];

        wxBoxSizer* rowHeader = new wxBoxSizer(wxHORIZONTAL);

        wxString driveName = drive.Model.empty() ? wxString::Format("Drive #%zu", i + 1) : wxString(drive.Model);
        if (!drive.DriveLetter.empty())
            driveName = drive.DriveLetter + " " + driveName;
        if (drive.Type == StorageDeviceType::NVMe)
            driveName += " (NVMe SSD)";
        else if (drive.Type == StorageDeviceType::SSD)
            driveName += " (SSD)";
        else if (drive.Type == StorageDeviceType::HDD)
            driveName += " (HDD)";

        auto* nameLabel = new wxStaticText(m_StorageCard, wxID_ANY, driveName);
        nameLabel->SetFont(FontManager::SemiBold(10));
        nameLabel->SetForegroundColour(wxColour(45, 45, 45));

        wxString capStr = wxString::Format("%.0f GB", drive.CapacityGB);
        auto* capLabel = new wxStaticText(m_StorageCard, wxID_ANY, capStr);
        capLabel->SetFont(FontManager::Regular(10));
        capLabel->SetForegroundColour(wxColour(120, 120, 120));

        rowHeader->Add(nameLabel, 0, wxALIGN_CENTER_VERTICAL);
        rowHeader->AddStretchSpacer();
        rowHeader->Add(capLabel, 0, wxALIGN_CENTER_VERTICAL);

        m_StorageDrivesSizer->Add(rowHeader, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
        m_StorageDrivesSizer->AddSpacer(5);

        int healthVal = drive.SMART.HealthPercent > 0 ? drive.SMART.HealthPercent : 100;
        wxGauge* driveGauge = new wxGauge(m_StorageCard, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 6));
        driveGauge->SetValue(healthVal);

        m_StorageDrivesSizer->Add(driveGauge, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
        m_StorageDrivesSizer->AddSpacer(5);

        // Health thresholds: >=50 Healthy (green), 30-49 Attention (amber), <30 Critical (red).
        wxColour noteColour = healthVal >= 50 ? wxColour(35, 180, 70) :
                             healthVal >= 30 ? wxColour(240, 173, 0) :
                                               wxColour(220, 60, 60);

        if (healthVal < 30 || drive.IsFailing)
            failingDrives++;
        else if (healthVal < 50)
            warningDrives++;

        wxString subText = wxString::Format("Health: %d%% (%s)", healthVal, drive.HealthStatus.empty() ? "Good" : drive.HealthStatus);
        if (drive.SMART.EstimatedRemainingYears > 0.0)
            subText += wxString::Format("  \u2022  Est. Lifetime: ~%.1f Years", drive.SMART.EstimatedRemainingYears);
        if (drive.SMART.Temperature > 0.0)
            subText += wxString::Format("  \u2022  Temp: %.0f \u00B0C", drive.SMART.Temperature);

        auto* healthLabel = new wxStaticText(m_StorageCard, wxID_ANY, subText);
        healthLabel->SetFont(FontManager::Medium(9));
        healthLabel->SetForegroundColour(noteColour);

        m_StorageDrivesSizer->Add(healthLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
        m_StorageDrivesSizer->AddSpacer(10);
    }

    if (info.Storage.Drives.empty())
    {
        auto* emptyLabel = new wxStaticText(m_StorageCard, wxID_ANY, "No physical drives detected");
        emptyLabel->SetFont(FontManager::Regular(10));
        m_StorageDrivesSizer->Add(emptyLabel, 0, wxLEFT | wxTOP, 20);
    }

    m_StorageCard->Layout();

    // 6. Overall Summary & Status Dot
    if (failingDrives > 0)
    {
        m_StatusDot->SetBackgroundColour(wxColour(220, 60, 60));
        m_StatusText->SetLabel("CRITICAL ATTENTION");
        m_SummaryBadgeText->SetLabel("Storage degradation detected");
        m_SummaryBadgeText->SetForegroundColour(wxColour(220, 60, 60));
        m_SummaryBadge->SetBackgroundColour(wxColour(255, 235, 238));
        m_SummaryBody->SetLabel("One or more drives have reported critical SMART warnings or high error rates. We recommend immediately creating a backup of your important files.");
    }
    else if (warningDrives > 0 || (hasBattery && !info.Batteries[0].Healthy))
    {
        m_StatusDot->SetBackgroundColour(wxColour(240, 173, 0));
        m_StatusText->SetLabel("ATTENTION NEEDED");
        m_SummaryBadgeText->SetLabel("Minor wear detected");
        m_SummaryBadgeText->SetForegroundColour(wxColour(240, 173, 0));
        m_SummaryBadge->SetBackgroundColour(wxColour(255, 248, 220));
        m_SummaryBody->SetLabel("Your system is functional, but components such as the battery or storage drive are showing normal wear. Review the Report panel for detailed longevity statistics.");
    }
    else
    {
        m_StatusDot->SetBackgroundColour(wxColour(40, 190, 80));
        m_StatusText->SetLabel("SYSTEM HEALTHY");
        m_SummaryBadgeText->SetLabel("All components operating normally");
        m_SummaryBadgeText->SetForegroundColour(wxColour(35, 180, 70));
        m_SummaryBadge->SetBackgroundColour(wxColour(235, 248, 238));
        m_SummaryBody->SetLabel("Your system hardware is in great health. Storage drives, processor, memory, and security subsystems are operating within optimal parameters.");
    }

    m_SummaryCard->Layout();
    Layout();
}
