#pragma once

#include <wx/wx.h>

class Sidebar : public wxPanel
{
public:
    explicit Sidebar(wxWindow* parent);

private:
    wxStaticBitmap* m_Logo = nullptr;
    wxStaticText*   m_Title = nullptr;

    wxButton* m_Dashboard = nullptr;
    wxButton* m_Test = nullptr;
    wxButton* m_Report = nullptr;
    wxButton* m_Settings = nullptr;
};