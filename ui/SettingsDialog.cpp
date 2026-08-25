#include "SettingsDialog.hpp"
#include "controls/CardPanel.hpp"
#include "FontManager.hpp"
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <windows.h>

namespace
{
    // Actually writes/removes the "start with Windows" registry entry,
    // instead of just remembering the checkbox state.
    void ApplyStartWithWindows(bool enable)
    {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
            return;

        if (enable)
        {
            wchar_t exePath[MAX_PATH]{};
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
            RegSetValueExW(hKey, L"SysLifeChecker", 0, REG_SZ,
                reinterpret_cast<const BYTE*>(exePath),
                static_cast<DWORD>((wcslen(exePath) + 1) * sizeof(wchar_t)));
        }
        else
        {
            RegDeleteValueW(hKey, L"SysLifeChecker");
        }

        RegCloseKey(hKey);
    }
}

SettingsDialog::SettingsDialog(wxWindow* parent, std::function<void(const Settings&)> onChanged)
    : wxPanel(parent), m_OnChanged(std::move(onChanged))
{
    SetBackgroundColour(wxColour(245, 247, 250));
    BuildUI();
}

void SettingsDialog::NotifyChanged()
{
    if (m_OnChanged)
        m_OnChanged(m_Settings);
}

void SettingsDialog::BuildUI()
{
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    wxScrolledWindow* scroll = new wxScrolledWindow(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    scroll->SetScrollRate(0, 20);
    scroll->SetBackgroundColour(wxColour(245, 247, 250));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    auto* title = new wxStaticText(scroll, wxID_ANY, "Settings");
    title->SetFont(FontManager::SemiBold(20));
    mainSizer->Add(title, 0, wxLEFT | wxTOP | wxRIGHT, 24);
    mainSizer->AddSpacer(24);

    //===========================================
    // General card
    //===========================================
    {
        CardPanel* card = new CardPanel(scroll);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        auto* heading = new wxStaticText(card, wxID_ANY, "General");
        heading->SetFont(FontManager::Medium(12));
        heading->SetForegroundColour(wxColour(30, 41, 59));
        sizer->Add(heading, 0, wxLEFT | wxTOP, 20);
        sizer->AddSpacer(16);

        // Refresh interval -- this one actually takes effect: MainFrame
        // restarts its background refresh timer with this value.
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);

            auto* label = new wxStaticText(card, wxID_ANY, "Refresh interval (seconds)");
            label->SetFont(FontManager::Regular(10));
            label->SetForegroundColour(wxColour(90, 90, 90));

            m_RefreshSpin = new wxSpinCtrl(
                card, wxID_ANY, "60", wxDefaultPosition, wxSize(80, -1),
                wxSP_ARROW_KEYS, 5, 3600, 60);

            m_RefreshSpin->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent&)
            {
                m_Settings.RefreshIntervalSeconds = m_RefreshSpin->GetValue();
                NotifyChanged();
            });
            // Also fires on typed entry (not just the arrow buttons).
            m_RefreshSpin->Bind(wxEVT_TEXT, [this](wxCommandEvent&)
            {
                m_Settings.RefreshIntervalSeconds = m_RefreshSpin->GetValue();
                NotifyChanged();
            });

            row->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(m_RefreshSpin, 0, wxALIGN_CENTER_VERTICAL);

            sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Start with Windows
        // TODO: wire to an actual HKCU\...\Run registry entry. Stored in
        // m_Settings for now but not yet actioned.
        {
            m_StartupCheck = new wxCheckBox(card, wxID_ANY, "Start SysLifeChecker with Windows");
            m_StartupCheck->SetFont(FontManager::Regular(10));
            m_StartupCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&)
            {
                m_Settings.StartWithWindows = m_StartupCheck->GetValue();
                ApplyStartWithWindows(m_Settings.StartWithWindows);
                NotifyChanged();
            });
            sizer->Add(m_StartupCheck, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Minimize to tray -- MainFrame actually hides the window and
        // shows a tray icon on minimize when this is checked.
        {
            m_TrayCheck = new wxCheckBox(card, wxID_ANY, "Minimize to system tray");
            m_TrayCheck->SetFont(FontManager::Regular(10));
            m_TrayCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&)
            {
                m_Settings.MinimizeToTray = m_TrayCheck->GetValue();
                NotifyChanged();
            });
            sizer->Add(m_TrayCheck, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        card->SetSizer(sizer);
        mainSizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT, 24);
        mainSizer->AddSpacer(20);
    }

    //===========================================
    // Units & Appearance card
    //===========================================
    {
        CardPanel* card = new CardPanel(scroll);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        auto* heading = new wxStaticText(card, wxID_ANY, "Units & Appearance");
        heading->SetFont(FontManager::Medium(12));
        heading->SetForegroundColour(wxColour(30, 41, 59));
        sizer->Add(heading, 0, wxLEFT | wxTOP, 20);
        sizer->AddSpacer(16);

        // Capacity units
        // TODO: Dashboard/Report currently always format in GB. Stored
        // here but formatting helpers don't consult it yet.
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            auto* label = new wxStaticText(card, wxID_ANY, "Capacity units");
            label->SetFont(FontManager::Regular(10));
            label->SetForegroundColour(wxColour(90, 90, 90));

            wxString choices[] = { "GB (decimal)", "GiB (binary)" };
            m_UnitsChoice = new wxChoice(card, wxID_ANY, wxDefaultPosition, wxSize(160, -1), 2, choices);
            m_UnitsChoice->SetSelection(0);
            m_UnitsChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&)
            {
                m_Settings.UseGiB = (m_UnitsChoice->GetSelection() == 1);
                NotifyChanged();
            });

            row->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(m_UnitsChoice, 0, wxALIGN_CENTER_VERTICAL);

            sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Temperature units
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            auto* label = new wxStaticText(card, wxID_ANY, "Temperature units");
            label->SetFont(FontManager::Regular(10));
            label->SetForegroundColour(wxColour(90, 90, 90));

            wxString choices[] = { "Celsius (\u00B0C)", "Fahrenheit (\u00B0F)" };
            m_TempChoice = new wxChoice(card, wxID_ANY, wxDefaultPosition, wxSize(160, -1), 2, choices);
            m_TempChoice->SetSelection(0);
            m_TempChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&)
            {
                m_Settings.UseFahrenheit = (m_TempChoice->GetSelection() == 1);
                NotifyChanged();
            });

            row->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(m_TempChoice, 0, wxALIGN_CENTER_VERTICAL);

            sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Theme
        // TODO: Theme.cpp defines the color constants but panels don't yet
        // re-read them at runtime, so switching this doesn't repaint the
        // app. Stored so the wiring can be finished later.
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            auto* label = new wxStaticText(card, wxID_ANY, "Theme");
            label->SetFont(FontManager::Regular(10));
            label->SetForegroundColour(wxColour(90, 90, 90));

            wxString choices[] = { "Light", "Dark", "Match system" };
            m_ThemeChoice = new wxChoice(card, wxID_ANY, wxDefaultPosition, wxSize(160, -1), 3, choices);
            m_ThemeChoice->SetSelection(0);
            m_ThemeChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&)
            {
                m_Settings.ThemeIndex = m_ThemeChoice->GetSelection();
                NotifyChanged();
            });

            row->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(m_ThemeChoice, 0, wxALIGN_CENTER_VERTICAL);

            sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Advanced/technical detail toggle -- ties into the
        // Simple/Advanced switch on the Report panel.
        {
            m_AdvancedCheck = new wxCheckBox(card, wxID_ANY, "Show advanced technical details by default");
            m_AdvancedCheck->SetFont(FontManager::Regular(10));
            m_AdvancedCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&)
            {
                m_Settings.AdvancedDetailsByDefault = m_AdvancedCheck->GetValue();
                NotifyChanged();
            });
            sizer->Add(m_AdvancedCheck, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        card->SetSizer(sizer);
        mainSizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT, 24);
        mainSizer->AddSpacer(20);
    }

    //===========================================
    // About card
    //===========================================
    {
        CardPanel* card = new CardPanel(scroll);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->AddSpacer(20);

        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        row->AddSpacer(20);

        wxBitmap logo("resources/icons/SysLifeChecker_64.png", wxBITMAP_TYPE_PNG);
        if (logo.IsOk())
        {
            wxStaticBitmap* logoBmp = new wxStaticBitmap(card, wxID_ANY, logo);
            row->Add(logoBmp, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
        }

        wxBoxSizer* textCol = new wxBoxSizer(wxVERTICAL);

        auto* name = new wxStaticText(card, wxID_ANY, "SysLifeChecker");
        name->SetFont(FontManager::SemiBold(13));
        name->SetForegroundColour(wxColour(30, 41, 59));

        auto* version = new wxStaticText(card, wxID_ANY, "Version 1.0.0");
        version->SetFont(FontManager::Regular(10));
        version->SetForegroundColour(wxColour(120, 120, 120));

        auto* author = new wxStaticText(card, wxID_ANY, "Developed by Maaz Ali (maazali04)");
        author->SetFont(FontManager::Regular(9));
        author->SetForegroundColour(wxColour(140, 140, 140));

        textCol->Add(name);
        textCol->AddSpacer(4);
        textCol->Add(version);
        textCol->AddSpacer(2);
        textCol->Add(author);

        row->Add(textCol, 0, wxALIGN_CENTER_VERTICAL);

        sizer->Add(row, 0, wxBOTTOM, 20);

        card->SetSizer(sizer);
        mainSizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 24);
    }

    scroll->SetSizer(mainSizer);
    rootSizer->Add(scroll, 1, wxEXPAND);
    SetSizer(rootSizer);
}
