#include "pch.h"

#include "ComparisonDlg.h"

#include <atlconv.h>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

b5cache::SimulationConfig MakeConfig(
    const b5cache::MappingKind mapping,
    const b5cache::ReplacementKind replacement,
    const std::size_t associativity) {
    b5cache::SimulationConfig config;
    config.l1 = {"L1", 64, 16, associativity, mapping, replacement};
    config.l2 = {"L2", 128, 16, associativity == 4 ? 8 : associativity, mapping, replacement};
    return config;
}

}  // namespace

BEGIN_MESSAGE_MAP(CComparisonDlg, CDialogEx)
    ON_LBN_SELCHANGE(IDC_LIST_COMPARE_PLANS, &CComparisonDlg::OnPlanSelected)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE_ADD, &CComparisonDlg::OnAddCurrent)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE_UPDATE, &CComparisonDlg::OnUpdateCurrent)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE_REMOVE, &CComparisonDlg::OnRemove)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE_PRESETS, &CComparisonDlg::OnTeachingPresets)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE_TEACHING, &CComparisonDlg::OnUseTeachingTrace)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE_RUN, &CComparisonDlg::OnRunComparison)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE_LOAD, &CComparisonDlg::OnLoadSelected)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE_EXPORT, &CComparisonDlg::OnExportComparison)
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

CComparisonDlg::CComparisonDlg(
    b5cache::SimulationConfig currentConfig,
    std::vector<b5cache::MemoryAccess> trace,
    std::vector<b5cache::ComparisonPlan> plans,
    CWnd* parent)
    : CDialogEx(IDD_COMPARISON_DIALOG, parent),
      currentConfig_(std::move(currentConfig)),
      trace_(std::move(trace)),
      plans_(std::move(plans)) {}

const std::vector<b5cache::ComparisonPlan>& CComparisonDlg::Plans() const noexcept { return plans_; }
const std::vector<b5cache::MemoryAccess>& CComparisonDlg::Trace() const noexcept { return trace_; }
bool CComparisonDlg::LoadRequested() const noexcept { return loadRequested_; }

const b5cache::ComparisonPlan* CComparisonDlg::SelectedPlan() const noexcept {
    return loadedPlanIndex_ < 0 || loadedPlanIndex_ >= static_cast<int>(plans_.size())
        ? nullptr
        : &plans_[static_cast<std::size_t>(loadedPlanIndex_)];
}

BOOL CComparisonDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();
    RefreshPlans();
    CString message;
    message.Format(L"%llu parsed accesses. All plans use this exact sequence.",
                   static_cast<unsigned long long>(trace_.size()));
    SetDlgItemText(IDC_STATIC_COMPARE_TRACE, message);
    RefreshResults();
    return TRUE;
}

int CComparisonDlg::SelectedIndex() const {
    const auto* list = static_cast<CListBox*>(GetDlgItem(IDC_LIST_COMPARE_PLANS));
    return list == nullptr ? LB_ERR : list->GetCurSel();
}

std::string CComparisonDlg::ReadName() const {
    CString name;
    GetDlgItemText(IDC_EDIT_COMPARE_NAME, name);
    const CW2A utf8(name, CP_UTF8);
    return utf8.m_psz == nullptr ? std::string{} : std::string(utf8.m_psz);
}

void CComparisonDlg::RefreshPlans() {
    auto* list = static_cast<CListBox*>(GetDlgItem(IDC_LIST_COMPARE_PLANS));
    if (list == nullptr) return;
    const auto oldSelection = list->GetCurSel();
    list->ResetContent();
    for (const auto& plan : plans_) {
        const CA2W name(plan.name.c_str(), CP_UTF8);
        list->AddString(name);
    }
    if (!plans_.empty()) {
        list->SetCurSel(oldSelection >= 0 && oldSelection < static_cast<int>(plans_.size()) ? oldSelection : 0);
        OnPlanSelected();
    } else {
        SetDlgItemText(IDC_EDIT_COMPARE_NAME, L"");
    }
}

void CComparisonDlg::RefreshResults() {
    std::ostringstream text;
    if (results_.empty()) {
        text << "Select 2-3 plans and run comparison.";
    } else {
        text << std::fixed << std::setprecision(2);
        for (const auto& result : results_) {
            const auto& s = result.statistics;
            text << result.planName << "\r\n"
                 << "  A " << s.accesses << " | L1 " << s.l1Hits << " | L2 " << s.l2Hits
                 << " | Miss " << s.memoryMisses << "\r\n"
                 << "  L1 " << s.L1HitRate() * 100.0 << "% | Overall " << s.OverallHitRate() * 100.0
                 << "% | Miss " << s.MissRate() * 100.0 << "%\r\n";
        }
    }
    const CA2W wide(text.str().c_str(), CP_UTF8);
    SetDlgItemText(IDC_EDIT_COMPARE_RESULTS, wide);
    if (auto* chart = GetDlgItem(IDC_STATIC_COMPARE_CHART); chart != nullptr) chart->Invalidate(FALSE);
    if (auto* exportButton = GetDlgItem(IDC_BUTTON_COMPARE_EXPORT); exportButton != nullptr) {
        exportButton->EnableWindow(results_.empty() ? FALSE : TRUE);
    }
}

void CComparisonDlg::OnPlanSelected() {
    const auto index = SelectedIndex();
    if (index >= 0) {
        const CA2W name(plans_[static_cast<std::size_t>(index)].name.c_str(), CP_UTF8);
        SetDlgItemText(IDC_EDIT_COMPARE_NAME, name);
    }
}

void CComparisonDlg::OnAddCurrent() {
    if (plans_.size() >= 3) { AfxMessageBox(L"A comparison supports at most three plans.", MB_OK | MB_ICONINFORMATION); return; }
    auto name = ReadName();
    if (name.empty()) name = "Plan " + std::to_string(plans_.size() + 1);
    plans_.push_back({name, currentConfig_});
    results_.clear();
    RefreshPlans();
    RefreshResults();
}

void CComparisonDlg::OnUpdateCurrent() {
    const auto index = SelectedIndex();
    if (index < 0) { AfxMessageBox(L"Select a plan first.", MB_OK | MB_ICONINFORMATION); return; }
    const auto name = ReadName();
    if (name.empty()) { AfxMessageBox(L"Plan name must not be empty.", MB_OK | MB_ICONERROR); return; }
    plans_[static_cast<std::size_t>(index)] = {name, currentConfig_};
    results_.clear();
    RefreshPlans();
    RefreshResults();
}

void CComparisonDlg::OnRemove() {
    const auto index = SelectedIndex();
    if (index < 0) return;
    plans_.erase(plans_.begin() + index);
    results_.clear();
    RefreshPlans();
    RefreshResults();
}

void CComparisonDlg::OnTeachingPresets() {
    plans_ = {
        {"Direct + FIFO", MakeConfig(b5cache::MappingKind::Direct, b5cache::ReplacementKind::Fifo, 1)},
        {"2-way Set + LRU", MakeConfig(b5cache::MappingKind::SetAssociative, b5cache::ReplacementKind::Lru, 2)},
        {"Fully Associative + LRU", MakeConfig(b5cache::MappingKind::FullyAssociative, b5cache::ReplacementKind::Lru, 4)},
    };
    results_.clear();
    RefreshPlans();
    RefreshResults();
}

std::vector<b5cache::MemoryAccess> CComparisonDlg::TeachingTrace() {
    return {{0x00, false}, {0x80, false}, {0x00, false}, {0x80, false},
            {0x40, false}, {0x00, false}, {0x80, false}, {0x00, false}};
}

void CComparisonDlg::OnUseTeachingTrace() {
    trace_ = TeachingTrace();
    SetDlgItemText(IDC_STATIC_COMPARE_TRACE,
                   L"Teaching trace: Direct, 2-way Set and Fully Associative produce distinct miss rates.");
    results_.clear();
    RefreshResults();
}

void CComparisonDlg::OnRunComparison() {
    results_.clear();
    try {
        results_ = b5cache::ComparisonRunner::Run(plans_, trace_);
        RefreshResults();
    } catch (const std::exception& error) {
        RefreshResults();
        const CA2W message(error.what(), CP_UTF8);
        AfxMessageBox(message, MB_OK | MB_ICONERROR);
    }
}

void CComparisonDlg::OnLoadSelected() {
    const auto index = SelectedIndex();
    if (index < 0) { AfxMessageBox(L"Select a plan to load.", MB_OK | MB_ICONINFORMATION); return; }
    loadedPlanIndex_ = index;
    loadRequested_ = true;
    CDialogEx::OnOK();
}

void CComparisonDlg::OnExportComparison() {
    if (results_.empty()) {
        AfxMessageBox(L"Run the comparison before exporting.", MB_OK | MB_ICONINFORMATION);
        return;
    }

    b5cache::ExperimentExportData data;
    data.comparisonPlans = plans_;
    data.comparisonResults = results_;
    data.trace = trace_;
    data.exportTime = b5cache::ExperimentExporter::CurrentTimeStamp();

    const std::string stamp = b5cache::ExperimentExporter::FileTimeStamp();
    CString defaultName;
    defaultName.Format(L"B5CacheComparison_%S.csv", stamp.c_str());
    CFileDialog saveDialog(
        FALSE,
        L"csv",
        defaultName,
        OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
        L"CSV Files (*.csv)|*.csv|Text Files (*.txt)|*.txt||");
    if (saveDialog.DoModal() != IDOK) {
        return;  // Cancelling the save dialog is not an error.
    }

    const std::filesystem::path path(saveDialog.GetPathName().GetString());
    try {
        const bool asCsv = _wcsicmp(path.extension().c_str(), L".csv") == 0;
        const std::string content = asCsv
            ? b5cache::ExperimentExporter::FormatComparisonCsv(data)
            : b5cache::ExperimentExporter::FormatComparisonTxt(data);
        b5cache::ExperimentExporter::WriteUtf8File(path, content);

        CString message;
        message.Format(L"Exported comparison to: %s", saveDialog.GetPathName().GetString());
        SetDlgItemText(IDC_STATIC_COMPARE_TRACE, message);
    } catch (const std::exception& error) {
        const CA2W wideMessage(error.what(), CP_UTF8);
        AfxMessageBox(wideMessage, MB_OK | MB_ICONERROR);
    }
}

void CComparisonDlg::OnDrawItem(const int controlId, LPDRAWITEMSTRUCT drawItem) {
    if (controlId != IDC_STATIC_COMPARE_CHART) { CDialogEx::OnDrawItem(controlId, drawItem); return; }
    CDC dc; dc.Attach(drawItem->hDC);
    const CRect area(drawItem->rcItem);
    dc.FillSolidRect(area, RGB(250, 251, 253));
    dc.SetBkMode(TRANSPARENT);
    if (results_.empty()) {
        dc.DrawText(L"Run comparison to draw Overall / Miss bars.", const_cast<CRect*>(&area), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else {
        const int rowHeight = (std::max)(1, area.Height() / static_cast<int>(results_.size()));
        for (std::size_t index = 0; index < results_.size(); ++index) {
            const int top = area.top + static_cast<int>(index) * rowHeight;
            const int labelWidth = 230;
            const int valueWidth = 145;
            const int barWidth = (std::max)(1, area.Width() - labelWidth - valueWidth - 10);
            const auto& s = results_[index].statistics;
            CRect label(area.left + 4, top, area.left + labelWidth, top + rowHeight);
            const CA2W name(results_[index].planName.c_str(), CP_UTF8);
            const CString displayName(name);
            dc.DrawText(displayName, &label, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            const int overall = static_cast<int>(barWidth * s.OverallHitRate());
            const int miss = static_cast<int>(barWidth * s.MissRate());
            dc.FillSolidRect(area.left + labelWidth, top + 3, overall, 7, RGB(62, 168, 101));
            dc.FillSolidRect(area.left + labelWidth, top + 12, miss, 7, RGB(232, 112, 78));
            CString values; values.Format(L"O %.0f%%  M %.0f%%", s.OverallHitRate() * 100.0, s.MissRate() * 100.0);
            CRect value(area.right - valueWidth - 4, top, area.right - 4, top + rowHeight);
            dc.DrawText(values, &value, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        }
    }
    dc.Detach();
}
