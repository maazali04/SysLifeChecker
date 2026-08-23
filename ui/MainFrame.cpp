#include "MainFrame.hpp"
#include "Sidebar.hpp"
#include "MenuBar.hpp"
#include "DashboardPanel.hpp"
#include "ReportPanel.hpp"
#include "SettingsDialog.hpp"
#include "TestPanel.hpp"
#include <wx/icon.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    wxEND_EVENT_TABLE()

        MainFrame::MainFrame()
    : wxFrame(nullptr,
              wxID_ANY,
              "SysLifeChecker",
              wxDefaultPosition,
              wxSize(1200, 700))
{
    SetIcon(wxIcon("resources/icons/SysLifeChecker.ico",
                   wxBITMAP_TYPE_ICO));

    SetMenuBar(MenuBar::Create());
    wxPanel *root = new wxPanel(this);

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    //--------------------------------------------------
    // Content area: all four pages live here, stacked in
    // one sizer, with only the active one shown.
    //--------------------------------------------------
    m_ContentContainer = new wxPanel(root);
    wxBoxSizer *contentSizer = new wxBoxSizer(wxVERTICAL);

    m_DashboardPanel = new DashboardPanel(m_ContentContainer);
    m_TestPanel = new TestPanel(m_ContentContainer);
    m_ReportPanel = new ReportPanel(m_ContentContainer);
    m_SettingsPanel = new SettingsDialog(m_ContentContainer);

    contentSizer->Add(m_DashboardPanel, 1, wxEXPAND);
    contentSizer->Add(m_TestPanel, 1, wxEXPAND);
    contentSizer->Add(m_ReportPanel, 1, wxEXPAND);
    contentSizer->Add(m_SettingsPanel, 1, wxEXPAND);

    m_ContentContainer->SetSizer(contentSizer);

    contentSizer->Show(m_TestPanel, false);
    contentSizer->Show(m_ReportPanel, false);
    contentSizer->Show(m_SettingsPanel, false);

    //--------------------------------------------------
    // Sidebar: created after the content pages exist, since
    // it selects "Dashboard" immediately and fires the
    // navigation callback during construction.
    //--------------------------------------------------
    Sidebar *sidebar = new Sidebar(root, [this](int page)
                                    { ShowPage(page); });

    sizer->Add(sidebar, 0, wxEXPAND | wxALL, 0);
    sizer->Add(m_ContentContainer, 1, wxEXPAND);

    root->SetSizer(sizer);

    Centre();
}

void MainFrame::ShowPage(int index)
{
    wxSizer *sizer = m_ContentContainer->GetSizer();
    if (!sizer)
        return;

    sizer->Show(m_DashboardPanel, index == 0);
    sizer->Show(m_TestPanel, index == 1);
    sizer->Show(m_ReportPanel, index == 2);
    sizer->Show(m_SettingsPanel, index == 3);

    sizer->Layout();
}
