#pragma once

#include <wx/colour.h>

class Theme
{
public:
    // ===== Main UI =====

    static const wxColour Background;
    static const wxColour Surface;
    static const wxColour Sidebar;
    static const wxColour Border;

    // ===== Text =====

    static const wxColour TextPrimary;
    static const wxColour TextSecondary;
    static const wxColour TextDisabled;

    // ===== Brand =====

    static const wxColour Accent;
    static const wxColour AccentHover;
    static const wxColour AccentPressed;

    // ===== Status =====

    static const wxColour Healthy;
    static const wxColour HealthyBackground;

    static const wxColour Warning;
    static const wxColour WarningBackground;

    static const wxColour Critical;
    static const wxColour CriticalBackground;

    static const wxColour Neutral;
    static const wxColour NeutralBackground;

    // ===== Layout =====

    static constexpr int SidebarWidth = 230;

    static constexpr int CardRadius = 10;

    static constexpr int PaddingSmall = 8;
    static constexpr int PaddingMedium = 16;
    static constexpr int PaddingLarge = 24;
};