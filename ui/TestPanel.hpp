#pragma once

#include <wx/wx.h>
#include <atomic>
#include <memory>
#include "SystemInfo.hpp"

class CardPanel;

wxDECLARE_EVENT(EVT_TEST_LOG_UPDATE, wxCommandEvent);
wxDECLARE_EVENT(EVT_TEST_FINISHED, wxCommandEvent);

class TestPanel : public wxPanel
{
public:
    explicit TestPanel(wxWindow* parent);
    ~TestPanel();

    void UpdateData(const SystemInfo& info);
    void RunAllTests();

private:
    void BuildUI();
    void AppendLog(const wxString& message);

    SystemInfo m_LatestInfo;

    wxButton* m_RunBtn = nullptr;
    wxGauge* m_ProgressGauge = nullptr;
    wxTextCtrl* m_LogText = nullptr;
    wxStaticText* m_StatusLabel = nullptr;

    // Test result badge indicators
    struct TestItem
    {
        wxString name;
        CardPanel* card = nullptr;
        wxStaticText* statusText = nullptr;
        wxPanel* dot = nullptr;
    };
    std::vector<TestItem> m_TestItems;

    std::atomic<bool> m_Testing{false};
    std::shared_ptr<std::atomic<bool>> m_Alive;

    void OnRunTestsClicked(wxCommandEvent& event);
    void OnTestLogUpdate(wxCommandEvent& event);
    void OnTestFinished(wxCommandEvent& event);
};
