#include "Sidebar.hpp"
#include "controls/SidebarItem.hpp"
#include <wx/statline.h>
#include <ctime>

Sidebar::Sidebar(wxWindow *parent)
    : wxPanel(parent,
              wxID_ANY,
              wxDefaultPosition,
              wxSize(220, -1))
{
    SetBackgroundColour(*wxWHITE);

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

    m_Title->SetFont(
        wxFontInfo(11).Bold());

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
                                 { SelectItem(m_Dashboard); });

    m_Test->SetClickHandler([this]()
                            { SelectItem(m_Test); });

    m_Report->SetClickHandler([this]()
                              { SelectItem(m_Report); });

    m_Settings->SetClickHandler([this]()
                                { SelectItem(m_Settings); });

    SelectItem(m_Dashboard);
    mainSizer->Add(m_Dashboard, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    mainSizer->Add(m_Test, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    mainSizer->Add(m_Report, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    mainSizer->AddSpacer(8);

    wxStaticLine *line2 = new wxStaticLine(this);

    mainSizer->Add(line2,
                   0,
                   wxEXPAND | wxLEFT | wxRIGHT,
                   12);

    mainSizer->AddSpacer(12);
    mainSizer->Add(m_Settings, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    std::time_t now = std::time(nullptr);
    std::tm *t = std::localtime(&now);

    int year = 1900 + t->tm_year;

wxString text;
text.Printf(L"\u00A9 %d SysLifeChecker", year);

wxStaticText* copyright =
    new wxStaticText(
        this,
        wxID_ANY,
        text);
        
    copyright->SetFont(
        wxFontInfo(8));
    copyright->SetForegroundColour(
        wxColour(140, 140, 140));

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
}

void Sidebar::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(this);

    dc.SetPen(wxPen(wxColour(225, 225, 225), 1));

    int w, h;
    GetSize(&w, &h);

    dc.DrawLine(w - 1, 0, w - 1, h);
}

void Sidebar::SelectItem(SidebarItem *item)
{
    m_Dashboard->SetSelected(false);
    m_Test->SetSelected(false);
    m_Report->SetSelected(false);
    m_Settings->SetSelected(false);

    item->SetSelected(true);
}