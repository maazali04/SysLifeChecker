#pragma once

#include <wx/wx.h>

class DashboardPanel;
class ReportPanel;
class SettingsDialog;
class TestPanel;

class MainFrame : public wxFrame
{
public:
    MainFrame();

private:
    wxDECLARE_EVENT_TABLE();

    wxPanel* m_ContentContainer = nullptr;
    DashboardPanel* m_DashboardPanel = nullptr;
    TestPanel* m_TestPanel = nullptr;
    ReportPanel* m_ReportPanel = nullptr;
    SettingsDialog* m_SettingsPanel = nullptr;

    // Page indices: 0 = Dashboard, 1 = Test, 2 = Report, 3 = Settings
    void ShowPage(int index);
};
