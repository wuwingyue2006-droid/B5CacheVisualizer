#pragma once

#include "core/CacheSimulator.h"

class CB5CacheVisualizerDlg final : public CDialogEx {
public:
    explicit CB5CacheVisualizerDlg(CWnd* parent = nullptr);

    enum { IDD = IDD_B5CACHEVISUALIZER_DIALOG };

protected:
    void DoDataExchange(CDataExchange* dataExchange) override;
    BOOL OnInitDialog() override;

    DECLARE_MESSAGE_MAP()

private:
    afx_msg void OnRunTrace();
    afx_msg void OnResetSimulation();

    void ShowSummary();

    b5cache::CacheSimulator simulator_;
};
