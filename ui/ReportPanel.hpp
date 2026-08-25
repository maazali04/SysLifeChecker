#pragma once

#include <wx/wx.h>
#include <map>
#include <vector>
#include "SystemInfo.hpp"

class CardPanel;

class ReportPanel : public wxPanel
{
public:
    explicit ReportPanel(wxWindow* parent);

    enum class ExportFormat { Prompt, HTML, JSON, Text };

    void UpdateData(const SystemInfo& info);
    void ExportReport(ExportFormat format = ExportFormat::Prompt);

private:
    void BuildUI();
    void ShowCategory(const wxString& category);
    void PopulateReportData(const SystemInfo& info);

    wxListBox* m_CategoryList = nullptr;
    wxScrolledWindow* m_DetailScroll = nullptr;
    wxBoxSizer* m_DetailSizer = nullptr;
    wxStaticText* m_DetailTitle = nullptr;
    wxButton* m_ExportBtn = nullptr;

    SystemInfo m_LatestInfo;
    std::map<wxString, std::vector<std::pair<wxString, wxString>>> m_ReportData;

    void OnCategorySelected(wxCommandEvent& event);
    void OnExportClicked(wxCommandEvent& event);
};
