#pragma once

#include <wx/event.h>
#include <memory>
#include <atomic>
#include "SystemInfo.hpp"

// Fired on the main thread once a background refresh finishes.
wxDECLARE_EVENT(EVT_SYSTEM_INFO_READY, wxCommandEvent);

// Runs GetSystemInfo() (WMI + smartctl) on a background thread so the UI
// never freezes, then hands the result back to `handler` via
// EVT_SYSTEM_INFO_READY. Call LatestInfo() from the handler to read it.
//
// This is the "glue" between the data layer (include/SystemInfo.hpp +
// src/*.cpp) and the UI layer (ui/*.cpp) described in the architecture plan.
class SystemMonitorService
{
public:
    explicit SystemMonitorService(wxEvtHandler* handler);
    ~SystemMonitorService();

    SystemMonitorService(const SystemMonitorService&) = delete;
    SystemMonitorService& operator=(const SystemMonitorService&) = delete;

    // No-op if a refresh is already in flight.
    void RefreshAsync();

    const SystemInfo& LatestInfo() const { return m_Latest; }
    bool HasData() const { return m_HasData; }

private:
    wxEvtHandler* m_Handler;
    SystemInfo m_Latest;
    bool m_HasData = false;
    std::atomic<bool> m_Refreshing{false};

    // Shared with the background thread so it can tell if this service
    // (and therefore `this`) has already been destroyed before touching it.
    std::shared_ptr<std::atomic<bool>> m_Alive;
};
