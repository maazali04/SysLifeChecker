#include "SettingsDialog.hpp"
#include "controls/CardPanel.hpp"
#include "FontManager.hpp"
#include <wx/statline.h>
#include <wx/spinctrl.h>

SettingsDialog::SettingsDialog(wxWindow* parent)
    : wxPanel(parent)
{
    SetBackgroundColour(wxColour(245, 247, 250));
    BuildUI();
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

        // Refresh interval
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);

            auto* label = new wxStaticText(card, wxID_ANY, "Refresh interval (seconds)");
            label->SetFont(FontManager::Regular(10));
            label->SetForegroundColour(wxColour(90, 90, 90));

            wxSpinCtrl* spin = new wxSpinCtrl(
                card, wxID_ANY, "60", wxDefaultPosition, wxSize(80, -1),
                wxSP_ARROW_KEYS, 5, 3600, 60);

            row->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(spin, 0, wxALIGN_CENTER_VERTICAL);

            sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Start with Windows
        {
            wxCheckBox* startup = new wxCheckBox(card, wxID_ANY, "Start SysLifeChecker with Windows");
            startup->SetFont(FontManager::Regular(10));
            sizer->Add(startup, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Minimize to tray
        {
            wxCheckBox* tray = new wxCheckBox(card, wxID_ANY, "Minimize to system tray");
            tray->SetFont(FontManager::Regular(10));
            sizer->Add(tray, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
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
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            auto* label = new wxStaticText(card, wxID_ANY, "Capacity units");
            label->SetFont(FontManager::Regular(10));
            label->SetForegroundColour(wxColour(90, 90, 90));

            wxString choices[] = { "GB (decimal)", "GiB (binary)" };
            wxChoice* unitChoice = new wxChoice(card, wxID_ANY, wxDefaultPosition, wxSize(160, -1), 2, choices);
            unitChoice->SetSelection(0);

            row->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(unitChoice, 0, wxALIGN_CENTER_VERTICAL);

            sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Temperature units
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            auto* label = new wxStaticText(card, wxID_ANY, "Temperature units");
            label->SetFont(FontManager::Regular(10));
            label->SetForegroundColour(wxColour(90, 90, 90));

            wxString choices[] = { "Celsius (\u00B0C)", "Fahrenheit (\u00B0F)" };
            wxChoice* tempChoice = new wxChoice(card, wxID_ANY, wxDefaultPosition, wxSize(160, -1), 2, choices);
            tempChoice->SetSelection(0);

            row->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(tempChoice, 0, wxALIGN_CENTER_VERTICAL);

            sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Theme
        {
            wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
            auto* label = new wxStaticText(card, wxID_ANY, "Theme");
            label->SetFont(FontManager::Regular(10));
            label->SetForegroundColour(wxColour(90, 90, 90));

            wxString choices[] = { "Light", "Dark", "Match system" };
            wxChoice* themeChoice = new wxChoice(card, wxID_ANY, wxDefaultPosition, wxSize(160, -1), 3, choices);
            themeChoice->SetSelection(0);

            row->Add(label, 0, wxALIGN_CENTER_VERTICAL);
            row->AddStretchSpacer();
            row->Add(themeChoice, 0, wxALIGN_CENTER_VERTICAL);

            sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        // Advanced/technical detail toggle -- ties into the
        // Simple/Advanced switch on the Report panel.
        {
            wxCheckBox* advanced = new wxCheckBox(card, wxID_ANY, "Show advanced technical details by default");
            advanced->SetFont(FontManager::Regular(10));
            sizer->Add(advanced, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
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

        auto* version = new wxStaticText(card, wxID_ANY, "Version 0.1.0");
        version->SetFont(FontManager::Regular(10));
        version->SetForegroundColour(wxColour(120, 120, 120));

        textCol->Add(name);
        textCol->AddSpacer(4);
        textCol->Add(version);

        row->Add(textCol, 0, wxALIGN_CENTER_VERTICAL);

        sizer->Add(row, 0, wxBOTTOM, 20);

        card->SetSizer(sizer);
        mainSizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 24);
    }

    scroll->SetSizer(mainSizer);
    rootSizer->Add(scroll, 1, wxEXPAND);
    SetSizer(rootSizer);
}
