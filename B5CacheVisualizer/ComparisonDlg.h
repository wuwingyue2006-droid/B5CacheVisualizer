#pragma once

#include "experiment/ComparisonRunner.h"
#include "export/ExperimentExporter.h"
#include "resource.h"

#include <vector>

class CComparisonDlg final : public CDialogEx {
public:
    CComparisonDlg(
        b5cache::SimulationConfig currentConfig,
        std::vector<b5cache::MemoryAccess> trace,
        std::vector<b5cache::ComparisonPlan> plans,
        CWnd* parent = nullptr);

    enum { IDD = IDD_COMPARISON_DIALOG };

    const std::vector<b5cache::ComparisonPlan>& Plans() const noexcept;
    const std::vector<b5cache::MemoryAccess>& Trace() const noexcept;
    bool LoadRequested() const noexcept;
    const b5cache::ComparisonPlan* SelectedPlan() const noexcept;

protected:
    BOOL OnInitDialog() override;
    DECLARE_MESSAGE_MAP()

private:
    afx_msg void OnPlanSelected();
    afx_msg void OnAddCurrent();
    afx_msg void OnUpdateCurrent();
    afx_msg void OnRemove();
    afx_msg void OnTeachingPresets();
    afx_msg void OnUseTeachingTrace();
    afx_msg void OnRunComparison();
    afx_msg void OnLoadSelected();
    afx_msg void OnExportComparison();
    afx_msg void OnDrawItem(int controlId, LPDRAWITEMSTRUCT drawItem);

    void RefreshPlans();
    void RefreshResults();
    int SelectedIndex() const;
    std::string ReadName() const;
    static std::vector<b5cache::MemoryAccess> TeachingTrace();

    b5cache::SimulationConfig currentConfig_;
    std::vector<b5cache::MemoryAccess> trace_;
    std::vector<b5cache::ComparisonPlan> plans_;
    std::vector<b5cache::ComparisonResult> results_;
    int loadedPlanIndex_ = -1;
    bool loadRequested_ = false;
};
