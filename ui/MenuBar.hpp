#pragma once

#include <wx/wx.h>

enum MenuID
{
    ID_MENU_NEW_SCAN = wxID_HIGHEST + 1,
    ID_MENU_SAVE_REPORT,
    ID_MENU_EXPORT_PDF,
    ID_MENU_EXPORT_HTML,
    ID_MENU_EXPORT_JSON,
    ID_MENU_VIEW_DASHBOARD,
    ID_MENU_VIEW_TEST,
    ID_MENU_VIEW_REPORT,
    ID_MENU_VIEW_SETTINGS,
    ID_MENU_RUN_ALL_TESTS
};

class MenuBar
{
public:
    static wxMenuBar* Create();
};