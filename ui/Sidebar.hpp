#pragma once

#include <wx/wx.h>

class SidebarItem;

class Sidebar : public wxPanel
{
public:
    explicit Sidebar(wxWindow* parent);

private:

void OnPaint(wxPaintEvent& event);
    wxStaticBitmap* m_Logo = nullptr;
    wxStaticText*   m_Title = nullptr;

    SidebarItem* m_Dashboard = nullptr;
    SidebarItem* m_Test = nullptr;
    SidebarItem* m_Report = nullptr;
    SidebarItem* m_Settings = nullptr;
    void SelectItem(SidebarItem* item);
};