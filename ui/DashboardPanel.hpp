#pragma once

#include <wx/wx.h>
#include <wx/statline.h>
#include "SystemInfo.hpp"

class CardPanel;

class DashboardPanel : public wxPanel
{
public:
    explicit DashboardPanel(wxWindow* parent);

    void UpdateData(const SystemInfo& info);

private:
    void BuildUI();

    // Header controls
    wxStaticText* m_LastScan = nullptr;
    wxPanel* m_StatusDot = nullptr;
    wxStaticText* m_StatusText = nullptr;

    // Battery card controls
    CardPanel* m_BatteryCard = nullptr;
    wxBoxSizer* m_BatterySizer = nullptr;
    wxStaticText* m_BatteryPercent = nullptr;
    wxGauge* m_BatteryGauge = nullptr;
    CardPanel* m_BatteryBadge = nullptr;
    wxStaticText* m_BatteryBadgeText = nullptr;
    wxBoxSizer* m_BatteryDetailsSizer = nullptr;

    // Specs card controls
    CardPanel* m_SpecsCard = nullptr;
    wxBoxSizer* m_SpecsSizer = nullptr;
    wxBoxSizer* m_SpecsDetailsSizer = nullptr;

    // Validation card controls
    CardPanel* m_ValidCard = nullptr;
    wxBoxSizer* m_ValidSizer = nullptr;
    wxBoxSizer* m_ValidDetailsSizer = nullptr;

    // Storage card controls
    CardPanel* m_StorageCard = nullptr;
    wxBoxSizer* m_StorageSizer = nullptr;
    wxBoxSizer* m_StorageDrivesSizer = nullptr;

    // Summary card controls
    CardPanel* m_SummaryCard = nullptr;
    wxBoxSizer* m_SummarySizer = nullptr;
    CardPanel* m_SummaryBadge = nullptr;
    wxStaticText* m_SummaryBadgeText = nullptr;
    wxStaticText* m_SummaryBody = nullptr;
};