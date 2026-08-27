#include "FontManager.hpp"

#include <wx/font.h>

bool FontManager::m_Initialized = false;
wxString FontManager::m_ActiveFamily = "Segoe UI";

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

    bool loaded = true;

#if wxCHECK_VERSION(3,1,6)
    loaded = loaded && wxFont::AddPrivateFont(RegularPath);
    loaded = loaded && wxFont::AddPrivateFont(MediumPath);
    loaded = loaded && wxFont::AddPrivateFont(SemiBoldPath);
    loaded = loaded && wxFont::AddPrivateFont(BoldPath);
#else
    loaded = false;
#endif

    // "Inter" only actually renders correctly if every weight registered.
    // If any file is missing/corrupt or this wx build is too old to load
    // private fonts at all, fall back to Segoe UI -- a clean, genuinely
    // professional system font -- rather than an unresolved "Inter" that
    // Windows silently swaps for a default (often ugly) fallback face.
    m_ActiveFamily = loaded ? wxString("Inter") : wxString("Segoe UI");

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
        m_ActiveFamily);
}

wxFont FontManager::Medium(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_MEDIUM,
        false,
        m_ActiveFamily);
}

wxFont FontManager::SemiBold(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_SEMIBOLD,
        false,
        m_ActiveFamily);
}

wxFont FontManager::Bold(int size)
{
    return wxFont(
        size,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD,
        false,
        m_ActiveFamily);
}
