#pragma once

#include <wx/wx.h>

// Note: despite the file name, this is an inline wxPanel (like Dashboard
// and Report) so it swaps into the same content area from the sidebar,
// rather than a modal wxDialog.
class SettingsDialog : public wxPanel
{
public:
    explicit SettingsDialog(wxWindow* parent);

private:
    void BuildUI();
};
