#include "DashboardPanel.hpp"
#include "controls/CardPanel.hpp"
#include "FontManager.hpp"
#include <wx/statline.h>

DashboardPanel::DashboardPanel(wxWindow *parent)
    : wxPanel(parent)
{
    SetBackgroundColour(wxColour(245, 247, 250));

    BuildUI();
}

void DashboardPanel::BuildUI()
{
    wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);

    //--------------------------------------------------
    // Scroll Window
    //--------------------------------------------------

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

    auto *title =
        new wxStaticText(
            scroll,
            wxID_ANY,
            "Dashboard");

    title->SetFont(FontManager::SemiBold(20));

    header->Add(title, 0, wxALIGN_CENTER_VERTICAL);

    header->AddStretchSpacer();

    auto *lastScan =
        new wxStaticText(
            scroll,
            wxID_ANY,
            "Last Scan : Jul 13, 2026   09:42 AM");

    lastScan->SetFont(FontManager::Regular(10));

    header->Add(lastScan, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

    CardPanel *status = new CardPanel(scroll);
    status->SetMinSize(wxSize(170, 40));

    wxBoxSizer *statusSizer = new wxBoxSizer(wxHORIZONTAL);

    statusSizer->AddSpacer(15);

    // dot
    wxPanel *statusDot = new wxPanel(status);
    statusDot->SetMinSize(wxSize(10, 10));
    statusDot->SetMaxSize(wxSize(10, 10));

    // Dummy color
    statusDot->SetBackgroundColour(wxColour(40, 190, 80)); // Green
    // Later:
    // Green  -> wxColour(35,180,70)
    // Yellow -> wxColour(255,185,0)
    // Red    -> wxColour(220,60,60)
    // Gray   -> wxColour(150,150,150)

    statusSizer->Add(statusDot,
                     0,
                     wxALIGN_CENTER_VERTICAL | wxRIGHT,
                     8);

    auto *txt =
        new wxStaticText(
            status,
            wxID_ANY,
            "HEALTHY"); // 🟡 Attention //  🔴  Critical

    txt->SetFont(FontManager::Medium(10));

    statusSizer->Add(txt, 0, wxALIGN_CENTER_VERTICAL);

    status->SetSizer(statusSizer);

    header->Add(status,
                0,
                wxALIGN_CENTER_VERTICAL);

    mainSizer->Add(
        header,
        0,
        wxEXPAND | wxLEFT | wxRIGHT | wxTOP,
        24);

    mainSizer->AddSpacer(24);

    // top row
    wxBoxSizer *row1 = new wxBoxSizer(wxHORIZONTAL);

    CardPanel *battery = new CardPanel(scroll);
    CardPanel *specs = new CardPanel(scroll);
    CardPanel *valid = new CardPanel(scroll);

    battery->SetMinSize(wxSize(0, 260));
    specs->SetMinSize(wxSize(0, 260));
    valid->SetMinSize(wxSize(0, 260));

    row1->Add(battery, 1, wxRIGHT, 18);
    row1->Add(specs, 1, wxRIGHT, 18);
    row1->Add(valid, 1);

     //===========================================
    // 🔋 BATTERY CARD INTERNAL LAYOUT
    //===========================================
    wxBoxSizer *batterySizer = new wxBoxSizer(wxVERTICAL);
    
    // 🟢 CHANGED variable name from 'title' to 'batteryTitle' to fix the redeclaration error
    wxStaticText *batteryTitle = new wxStaticText(battery,
                                                  wxID_ANY,
                                                  "Battery Health");
    batteryTitle->SetFont(FontManager::Medium(12));
    batteryTitle->SetForegroundColour(wxColour(30, 41, 59));
    batterySizer->Add(batteryTitle,
                      0,
                      wxLEFT | wxTOP,
                      20);

    // percentage
    wxStaticText* percent = new wxStaticText(battery,
                                             wxID_ANY,
                                             "87%");
    percent->SetFont(FontManager::Bold(28));
    percent->SetForegroundColour(wxColour(35, 180, 70));

    batterySizer->AddSpacer(10);
    batterySizer->Add(percent, 0, wxALIGN_CENTER_HORIZONTAL);

    // progressbar
    wxGauge* gauge = new wxGauge(battery,
                                 wxID_ANY,
                                 100,
                                 wxDefaultPosition,
                                 wxSize(180, 8));
    gauge->SetValue(87);

    batterySizer->AddSpacer(10);
    batterySizer->Add(gauge, 0, wxALIGN_CENTER_HORIZONTAL);

    // status badge
    CardPanel* badge = new CardPanel(battery);
    badge->SetBackgroundColour(wxColour(235, 248, 238));

    wxBoxSizer* badgeSizer = new wxBoxSizer(wxHORIZONTAL);
    badgeSizer->AddSpacer(12);

    wxStaticText* badgeText = new wxStaticText(badge,
                                               wxID_ANY,
                                               "Good");
    badgeText->SetFont(FontManager::SemiBold(9));
    badgeText->SetForegroundColour(wxColour(35, 180, 70));

    badgeSizer->Add(badgeText, 0, wxALIGN_CENTER_VERTICAL);
    badgeSizer->AddSpacer(12);
    badge->SetSizerAndFit(badgeSizer);

    batterySizer->AddSpacer(12);
    batterySizer->Add(badge, 0, wxALIGN_CENTER_HORIZONTAL);

    // Separator
    wxStaticLine* line = new wxStaticLine(battery);
    batterySizer->AddSpacer(15);
    batterySizer->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
    batterySizer->AddSpacer(15); // Balanced layout padding
    
    // Lambda row design engine
    auto AddInfoRow = [&](const wxString& name, const wxString& value)
    {
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);

        auto* left = new wxStaticText(battery, wxID_ANY, name);
        left->SetFont(FontManager::Regular(10));
        left->SetForegroundColour(wxColour(120, 120, 120));

        auto* right = new wxStaticText(battery, wxID_ANY, value);
        right->SetFont(FontManager::SemiBold(10));
        right->SetForegroundColour(wxColour(55, 55, 55));

        row->Add(left, 0, wxALIGN_CENTER_VERTICAL);
        row->AddStretchSpacer();
        row->Add(right, 0, wxALIGN_CENTER_VERTICAL);

        // 🟢 Optimized padding setup so it matches your card boundaries beautifully
        batterySizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    };

    AddInfoRow("Current Capacity", "47.8 Wh");
    AddInfoRow("Design Capacity", "55.0 Wh");
    AddInfoRow("Cycle Count", "182");
    AddInfoRow("Manufacturer", "LG Chem");

    battery->SetSizer(batterySizer);
    //===========================================

    //===========================================
    // 🧩 HARDWARE SPECS CARD
    //===========================================
    {
        wxBoxSizer* specsSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* specsTitle = new wxStaticText(specs, wxID_ANY, "Hardware Specs");
        specsTitle->SetFont(FontManager::Medium(12));
        specsTitle->SetForegroundColour(wxColour(30, 41, 59));
        specsSizer->Add(specsTitle, 0, wxLEFT | wxTOP, 20);
        specsSizer->AddSpacer(16);

        auto AddSpecRow = [&](const wxString& name, const wxString& value)
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);

            auto* left = new wxStaticText(specs, wxID_ANY, name);
            left->SetFont(FontManager::Regular(10));
            left->SetForegroundColour(wxColour(120, 120, 120));

            auto* right = new wxStaticText(specs, wxID_ANY, value);
            right->SetFont(FontManager::SemiBold(10));
            right->SetForegroundColour(wxColour(55, 55, 55));

            row->Add(left, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(right, 0, wxALIGN_CENTER_VERTICAL);

            specsSizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 14);
        };

        // TODO: replace with live values from GetCPUInfo()/GetRAMInfo()/
        // GetGPUInfo()/GetStorageInfo()/GetWindowsInfo() via the service layer.
        AddSpecRow("Processor", "Intel Core i7-12700H");
        AddSpecRow("Cores / Threads", "14 / 20");
        AddSpecRow("Memory", "16.0 GB DDR4");
        AddSpecRow("Graphics", "NVIDIA RTX 3060");
        AddSpecRow("Storage", "1.0 TB (SSD + HDD)");
        AddSpecRow("OS", "Windows 11 Pro, 24H2");

        specs->SetSizer(specsSizer);
    }

    //===========================================
    // ✅ SYSTEM VALIDATION CARD
    //===========================================
    {
        wxBoxSizer* validSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* validTitle = new wxStaticText(valid, wxID_ANY, "System Validation");
        validTitle->SetFont(FontManager::Medium(12));
        validTitle->SetForegroundColour(wxColour(30, 41, 59));
        validSizer->Add(validTitle, 0, wxLEFT | wxTOP, 20);
        validSizer->AddSpacer(16);

        auto AddCheckRow = [&](const wxString& name, bool ok, const wxString& value)
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);

            wxPanel* dot = new wxPanel(valid);
            dot->SetMinSize(wxSize(8, 8));
            dot->SetMaxSize(wxSize(8, 8));
            dot->SetBackgroundColour(ok ? wxColour(35, 180, 70) : wxColour(220, 60, 60));

            row->Add(dot, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

            auto* left = new wxStaticText(valid, wxID_ANY, name);
            left->SetFont(FontManager::Regular(10));
            left->SetForegroundColour(wxColour(90, 90, 90));

            auto* right = new wxStaticText(valid, wxID_ANY, value);
            right->SetFont(FontManager::SemiBold(10));
            right->SetForegroundColour(ok ? wxColour(35, 180, 70) : wxColour(220, 60, 60));

            row->Add(left, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(right, 0, wxALIGN_CENTER_VERTICAL);

            validSizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 14);
        };

        // TODO: replace with live values from GetSecurityInfo()/GetBIOSInfo()
        // via the service layer.
        AddCheckRow("Secure Boot", true, "Enabled");
        AddCheckRow("TPM", true, "Ready");
        AddCheckRow("Antivirus", true, "Active");
        AddCheckRow("Firewall", true, "On");
        AddCheckRow("Windows Update", true, "Up to date");
        AddCheckRow("BitLocker", false, "Off");

        valid->SetSizer(validSizer);
    }

// adding rows
    mainSizer->Add(
        row1,
        0,
        wxEXPAND | wxLEFT | wxRIGHT,
        24);

    mainSizer->AddSpacer(24);

    // bottom row
    wxBoxSizer *row2 = new wxBoxSizer(wxHORIZONTAL);

    CardPanel *storage = new CardPanel(scroll);
    CardPanel *summary = new CardPanel(scroll);

    storage->SetMinSize(wxSize(0, 320));
    summary->SetMinSize(wxSize(0, 320));

    row2->Add(storage, 3, wxRIGHT, 18);
    row2->Add(summary, 2);

    //===========================================
    // 💾 STORAGE CARD
    //===========================================
    {
        wxBoxSizer* storageSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* storageTitle = new wxStaticText(storage, wxID_ANY, "Storage");
        storageTitle->SetFont(FontManager::Medium(12));
        storageTitle->SetForegroundColour(wxColour(30, 41, 59));
        storageSizer->Add(storageTitle, 0, wxLEFT | wxTOP, 20);
        storageSizer->AddSpacer(18);

        auto AddDriveRow = [&](const wxString& name, const wxString& capacity,
                                int healthPercent, const wxString& note)
        {
            wxBoxSizer* rowHeader = new wxBoxSizer(wxHORIZONTAL);

            auto* nameLabel = new wxStaticText(storage, wxID_ANY, name);
            nameLabel->SetFont(FontManager::SemiBold(11));
            nameLabel->SetForegroundColour(wxColour(45, 45, 45));

            auto* capLabel = new wxStaticText(storage, wxID_ANY, capacity);
            capLabel->SetFont(FontManager::Regular(10));
            capLabel->SetForegroundColour(wxColour(120, 120, 120));

            rowHeader->Add(nameLabel, 0, wxALIGN_CENTER_VERTICAL);
            rowHeader->AddStretchSpacer();
            rowHeader->Add(capLabel, 0, wxALIGN_CENTER_VERTICAL);

            storageSizer->Add(rowHeader, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
            storageSizer->AddSpacer(6);

            wxGauge* driveGauge = new wxGauge(storage, wxID_ANY, 100,
                                          wxDefaultPosition, wxSize(-1, 8));
            driveGauge->SetValue(healthPercent);

            storageSizer->Add(driveGauge, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);
            storageSizer->AddSpacer(6);

            wxColour noteColour =
                healthPercent >= 80 ? wxColour(35, 180, 70) :
                healthPercent >= 50 ? wxColour(240, 173, 0) :
                                       wxColour(220, 60, 60);

            wxString label;
            label.Printf("Health: %d%%   %s", healthPercent, note);

            auto* healthLabel = new wxStaticText(storage, wxID_ANY, label);
            healthLabel->SetFont(FontManager::Medium(9));
            healthLabel->SetForegroundColour(noteColour);

            storageSizer->Add(healthLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
            storageSizer->AddSpacer(16);
        };

        // TODO: replace with live values from GetStorageInfo().Drives[].SMART
        // via the service layer (one AddDriveRow per StorageDevice).
        AddDriveRow("Drive C: (SSD)", "512 GB, 61% used", 96, "Excellent");
        AddDriveRow("Drive D: (HDD)", "1.0 TB, 44% used", 82, "Good");

        storage->SetSizer(storageSizer);
    }

    //===========================================
    // 📋 OVERALL SUMMARY CARD
    //===========================================
    {
        wxBoxSizer* summarySizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* summaryTitle = new wxStaticText(summary, wxID_ANY, "Overall Summary");
        summaryTitle->SetFont(FontManager::Medium(12));
        summaryTitle->SetForegroundColour(wxColour(30, 41, 59));
        summarySizer->Add(summaryTitle, 0, wxLEFT | wxTOP, 20);
        summarySizer->AddSpacer(14);

        CardPanel* summaryBadge = new CardPanel(summary);
        summaryBadge->SetBackgroundColour(wxColour(235, 248, 238));

        wxBoxSizer* summaryBadgeSizer = new wxBoxSizer(wxHORIZONTAL);
        summaryBadgeSizer->AddSpacer(12);
        wxStaticText* summaryBadgeText = new wxStaticText(summaryBadge, wxID_ANY, "No critical issues found");
        summaryBadgeText->SetFont(FontManager::SemiBold(9));
        summaryBadgeText->SetForegroundColour(wxColour(35, 180, 70));
        summaryBadgeSizer->Add(summaryBadgeText, 0, wxALIGN_CENTER_VERTICAL);
        summaryBadgeSizer->AddSpacer(12);
        summaryBadge->SetSizerAndFit(summaryBadgeSizer);

        summarySizer->Add(summaryBadge, 0, wxLEFT | wxRIGHT, 20);
        summarySizer->AddSpacer(16);

        // TODO: generate this sentence in the service layer from the
        // aggregated SystemInfo instead of hard-coding it here.
        wxStaticText* body = new wxStaticText(
            summary, wxID_ANY,
            "Your system is in good health. Battery is holding a strong "
            "charge and storage drives are performing well with plenty of "
            "free space. Keep Windows Update current and consider enabling "
            "BitLocker for extra protection.");

        body->SetFont(FontManager::Regular(10));
        body->SetForegroundColour(wxColour(90, 90, 90));
        body->Wrap(260);

        summarySizer->Add(body, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);

        summary->SetSizer(summarySizer);
    }

    mainSizer->Add(
        row2,
        0,
        wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,
        24);

    // last
    scroll->SetSizer(mainSizer);

    rootSizer->Add(scroll, 1, wxEXPAND);

    SetSizer(rootSizer);
}
