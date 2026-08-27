#include "Sidebar.hpp"
#include "FontManager.hpp"
#include "Theme.hpp"
#include "controls/SidebarItem.hpp"
#include <wx/statline.h>
#include <wx/dcbuffer.h>
#include <ctime>

Sidebar::Sidebar(wxWindow *parent, std::function<void(int)> onNavigate)
    : wxPanel(parent,
              wxID_ANY,
              wxDefaultPosition,
              wxSize(220, -1)),
      m_OnNavigate(std::move(onNavigate))
{
    SetBackgroundColour(Theme::SidebarBackground());

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    //==========================
    // Logo + Title
    //==========================

    wxBoxSizer *headerSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap logo(
        "resources/icons/SysLifeChecker_28.png",
        wxBITMAP_TYPE_PNG);

    if (logo.IsOk())
    {
        m_Logo = new wxStaticBitmap(
            this,
            wxID_ANY,
            logo);

        headerSizer->Add(
            m_Logo,
            0,
            wxALIGN_CENTER_VERTICAL);
    }

    m_Title = new wxStaticText(
        this,
        wxID_ANY,
        "SysLifeChecker");

    m_Title->SetFont(FontManager::Bold(13));
    m_Title->SetForegroundColour(Theme::SidebarTextColour());

    //  increase size between logo and text
    headerSizer->Add(
        m_Title,
        0,
        wxLEFT | wxALIGN_CENTER_VERTICAL,
        14);

    mainSizer->Add(
        headerSizer,
        0,
        wxLEFT | wxTOP,
        18);
    mainSizer->AddSpacer(25);

    wxStaticLine *line = new wxStaticLine(this);
    line->SetForegroundColour(wxColour(45,45,45));

    mainSizer->Add(
        line,
        0,
        wxEXPAND | wxLEFT | wxRIGHT,
        12);

    mainSizer->AddSpacer(25);

    //------------------------
    // Navigation
    //------------------------

    m_Dashboard = new SidebarItem(
        this,
        "Dashboard",
        "resources/icons/sidebar/dashboard_black.png",
        "resources/icons/sidebar/dashboard_blue.png");
    m_Test = new SidebarItem(
        this,
        "Test",
        "resources/icons/sidebar/test_black.png",
        "resources/icons/sidebar/test_blue.png");

    m_Report = new SidebarItem(
        this,
        "Report",
        "resources/icons/sidebar/report_black.png",
        "resources/icons/sidebar/report_blue.png");

    m_Settings = new SidebarItem(
        this,
        "Settings",
        "resources/icons/sidebar/settings_black.png",
        "resources/icons/sidebar/settings_blue.png");

    //  Select Dashboard initially
    m_Dashboard->SetClickHandler([this]()
                                 { SelectItem(m_Dashboard, 0); });

    m_Test->SetClickHandler([this]()
                            { SelectItem(m_Test, 1); });

    m_Report->SetClickHandler([this]()
                              { SelectItem(m_Report, 2); });

    m_Settings->SetClickHandler([this]()
                                { SelectItem(m_Settings, 3); });

    SelectItem(m_Dashboard, 0);
    mainSizer->Add(m_Dashboard, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    mainSizer->Add(m_Test, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    mainSizer->Add(m_Report, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    mainSizer->AddSpacer(8);

    wxStaticLine *line2 = new wxStaticLine(this);

    mainSizer->Add(line2,
                   0,
                   wxEXPAND | wxLEFT | wxRIGHT,
                   12);

    mainSizer->AddSpacer(8);
    mainSizer->Add(m_Settings, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    std::time_t now = std::time(nullptr);
    std::tm *t = std::localtime(&now);

    int year = 1900 + t->tm_year;

    wxString text;
    text.Printf(L"\u00A9 %d SysLifeChecker", year);

    wxStaticText *copyright =
        new wxStaticText(
            this,
            wxID_ANY,
            text);

    copyright->SetFont(FontManager::Regular(8));
    copyright->SetForegroundColour(Theme::SidebarTextColour());
    m_Copyright = copyright;

    mainSizer->AddStretchSpacer();

    mainSizer->Add(
        copyright,
        0,
        wxALIGN_CENTER | wxBOTTOM,
        14);

    SetSizer(mainSizer);
    Bind(wxEVT_PAINT,
         &Sidebar::OnPaint,
         this);
    Bind(wxEVT_SIZE, &Sidebar::OnSize, this);
}

void Sidebar::OnSize(wxSizeEvent& event)
{
    Refresh(false);

    event.Skip();
}

void Sidebar::OnPaint(wxPaintEvent&)
{
    wxBufferedPaintDC dc(this);

    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();

    dc.SetPen(wxPen(Theme::BorderColour(),1));

    int w, h;
    GetClientSize(&w, &h);

    dc.DrawLine(w - 1, 0, w - 1, h);
}
void Sidebar::SelectItem(SidebarItem *item, int pageIndex)
{
    m_Dashboard->SetSelected(false);
    m_Test->SetSelected(false);
    m_Report->SetSelected(false);
    m_Settings->SetSelected(false);

    item->SetSelected(true);

    if (m_OnNavigate)
        m_OnNavigate(pageIndex);
}

void Sidebar::SelectPage(int pageIndex)
{
    if (pageIndex == 0 && m_Dashboard) SelectItem(m_Dashboard, 0);
    else if (pageIndex == 1 && m_Test) SelectItem(m_Test, 1);
    else if (pageIndex == 2 && m_Report) SelectItem(m_Report, 2);
    else if (pageIndex == 3 && m_Settings) SelectItem(m_Settings, 3);
}

void Sidebar::ApplyTheme()
{
    SetBackgroundColour(Theme::SidebarBackground());

    if (m_Title)
        m_Title->SetForegroundColour(Theme::SidebarTextColour());

    if (m_Copyright)
        m_Copyright->SetForegroundColour(Theme::SidebarTextColour());

    if (m_Dashboard) m_Dashboard->ApplyTheme();
    if (m_Test) m_Test->ApplyTheme();
    if (m_Report) m_Report->ApplyTheme();
    if (m_Settings) m_Settings->ApplyTheme();

    Refresh();
}