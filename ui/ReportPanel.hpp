#pragma once

#include <wx/wx.h>

class ReportPanel : public wxPanel
{
public:
    explicit ReportPanel(wxWindow* parent);

private:
    void BuildUI();
    void ShowCategory(const wxString& category);

    wxListBox* m_CategoryList = nullptr;
    wxPanel* m_DetailContainer = nullptr;
    wxBoxSizer* m_DetailSizer = nullptr;
    wxStaticText* m_DetailTitle = nullptr;

    void OnCategorySelected(wxCommandEvent& event);
};
