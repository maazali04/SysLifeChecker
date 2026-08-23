#include "ReportPanel.hpp"
#include "controls/CardPanel.hpp"
#include "FontManager.hpp"
#include <wx/statline.h>
#include <map>
#include <vector>

namespace
{
    // TODO: replace this placeholder table with real values pulled from
    // SystemInfo (CPUInfo, RAMInfo, GPUInfo, StorageInfo, ...) via the
    // service layer described in the architecture plan. One entry per
    // wxListBox category below.
    const std::map<wxString, std::vector<std::pair<wxString, wxString>>>& ReportData()
    {
        static const std::map<wxString, std::vector<std::pair<wxString, wxString>>> data =
        {
            { "CPU", {
                { "Name", "Intel Core i7-12700H" },
                { "Architecture", "x64" },
                { "Physical Cores", "14" },
                { "Logical Processors", "20" },
                { "Base Clock", "2.30 GHz" },
                { "Current Usage", "18%" },
            }},
            { "RAM", {
                { "Total Capacity", "16.0 GB" },
                { "Type", "DDR4" },
                { "Speed", "3200 MHz" },
                { "Modules", "2" },
                { "Usage", "54%" },
            }},
            { "GPU", {
                { "Name", "NVIDIA GeForce RTX 3060" },
                { "Dedicated VRAM", "6.0 GB" },
                { "Driver Version", "32.0.15.6094" },
            }},
            { "Storage", {
                { "Drive C: (SSD)", "512 GB, 96% health" },
                { "Drive D: (HDD)", "1.0 TB, 82% health" },
            }},
            { "Battery", {
                { "Health", "87%" },
                { "Cycle Count", "182" },
                { "Manufacturer", "LG Chem" },
            }},
            { "Network", {
                { "Adapters", "2 (1 connected)" },
                { "Internet", "Available" },
            }},
            { "BIOS / Motherboard", {
                { "BIOS Mode", "UEFI" },
                { "Secure Boot", "Enabled" },
                { "TPM", "2.0, Ready" },
            }},
            { "Windows", {
                { "Edition", "Windows 11 Pro" },
                { "Build", "26100" },
                { "Version", "24H2" },
            }},
            { "Security", {
                { "Firewall", "On" },
                { "Antivirus", "Active" },
                { "BitLocker", "Off" },
            }},
        };
        return data;
    }
}

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

    auto* title = new wxStaticText(this, wxID_ANY, "Report");
    title->SetFont(FontManager::SemiBold(20));
    header->Add(title, 0, wxALIGN_CENTER_VERTICAL);

    header->AddStretchSpacer();

    // TODO: wire this to real export (txt/json now, PDF later via the
    // nlohmann::json already vendored in include/).
    wxButton* exportBtn = new wxButton(this, wxID_ANY, "Export Report");
    header->Add(exportBtn, 0, wxALIGN_CENTER_VERTICAL);

    rootSizer->Add(header, 0, wxEXPAND | wxALL, 24);

    //--------------------------------------------------
    // Body: category list (left) + detail (right)
    //--------------------------------------------------
    wxBoxSizer* body = new wxBoxSizer(wxHORIZONTAL);

    CardPanel* listCard = new CardPanel(this);
    listCard->SetMinSize(wxSize(230, -1));

    wxBoxSizer* listSizer = new wxBoxSizer(wxVERTICAL);
    listSizer->AddSpacer(12);

    m_CategoryList = new wxListBox(listCard, wxID_ANY);
    m_CategoryList->SetFont(FontManager::Regular(11));

    for (const auto& entry : ReportData())
        m_CategoryList->Append(entry.first);

    listSizer->Add(m_CategoryList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    listCard->SetSizer(listSizer);

    m_CategoryList->Bind(wxEVT_LISTBOX, &ReportPanel::OnCategorySelected, this);

    body->Add(listCard, 0, wxEXPAND | wxRIGHT, 18);

    CardPanel* detailCard = new CardPanel(this);
    m_DetailContainer = detailCard;

    m_DetailSizer = new wxBoxSizer(wxVERTICAL);

    m_DetailTitle = new wxStaticText(detailCard, wxID_ANY, "Select a category");
    m_DetailTitle->SetFont(FontManager::Medium(13));
    m_DetailTitle->SetForegroundColour(wxColour(30, 41, 59));

    m_DetailSizer->Add(m_DetailTitle, 0, wxLEFT | wxTOP, 20);
    m_DetailSizer->AddSpacer(16);

    detailCard->SetSizer(m_DetailSizer);

    body->Add(detailCard, 1, wxEXPAND);

    rootSizer->Add(body, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 24);

    SetSizer(rootSizer);

    if (!ReportData().empty())
    {
        m_CategoryList->SetSelection(0);
        ShowCategory(ReportData().begin()->first);
    }
}

void ReportPanel::OnCategorySelected(wxCommandEvent& event)
{
    ShowCategory(event.GetString());
}

void ReportPanel::ShowCategory(const wxString& category)
{
    // Clear everything except the title (index 0) and its spacer.
    while (m_DetailSizer->GetItemCount() > 2)
        m_DetailSizer->Remove(2);

    for (auto* child : m_DetailContainer->GetChildren())
    {
        if (child != m_DetailTitle)
            child->Destroy();
    }

    m_DetailTitle->SetLabel(category);

    auto it = ReportData().find(category);
    if (it == ReportData().end())
    {
        m_DetailContainer->Layout();
        return;
    }

    for (const auto& row : it->second)
    {
        wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

        auto* left = new wxStaticText(m_DetailContainer, wxID_ANY, row.first);
        left->SetFont(FontManager::Regular(10));
        left->SetForegroundColour(wxColour(120, 120, 120));

        auto* right = new wxStaticText(m_DetailContainer, wxID_ANY, row.second);
        right->SetFont(FontManager::SemiBold(10));
        right->SetForegroundColour(wxColour(55, 55, 55));

        rowSizer->Add(left, 0, wxALIGN_CENTER_VERTICAL);
        rowSizer->AddStretchSpacer();
        rowSizer->Add(right, 0, wxALIGN_CENTER_VERTICAL);

        m_DetailSizer->Add(rowSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 14);
    }

    m_DetailContainer->Layout();
}
