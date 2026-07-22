#include "Theme.hpp"

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
// SSD Healthy
// Battery Good
// Drivers OK

const wxColour Theme::Healthy(0, 120, 215);
const wxColour Theme::HealthyBackground(230, 243, 255);

// ===== Warning =====
// Battery getting old
// SSD 75% health

const wxColour Theme::Warning(240, 173, 0);
const wxColour Theme::WarningBackground(255, 248, 220);

// ===== Critical =====
// HDD failing
// Battery dead
// RAM errors

const wxColour Theme::Critical(220, 53, 69);
const wxColour Theme::CriticalBackground(255, 235, 238);

// ===== Neutral =====
// CPU
// RAM Size
// GPU
// Motherboard

const wxColour Theme::Neutral(60, 60, 60);
const wxColour Theme::NeutralBackground(245, 245, 245);