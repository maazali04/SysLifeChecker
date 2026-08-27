#include "ReportPanel.hpp"
#include "controls/CardPanel.hpp"
#include "FontManager.hpp"
#include "Storage.hpp"
#include "Battery.hpp"
#include "RAM.hpp"
#include "GPU.hpp"
#include "Network.hpp"
#include "WindowsInfo.hpp"
#include "Security.hpp"

#include <wx/statline.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <ctime>

using json = nlohmann::json;

ReportPanel::ReportPanel(wxWindow* parent)
    : wxPanel(parent)
{
    SetBackgroundColour(wxColour(245, 247, 250));
    BuildUI();
}

void ReportPanel::BuildUI()
{
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    //--------------------------------------------------
    // Header
    //--------------------------------------------------
    wxBoxSizer* header = new wxBoxSizer(wxHORIZONTAL);

    auto* title = new wxStaticText(this, wxID_ANY, "Full System Report");
    title->SetFont(FontManager::SemiBold(20));
    header->Add(title, 0, wxALIGN_CENTER_VERTICAL);

    header->AddStretchSpacer();

    m_ExportBtn = new wxButton(this, wxID_ANY, "Export Report...");
    m_ExportBtn->SetFont(FontManager::Medium(10));
    m_ExportBtn->Bind(wxEVT_BUTTON, &ReportPanel::OnExportClicked, this);
    header->Add(m_ExportBtn, 0, wxALIGN_CENTER_VERTICAL);

    rootSizer->Add(header, 0, wxEXPAND | wxALL, 24);

    //--------------------------------------------------
    // Body: category list (left) + detail (right)
    //--------------------------------------------------
    wxBoxSizer* body = new wxBoxSizer(wxHORIZONTAL);

    CardPanel* listCard = new CardPanel(this);
    listCard->SetMinSize(wxSize(240, -1));

    wxBoxSizer* listSizer = new wxBoxSizer(wxVERTICAL);
    listSizer->AddSpacer(12);

    m_CategoryList = new wxListBox(listCard, wxID_ANY);
    m_CategoryList->SetFont(FontManager::Medium(11));

    listSizer->Add(m_CategoryList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    listCard->SetSizer(listSizer);

    m_CategoryList->Bind(wxEVT_LISTBOX, &ReportPanel::OnCategorySelected, this);

    body->Add(listCard, 0, wxEXPAND | wxRIGHT, 18);

    CardPanel* detailCard = new CardPanel(this);
    wxBoxSizer* cardSizer = new wxBoxSizer(wxVERTICAL);

    m_DetailTitle = new wxStaticText(detailCard, wxID_ANY, "Select a Category");
    m_DetailTitle->SetFont(FontManager::SemiBold(14));
    m_DetailTitle->SetForegroundColour(wxColour(30, 41, 59));
    cardSizer->Add(m_DetailTitle, 0, wxLEFT | wxTOP | wxRIGHT, 20);
    cardSizer->AddSpacer(14);

    m_DetailScroll = new wxScrolledWindow(detailCard, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_DetailScroll->SetScrollRate(0, 15);
    m_DetailScroll->SetBackgroundColour(*wxWHITE);

    m_DetailSizer = new wxBoxSizer(wxVERTICAL);
    m_DetailScroll->SetSizer(m_DetailSizer);

    cardSizer->Add(m_DetailScroll, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);
    detailCard->SetSizer(cardSizer);

    body->Add(detailCard, 1, wxEXPAND);

    rootSizer->Add(body, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 24);

    SetSizer(rootSizer);
}

void ReportPanel::PopulateReportData(const SystemInfo& info)
{
    m_ReportData.clear();

    // 1. CPU
    std::vector<std::pair<wxString, wxString>> cpuData;
    cpuData.emplace_back("Processor Name", info.CPU.Name.empty() ? "N/A" : info.CPU.Name);
    cpuData.emplace_back("Manufacturer", info.CPU.Manufacturer.empty() ? "N/A" : info.CPU.Manufacturer);
    cpuData.emplace_back("Architecture", info.CPU.Is64BitOS ? "x64 (64-bit)" : "x86 (32-bit)");
    cpuData.emplace_back("Physical Cores", wxString::Format("%u", info.CPU.PhysicalCores ? info.CPU.PhysicalCores : info.CPU.LogicalProcessors));
    cpuData.emplace_back("Logical Processors", wxString::Format("%u", info.CPU.LogicalProcessors));
    if (info.CPU.Frequency.BaseClockGHz > 0.0)
        cpuData.emplace_back("Base Clock", wxString::Format("%.2f GHz", info.CPU.Frequency.BaseClockGHz));
    if (info.CPU.Frequency.CurrentClockGHz > 0.0)
        cpuData.emplace_back("Current Clock", wxString::Format("%.2f GHz", info.CPU.Frequency.CurrentClockGHz));
    if (info.CPU.Cache.L3KB > 0)
        cpuData.emplace_back("L3 Cache", wxString::Format("%u KB (%.1f MB)", info.CPU.Cache.L3KB, info.CPU.Cache.L3KB / 1024.0));
    if (info.CPU.Cache.L2KB > 0)
        cpuData.emplace_back("L2 Cache", wxString::Format("%u KB", info.CPU.Cache.L2KB));
    cpuData.emplace_back("Hyper-Threading / SMT", info.CPU.HyperThreadingSupported ? "Supported / Enabled" : "Disabled / N/A");
    cpuData.emplace_back("Virtualization (VT-x / AMD-V)", info.CPU.Features.Virtualization ? "Enabled" : "Disabled");
    cpuData.emplace_back("Current CPU Load", wxString::Format("%.1f%%", info.CPU.Status.UsagePercent));
    m_ReportData["1. Processor (CPU)"] = cpuData;

    // 2. RAM
    std::vector<std::pair<wxString, wxString>> ramData;
    ramData.emplace_back("Total Installed RAM", wxString::Format("%.2f GB", info.RAM.TotalCapacityGB));
    ramData.emplace_back("Memory Type", RAMTypeToString(info.RAM.Type));
    if (info.RAM.SpeedMHz > 0)
        ramData.emplace_back("Configured Speed", wxString::Format("%u MHz", info.RAM.SpeedMHz));
    ramData.emplace_back("Channel Mode", info.RAM.ChannelMode == MemoryChannelMode::Dual ? "Dual Channel" :
        (info.RAM.ChannelMode == MemoryChannelMode::Single ? "Single Channel" : "Multi-Channel"));
    ramData.emplace_back("Installed Modules / Slots", wxString::Format("%u / %u", info.RAM.InstalledModules, info.RAM.TotalSlots ? info.RAM.TotalSlots : info.RAM.InstalledModules));
    ramData.emplace_back("Used Memory", wxString::Format("%.2f GB (%.1f%%)", info.RAM.UsedMemoryBytes / (1024.0*1024.0*1024.0), info.RAM.UsagePercent));
    ramData.emplace_back("Available Memory", wxString::Format("%.2f GB", info.RAM.AvailableMemoryBytes / (1024.0*1024.0*1024.0)));
    ramData.emplace_back("Virtual Memory (Pagefile)", wxString::Format("%.2f GB Total (%.2f GB Available)",
        info.RAM.TotalVirtualMemoryBytes / (1024.0*1024.0*1024.0), info.RAM.AvailableVirtualMemoryBytes / (1024.0*1024.0*1024.0)));
    for (size_t i = 0; i < info.RAM.Modules.size(); ++i)
    {
        const auto& m = info.RAM.Modules[i];
        ramData.emplace_back(wxString::Format("Slot #%zu Module", i + 1),
            wxString::Format("%.0f GB %s (%s, %s)", m.CapacityGB, RAMTypeToString(m.Type), m.Manufacturer, m.PartNumber));
    }
    m_ReportData["2. Memory (RAM)"] = ramData;

    // 3. GPU
    std::vector<std::pair<wxString, wxString>> gpuData;
    gpuData.emplace_back("Total Graphics Devices", wxString::Format("%zu", info.GPUs.size()));
    for (size_t i = 0; i < info.GPUs.size(); ++i)
    {
        const auto& g = info.GPUs[i];
        gpuData.emplace_back(wxString::Format("GPU #%zu Name", i + 1), g.Name);
        gpuData.emplace_back(wxString::Format("GPU #%zu Vendor", i + 1), GPUVendorToString(g.Vendor));
        gpuData.emplace_back(wxString::Format("GPU #%zu Type", i + 1), GPUTypeToString(g.Type));
        if (g.DedicatedVRAMGB > 0.05)
            gpuData.emplace_back(wxString::Format("GPU #%zu Dedicated VRAM", i + 1), wxString::Format("%.2f GB", g.DedicatedVRAMGB));
        if (!g.Driver.DriverVersion.empty())
            gpuData.emplace_back(wxString::Format("GPU #%zu Driver Version", i + 1), g.Driver.DriverVersion + " (" + g.Driver.DriverDate + ")");
    }
    m_ReportData["3. Graphics (GPU)"] = gpuData;

    // 4. Storage & SMART
    std::vector<std::pair<wxString, wxString>> storageData;
    storageData.emplace_back("Total Storage Capacity", wxString::Format("%.2f GB", info.Storage.TotalCapacityGB));
    storageData.emplace_back("Total Free Space", wxString::Format("%.2f GB", info.Storage.FreeSpaceGB));
    storageData.emplace_back("Drive Count", wxString::Format("%u (NVMe: %u, SSD: %u, HDD: %u)",
        info.Storage.TotalDrives, info.Storage.NVMeCount, info.Storage.SSDCount, info.Storage.HDDCount));

    for (size_t i = 0; i < info.Storage.Drives.size(); ++i)
    {
        const auto& d = info.Storage.Drives[i];
        wxString prefix = wxString::Format("Drive #%zu (%s)", i + 1, d.DriveLetter.empty() ? "Disk" : d.DriveLetter);
        storageData.emplace_back(prefix + " Model", d.Model);
        storageData.emplace_back(prefix + " Type / Bus", wxString::Format("%s via %s", StorageDeviceTypeToString(d.Type), StorageBusTypeToString(d.Bus)));
        storageData.emplace_back(prefix + " Serial Number", d.SerialNumber);
        storageData.emplace_back(prefix + " Capacity", wxString::Format("%.2f GB", d.CapacityGB));
        storageData.emplace_back(prefix + " Health Status", wxString::Format("%d%% - %s", d.SMART.HealthPercent, d.HealthStatus));
        if (d.SMART.PowerOnHours > 0)
            storageData.emplace_back(prefix + " Power-On Hours", wxString::Format("%llu hours (~%.1f years)", d.SMART.PowerOnHours, d.SMART.PowerOnHours / (24.0 * 365.25)));
        if (d.SMART.Temperature > 0.0)
            storageData.emplace_back(prefix + " Operating Temp", wxString::Format(L"%.0f \u00B0C", d.SMART.Temperature));
        if (d.SMART.PercentageUsed > 0)
            storageData.emplace_back(prefix + " NVMe Life Used", wxString::Format("%llu%%", d.SMART.PercentageUsed));
        if (d.SMART.DataWrittenGB > 0)
            storageData.emplace_back(prefix + " Total Data Written", wxString::Format("%llu GB (~%.2f TB)", d.SMART.DataWrittenGB, d.SMART.DataWrittenGB / 1024.0));
        if (d.SMART.PowerCycles > 0)
            storageData.emplace_back(prefix + " Power Cycles", wxString::Format("%llu", d.SMART.PowerCycles));
        if (d.SMART.UnsafeShutdowns > 0)
            storageData.emplace_back(prefix + " Unsafe Shutdowns", wxString::Format("%llu", d.SMART.UnsafeShutdowns));
        storageData.emplace_back(prefix + " Recommendation", d.Recommendation);
    }
    m_ReportData["4. Storage & Drives"] = storageData;

    // 5. Battery & Power
    std::vector<std::pair<wxString, wxString>> batData;
    bool hasBat = !info.Batteries.empty() && info.Batteries[0].IsBatteryPresent;
    if (hasBat)
    {
        const auto& b = info.Batteries[0];
        batData.emplace_back("Battery Name / Model", b.Name);
        batData.emplace_back("Chemistry", BatteryChemistryToString(b.Chemistry));
        batData.emplace_back("Health Percentage", wxString::Format("%.1f%%", b.HealthPercent));
        batData.emplace_back("Design Capacity", wxString::Format("%.1f Wh (%llu mWh)", b.DesignCapacitymWh / 1000.0, b.DesignCapacitymWh));
        batData.emplace_back("Full Charge Capacity", wxString::Format("%.1f Wh (%llu mWh)", b.FullChargeCapacitymWh / 1000.0, b.FullChargeCapacitymWh));
        batData.emplace_back("Remaining Charge", wxString::Format("%llu%% (%llu mWh)", b.RemainingCapacityPercent, b.RemainingCapacitymWh));
        batData.emplace_back("Cycle Count", wxString::Format("%u cycles", b.CycleCount));
        batData.emplace_back("Charging Status", BatteryStatusToString(b.Status));
        batData.emplace_back("AC Power Connected", b.ACConnected ? "Yes" : "No");
        batData.emplace_back("Condition Assessment", b.ReplaceRecommended ? "Service / Replace Recommended" : (b.Healthy ? "Healthy & Optimal" : "Fair / Aging"));
    }
    else
    {
        batData.emplace_back("Power Source", "AC Mains Connected");
        batData.emplace_back("System Type", "Desktop Workstation (No internal battery)");
        batData.emplace_back("Power Stability", "Normal AC Supply");
    }
    m_ReportData["5. Battery & Power"] = batData;

    // 6. Network & Wireless
    std::vector<std::pair<wxString, wxString>> netData;
    netData.emplace_back("Internet Connection", info.Network.InternetAvailable ? "Connected & Online" : "Disconnected / Offline");
    netData.emplace_back("Total Network Adapters", wxString::Format("%u", info.Network.TotalAdapters));
    for (size_t i = 0; i < info.Network.Adapters.size(); ++i)
    {
        const auto& a = info.Network.Adapters[i];
        if (!a.PhysicalAdapter && a.IPv4.empty())
            continue;
        wxString prefix = wxString::Format("Adapter #%zu", i + 1);
        netData.emplace_back(prefix + " Name", a.Name);
        netData.emplace_back(prefix + " Type", NetworkAdapterTypeToString(a.Type));
        netData.emplace_back(prefix + " Status", NetworkStatusToString(a.Status));
        netData.emplace_back(prefix + " MAC Address", a.MACAddress.empty() ? "N/A" : a.MACAddress);
        if (!a.IPv4.empty())
            netData.emplace_back(prefix + " IPv4 Address", a.IPv4[0].Address + " (Mask: " + a.IPv4[0].SubnetMask + ")");
        if (a.WiFi.Connected)
        {
            netData.emplace_back(prefix + " Wi-Fi SSID", a.WiFi.SSID);
            netData.emplace_back(prefix + " Wi-Fi Standard", a.WiFi.Standard + " (" + a.WiFi.Band + ")");
            netData.emplace_back(prefix + " Signal Strength", wxString::Format("%d%% (%d dBm)", a.WiFi.SignalStrength, a.WiFi.RSSIdBm));
        }
    }
    m_ReportData["6. Network & Wireless"] = netData;

    // 7. Motherboard & BIOS
    std::vector<std::pair<wxString, wxString>> mbData;
    mbData.emplace_back("Motherboard Manufacturer", info.Motherboard.Manufacturer.empty() ? "N/A" : info.Motherboard.Manufacturer);
    mbData.emplace_back("Motherboard Product / Model", info.Motherboard.ProductName.empty() ? "N/A" : info.Motherboard.ProductName);
    mbData.emplace_back("Serial Number", info.Motherboard.SerialNumber.empty() ? "N/A" : info.Motherboard.SerialNumber);
    mbData.emplace_back("System UUID", info.Motherboard.UUID.empty() ? "N/A" : info.Motherboard.UUID);
    mbData.emplace_back("BIOS Vendor", info.BIOS.Vendor.empty() ? "N/A" : info.BIOS.Vendor);
    mbData.emplace_back("BIOS Version", info.BIOS.SMBIOSVersion.empty() ? info.BIOS.Version : info.BIOS.SMBIOSVersion);
    mbData.emplace_back("BIOS Release Date", info.BIOS.ReleaseDate);
    m_ReportData["7. Motherboard & BIOS"] = mbData;

    // 8. Windows OS
    std::vector<std::pair<wxString, wxString>> winData;
    winData.emplace_back("Product Name", info.Windows.ProductName);
    winData.emplace_back("Edition", WindowsEditionToString(info.Windows.Edition));
    winData.emplace_back("Display Version", info.Windows.DisplayVersion);
    winData.emplace_back("Build Number", info.Windows.Build);
    winData.emplace_back("System Uptime", FormatUptime(info.Windows.UptimeSeconds));
    winData.emplace_back("Installation Date", info.Windows.InstallDate);
    winData.emplace_back("Computer Name", info.Windows.ComputerName);
    winData.emplace_back("Registered User", info.Windows.CurrentUser);
    winData.emplace_back("Activation Status", info.Windows.Activation.Activated ? "Activated (" + info.Windows.Activation.ProductKeyChannel + ")" : "Unlicensed");
    m_ReportData["8. Windows OS"] = winData;

    // 9. Security & Protection
    std::vector<std::pair<wxString, wxString>> secData;
    secData.emplace_back("Secure Boot", info.Security.SecureBootEnabled ? "Enabled" : "Disabled");
    secData.emplace_back("TPM Status", info.Security.TPMPresent ?
        (info.Security.TPM == TPMVersion::TPM20 ? "TPM 2.0 (Active & Ready)" : "TPM Active") : "Not Present");
    secData.emplace_back("Microsoft Defender", info.Security.Defender.Enabled ? "Active & Protected" : "Disabled / Replaced");
    secData.emplace_back("Windows Firewall", info.Security.Firewall.Enabled ? "Active" : "Disabled");
    secData.emplace_back("User Account Control (UAC)", info.Security.UserAccountControlEnabled ? "Enabled" : "Disabled");
    secData.emplace_back("Memory Integrity (HVCI)", info.Security.MemoryIntegrityEnabled ? "Enabled" : "Disabled");
    m_ReportData["9. Security & Protection"] = secData;

    // 10. Audio & Cameras
    std::vector<std::pair<wxString, wxString>> devData;
    devData.emplace_back("Total Audio Devices", wxString::Format("%u", info.Audio.TotalDevices));
    for (size_t i = 0; i < info.Audio.Devices.size(); ++i)
    {
        const auto& a = info.Audio.Devices[i];
        devData.emplace_back(wxString::Format("Audio Endpoint #%zu", i + 1),
            a.Name + (a.DefaultPlayback ? " [Default Playback]" : ""));
    }
    devData.emplace_back("Total Cameras", wxString::Format("%u", info.Cameras.TotalCameras));
    for (size_t i = 0; i < info.Cameras.Cameras.size(); ++i)
    {
        const auto& c = info.Cameras.Cameras[i];
        devData.emplace_back(wxString::Format("Camera #%zu", i + 1), c.Name + (c.Connected ? " (Connected)" : ""));
    }
    m_ReportData["10. Audio & Imaging"] = devData;

    // 11. System Drivers
    std::vector<std::pair<wxString, wxString>> drvData;
    drvData.emplace_back("Total Signed Drivers Detected", wxString::Format("%u", info.Drivers.TotalDrivers));
    size_t maxDrivers = (info.Drivers.Drivers.size() > 25) ? 25 : info.Drivers.Drivers.size();
    for (size_t i = 0; i < maxDrivers; ++i)
    {
        const auto& d = info.Drivers.Drivers[i];
        drvData.emplace_back(d.DeviceName, d.Provider + " - v" + d.Version + " (" + d.Date + ")");
    }
    if (info.Drivers.Drivers.size() > 25)
    {
        drvData.emplace_back("Additional Drivers", wxString::Format("+%zu more installed device drivers", info.Drivers.Drivers.size() - 25));
    }
    m_ReportData["11. System Drivers"] = drvData;
}

void ReportPanel::UpdateData(const SystemInfo& info)
{
    m_LatestInfo = info;
    PopulateReportData(info);

    m_CategoryList->Clear();
    for (const auto& entry : m_ReportData)
    {
        m_CategoryList->Append(entry.first);
    }

    if (!m_ReportData.empty())
    {
        m_CategoryList->SetSelection(0);
        ShowCategory(m_ReportData.begin()->first);
    }
}

void ReportPanel::OnCategorySelected(wxCommandEvent& event)
{
    ShowCategory(event.GetString());
}

void ReportPanel::ShowCategory(const wxString& category)
{
    m_DetailSizer->Clear(true);
    m_DetailTitle->SetLabel(category);

    auto it = m_ReportData.find(category);
    if (it == m_ReportData.end())
    {
        m_DetailScroll->Layout();
        return;
    }

    for (const auto& row : it->second)
    {
        wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

        auto* left = new wxStaticText(m_DetailScroll, wxID_ANY, row.first);
        left->SetFont(FontManager::Regular(10));
        left->SetForegroundColour(wxColour(110, 110, 110));

        auto* right = new wxStaticText(m_DetailScroll, wxID_ANY, row.second);
        right->SetFont(FontManager::SemiBold(10));
        right->SetForegroundColour(wxColour(40, 40, 40));

        rowSizer->Add(left, 0, wxALIGN_CENTER_VERTICAL);
        rowSizer->AddStretchSpacer();
        rowSizer->Add(right, 0, wxALIGN_CENTER_VERTICAL);

        m_DetailSizer->Add(rowSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 14);

        wxStaticLine* line = new wxStaticLine(m_DetailScroll);
        m_DetailSizer->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    }

    m_DetailScroll->FitInside();
    m_DetailScroll->Layout();
}

void ReportPanel::OnExportClicked(wxCommandEvent&)
{
    ExportReport(ExportFormat::Prompt);
}

void ReportPanel::ExportReport(ExportFormat format)
{
    wxString wildcard;
    wxString defaultExt;

    switch (format)
    {
    case ExportFormat::HTML:
        wildcard = "HTML Report (*.html)|*.html";
        defaultExt = ".html";
        break;
    case ExportFormat::JSON:
        wildcard = "JSON Data (*.json)|*.json";
        defaultExt = ".json";
        break;
    case ExportFormat::Text:
        wildcard = "Text Document (*.txt)|*.txt";
        defaultExt = ".txt";
        break;
    case ExportFormat::Prompt:
    default:
        wildcard = "HTML Report (*.html)|*.html|JSON Data (*.json)|*.json|Text Document (*.txt)|*.txt";
        break;
    }

    wxFileDialog saveFileDialog(this, _("Save System Report"), "", "SysLifeChecker_Report" + defaultExt,
        wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    std::string path = saveFileDialog.GetPath().ToStdString();
    int filterIndex = saveFileDialog.GetFilterIndex();

    std::ofstream out(path);
    if (!out.is_open())
    {
        wxMessageBox("Could not open file for writing: " + saveFileDialog.GetPath(), "Export Error", wxICON_ERROR);
        return;
    }

    // When a specific format was requested (from the File > Export
    // submenu), honor it directly instead of re-deriving from the
    // dialog's filter index/path -- that's what made those menu items
    // behave identically to the generic "Save / Export Report..." button.
    bool wantsJson = (format == ExportFormat::JSON) ||
        (format == ExportFormat::Prompt && (filterIndex == 1 || path.find(".json") != std::string::npos));
    bool wantsHtml = (format == ExportFormat::HTML) ||
        (format == ExportFormat::Prompt && !wantsJson && (filterIndex == 0 || path.find(".html") != std::string::npos));

    if (wantsJson)
    {
        // JSON Export
        json j;
        for (const auto& cat : m_ReportData)
        {
            json items;
            for (const auto& item : cat.second)
            {
                items[item.first.ToStdString()] = item.second.ToStdString();
            }
            j[cat.first.ToStdString()] = items;
        }
        out << j.dump(4);
    }
    else if (wantsHtml)
    {
        // HTML Export
        out << "<!DOCTYPE html>\n<html>\n<head>\n";
        out << "<meta charset=\"utf-8\">\n<title>SysLifeChecker System Report</title>\n";
        out << "<style>\n";
        out << "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f5f7fa; color: #333; margin: 0; padding: 30px; }\n";
        out << ".container { max-width: 960px; margin: 0 auto; background: #fff; border-radius: 12px; box-shadow: 0 4px 16px rgba(0,0,0,0.08); padding: 36px; }\n";
        out << "h1 { color: #0078d7; margin-top: 0; display: flex; align-items: center; gap: 12px; border-bottom: 2px solid #eef2f6; padding-bottom: 16px; }\n";
        out << "h2 { color: #1e293b; margin-top: 28px; border-bottom: 1px solid #e2e8f0; padding-bottom: 8px; font-size: 1.2rem; }\n";
        out << "table { width: 100%; border-collapse: collapse; margin-top: 10px; }\n";
        out << "th, td { padding: 10px 14px; text-align: left; border-bottom: 1px solid #f1f5f9; }\n";
        out << "th { width: 38%; color: #64748b; font-weight: 500; }\n";
        out << "td { width: 62%; color: #1e293b; font-weight: 600; }\n";
        out << ".footer { margin-top: 36px; text-align: center; color: #94a3b8; font-size: 0.85rem; border-top: 1px solid #e2e8f0; padding-top: 16px; }\n";
        out << "</style>\n</head>\n<body>\n";
        out << "<div class=\"container\">\n";
        out << "<h1>SysLifeChecker &bull; System Specification & Diagnostic Report</h1>\n";

        for (const auto& cat : m_ReportData)
        {
            out << "<h2>" << cat.first.ToStdString() << "</h2>\n";
            out << "<table>\n";
            for (const auto& item : cat.second)
            {
                out << "<tr><th>" << item.first.ToStdString() << "</th><td>" << item.second.ToStdString() << "</td></tr>\n";
            }
            out << "</table>\n";
        }

        out << "<div class=\"footer\">Generated by SysLifeChecker on " << __DATE__ << "</div>\n";
        out << "</div>\n</body>\n</html>\n";
    }
    else
    {
        // Text Export
        out << "================================================================================\n";
        out << "SysLifeChecker System Specification & Diagnostic Report\n";
        out << "================================================================================\n\n";

        for (const auto& cat : m_ReportData)
        {
            out << "[" << cat.first.ToStdString() << "]\n";
            out << "--------------------------------------------------------------------------------\n";
            for (const auto& item : cat.second)
            {
                out << std::left << std::setw(32) << item.first.ToStdString() << " : " << item.second.ToStdString() << "\n";
            }
            out << "\n";
        }
    }

    out.close();
    wxMessageBox("System report successfully exported to:\n" + saveFileDialog.GetPath(), "Export Successful", wxICON_INFORMATION);
}
