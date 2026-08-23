#pragma once
#include <wx/panel.h>

class CardPanel : public wxPanel {
public:
    CardPanel(wxWindow* parent);

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event); // 🟢 ADD THIS LINE

    wxDECLARE_EVENT_TABLE();
};
