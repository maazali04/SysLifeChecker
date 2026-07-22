#include "App.hpp"
#include "MainFrame.hpp"
#include "FontManager.hpp"

wxIMPLEMENT_APP(SysLifeCheckerApp);

bool SysLifeCheckerApp::OnInit()
{
    FontManager::Initialize();
    MainFrame* frame = new MainFrame();

    frame->Show(true);

    return true;
}
