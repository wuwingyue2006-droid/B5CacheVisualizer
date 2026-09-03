#pragma once

#include "resource.h"
#include "trace/TraceGenerator.h"

#include <vector>

class CTraceGeneratorDlg final : public CDialogEx {
public:
    explicit CTraceGeneratorDlg(CWnd* parent = nullptr);

    enum { IDD = IDD_TRACE_GENERATOR_DIALOG };

    const std::vector<b5cache::MemoryAccess>& GeneratedTrace() const noexcept;

protected:
    BOOL OnInitDialog() override;
    void OnOK() override;

    DECLARE_MESSAGE_MAP()

private:
    afx_msg void OnModeChanged();
    afx_msg void OnPresetChanged();

    b5cache::TraceGenerationConfig ReadConfig() const;
    void ApplyPreset();
    void UpdateControlStates();

    std::vector<b5cache::MemoryAccess> generatedTrace_;
};
