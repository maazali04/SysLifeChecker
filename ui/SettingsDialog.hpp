#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <functional>

// Note: despite the file name, this is an inline wxPanel (like Dashboard
// and Report) so it swaps into the same content area from the sidebar,
// rather than a modal wxDialog.
class SettingsDialog : public wxPanel
{
public:
    struct Settings
    {
        int RefreshIntervalSeconds = 60;
        bool StartWithWindows = false;
        bool MinimizeToTray = false;
        bool UseGiB = false;
        bool UseFahrenheit = false;
        int ThemeIndex = 0; // 0 = Light, 1 = Dark, 2 = Match system
        bool AdvancedDetailsByDefault = false;
    };

    // onChanged fires whenever the user changes any control here, so the
    // owner (MainFrame) can react -- e.g. restart its refresh timer with
    // the new interval.
    explicit SettingsDialog(wxWindow* parent, std::function<void(const Settings&)> onChanged = nullptr);

    const Settings& GetSettings() const { return m_Settings; }

private:
    void BuildUI();
    void NotifyChanged();

    Settings m_Settings;
    std::function<void(const Settings&)> m_OnChanged;

    wxSpinCtrl* m_RefreshSpin = nullptr;
    wxCheckBox* m_StartupCheck = nullptr;
    wxCheckBox* m_TrayCheck = nullptr;
    wxChoice* m_UnitsChoice = nullptr;
    wxChoice* m_TempChoice = nullptr;
    wxChoice* m_ThemeChoice = nullptr;
    wxCheckBox* m_AdvancedCheck = nullptr;
};
