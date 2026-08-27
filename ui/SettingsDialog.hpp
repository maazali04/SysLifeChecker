#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <functional>

// Note: despite the file name, this is an inline wxPanel (like Dashboard
// and Report) so it swaps into the same content area from the sidebar,
// rather than a modal wxDialog.
//
// Kept intentionally minimal: only settings that actually do something
// are here. See MainFrame's callback usage for what each one wires to.
class SettingsDialog : public wxPanel
{
public:
    struct Settings
    {
        int RefreshIntervalSeconds = 60;
        bool DarkMode = false;
    };

    // onChanged fires whenever the user changes a control here, so the
    // owner (MainFrame) can react -- restart its refresh timer, or
    // re-apply the theme app-wide.
    explicit SettingsDialog(wxWindow* parent, std::function<void(const Settings&)> onChanged = nullptr);

    const Settings& GetSettings() const { return m_Settings; }

private:
    void BuildUI();
    void NotifyChanged();

    Settings m_Settings;
    std::function<void(const Settings&)> m_OnChanged;

    wxSpinCtrl* m_RefreshSpin = nullptr;
    wxCheckBox* m_DarkModeCheck = nullptr;
};
