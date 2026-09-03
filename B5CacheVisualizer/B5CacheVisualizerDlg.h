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
    struct LevelDisplayState {
        bool accessed = false;
        bool hit = false;
        bool dirty = false;
        bool evicted = false;
        std::size_t setIndex = 0;
        std::size_t lineIndex = b5cache::kInvalidIndex;
        std::uint64_t blockNumber = 0;
        std::uint64_t tag = 0;
        std::uint64_t offset = 0;
        std::uint64_t evictedBlock = 0;
    };

    afx_msg void OnApplyConfig();
    afx_msg void OnImportTrace();
    afx_msg void OnClearTrace();
    afx_msg void OnStepTrace();
    afx_msg void OnRunAllTrace();
    afx_msg void OnResetSimulation();
    afx_msg void OnDrawItem(int controlId, LPDRAWITEMSTRUCT drawItem);
    afx_msg void OnCustomDrawCacheView(NMHDR* notification, LRESULT* result);

    bool ReadConfiguration(b5cache::SimulationConfig& config) const;
    bool ParseTraceFromEditor(std::vector<b5cache::MemoryAccess>& accesses) const;
    bool LoadTraceFromEditor(std::vector<b5cache::MemoryAccess>& accesses);
    void ResetSession();
    void ExecuteNextAccess();
    void RefreshCacheViews(const b5cache::AccessResult* latest = nullptr);
    void RefreshStatistics();
    void RecordStatisticsPoint();
    void RefreshStatisticsCharts() const;
    void DrawOutcomeChart(CDC& dc, const CRect& bounds) const;
    void DrawHitRateChart(CDC& dc, const CRect& bounds) const;
    LevelDisplayState BuildLevelDisplayState(
        const b5cache::CacheLevel& level,
        const b5cache::LevelAccessDetail& detail,
        bool accessed) const;
    void RefreshAccessVisuals() const;
    void DrawAddressBreakdown(CDC& dc, const CRect& bounds) const;
    void DrawAccessPath(CDC& dc, const CRect& bounds) const;
    void RefreshTraceStatus();
    void ShowUserError(const std::string& message) const;
    void ClearCacheView(CListCtrl& view) const;
    void SetupCacheViewColumns(CListCtrl& view) const;
    void RefreshLastResult(const b5cache::AccessResult* result);

    std::unique_ptr<b5cache::CacheSimulator> simulator_;
    std::vector<b5cache::MemoryAccess> trace_;
    std::size_t nextIndex_ = 0;
    b5cache::AccessResult lastResult_;
    bool hasLastResult_ = false;
    std::vector<double> overallHitRateHistory_;
    CListCtrl l1CacheView_;
    CListCtrl l2CacheView_;
};
