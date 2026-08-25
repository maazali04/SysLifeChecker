#pragma once

#include <wx/wx.h>
#include <wx/panel.h>
#include <functional>

class SidebarItem;

class Sidebar : public wxPanel
{
public:
    // Page indices: 0 = Dashboard, 1 = Test, 2 = Report, 3 = Settings
    explicit Sidebar(wxWindow *parent, std::function<void(int)> onNavigate = nullptr);
    void SelectPage(int pageIndex);

private:
    void OnPaint(wxPaintEvent &event);
    void OnSize(wxSizeEvent& event);
    wxStaticBitmap *m_Logo = nullptr;
    wxStaticText *m_Title = nullptr;

    SidebarItem *m_Dashboard = nullptr;
    SidebarItem *m_Test = nullptr;
    SidebarItem *m_Report = nullptr;
    SidebarItem *m_Settings = nullptr;

    std::function<void(int)> m_OnNavigate;

    void SelectItem(SidebarItem *item, int pageIndex);
};
