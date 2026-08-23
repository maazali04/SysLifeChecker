#include "TestPanel.hpp"
#include "controls/CardPanel.hpp"
#include "FontManager.hpp"

TestPanel::TestPanel(wxWindow* parent)
    : wxPanel(parent)
{
    SetBackgroundColour(wxColour(245, 247, 250));
    BuildUI();
}

void TestPanel::BuildUI()
{
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    auto* title = new wxStaticText(this, wxID_ANY, "Test");
    title->SetFont(FontManager::SemiBold(20));
    rootSizer->Add(title, 0, wxLEFT | wxTOP | wxRIGHT, 24);
    rootSizer->AddSpacer(24);

    CardPanel* card = new CardPanel(this);
    wxBoxSizer* cardSizer = new wxBoxSizer(wxVERTICAL);
    cardSizer->AddStretchSpacer();

    auto* heading = new wxStaticText(card, wxID_ANY, "Hardware Test Suite");
    heading->SetFont(FontManager::Medium(13));
    heading->SetForegroundColour(wxColour(30, 41, 59));
    cardSizer->Add(heading, 0, wxALIGN_CENTER_HORIZONTAL);
    cardSizer->AddSpacer(10);

    // TODO: wire "Run All Tests" to TestCPU()/TestRAM()/TestStorage()/
    // TestGPU()/TestBattery()/... already implemented in src/Test.cpp,
    // running on a background thread and streaming results into this panel.
    auto* body = new wxStaticText(
        card, wxID_ANY,
        "Battery, CPU, RAM, storage, and GPU diagnostics will run from here.");
    body->SetFont(FontManager::Regular(10));
    body->SetForegroundColour(wxColour(120, 120, 120));
    cardSizer->Add(body, 0, wxALIGN_CENTER_HORIZONTAL);
    cardSizer->AddSpacer(20);

    wxButton* runBtn = new wxButton(card, wxID_ANY, "Run All Tests");
    cardSizer->Add(runBtn, 0, wxALIGN_CENTER_HORIZONTAL);

    cardSizer->AddStretchSpacer();
    card->SetSizer(cardSizer);

    rootSizer->Add(card, 1, wxEXPAND | wxALL, 24);

    SetSizer(rootSizer);
}
