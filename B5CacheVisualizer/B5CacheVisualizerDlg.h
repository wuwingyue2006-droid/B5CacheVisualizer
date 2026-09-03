#pragma once

#include "VisualizationController.h"
#include "core/CacheSimulator.h"
#include "experiment/ComparisonRunner.h"

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
    afx_msg void OnPreviousFrame();
    afx_msg void OnAutoPlay();
    afx_msg void OnPausePlayback();
    afx_msg void OnStopPlayback();
    afx_msg void OnPlaybackSpeedChanged();
    afx_msg void OnGenerateTrace();
    afx_msg void OnCompareStrategies();
    afx_msg void OnTraceTextChanged();
    afx_msg void OnConfigurationChanged();
    afx_msg void OnTimer(UINT_PTR timerId);
    afx_msg void OnDestroy();
    afx_msg void OnDrawItem(int controlId, LPDRAWITEMSTRUCT drawItem);
    afx_msg void OnCustomDrawCacheView(NMHDR* notification, LRESULT* result);

    bool ReadConfiguration(b5cache::SimulationConfig& config) const;
    bool ParseTraceFromEditor(std::vector<b5cache::MemoryAccess>& accesses) const;
    bool LoadTraceFromEditor(std::vector<b5cache::MemoryAccess>& accesses);
    void ResetSession();
    bool ExecuteNextAccess(bool showEndMessage = true);
    b5cacheui::VisualizationFrame CaptureFrame(const b5cache::AccessResult& result) const;
    bool CanAdvancePlayback() const noexcept;
    void RefreshCurrentFrame();
    void RefreshCacheViews(const b5cacheui::VisualizationFrame* frame = nullptr);
    void RefreshStatistics(const b5cache::StatisticsSnapshot* displayed = nullptr);
    void RefreshStatisticsCharts() const;
    void DrawOutcomeChart(CDC& dc, const CRect& bounds) const;
    void DrawHitRateChart(CDC& dc, const CRect& bounds) const;
    LevelDisplayState BuildLevelDisplayState(
        const b5cacheui::CacheSetsSnapshot& sets,
        const b5cache::CacheLevelConfig& config,
        const b5cache::MemoryAccess& request,
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
    void StartPlaybackTimer();
    void StopPlaybackTimer();
    void FinishPlayback();
    void UpdateControlStates();
    b5cache::StatisticsSnapshot DisplayedStatistics() const noexcept;

    std::unique_ptr<b5cache::CacheSimulator> simulator_;
    std::vector<b5cache::MemoryAccess> trace_;
    std::vector<b5cache::ComparisonPlan> comparisonPlans_;
    b5cacheui::VisualizationController visualization_;
    UINT_PTR playbackTimerId_ = 0;
    bool initialized_ = false;
    bool suppressTraceChange_ = true;
    bool suppressConfigurationChange_ = true;
    bool traceDirty_ = false;
    bool configurationDirty_ = false;
    CListCtrl l1CacheView_;
    CListCtrl l2CacheView_;
};
