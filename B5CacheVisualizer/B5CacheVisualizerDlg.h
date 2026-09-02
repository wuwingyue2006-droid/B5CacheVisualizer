#pragma once

#include "core/CacheSimulator.h"

#include <memory>
#include <vector>

class CB5CacheVisualizerDlg final : public CDialogEx {
public:
    explicit CB5CacheVisualizerDlg(CWnd* parent = nullptr);

    enum { IDD = IDD_B5CACHEVISUALIZER_DIALOG };

protected:
    void DoDataExchange(CDataExchange* dataExchange) override;
    BOOL OnInitDialog() override;

    DECLARE_MESSAGE_MAP()

private:
    afx_msg void OnApplyConfig();
    afx_msg void OnImportTrace();
    afx_msg void OnClearTrace();
    afx_msg void OnStepTrace();
    afx_msg void OnRunAllTrace();
    afx_msg void OnResetSimulation();

    bool ReadConfiguration(b5cache::SimulationConfig& config) const;
    bool LoadTraceFromEditor(std::vector<b5cache::MemoryAccess>& accesses) const;
    void ResetSession();
    void ExecuteNextAccess();
    void RefreshCacheViews(const b5cache::AccessResult* latest = nullptr);
    void RefreshStatistics();
    void RefreshTraceStatus();
    void ShowUserError(const std::string& message) const;
    void ClearCacheView(CListCtrl& view) const;
    void SetupCacheViewColumns(CListCtrl& view) const;

    std::unique_ptr<b5cache::CacheSimulator> simulator_;
    std::vector<b5cache::MemoryAccess> trace_;
    std::size_t nextIndex_ = 0;
    b5cache::AccessResult lastResult_;
    bool hasLastResult_ = false;
    CListCtrl l1CacheView_;
    CListCtrl l2CacheView_;
};
