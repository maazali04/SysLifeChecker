#include "SidebarItem.hpp"
#include "../FontManager.hpp"

SidebarItem::SidebarItem(
    wxWindow* parent,
    const wxString& text,
    const wxString& blackIcon,
    const wxString& blueIcon)
    : wxPanel(parent,
              wxID_ANY,
              wxDefaultPosition,
              wxSize(-1,42),
              wxBORDER_NONE)
{
    SetBackgroundColour(*wxWHITE);

    m_BlackBitmap.LoadFile(blackIcon, wxBITMAP_TYPE_PNG);
    m_BlueBitmap.LoadFile(blueIcon, wxBITMAP_TYPE_PNG);

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->AddSpacer(18);

    m_Icon = new wxStaticBitmap(
        this,
        wxID_ANY,
        m_BlackBitmap);

    sizer->Add(
        m_Icon,
        0,
        wxALIGN_CENTER_VERTICAL);

    sizer->AddSpacer(14);

    m_Text = new wxStaticText(
        this,
        wxID_ANY,
        text);

    m_Text->SetFont(FontManager::Medium(13));
    m_Text->SetForegroundColour(wxColour(45,45,45));

    sizer->Add(
        m_Text,
        0,
        wxALIGN_CENTER_VERTICAL);

    sizer->AddStretchSpacer();

    SetSizer(sizer);

    auto hand = wxCursor(wxCURSOR_HAND);

    SetCursor(hand);
    m_Text->SetCursor(hand);
    m_Icon->SetCursor(hand);

    Bind(wxEVT_ENTER_WINDOW,&SidebarItem::OnMouseMove,this);
    Bind(wxEVT_LEAVE_WINDOW,&SidebarItem::OnMouseLeave,this);
    Bind(wxEVT_LEFT_DOWN,&SidebarItem::OnClick,this);

    m_Text->Bind(wxEVT_LEFT_DOWN,&SidebarItem::OnClick,this);
    m_Icon->Bind(wxEVT_LEFT_DOWN,&SidebarItem::OnClick,this);

    m_Text->Bind(wxEVT_ENTER_WINDOW,&SidebarItem::OnMouseMove,this);
    m_Icon->Bind(wxEVT_ENTER_WINDOW,&SidebarItem::OnMouseMove,this);

    m_Text->Bind(wxEVT_LEAVE_WINDOW,&SidebarItem::OnMouseLeave,this);
    m_Icon->Bind(wxEVT_LEAVE_WINDOW,&SidebarItem::OnMouseLeave,this);
}

void SidebarItem::SetSelected(bool selected)
{
    m_Selected = selected;

    if(selected)
    {
        SetBackgroundColour(wxColour(242,245,250));
        m_Text->SetForegroundColour(wxColour(0,120,215));
        m_Icon->SetBitmap(m_BlueBitmap);
    }
    else
    {
        SetBackgroundColour(*wxWHITE);
        m_Text->SetForegroundColour(wxColour(45,45,45));
        m_Icon->SetBitmap(m_BlackBitmap);
    }

    Refresh();
}

void SidebarItem::OnMouseMove(wxMouseEvent& event)
{
    if(!m_Selected)
    {
        SetBackgroundColour(wxColour(242,245,250));
        m_Text->SetForegroundColour(wxColour(0,120,215));
        m_Icon->SetBitmap(m_BlueBitmap);

        Refresh();
    }

    event.Skip();
}

void SidebarItem::OnMouseLeave(wxMouseEvent& event)
{
    if(!m_Selected)
    {
        SetBackgroundColour(*wxWHITE);
        m_Text->SetForegroundColour(wxColour(45,45,45));
        m_Icon->SetBitmap(m_BlackBitmap);

        Refresh();
    }

    event.Skip();
}

void SidebarItem::OnClick(wxMouseEvent& event)
{
    if(m_ClickHandler)
        m_ClickHandler();

    event.Skip();
}

void SidebarItem::SetClickHandler(std::function<void()> handler)
{
    m_ClickHandler = handler;
}