#include "MainFrame.hpp"
#include "Sidebar.hpp"
#include "MenuBar.hpp"
#include "DashboardPanel.hpp"
#include "ReportPanel.hpp"
#include "SettingsDialog.hpp"
#include "TestPanel.hpp"
#include "FontManager.hpp"
#include <wx/icon.h>
#include <wx/aboutdlg.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
wxEND_EVENT_TABLE()

namespace
{
    // wxTimer defaults to id wxID_ANY (-1) for both timers, which makes
    // Bind()'s id filter unable to tell them apart. Give each its own id.
    constexpr int ID_LOADING_PULSE_TIMER = wxID_HIGHEST + 500;
    constexpr int ID_REFRESH_TIMER = wxID_HIGHEST + 501;
}

MainFrame::MainFrame()
    : wxFrame(nullptr,
              wxID_ANY,
              "SysLifeChecker - Hardware & Life Monitor",
              wxDefaultPosition,
              wxSize(1240, 740)),
      m_LoadingPulseTimer(this, ID_LOADING_PULSE_TIMER),
      m_RefreshTimer(this, ID_REFRESH_TIMER)
{
    SetIcon(wxIcon("resources/icons/SysLifeChecker.ico", wxBITMAP_TYPE_ICO));

    SetMenuBar(MenuBar::Create());

    wxPanel *root = new wxPanel(this);
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    //--------------------------------------------------
    // Content area
    //--------------------------------------------------
    m_ContentContainer = new wxPanel(root);
    wxBoxSizer *contentSizer = new wxBoxSizer(wxVERTICAL);

    // Loading page, shown first while the initial hardware scan runs.
    m_LoadingPanel = new wxPanel(m_ContentContainer);
    m_LoadingPanel->SetBackgroundColour(wxColour(245, 247, 250));
    {
        wxBoxSizer* loadingSizer = new wxBoxSizer(wxVERTICAL);
        loadingSizer->AddStretchSpacer();

        auto* label = new wxStaticText(m_LoadingPanel, wxID_ANY, "Scanning your system...");
        label->SetFont(FontManager::Medium(13));
        label->SetForegroundColour(wxColour(80, 80, 80));
        loadingSizer->Add(label, 0, wxALIGN_CENTER_HORIZONTAL);
        loadingSizer->AddSpacer(14);

        m_LoadingGauge = new wxGauge(m_LoadingPanel, wxID_ANY, 100,
            wxDefaultPosition, wxSize(240, 8), wxGA_HORIZONTAL);
        m_LoadingGauge->Pulse();
        loadingSizer->Add(m_LoadingGauge, 0, wxALIGN_CENTER_HORIZONTAL);

        loadingSizer->AddStretchSpacer();
        m_LoadingPanel->SetSizer(loadingSizer);
    }

    m_DashboardPanel = new DashboardPanel(m_ContentContainer);
    m_TestPanel = new TestPanel(m_ContentContainer);
    m_ReportPanel = new ReportPanel(m_ContentContainer);
    m_SettingsPanel = new SettingsDialog(m_ContentContainer,
        [this](const SettingsDialog::Settings& settings)
        {
            // Restart the refresh timer with the new interval.
            m_RefreshTimer.Stop();
            m_RefreshTimer.Start(settings.RefreshIntervalSeconds * 1000);
        });

    contentSizer->Add(m_LoadingPanel, 1, wxEXPAND);
    contentSizer->Add(m_DashboardPanel, 1, wxEXPAND);
    contentSizer->Add(m_TestPanel, 1, wxEXPAND);
    contentSizer->Add(m_ReportPanel, 1, wxEXPAND);
    contentSizer->Add(m_SettingsPanel, 1, wxEXPAND);

    m_ContentContainer->SetSizer(contentSizer);

    contentSizer->Show(m_DashboardPanel, false);
    contentSizer->Show(m_TestPanel, false);
    contentSizer->Show(m_ReportPanel, false);
    contentSizer->Show(m_SettingsPanel, false);

    //--------------------------------------------------
    // Sidebar
    //--------------------------------------------------
    m_Sidebar = new Sidebar(root, [this](int page) {
        ShowPage(page);
    });

    sizer->Add(m_Sidebar, 0, wxEXPAND | wxALL, 0);
    sizer->Add(m_ContentContainer, 1, wxEXPAND);

    root->SetSizer(sizer);

    // Bind Background Hardware Monitoring Event
    Bind(EVT_SYSTEM_INFO_READY, &MainFrame::OnSystemInfoReady, this);
    Bind(wxEVT_TIMER, &MainFrame::OnRefreshTimer, this, m_RefreshTimer.GetId());
    Bind(wxEVT_TIMER, &MainFrame::OnLoadingPulseTimer, this, m_LoadingPulseTimer.GetId());

    // Bind Menu Events
    Bind(wxEVT_MENU, &MainFrame::OnMenuNewScan, this, ID_MENU_NEW_SCAN);
    Bind(wxEVT_MENU, &MainFrame::OnMenuExportReport, this, ID_MENU_SAVE_REPORT);
    Bind(wxEVT_MENU, &MainFrame::OnMenuExportReport, this, ID_MENU_EXPORT_HTML);
    Bind(wxEVT_MENU, &MainFrame::OnMenuExportReport, this, ID_MENU_EXPORT_JSON);
    Bind(wxEVT_MENU, &MainFrame::OnMenuRunTests, this, ID_MENU_RUN_ALL_TESTS);
    Bind(wxEVT_MENU, &MainFrame::OnMenuAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainFrame::OnMenuExit, this, wxID_EXIT);

    Bind(wxEVT_MENU, [this](wxCommandEvent&) { if (m_Sidebar) m_Sidebar->SelectPage(0); else ShowPage(0); }, ID_MENU_VIEW_DASHBOARD);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { if (m_Sidebar) m_Sidebar->SelectPage(1); else ShowPage(1); }, ID_MENU_VIEW_TEST);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { if (m_Sidebar) m_Sidebar->SelectPage(2); else ShowPage(2); }, ID_MENU_VIEW_REPORT);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { if (m_Sidebar) m_Sidebar->SelectPage(3); else ShowPage(3); }, ID_MENU_VIEW_SETTINGS);

    Centre();

    // Animate the loading gauge (wxGauge needs Pulse() called
    // periodically to show indeterminate progress).
    m_LoadingPulseTimer.Start(100);

    // Start background hardware scanning. The first result flips us
    // from the loading page over to the Dashboard (see OnSystemInfoReady).
    m_MonitorService = std::make_unique<SystemMonitorService>(this);
    m_MonitorService->RefreshAsync();

    // Default periodic refresh; Settings can change this at runtime.
    m_RefreshTimer.Start(60000);
}

MainFrame::~MainFrame()
{
}

void MainFrame::ShowPage(int index)
{
    wxSizer *sizer = m_ContentContainer->GetSizer();
    if (!sizer)
        return;

    // Once the first scan has completed we never show the loading page
    // again; until then, keep it up regardless of what the sidebar asks
    // for (there's nothing real to show yet).
    if (!m_FirstLoadComplete)
        return;

    sizer->Show(m_LoadingPanel, false);
    sizer->Show(m_DashboardPanel, index == 0);
    sizer->Show(m_TestPanel, index == 1);
    sizer->Show(m_ReportPanel, index == 2);
    sizer->Show(m_SettingsPanel, index == 3);

    m_ContentContainer->Layout();
}

void MainFrame::RefreshHardwareData()
{
    if (m_MonitorService)
    {
        m_MonitorService->RefreshAsync();
    }
}

void MainFrame::OnSystemInfoReady(wxCommandEvent&)
{
    if (!m_MonitorService || !m_MonitorService->HasData())
        return;

    const SystemInfo& info = m_MonitorService->LatestInfo();

    if (m_DashboardPanel)
        m_DashboardPanel->UpdateData(info);

    if (m_ReportPanel)
        m_ReportPanel->UpdateData(info);

    if (m_TestPanel)
        m_TestPanel->UpdateData(info);

    if (!m_FirstLoadComplete)
    {
        m_FirstLoadComplete = true;
        m_LoadingPulseTimer.Stop();

        wxSizer* sizer = m_ContentContainer->GetSizer();
        if (sizer)
        {
            sizer->Show(m_LoadingPanel, false);
            sizer->Show(m_DashboardPanel, true);
            m_ContentContainer->Layout();
        }
    }
}

void MainFrame::OnRefreshTimer(wxTimerEvent&)
{
    RefreshHardwareData();
}

void MainFrame::OnLoadingPulseTimer(wxTimerEvent&)
{
    if (m_LoadingGauge)
        m_LoadingGauge->Pulse();
}

void MainFrame::OnMenuNewScan(wxCommandEvent&)
{
    RefreshHardwareData();
}

void MainFrame::OnMenuExportReport(wxCommandEvent& event)
{
    if (!m_ReportPanel)
        return;

    // The File > Export submenu has specific-format entries; the
    // top-level "Save / Export Report..." stays a generic prompt.
    switch (event.GetId())
    {
    case ID_MENU_EXPORT_HTML:
        m_ReportPanel->ExportReport(ReportPanel::ExportFormat::HTML);
        break;
    case ID_MENU_EXPORT_JSON:
        m_ReportPanel->ExportReport(ReportPanel::ExportFormat::JSON);
        break;
    default:
        m_ReportPanel->ExportReport(ReportPanel::ExportFormat::Prompt);
        break;
    }
}

void MainFrame::OnMenuRunTests(wxCommandEvent&)
{
    if (m_Sidebar)
        m_Sidebar->SelectPage(1);
    else
        ShowPage(1);

    if (m_TestPanel)
        m_TestPanel->RunAllTests();
}

void MainFrame::OnMenuAbout(wxCommandEvent&)
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("SysLifeChecker");
    aboutInfo.SetVersion("1.0.0");
    aboutInfo.SetDescription(
        "Lightweight, accessible System Specification, SSD/HDD Lifetime, Battery Health, "
        "and Hardware Diagnostics Software.");
    aboutInfo.SetCopyright("(C) 2026 SysLifeChecker Contributors");
    aboutInfo.AddDeveloper("SysLifeChecker Team");

    wxAboutBox(aboutInfo, this);
}

void MainFrame::OnMenuExit(wxCommandEvent&)
{
    Close(true);
}
