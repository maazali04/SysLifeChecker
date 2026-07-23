#include "FontManager.hpp"

#include <wx/font.h>

bool FontManager::m_Initialized = false;

const std::string FontManager::FontFolder =
    "resources/fonts/";

const std::string FontManager::RegularPath =
    FontFolder + "Inter-Regular.ttf";

const std::string FontManager::MediumPath =
    FontFolder + "Inter-Medium.ttf";

const std::string FontManager::SemiBoldPath =
    FontFolder + "Inter-SemiBold.ttf";

const std::string FontManager::BoldPath =
    FontFolder + "Inter-Bold.ttf";

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
        FontFamily);
}

wxFont FontManager::Medium(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_MEDIUM,
        false,
        FontFamily);
}

wxFont FontManager::SemiBold(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_SEMIBOLD,
        false,
        FontFamily);
}

wxFont FontManager::Bold(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD,
        false,
        FontFamily);
}