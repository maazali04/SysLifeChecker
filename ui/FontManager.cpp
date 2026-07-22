#include "FontManager.hpp"

#include <wx/font.h>
#include <wx/filename.h>

bool FontManager::m_Initialized = false;

std::string FontManager::m_FontFolder = "resources/fonts/";

std::string FontManager::RegularPath =
    m_FontFolder + "Inter-Regular.ttf";

std::string FontManager::MediumPath =
    m_FontFolder + "Inter-Medium.ttf";

std::string FontManager::SemiBoldPath =
    m_FontFolder + "Inter-SemiBold.ttf";

std::string FontManager::BoldPath =
    m_FontFolder + "Inter-Bold.ttf";

bool FontManager::Initialize()
{
    if (m_Initialized)
        return true;

#if wxCHECK_VERSION(3,1,6)

    wxFont::AddPrivateFont(RegularPath);
    wxFont::AddPrivateFont(MediumPath);
    wxFont::AddPrivateFont(SemiBoldPath);
    wxFont::AddPrivateFont(BoldPath);

#endif

    m_Initialized = true;

    return true;
}

wxFont FontManager::Regular(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL,
        false,
        "Inter");
}

wxFont FontManager::Medium(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_MEDIUM,
        false,
        "Inter");
}

wxFont FontManager::SemiBold(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_SEMIBOLD,
        false,
        "Inter");
}

wxFont FontManager::Bold(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD,
        false,
        "Inter");
}