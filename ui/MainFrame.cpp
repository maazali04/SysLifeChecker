#include "MainFrame.hpp"
#include "Sidebar.hpp"
#include "MenuBar.hpp"
#include <wx/icon.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    wxEND_EVENT_TABLE()

        MainFrame::MainFrame()
    : wxFrame(nullptr,
              wxID_ANY,
              "SysLifeChecker",
              wxDefaultPosition,
              wxSize(1200, 700))
{
    SetIcon(wxIcon("resources/icons/SysLifeChecker.ico",
                   wxBITMAP_TYPE_ICO));

    SetMenuBar(MenuBar::Create());
    wxPanel *root = new wxPanel(this);

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    Sidebar *sidebar = new Sidebar(root);

    wxPanel *content = new wxPanel(root);
    content->SetBackgroundColour(*wxWHITE);

    sizer->Add(sidebar, 0, wxEXPAND);
    sizer->Add(content, 1, wxEXPAND);

    root->SetSizer(sizer);

    Centre();
}
