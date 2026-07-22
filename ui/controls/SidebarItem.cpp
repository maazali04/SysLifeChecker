#include "SidebarItem.hpp"

SidebarItem::SidebarItem(
    wxWindow *parent,
    const wxString &text,
    const wxString &blackIcon,
    const wxString &blueIcon)
    : wxPanel(
          parent,
          wxID_ANY,
          wxDefaultPosition,
          wxSize(-1, 40))
{
    SetDoubleBuffered(true);
    SetBackgroundColour(*wxWHITE);
    if (!m_BlackBitmap.LoadFile(blackIcon, wxBITMAP_TYPE_PNG))
    {
        wxLogError("Failed to load %s", blackIcon);
    }

    if (!m_BlueBitmap.LoadFile(blueIcon, wxBITMAP_TYPE_PNG))
    {
        wxLogError("Failed to load %s", blueIcon);
    }

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->AddSpacer(16);

    m_Icon = new wxStaticBitmap(
        this,
        wxID_ANY,
        m_BlackBitmap);

    sizer->Add(
        m_Icon,
        0,
        wxALIGN_CENTER_VERTICAL);

    sizer->AddSpacer(12);

    m_Text = new wxStaticText(this,
                              wxID_ANY,
                              text);

    wxFont font(
        10,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_SEMIBOLD);

    m_Text->SetFont(font);

    sizer->Add(m_Text,
               0,
               wxALIGN_CENTER_VERTICAL);

    sizer->AddStretchSpacer();

    SetSizer(sizer);

    Bind(wxEVT_LEFT_DOWN,
         &SidebarItem::OnClick,
         this);

    m_Text->Bind(wxEVT_LEFT_DOWN,
                 &SidebarItem::OnClick,
                 this);
    m_Icon->Bind(wxEVT_LEFT_DOWN,
                 &SidebarItem::OnClick,
                 this);

    Bind(wxEVT_MOTION,
         &SidebarItem::OnMouseMove,
         this);

    Bind(wxEVT_LEAVE_WINDOW,
         &SidebarItem::OnMouseLeave,
         this);

    m_Icon->Bind(wxEVT_MOTION,
                 &SidebarItem::OnMouseMove,
                 this);

    m_Text->Bind(wxEVT_MOTION,
                 &SidebarItem::OnMouseMove,
                 this);

    SetCursor(wxCursor(wxCURSOR_HAND));
    m_Text->SetCursor(wxCursor(wxCURSOR_HAND));
    m_Icon->SetCursor(wxCursor(wxCURSOR_HAND));
    
}

void SidebarItem::SetSelected(bool selected)
{
    m_Selected = selected;

    if (selected)
    {
        SetBackgroundColour(wxColour(235, 235, 235));
        m_Text->SetForegroundColour(wxColour(0, 120, 215));
        m_Icon->SetBitmap(m_BlueBitmap);
    }
    else
    {
        SetBackgroundColour(*wxWHITE);
        m_Text->SetForegroundColour(wxColour(60, 60, 60));
        m_Icon->SetBitmap(m_BlackBitmap);
    }

    Refresh();
}

void SidebarItem::OnMouseMove(wxMouseEvent& event)
{
    if (!m_Selected)
    {
        SetBackgroundColour(wxColour(235,235,235));
        m_Text->SetForegroundColour(wxColour(0,120,215));
        m_Icon->SetBitmap(m_BlueBitmap);
        Refresh();
    }

    event.Skip();
}

void SidebarItem::OnMouseLeave(wxMouseEvent& event)
{
    if (!m_Selected)
    {
        SetBackgroundColour(*wxWHITE);
        m_Text->SetForegroundColour(wxColour(60,60,60));
        m_Icon->SetBitmap(m_BlackBitmap);
        Refresh();
    }

    event.Skip();
}

void SidebarItem::OnClick(wxMouseEvent &event)
{
    if (m_ClickHandler)
        m_ClickHandler();

    event.Skip();
}

void SidebarItem::SetClickHandler(std::function<void()> handler)
{
    m_ClickHandler = handler;
}