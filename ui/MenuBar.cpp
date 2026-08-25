#include "MenuBar.hpp"

wxMenuBar* MenuBar::Create()
{
    wxMenuBar* menuBar = new wxMenuBar();

    // ================= FILE =================
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_MENU_NEW_SCAN, "&New Scan\tCtrl+R");
    fileMenu->Append(ID_MENU_SAVE_REPORT, "&Save / Export Report...\tCtrl+S");

    wxMenu* exportMenu = new wxMenu();
    exportMenu->Append(ID_MENU_EXPORT_HTML, "HTML Report (.html)");
    exportMenu->Append(ID_MENU_EXPORT_JSON, "JSON Data (.json)");
    exportMenu->Append(ID_MENU_SAVE_REPORT, "Text Summary (.txt)");
    fileMenu->AppendSubMenu(exportMenu, "Export");

    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4");

    // ================= VIEW =================
    wxMenu* viewMenu = new wxMenu();
    viewMenu->Append(ID_MENU_VIEW_DASHBOARD, "&Dashboard\tCtrl+1");
    viewMenu->Append(ID_MENU_VIEW_TEST, "&Hardware Tests\tCtrl+2");
    viewMenu->Append(ID_MENU_VIEW_REPORT, "&Full Report\tCtrl+3");
    viewMenu->Append(ID_MENU_VIEW_SETTINGS, "&Settings\tCtrl+4");
    viewMenu->AppendSeparator();
    viewMenu->Append(ID_MENU_NEW_SCAN, "&Refresh Hardware Data\tF5");

    // ================= TEST =================
    wxMenu* testMenu = new wxMenu();
    testMenu->Append(ID_MENU_RUN_ALL_TESTS, "&Run All Diagnostics\tF6");

    // ================= HELP =================
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About SysLifeChecker");

    // ================= ADD TO MENU BAR =================
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(viewMenu, "&View");
    menuBar->Append(testMenu, "&Test");
    menuBar->Append(helpMenu, "&Help");

    return menuBar;
}