#include "MenuBar.hpp"

wxMenuBar* MenuBar::Create()
{
    wxMenuBar* menuBar = new wxMenuBar();

    // ================= FILE =================

    wxMenu* fileMenu = new wxMenu();

    fileMenu->Append(wxID_ANY, "New Scan");
    fileMenu->Append(wxID_ANY, "Open Report...");
    fileMenu->Append(wxID_ANY, "Save Report...");

    wxMenu* exportMenu = new wxMenu();
    exportMenu->Append(wxID_ANY, "PDF");
    exportMenu->Append(wxID_ANY, "HTML");
    exportMenu->Append(wxID_ANY, "JSON");

    fileMenu->AppendSubMenu(exportMenu, "Export");

    fileMenu->AppendSeparator();

    fileMenu->Append(wxID_EXIT, "Exit");

    // ================= VIEW =================

    wxMenu* viewMenu = new wxMenu();

    viewMenu->Append(wxID_ANY, "Dashboard");
    viewMenu->Append(wxID_ANY, "Test");
    viewMenu->Append(wxID_ANY, "Report");

    viewMenu->AppendSeparator();

    viewMenu->Append(wxID_ANY, "Refresh");
    viewMenu->Append(wxID_ANY, "Reset Layout");
    viewMenu->AppendCheckItem(wxID_ANY, "Fullscreen");

    // ================= TEST =================

    wxMenu* testMenu = new wxMenu();

    testMenu->Append(wxID_ANY, "Run All Tests");
    testMenu->Append(wxID_ANY, "Run Selected Tests");
    testMenu->Append(wxID_ANY, "Stop Testing");

    testMenu->AppendSeparator();

    testMenu->Append(wxID_ANY, "Battery Test");
    testMenu->Append(wxID_ANY, "CPU Test");
    testMenu->Append(wxID_ANY, "RAM Test");
    testMenu->Append(wxID_ANY, "SSD Test");
    testMenu->Append(wxID_ANY, "GPU Test");

    // ================= TOOLS =================

    wxMenu* toolsMenu = new wxMenu();

    toolsMenu->Append(wxID_PREFERENCES, "Settings");
    toolsMenu->Append(wxID_ANY, "Language");
    toolsMenu->Append(wxID_ANY, "Theme");
    toolsMenu->Append(wxID_ANY, "Logs");
    toolsMenu->Append(wxID_ANY, "Developer Mode");

    // ================= HELP =================

    wxMenu* helpMenu = new wxMenu();

    helpMenu->Append(wxID_ANY, "User Guide");
    helpMenu->Append(wxID_ANY, "Check for Updates");

    helpMenu->AppendSeparator();

    helpMenu->Append(wxID_ABOUT, "About SysLifeChecker");

    // ================= ADD TO MENU BAR =================

    menuBar->Append(fileMenu, "&File");
    menuBar->Append(viewMenu, "&View");
    menuBar->Append(testMenu, "&Test");
    menuBar->Append(toolsMenu, "&Tools");
    menuBar->Append(helpMenu, "&Help");

    return menuBar;
}