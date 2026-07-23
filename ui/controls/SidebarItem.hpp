#pragma once
#include <functional>
#include <wx/wx.h>

class SidebarItem : public wxPanel
{
public:
    SidebarItem(wxWindow *parent,
                const wxString &text,
                const wxString &blackIcon,
                const wxString &blueIcon);
    wxStaticBitmap *m_Icon = nullptr;

    wxBitmap m_BlackBitmap;
    wxBitmap m_BlueBitmap;

    void SetSelected(bool selected);
    wxString GetText() const
    {
        return m_Text->GetLabel();
    }
    void SetClickHandler(std::function<void()> handler);

private:
    void OnMouseMove(wxMouseEvent &event);
    void OnMouseLeave(wxMouseEvent &event);
    void OnClick(wxMouseEvent &event);
    std::function<void()> m_ClickHandler;

    wxStaticText *m_Text = nullptr;

    bool m_Selected = false;
    bool m_Hovered = false;
};