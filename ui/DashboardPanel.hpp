#pragma once

#include <wx/wx.h>
#include <vector>

class CardPanel;

class DashboardPanel : public wxPanel
{
public:
    DashboardPanel(wxWindow* parent);

private:

    void BuildUI();

};