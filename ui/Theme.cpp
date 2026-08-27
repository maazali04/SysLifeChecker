#include "Theme.hpp"
#include "Sidebar.hpp"
#include <wx/window.h>
#include <wx/stattext.h>
#include <algorithm>

// ===== Main =====

const wxColour Theme::Background(245, 247, 250);
const wxColour Theme::Surface(255, 255, 255);
const wxColour Theme::Sidebar(255, 255, 255);
const wxColour Theme::Border(220, 223, 228);

// ===== Text =====

const wxColour Theme::TextPrimary(30, 30, 30);
const wxColour Theme::TextSecondary(110, 110, 110);
const wxColour Theme::TextDisabled(170, 170, 170);

// ===== Accent =====

const wxColour Theme::Accent(0, 120, 215);
const wxColour Theme::AccentHover(25, 140, 235);
const wxColour Theme::AccentPressed(0, 95, 184);

// ===== Healthy =====

const wxColour Theme::Healthy(0, 120, 215);
const wxColour Theme::HealthyBackground(230, 243, 255);

// ===== Warning =====

const wxColour Theme::Warning(240, 173, 0);
const wxColour Theme::WarningBackground(255, 248, 220);

// ===== Critical =====

const wxColour Theme::Critical(220, 53, 69);
const wxColour Theme::CriticalBackground(255, 235, 238);

// ===== Neutral =====

const wxColour Theme::Neutral(60, 60, 60);
const wxColour Theme::NeutralBackground(245, 245, 245);

// ===== Dark mode =====

bool Theme::IsDark = false;

wxColour Theme::PageBackground()
{
    return IsDark ? wxColour(24, 26, 30) : wxColour(245, 247, 250);
}

wxColour Theme::CardBackground()
{
    return IsDark ? wxColour(38, 40, 46) : wxColour(255, 255, 255);
}

wxColour Theme::BorderColour()
{
    return IsDark ? wxColour(60, 62, 68) : wxColour(235, 235, 235);
}

wxColour Theme::SidebarBackground()
{
    return IsDark ? wxColour(30, 32, 36) : wxColour(255, 255, 255);
}

wxColour Theme::SidebarItemHighlightBackground()
{
    return IsDark ? wxColour(46, 49, 54) : wxColour(242, 245, 250);
}

wxColour Theme::SidebarTextColour()
{
    return IsDark ? wxColour(230, 232, 235) : wxColour(45, 45, 45);
}

void Theme::SetDark(bool dark)
{
    IsDark = dark;
}

namespace
{
    // All of the app's plain text (as opposed to green/amber/red/blue
    // status colours) is a neutral grey where R, G and B are close
    // together. Status colours always have a wide spread between their
    // channels (e.g. amber 240,173,0 or blue 0,120,215), so this
    // reliably tells "just text" apart from "meaningful status colour"
    // without having to hard-code every colour used across the app.
    bool IsNeutralGrey(const wxColour& c)
    {
        int mx = std::max({c.Red(), c.Green(), c.Blue()});
        int mn = std::min({c.Red(), c.Green(), c.Blue()});
        return (mx - mn) < 40;
    }
}

void Theme::ApplyRecursive(wxWindow* window)
{
    if (!window)
        return;

    // The sidebar deliberately uses a different palette (a grey rail,
    // not the page background) and manages its own item hover/selected
    // states, so it re-themes itself rather than going through the
    // generic walk below.
    if (auto* sidebar = dynamic_cast<Sidebar*>(window))
    {
        sidebar->ApplyTheme();
        return;
    }

    window->SetBackgroundColour(PageBackground());

    if (auto* text = dynamic_cast<wxStaticText*>(window))
    {
        wxColour fg = text->GetForegroundColour();
        if (IsNeutralGrey(fg))
            text->SetForegroundColour(IsDark ? wxColour(210, 213, 218) : wxColour(60, 60, 60));
    }

    window->Refresh();

    for (wxWindow* child : window->GetChildren())
        ApplyRecursive(child);
}
