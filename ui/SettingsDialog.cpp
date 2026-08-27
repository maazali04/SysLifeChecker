#include "SettingsDialog.hpp"
#include "controls/CardPanel.hpp"
#include "FontManager.hpp"
#include <wx/statline.h>
#include <wx/spinctrl.h>

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

    auto* title = new wxStaticText(this, wxID_ANY, "Settings");
    title->SetFont(FontManager::SemiBold(20));
    rootSizer->Add(title, 0, wxLEFT | wxTOP | wxRIGHT, 24);
    rootSizer->AddSpacer(24);

    //===========================================
    // General card -- just the two settings that
    // actually do something.
    //===========================================
    {
        CardPanel* card = new CardPanel(this);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->AddSpacer(20);

        // Refresh interval
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

        wxStaticLine* line = new wxStaticLine(card);
        sizer->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);

        // Dark mode -- recolors the whole app immediately.
        {
            m_DarkModeCheck = new wxCheckBox(card, wxID_ANY, "Dark mode");
            m_DarkModeCheck->SetFont(FontManager::Regular(10));
            m_DarkModeCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&)
            {
                m_Settings.DarkMode = m_DarkModeCheck->GetValue();
                NotifyChanged();
            });
            sizer->Add(m_DarkModeCheck, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        card->SetSizer(sizer);
        rootSizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT, 24);
        rootSizer->AddSpacer(20);
    }

    //===========================================
    // About card
    //===========================================
    {
        CardPanel* card = new CardPanel(this);
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
        rootSizer->Add(card, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 24);
    }

    rootSizer->AddStretchSpacer();
    SetSizer(rootSizer);
}
