#include "Sidebar.hpp"

Sidebar::Sidebar(wxWindow* parent)
    : wxPanel(parent,
              wxID_ANY,
              wxDefaultPosition,
              wxSize(220, -1))
{
    SetBackgroundColour(wxColour(245,245,245));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    //------------------------
    // Logo
    //------------------------

    wxBitmap logo("resources/icons/SysLifeChecker.png",
                  wxBITMAP_TYPE_PNG);

    if (logo.IsOk())
    {
        m_Logo = new wxStaticBitmap(this,
                                    wxID_ANY,
                                    logo);

        mainSizer->Add(m_Logo,
                       0,
                       wxALIGN_CENTER | wxTOP,
                       20);
    }

    //------------------------
    // Title
    //------------------------

    m_Title = new wxStaticText(this,
                               wxID_ANY,
                               "SysLifeChecker");

    wxFont font(
        13,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD);

    m_Title->SetFont(font);

    mainSizer->Add(
        m_Title,
        0,
        wxALIGN_CENTER | wxTOP,
        10);

    mainSizer->AddSpacer(30);

    //------------------------
    // Buttons
    //------------------------

    m_Dashboard = new wxButton(this, wxID_ANY, "Dashboard");
    m_Test      = new wxButton(this, wxID_ANY, "Test");
    m_Report    = new wxButton(this, wxID_ANY, "Report");
    m_Settings  = new wxButton(this, wxID_ANY, "Settings");

    wxButton* buttons[]
    {
        m_Dashboard,
        m_Test,
        m_Report,
        m_Settings
    };

    for (auto button : buttons)
    {
        button->SetMinSize(wxSize(180,40));

        mainSizer->Add(
            button,
            0,
            wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,
            15);
    }

    mainSizer->AddStretchSpacer();

    SetSizer(mainSizer);
}