#pragma once

#include <wx/wx.h>

class TestPanel : public wxPanel
{
public:
    explicit TestPanel(wxWindow* parent);

private:
    void BuildUI();
};
