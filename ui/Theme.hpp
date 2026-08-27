#pragma once

#include <wx/colour.h>

class wxWindow;

class Theme
{
public:
    // ===== Main UI (light mode base values) =====

    static const wxColour Background;
    static const wxColour Surface;
    static const wxColour SidebarLegacy; // unused legacy constant; see SidebarBackground() below for the real, dynamic value
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

    // ===== Dark mode =====
    // Real, working light/dark toggle. Rather than requiring every panel
    // to be rewritten to pull colours from here individually, SetDark() +
    // ApplyRecursive() walk the actual live window tree and recolor it:
    // page/card backgrounds always switch, and any label using one of our
    // existing neutral greys (not a semantic green/amber/red/blue status
    // colour, which are left alone) switches too.
    static bool IsDark;

    static wxColour PageBackground();
    static wxColour CardBackground();
    static wxColour BorderColour();

    // Sidebar gets its own distinct palette (a grey, not the same shade
    // as the page background) so it reads as a separate navigation rail
    // in both light and dark mode.
    static wxColour SidebarBackground();
    static wxColour SidebarItemHighlightBackground();
    static wxColour SidebarTextColour();

    static void SetDark(bool dark);
    static void ApplyRecursive(wxWindow* window);
};
