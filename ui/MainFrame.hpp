#pragma once

#include <wx/wx.h>
#include <memory>
#include "SystemMonitorService.hpp"

class DashboardPanel;
class ReportPanel;
class SettingsDialog;
class TestPanel;
class Sidebar;

class MainFrame : public wxFrame
{
public:
    MainFrame();
    ~MainFrame();

    void ShowPage(int index);
    void RefreshHardwareData();

private:
    wxDECLARE_EVENT_TABLE();

    wxPanel* m_ContentContainer = nullptr;
    DashboardPanel* m_DashboardPanel = nullptr;
    TestPanel* m_TestPanel = nullptr;
    ReportPanel* m_ReportPanel = nullptr;
    SettingsDialog* m_SettingsPanel = nullptr;
    Sidebar* m_Sidebar = nullptr;

    // Shown in place of the pages above until the first hardware scan
    // completes, so the app never displays hard-coded placeholder values.
    wxPanel* m_LoadingPanel = nullptr;
    wxGauge* m_LoadingGauge = nullptr;
    wxTimer m_LoadingPulseTimer;
    bool m_FirstLoadComplete = false;

    // Periodic background refresh; interval is controlled from Settings.
    wxTimer m_RefreshTimer;

    std::unique_ptr<SystemMonitorService> m_MonitorService;

    void OnSystemInfoReady(wxCommandEvent& event);
    void OnRefreshTimer(wxTimerEvent& event);
    void OnLoadingPulseTimer(wxTimerEvent& event);
    void OnMenuNewScan(wxCommandEvent& event);
    void OnMenuExportReport(wxCommandEvent& event);
    void OnMenuRunTests(wxCommandEvent& event);
    void OnMenuAbout(wxCommandEvent& event);
    void OnMenuExit(wxCommandEvent& event);
};
