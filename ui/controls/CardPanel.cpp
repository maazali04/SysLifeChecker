#include "CardPanel.hpp"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

wxBEGIN_EVENT_TABLE(CardPanel, wxPanel)
    EVT_PAINT(CardPanel::OnPaint)
    EVT_SIZE(CardPanel::OnSize) // 🟢 ADD THIS LINE: Listen for resize events
wxEND_EVENT_TABLE()

CardPanel::CardPanel(wxWindow* parent)
    : wxPanel(parent,
              wxID_ANY,
              wxDefaultPosition,
              wxDefaultSize,
              wxBORDER_NONE)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(*wxWHITE);
}

// 🟢 ADD THIS NEW FUNCTION
void CardPanel::OnSize(wxSizeEvent& event)
{
    Refresh();    // Tells Windows to mark the entire panel as dirty/empty
    Update();     // Forces an immediate repaint right now instead of waiting
    event.Skip(); // Allows wxWidgets to continue handling internal layout sizer math
}

void CardPanel::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);

    // 1. Get the background color of the PARENT window (the space behind this card)
    wxColour parentBg = GetParent() ? GetParent()->GetBackgroundColour() : wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK);

    // 2. Clear the canvas using the parent's color so corners match the background canvas
    dc.SetBackground(wxBrush(parentBg));
    dc.Clear();

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    if (!gc) return;

    // Turn on high-quality antialiasing so the curves are silky smooth
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

    wxRect r = GetClientRect();
    r.Deflate(1); // Leave a 1-pixel margin so the stroke outline isn't cut off

    // 3. Draw the solid white card body FILL with rounded corners
    gc->SetPen(*wxTRANSPARENT_PEN); // Turn off the pen for the fill step
    gc->SetBrush(wxBrush(*wxWHITE));
    gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, 8);

    // 4. Draw the subtle border OUTLINE on top of the rounded shape
    gc->SetPen(wxPen(wxColour(235, 235, 235), 1));
    gc->SetBrush(*wxTRANSPARENT_BRUSH); // Don't overwrite the inside fill
    gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, 8);

    delete gc;
}

