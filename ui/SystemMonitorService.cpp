#include "SystemMonitorService.hpp"
#include <thread>
#include <mutex>

wxDEFINE_EVENT(EVT_SYSTEM_INFO_READY, wxCommandEvent);

static std::mutex g_ServiceMutex;

SystemMonitorService::SystemMonitorService(wxEvtHandler* handler)
    : m_Handler(handler),
      m_Alive(std::make_shared<std::atomic<bool>>(true))
{
}

SystemMonitorService::~SystemMonitorService()
{
    if (m_Alive)
    {
        *m_Alive = false;
    }
}

void SystemMonitorService::RefreshAsync()
{
    if (m_Refreshing.exchange(true))
        return; // Already refreshing

    auto alive = m_Alive;
    wxEvtHandler* handler = m_Handler;

    std::thread([this, alive, handler]() {
        if (!InitializeWMI())
        {
            m_Refreshing = false;
            return;
        }

        SystemInfo info = GetSystemInfo();

        ShutdownWMI();

        if (!alive || !(*alive))
            return; // Service was destroyed while thread was running

        {
            std::lock_guard<std::mutex> lock(g_ServiceMutex);
            m_Latest = std::move(info);
            m_HasData = true;
        }

        m_Refreshing = false;

        if (handler && alive && *alive)
        {
            wxCommandEvent* evt = new wxCommandEvent(EVT_SYSTEM_INFO_READY);
            wxQueueEvent(handler, evt);
        }
    }).detach();
}
