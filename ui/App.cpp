#include "App.hpp"
#include "MainFrame.hpp"
#include "FontManager.hpp"
#include "wx/image.h"

wxIMPLEMENT_APP(SysLifeCheckerApp);

bool SysLifeCheckerApp::OnInit()
{
    wxInitAllImageHandlers();
    FontManager::Initialize();
    MainFrame* frame = new MainFrame();

    frame->Show(true);

    return true;
}
