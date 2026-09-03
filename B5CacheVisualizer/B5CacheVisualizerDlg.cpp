#include "pch.h"

#include "B5CacheVisualizer.h"
#include "B5CacheVisualizerDlg.h"
#include "trace/MemoryTraceParser.h"

#include <atlconv.h>
#include <algorithm>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

const wchar_t* ToWide(const std::string& value) {
    static std::wstring converted;
    converted.assign(value.begin(), value.end());
    return converted.c_str();
}

std::wstring ToString(std::size_t value) {
    return std::to_wstring(static_cast<unsigned long long>(value));
}

std::wstring OutcomeText(const b5cache::AccessOutcome outcome) {
    switch (outcome) {
    case b5cache::AccessOutcome::L1Hit:
        return L"L1 Hit";
    case b5cache::AccessOutcome::L2Hit:
        return L"L2 Hit";
    case b5cache::AccessOutcome::MemoryMiss:
        return L"Memory Miss";
    default:
        return L"Unknown";
    }
}

std::wstring ToText(const b5cache::MemoryAccess& access) {
    std::wstringstream stream;
    stream << (access.isWrite ? L"W" : L"R") << L" 0x" << std::hex << std::uppercase << access.address;
    return stream.str();
}

bool SameTrace(
    const std::vector<b5cache::MemoryAccess>& left,
    const std::vector<b5cache::MemoryAccess>& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t index = 0; index < left.size(); ++index) {
        if (left[index].address != right[index].address ||
            left[index].isWrite != right[index].isWrite) {
            return false;
        }
    }
    return true;
}

}  // namespace

BEGIN_MESSAGE_MAP(CB5CacheVisualizerDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CB5CacheVisualizerDlg::OnApplyConfig)
    ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CB5CacheVisualizerDlg::OnImportTrace)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_TRACE, &CB5CacheVisualizerDlg::OnClearTrace)
    ON_BN_CLICKED(IDC_BUTTON_STEP, &CB5CacheVisualizerDlg::OnStepTrace)
    ON_BN_CLICKED(IDC_BUTTON_RUN_ALL, &CB5CacheVisualizerDlg::OnRunAllTrace)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CB5CacheVisualizerDlg::OnResetSimulation)
    ON_WM_DRAWITEM()
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_L1_CACHE_VIEW, &CB5CacheVisualizerDlg::OnCustomDrawCacheView)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_L2_CACHE_VIEW, &CB5CacheVisualizerDlg::OnCustomDrawCacheView)
END_MESSAGE_MAP()

CB5CacheVisualizerDlg::CB5CacheVisualizerDlg(CWnd* parent)
    : CDialogEx(IDD_B5CACHEVISUALIZER_DIALOG, parent) {}

void CB5CacheVisualizerDlg::DoDataExchange(CDataExchange* dataExchange) {
    CDialogEx::DoDataExchange(dataExchange);
}

BOOL CB5CacheVisualizerDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    l1CacheView_.SubclassDlgItem(IDC_L1_CACHE_VIEW, this);
    l2CacheView_.SubclassDlgItem(IDC_L2_CACHE_VIEW, this);
    SetupCacheViewColumns(l1CacheView_);
    SetupCacheViewColumns(l2CacheView_);

    CComboBox* l1Mapping = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_L1_MAPPING));
    if (l1Mapping != nullptr) {
        l1Mapping->AddString(L"Direct Mapping");
        l1Mapping->AddString(L"Fully Associative");
        l1Mapping->AddString(L"Set Associative");
        l1Mapping->SetCurSel(0);
    }

    CComboBox* l1Replacement = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_L1_REPLACEMENT));
    if (l1Replacement != nullptr) {
        l1Replacement->AddString(L"FIFO");
        l1Replacement->AddString(L"LRU");
        l1Replacement->SetCurSel(0);
    }

    CComboBox* l2Mapping = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_L2_MAPPING));
    if (l2Mapping != nullptr) {
        l2Mapping->AddString(L"Direct Mapping");
        l2Mapping->AddString(L"Fully Associative");
        l2Mapping->AddString(L"Set Associative");
        l2Mapping->SetCurSel(0);
    }

    CComboBox* l2Replacement = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_L2_REPLACEMENT));
    if (l2Replacement != nullptr) {
        l2Replacement->AddString(L"FIFO");
        l2Replacement->AddString(L"LRU");
        l2Replacement->SetCurSel(0);
    }

    SetDlgItemInt(IDC_EDIT_L1_SIZE, 64, FALSE);
    SetDlgItemInt(IDC_EDIT_L1_BLOCK, 16, FALSE);
    SetDlgItemInt(IDC_EDIT_L1_ASSOC, 1, FALSE);
    SetDlgItemInt(IDC_EDIT_L2_SIZE, 128, FALSE);
    SetDlgItemInt(IDC_EDIT_L2_BLOCK, 16, FALSE);
    SetDlgItemInt(IDC_EDIT_L2_ASSOC, 1, FALSE);

    SetDlgItemText(
        IDC_EDIT_TRACE,
        L"# Miss -> L1 Hit -> L2 Hit -> Dirty/Eviction\r\n"
        L"R 0x00\r\n"
        L"R 0x00\r\n"
        L"R 0x40\r\n"
        L"R 0x00\r\n"
        L"W 0x80");

    simulator_ = std::make_unique<b5cache::CacheSimulator>(b5cache::CacheSimulator::DefaultConfig());
    try {
        if (LoadTraceFromEditor(trace_)) {
            ResetSession();
        }
    } catch (const std::exception& error) {
        ShowUserError(error.what());
    }

    RefreshStatistics();
    RefreshCacheViews();
    RefreshAccessVisuals();
    return TRUE;
}

bool CB5CacheVisualizerDlg::ReadConfiguration(b5cache::SimulationConfig& config) const {
    auto readUnsigned = [&](const int controlId, const char* fieldName) -> std::size_t {
        CString text;
        GetDlgItemText(controlId, text);
        if (text.IsEmpty()) {
            throw std::invalid_argument(std::string(fieldName) + " must not be empty.");
        }

        CString trimmed = text.Trim();
        if (trimmed.IsEmpty()) {
            throw std::invalid_argument(std::string(fieldName) + " must not be empty.");
        }

        wchar_t* endPointer = nullptr;
        errno = 0;
        const auto parsed = std::wcstoll(trimmed.GetString(), &endPointer, 10);
        if (errno != 0 || parsed < 0 || endPointer == trimmed.GetString() || *endPointer != '\0') {
            throw std::invalid_argument(std::string(fieldName) + " must be a valid positive integer.");
        }

        return static_cast<std::size_t>(parsed);
    };

    std::size_t l1Size = 0;
    std::size_t l1Block = 0;
    std::size_t l1Assoc = 0;
    std::size_t l2Size = 0;
    std::size_t l2Block = 0;
    std::size_t l2Assoc = 0;

    try {
        l1Size = readUnsigned(IDC_EDIT_L1_SIZE, "L1 Cache Size");
        l1Block = readUnsigned(IDC_EDIT_L1_BLOCK, "L1 Block Size");
        l1Assoc = readUnsigned(IDC_EDIT_L1_ASSOC, "L1 associativity");
        l2Size = readUnsigned(IDC_EDIT_L2_SIZE, "L2 Cache Size");
        l2Block = readUnsigned(IDC_EDIT_L2_BLOCK, "L2 Block Size");
        l2Assoc = readUnsigned(IDC_EDIT_L2_ASSOC, "L2 associativity");
    } catch (const std::exception& error) {
        ShowUserError(error.what());
        return false;
    }

    const auto l1MappingIndex = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_L1_MAPPING))->GetCurSel();
    const auto l1ReplacementIndex = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_L1_REPLACEMENT))->GetCurSel();
    const auto l2MappingIndex = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_L2_MAPPING))->GetCurSel();
    const auto l2ReplacementIndex = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_L2_REPLACEMENT))->GetCurSel();

    auto toMapping = [](const int index) {
        switch (index) {
        case 0:
            return b5cache::MappingKind::Direct;
        case 1:
            return b5cache::MappingKind::FullyAssociative;
        case 2:
            return b5cache::MappingKind::SetAssociative;
        default:
            throw std::invalid_argument("Unsupported mapping selection.");
        }
    };

    auto toReplacement = [](const int index) {
        switch (index) {
        case 0:
            return b5cache::ReplacementKind::Fifo;
        case 1:
            return b5cache::ReplacementKind::Lru;
        default:
            throw std::invalid_argument("Unsupported replacement selection.");
        }
    };

    try {
        config.l1 = {"L1", l1Size, l1Block, l1Assoc, toMapping(l1MappingIndex), toReplacement(l1ReplacementIndex)};
        config.l2 = {"L2", l2Size, l2Block, l2Assoc, toMapping(l2MappingIndex), toReplacement(l2ReplacementIndex)};
        return true;
    } catch (const std::exception& error) {
        ShowUserError(error.what());
        return false;
    }
}

bool CB5CacheVisualizerDlg::ParseTraceFromEditor(std::vector<b5cache::MemoryAccess>& accesses) const {
    CString traceText;
    GetDlgItemText(IDC_EDIT_TRACE, traceText);

    try {
        const CW2A narrowText(traceText, CP_UTF8);
        auto parsedTrace = b5cache::MemoryTraceParser::ParseText(narrowText.m_psz);
        accesses = std::move(parsedTrace);
        return true;
    } catch (const std::exception& error) {
        ShowUserError(error.what());
        return false;
    }
}

bool CB5CacheVisualizerDlg::LoadTraceFromEditor(std::vector<b5cache::MemoryAccess>& accesses) {
    std::vector<b5cache::MemoryAccess> parsedTrace;
    if (!ParseTraceFromEditor(parsedTrace)) {
        return false;
    }

    if (!SameTrace(parsedTrace, trace_)) {
        trace_ = parsedTrace;
        ResetSession();
    }
    accesses = std::move(parsedTrace);
    return true;
}

void CB5CacheVisualizerDlg::ResetSession() {
    if (simulator_ == nullptr) {
        simulator_ = std::make_unique<b5cache::CacheSimulator>(b5cache::CacheSimulator::DefaultConfig());
    }

    simulator_->Reset();
    nextIndex_ = 0;
    hasLastResult_ = false;
    lastResult_ = {};
    overallHitRateHistory_.clear();
    RefreshTraceStatus();
    RefreshStatistics();
    RefreshCacheViews();
    RefreshLastResult(nullptr);
}

void CB5CacheVisualizerDlg::ExecuteNextAccess() {
    if (simulator_ == nullptr) {
        ShowUserError("No simulator is available.");
        return;
    }

    if (trace_.empty()) {
        ShowUserError("Trace is empty. Add at least one access before stepping.");
        return;
    }

    if (nextIndex_ >= trace_.size()) {
        ShowUserError("The trace has already been fully executed.");
        return;
    }

    lastResult_ = simulator_->Access(trace_[nextIndex_]);
    hasLastResult_ = true;
    ++nextIndex_;
    RecordStatisticsPoint();
    RefreshTraceStatus();
    RefreshStatistics();
    RefreshCacheViews(&lastResult_);

    RefreshLastResult(&lastResult_);
}

void CB5CacheVisualizerDlg::RefreshCacheViews(const b5cache::AccessResult* latest) {
    ClearCacheView(l1CacheView_);
    ClearCacheView(l2CacheView_);

    auto renderLevel = [&](CListCtrl& view, const b5cache::CacheLevel& level, bool isL1) {
        const auto& sets = level.Sets();
        for (std::size_t setIndex = 0; setIndex < sets.size(); ++setIndex) {
            const auto& set = sets[setIndex];
            for (std::size_t lineIndex = 0; lineIndex < set.size(); ++lineIndex) {
                const auto& line = set[lineIndex];
                std::wstring state = L"Idle";

                if (latest != nullptr) {
                    if (isL1) {
                        if (latest->l1.evicted && latest->l1.setIndex == setIndex && latest->l1.lineIndex == lineIndex) {
                            state = L"Replaced -> New";
                        } else if (latest->outcome == b5cache::AccessOutcome::L1Hit && latest->l1.setIndex == setIndex && latest->l1.lineIndex == lineIndex) {
                            state = L"L1 Hit";
                        } else if (latest->l1.setIndex == setIndex && latest->l1.lineIndex == lineIndex && !latest->l1.hit) {
                            state = L"New";
                        }
                    } else {
                        if (latest->l2.evicted && latest->l2.setIndex == setIndex && latest->l2.lineIndex == lineIndex) {
                            state = L"Replaced -> New";
                        } else if (latest->outcome == b5cache::AccessOutcome::L2Hit && latest->l2.setIndex == setIndex && latest->l2.lineIndex == lineIndex) {
                            state = L"L2 Hit";
                        } else if (latest->l2.setIndex == setIndex && latest->l2.lineIndex == lineIndex && !latest->l2.hit) {
                            state = L"New";
                        }
                    }
                }

                const int itemIndex = view.InsertItem(view.GetItemCount(), std::to_wstring(static_cast<unsigned long long>(setIndex)).c_str());
                view.SetItemText(itemIndex, 1, std::to_wstring(static_cast<unsigned long long>(lineIndex)).c_str());
                view.SetItemText(itemIndex, 2, line.valid ? L"Yes" : L"No");
                view.SetItemText(itemIndex, 3, line.dirty ? L"Yes" : L"No");
                view.SetItemText(itemIndex, 4, std::to_wstring(static_cast<unsigned long long>(line.tag)).c_str());
                view.SetItemText(itemIndex, 5, std::to_wstring(static_cast<unsigned long long>(line.blockNumber)).c_str());
                view.SetItemText(itemIndex, 6, std::to_wstring(static_cast<unsigned long long>(line.insertedAt)).c_str());
                view.SetItemText(itemIndex, 7, std::to_wstring(static_cast<unsigned long long>(line.lastUsedAt)).c_str());
                view.SetItemText(itemIndex, 8, state.c_str());

            }
        }
    };

    renderLevel(l1CacheView_, simulator_->L1(), true);
    renderLevel(l2CacheView_, simulator_->L2(), false);
}

void CB5CacheVisualizerDlg::RefreshStatistics() {
    if (simulator_ == nullptr) {
        SetDlgItemText(IDC_EDIT_RESULT, L"No simulator available.");
        RefreshStatisticsCharts();
        return;
    }

    const auto snapshot = simulator_->Statistics();
    std::ostringstream output;
    output << "Accesses: " << snapshot.accesses << "\r\n"
           << "Reads: " << snapshot.reads << "\r\n"
           << "Writes: " << snapshot.writes << "\r\n"
           << "L1 Hits: " << snapshot.l1Hits << "\r\n"
           << "L2 Hits: " << snapshot.l2Hits << "\r\n"
           << "Misses: " << snapshot.memoryMisses << "\r\n"
           << "L1 Hit Rate: " << std::fixed << std::setprecision(2) << snapshot.L1HitRate() * 100.0 << "%\r\n"
           << "L2 Hit Rate: " << snapshot.L2HitRate() * 100.0 << "%\r\n"
           << "Overall Hit Rate: " << snapshot.OverallHitRate() * 100.0 << "%\r\n"
           << "Miss Rate: " << snapshot.MissRate() * 100.0 << "%";

    const CA2W wideOutput(output.str().c_str(), CP_UTF8);
    SetDlgItemText(IDC_EDIT_RESULT, wideOutput);

    RefreshStatisticsCharts();
}

void CB5CacheVisualizerDlg::RecordStatisticsPoint() {
    if (simulator_ == nullptr) {
        return;
    }
    overallHitRateHistory_.push_back(simulator_->Statistics().OverallHitRate());
}

void CB5CacheVisualizerDlg::RefreshStatisticsCharts() const {
    for (const int controlId : {IDC_STATIC_OUTCOME_CHART, IDC_STATIC_RATE_CHART}) {
        if (CWnd* chart = GetDlgItem(controlId); chart != nullptr) {
            chart->Invalidate(FALSE);
        }
    }
}

void CB5CacheVisualizerDlg::DrawOutcomeChart(CDC& dc, const CRect& bounds) const {
    const COLORREF l1Color = RGB(62, 168, 101);
    const COLORREF l2Color = RGB(65, 125, 220);
    const COLORREF missColor = RGB(232, 112, 78);
    const COLORREF colors[] = {l1Color, l2Color, missColor};

    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(45, 45, 45));
    CRect title(bounds.left + 6, bounds.top + 3, bounds.right - 4, bounds.top + 21);
    dc.DrawText(L"Outcome Mix", &title, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    if (simulator_ == nullptr || simulator_->Statistics().accesses == 0) {
        CRect empty(bounds.left + 6, bounds.top + 24, bounds.right - 6, bounds.bottom - 4);
        dc.SetTextColor(RGB(120, 120, 120));
        dc.DrawText(L"No accesses yet", &empty, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
        return;
    }

    const auto snapshot = simulator_->Statistics();
    const double rates[] = {snapshot.L1HitRate(), snapshot.L2HitRate(), snapshot.MissRate()};
    const wchar_t* labels[] = {L"L1", L"L2", L"Miss"};
    const int contentTop = title.bottom + 2;
    const int availableHeight = static_cast<int>(bounds.bottom - contentTop - 4);
    const int rowHeight = (std::max)(17, availableHeight / 3);
    const int chartWidth = static_cast<int>(bounds.Width());
    const int labelWidth = (std::min)(70, (std::max)(48, chartWidth / 3));

    for (int index = 0; index < 3; ++index) {
        const int top = contentTop + index * rowHeight;
        CString label;
        label.Format(L"%s %3.0f%%", labels[index], rates[index] * 100.0);
        CRect labelRect(bounds.left + 6, top, bounds.left + 6 + labelWidth, top + rowHeight);
        dc.SetTextColor(RGB(70, 70, 70));
        dc.DrawText(label, &labelRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

        CRect track(labelRect.right + 3, top + 4, bounds.right - 7, top + rowHeight - 4);
        if (track.Height() < 5) {
            track.bottom = track.top + 5;
        }
        dc.FillSolidRect(track, RGB(232, 235, 239));
        CRect fill = track;
        fill.right = fill.left + static_cast<int>(static_cast<double>(track.Width()) * rates[index] + 0.5);
        if (fill.right > fill.left) {
            dc.FillSolidRect(fill, colors[index]);
        }
    }
}

void CB5CacheVisualizerDlg::DrawHitRateChart(CDC& dc, const CRect& bounds) const {
    const COLORREF lineColor = RGB(65, 125, 220);
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(45, 45, 45));
    CRect title(bounds.left + 6, bounds.top + 3, bounds.right - 4, bounds.top + 21);
    dc.DrawText(L"Cumulative Hit Rate", &title, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    if (overallHitRateHistory_.empty()) {
        CRect empty(bounds.left + 6, bounds.top + 24, bounds.right - 6, bounds.bottom - 4);
        dc.SetTextColor(RGB(120, 120, 120));
        dc.DrawText(L"No accesses yet", &empty, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
        return;
    }

    CRect plot(bounds.left + 34, title.bottom + 11, bounds.right - 8, bounds.bottom - 15);
    if (plot.Width() < 10 || plot.Height() < 10) {
        return;
    }

    CPen gridPen(PS_SOLID, 1, RGB(210, 214, 220));
    CPen* oldPen = dc.SelectObject(&gridPen);
    dc.MoveTo(plot.left, plot.top);
    dc.LineTo(plot.left, plot.bottom);
    dc.LineTo(plot.right, plot.bottom);
    dc.MoveTo(plot.left, plot.top + plot.Height() / 2);
    dc.LineTo(plot.right, plot.top + plot.Height() / 2);

    dc.SetTextColor(RGB(110, 110, 110));
    CRect topLabel(bounds.left + 2, plot.top - 8, plot.left - 4, plot.top + 7);
    CRect bottomLabel(bounds.left + 2, plot.bottom - 8, plot.left - 3, plot.bottom + 7);
    dc.DrawText(L"100", &topLabel, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
    dc.DrawText(L"0", &bottomLabel, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    CPen ratePen(PS_SOLID, 2, lineColor);
    dc.SelectObject(&ratePen);
    const std::size_t pointCount = overallHitRateHistory_.size();
    for (std::size_t index = 0; index < pointCount; ++index) {
        const double xFraction = pointCount == 1
            ? 1.0
            : static_cast<double>(index) / static_cast<double>(pointCount - 1);
        const double rate = std::clamp(overallHitRateHistory_[index], 0.0, 1.0);
        const int x = plot.left + static_cast<int>(xFraction * static_cast<double>(plot.Width()) + 0.5);
        const int y = plot.bottom - static_cast<int>(rate * static_cast<double>(plot.Height()) + 0.5);
        if (index == 0) {
            dc.MoveTo(x, y);
        } else {
            dc.LineTo(x, y);
        }
        dc.Ellipse(x - 2, y - 2, x + 3, y + 3);
    }
    dc.SelectObject(oldPen);

    CString sampleLabel;
    sampleLabel.Format(L"%llu accesses", static_cast<unsigned long long>(pointCount));
    CRect xLabel(plot.left, plot.bottom + 1, plot.right, bounds.bottom - 1);
    dc.SetTextColor(RGB(95, 95, 95));
    dc.DrawText(sampleLabel, &xLabel, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
}

CB5CacheVisualizerDlg::LevelDisplayState CB5CacheVisualizerDlg::BuildLevelDisplayState(
    const b5cache::CacheLevel& level,
    const b5cache::LevelAccessDetail& detail,
    const bool accessed) const {
    LevelDisplayState state;
    state.accessed = accessed;
    if (!accessed || detail.lineIndex == b5cache::kInvalidIndex) {
        return state;
    }

    const auto& sets = level.Sets();
    if (detail.setIndex >= sets.size() || detail.lineIndex >= sets[detail.setIndex].size()) {
        return state;
    }

    const auto& line = sets[detail.setIndex][detail.lineIndex];
    state.hit = detail.hit;
    state.dirty = line.dirty;
    state.evicted = detail.evicted;
    state.setIndex = detail.setIndex;
    state.lineIndex = detail.lineIndex;
    state.blockNumber = line.blockNumber;
    state.tag = line.tag;
    state.evictedBlock = detail.evictedBlock;
    const auto blockSize = level.Config().blockSizeBytes;
    state.offset = blockSize == 0 ? 0 : lastResult_.request.address % blockSize;
    return state;
}

void CB5CacheVisualizerDlg::RefreshAccessVisuals() const {
    for (const int controlId : {IDC_STATIC_ADDRESS_VIEW, IDC_STATIC_ACCESS_PATH}) {
        if (CWnd* panel = GetDlgItem(controlId); panel != nullptr) {
            panel->Invalidate(FALSE);
        }
    }
}

void CB5CacheVisualizerDlg::DrawAddressBreakdown(CDC& dc, const CRect& bounds) const {
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(45, 45, 45));

    if (!hasLastResult_ || simulator_ == nullptr) {
        CRect title(bounds.left + 6, bounds.top + 4, bounds.right - 6, bounds.top + 23);
        dc.DrawText(L"Address Breakdown", &title, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        CRect empty(bounds.left + 6, bounds.top + 25, bounds.right - 6, bounds.bottom - 5);
        dc.SetTextColor(RGB(120, 120, 120));
        dc.DrawText(L"Step or Run All to inspect Tag / Set / Offset", &empty,
                    DT_CENTER | DT_WORDBREAK | DT_VCENTER);
        return;
    }

    CString requestText;
    requestText.Format(
        L"%s  Address 0x%llX",
        lastResult_.request.isWrite ? L"WRITE" : L"READ",
        static_cast<unsigned long long>(lastResult_.request.address));
    CRect title(bounds.left + 6, bounds.top + 3, bounds.right - 6, bounds.top + 22);
    dc.SetTextColor(lastResult_.request.isWrite ? RGB(178, 92, 0) : RGB(35, 93, 170));
    dc.DrawText(requestText, &title, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    const auto l1State = BuildLevelDisplayState(simulator_->L1(), lastResult_.l1, true);
    const bool l2Accessed = lastResult_.outcome != b5cache::AccessOutcome::L1Hit;
    const auto l2State = BuildLevelDisplayState(simulator_->L2(), lastResult_.l2, l2Accessed);
    const int rowTop = title.bottom + 1;
    const int rowHeight = static_cast<int>((bounds.bottom - rowTop - 3) / 2);

    auto drawLevel = [&](const wchar_t* levelName, const LevelDisplayState& state, const int top) {
        CRect summaryRect(bounds.left + 6, top, bounds.right - 6, top + 19);
        if (!state.accessed) {
            CString skipped;
            skipped.Format(L"%s  Not accessed (L1 hit)", levelName);
            dc.SetTextColor(RGB(125, 125, 125));
            dc.DrawText(skipped, &summaryRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
            CRect skippedBar(bounds.left + 6, top + 21, bounds.right - 6, top + rowHeight - 3);
            dc.FillSolidRect(skippedBar, RGB(235, 237, 240));
            return;
        }

        CString summary;
        summary.Format(
            L"%s  Block %llu | Line %llu | %s",
            levelName,
            static_cast<unsigned long long>(state.blockNumber),
            static_cast<unsigned long long>(state.lineIndex),
            state.hit ? L"Hit" : L"Fill");
        if (state.evicted) {
            CString eviction;
            eviction.Format(L" | Evict B%llu", static_cast<unsigned long long>(state.evictedBlock));
            summary += eviction;
        }
        if (state.dirty) {
            summary += L" | Dirty";
        }
        dc.SetTextColor(state.evicted ? RGB(170, 63, 37)
                                     : (state.hit ? RGB(24, 120, 65) : RGB(35, 93, 170)));
        dc.DrawText(summary, &summaryRect,
                    DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

        const COLORREF fills[] = {
            RGB(235, 225, 250),
            RGB(220, 237, 255),
            RGB(255, 237, 208)};
        CString values[3];
        values[0].Format(L"Tag %llu", static_cast<unsigned long long>(state.tag));
        values[1].Format(L"Set %llu", static_cast<unsigned long long>(state.setIndex));
        values[2].Format(L"Offset %llu", static_cast<unsigned long long>(state.offset));
        const int left = bounds.left + 6;
        const int width = static_cast<int>(bounds.right - bounds.left - 12);
        const int segmentWidth = width / 3;
        for (int index = 0; index < 3; ++index) {
            CRect segment(
                left + index * segmentWidth,
                top + 21,
                index == 2 ? bounds.right - 6 : left + (index + 1) * segmentWidth - 2,
                top + rowHeight - 3);
            dc.FillSolidRect(segment, fills[index]);
            dc.Draw3dRect(segment, RGB(185, 190, 198), RGB(185, 190, 198));
            dc.SetTextColor(RGB(55, 55, 55));
            dc.DrawText(values[index], &segment, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
        }
    };

    drawLevel(L"L1", l1State, rowTop);
    drawLevel(L"L2", l2State, rowTop + rowHeight);
}

void CB5CacheVisualizerDlg::DrawAccessPath(CDC& dc, const CRect& bounds) const {
    dc.SetBkMode(TRANSPARENT);
    CRect title(bounds.left + 5, bounds.top + 2, bounds.right - 5, bounds.top + 18);

    CString requestText = L"Access Path";
    if (hasLastResult_) {
        requestText.Format(
            L"Access Path  %s 0x%llX",
            lastResult_.request.isWrite ? L"W" : L"R",
            static_cast<unsigned long long>(lastResult_.request.address));
    }
    dc.SetTextColor(RGB(45, 45, 45));
    dc.DrawText(requestText, &title, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    const int margin = 5;
    const int gap = 7;
    const int availableWidth = static_cast<int>(bounds.Width()) - margin * 2 - gap * 3;
    const int nodeWidth = availableWidth / 4;
    const int nodeTop = title.bottom + 1;
    const int nodeBottom = (std::min)(nodeTop + 23, static_cast<int>(bounds.bottom - 17));
    CRect nodes[4];
    for (int index = 0; index < 4; ++index) {
        const int left = bounds.left + margin + index * (nodeWidth + gap);
        nodes[index] = CRect(left, nodeTop, left + nodeWidth, nodeBottom);
    }

    bool activeLinks[3] = {false, false, false};
    if (hasLastResult_) {
        activeLinks[0] = true;
        activeLinks[1] = lastResult_.outcome != b5cache::AccessOutcome::L1Hit;
        activeLinks[2] = lastResult_.outcome == b5cache::AccessOutcome::MemoryMiss;
    }
    for (int index = 0; index < 3; ++index) {
        CPen arrowPen(PS_SOLID, activeLinks[index] ? 2 : 1,
                      activeLinks[index] ? RGB(65, 125, 220) : RGB(180, 184, 190));
        CPen* oldPen = dc.SelectObject(&arrowPen);
        const int y = (nodes[index].top + nodes[index].bottom) / 2;
        dc.MoveTo(nodes[index].right + 1, y);
        dc.LineTo(nodes[index + 1].left - 1, y);
        dc.MoveTo(nodes[index + 1].left - 5, y - 3);
        dc.LineTo(nodes[index + 1].left - 1, y);
        dc.LineTo(nodes[index + 1].left - 5, y + 3);
        dc.SelectObject(oldPen);
    }

    CString labels[4] = {L"CPU", L"L1 Ready", L"L2 Ready", L"Memory"};
    COLORREF fills[4] = {
        RGB(225, 238, 255),
        RGB(235, 237, 240),
        RGB(235, 237, 240),
        RGB(235, 237, 240)};

    if (hasLastResult_) {
        labels[0] = lastResult_.request.isWrite ? L"CPU Write" : L"CPU Read";
        switch (lastResult_.outcome) {
        case b5cache::AccessOutcome::L1Hit:
            labels[1] = L"L1 Hit";
            labels[2] = L"L2 Skip";
            labels[3] = L"Mem Skip";
            fills[1] = RGB(216, 244, 225);
            break;
        case b5cache::AccessOutcome::L2Hit:
            labels[1] = L"L1 Miss";
            labels[2] = L"L2 Hit";
            labels[3] = L"Mem Skip";
            fills[1] = RGB(255, 226, 220);
            fills[2] = RGB(216, 244, 225);
            break;
        case b5cache::AccessOutcome::MemoryMiss:
            labels[1] = L"L1 Miss";
            labels[2] = L"L2 Miss";
            labels[3] = L"Mem Fetch";
            fills[1] = RGB(255, 226, 220);
            fills[2] = RGB(255, 226, 220);
            fills[3] = RGB(220, 237, 255);
            break;
        }
    }

    for (int index = 0; index < 4; ++index) {
        dc.FillSolidRect(nodes[index], fills[index]);
        dc.Draw3dRect(nodes[index], RGB(155, 160, 168), RGB(155, 160, 168));
        dc.SetTextColor(RGB(45, 45, 45));
        dc.DrawText(labels[index], &nodes[index], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    }

    CString action = L"Step or Run All to visualize one access.";
    COLORREF actionColor = RGB(110, 110, 110);
    if (hasLastResult_) {
        switch (lastResult_.outcome) {
        case b5cache::AccessOutcome::L1Hit:
            action = L"Finish in L1";
            actionColor = RGB(24, 120, 65);
            break;
        case b5cache::AccessOutcome::L2Hit:
            action = L"L2 -> Fill L1";
            actionColor = RGB(35, 93, 170);
            break;
        case b5cache::AccessOutcome::MemoryMiss:
            action = L"Memory -> Fill L2 -> Fill L1";
            actionColor = RGB(35, 93, 170);
            break;
        }
        if (lastResult_.l1.evicted || lastResult_.l2.evicted) {
            action += L" | Eviction";
            actionColor = RGB(170, 63, 37);
        }
        if (lastResult_.request.isWrite) {
            action += L" | Dirty";
            actionColor = RGB(178, 92, 0);
        }
    }
    CRect actionRect(bounds.left + 5, nodeBottom + 1, bounds.right - 5, bounds.bottom - 1);
    dc.SetTextColor(actionColor);
    dc.DrawText(action, &actionRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
}

void CB5CacheVisualizerDlg::OnDrawItem(const int controlId, LPDRAWITEMSTRUCT drawItem) {
    if (controlId != IDC_STATIC_OUTCOME_CHART &&
        controlId != IDC_STATIC_RATE_CHART &&
        controlId != IDC_STATIC_ADDRESS_VIEW &&
        controlId != IDC_STATIC_ACCESS_PATH) {
        CDialogEx::OnDrawItem(controlId, drawItem);
        return;
    }

    CDC dc;
    dc.Attach(drawItem->hDC);
    const CRect bounds(drawItem->rcItem);
    dc.FillSolidRect(bounds, RGB(250, 251, 253));

    CWnd* chart = GetDlgItem(controlId);
    CFont* oldFont = nullptr;
    if (chart != nullptr && chart->GetFont() != nullptr) {
        oldFont = dc.SelectObject(chart->GetFont());
    }

    if (controlId == IDC_STATIC_OUTCOME_CHART) {
        DrawOutcomeChart(dc, bounds);
    } else if (controlId == IDC_STATIC_RATE_CHART) {
        DrawHitRateChart(dc, bounds);
    } else if (controlId == IDC_STATIC_ADDRESS_VIEW) {
        DrawAddressBreakdown(dc, bounds);
    } else {
        DrawAccessPath(dc, bounds);
    }

    if (oldFont != nullptr) {
        dc.SelectObject(oldFont);
    }
    dc.Detach();
}

void CB5CacheVisualizerDlg::RefreshTraceStatus() {
    const auto total = trace_.size();
    CString summary;
    summary.Format(L"Current: %llu / %llu", static_cast<unsigned long long>(nextIndex_), static_cast<unsigned long long>(total));
    SetDlgItemText(IDC_STATIC_TRACE_INDEX, summary);
}

void CB5CacheVisualizerDlg::RefreshLastResult(const b5cache::AccessResult* result) {
    if (result == nullptr) {
        SetDlgItemText(IDC_STATIC_LAST_RESULT, L"Last: none");
        SetDlgItemText(IDC_STATIC_ACCESS_PATH, L"Access Path: no request");
        SetDlgItemText(IDC_STATIC_ADDRESS_VIEW, L"Address Breakdown: no request");
        RefreshAccessVisuals();
        return;
    }

    std::wstringstream summary;
    switch (result->outcome) {
    case b5cache::AccessOutcome::L1Hit:
        summary << L"Last: L1 Hit | L2 skipped";
        break;
    case b5cache::AccessOutcome::L2Hit:
        summary << L"Last: L2 Hit | Fill L1";
        break;
    case b5cache::AccessOutcome::MemoryMiss:
        summary << L"Last: Memory Miss | Fill L2 + L1";
        if (result->l1.evicted || result->l2.evicted) {
            summary << L" | Eviction";
        }
        break;
    }
    if (result->request.isWrite) {
        summary << L" | Dirty";
    }
    SetDlgItemText(IDC_STATIC_LAST_RESULT, summary.str().c_str());

    std::wstringstream accessiblePath;
    accessiblePath << L"Access Path: " << OutcomeText(result->outcome);
    if (result->outcome == b5cache::AccessOutcome::L2Hit) {
        accessiblePath << L", fill L1";
    } else if (result->outcome == b5cache::AccessOutcome::MemoryMiss) {
        accessiblePath << L", memory fills L2 then L1";
    }
    SetDlgItemText(IDC_STATIC_ACCESS_PATH, accessiblePath.str().c_str());

    std::wstringstream accessibleAddress;
    accessibleAddress << L"Address Breakdown for 0x" << std::hex << std::uppercase
                      << result->request.address;
    SetDlgItemText(IDC_STATIC_ADDRESS_VIEW, accessibleAddress.str().c_str());
    RefreshAccessVisuals();
}

void CB5CacheVisualizerDlg::ShowUserError(const std::string& message) const {
    const CA2W wideMessage(message.c_str(), CP_UTF8);
    AfxMessageBox(wideMessage, MB_OK | MB_ICONERROR);
}

void CB5CacheVisualizerDlg::ClearCacheView(CListCtrl& view) const {
    view.DeleteAllItems();
}

void CB5CacheVisualizerDlg::SetupCacheViewColumns(CListCtrl& view) const {
    if (view.GetHeaderCtrl() != nullptr && view.GetHeaderCtrl()->GetItemCount() > 0) {
        return;
    }

    view.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    view.InsertColumn(0, L"Set", LVCFMT_LEFT, 45);
    view.InsertColumn(1, L"Line", LVCFMT_LEFT, 40);
    view.InsertColumn(2, L"Valid", LVCFMT_LEFT, 45);
    view.InsertColumn(3, L"Dirty", LVCFMT_LEFT, 45);
    view.InsertColumn(4, L"Tag", LVCFMT_LEFT, 55);
    view.InsertColumn(5, L"Block", LVCFMT_LEFT, 60);
    view.InsertColumn(6, L"Inserted", LVCFMT_LEFT, 65);
    view.InsertColumn(7, L"Last Used", LVCFMT_LEFT, 65);
    view.InsertColumn(8, L"State", LVCFMT_LEFT, 85);
}

void CB5CacheVisualizerDlg::OnCustomDrawCacheView(NMHDR* notification, LRESULT* result) {
    auto* draw = reinterpret_cast<NMLVCUSTOMDRAW*>(notification);
    *result = CDRF_DODEFAULT;
    if (draw->nmcd.dwDrawStage == CDDS_PREPAINT) {
        *result = CDRF_NOTIFYITEMDRAW;
        return;
    }
    if (draw->nmcd.dwDrawStage != (CDDS_ITEMPREPAINT)) {
        return;
    }

    const auto itemIndex = static_cast<int>(draw->nmcd.dwItemSpec);
    CListCtrl* view = draw->nmcd.hdr.hwndFrom == l1CacheView_.GetSafeHwnd() ? &l1CacheView_ : &l2CacheView_;
    const CString state = view->GetItemText(itemIndex, 8);
    if (state == L"L1 Hit" || state == L"L2 Hit") {
        draw->clrText = RGB(0, 100, 0);
        draw->clrTextBk = RGB(220, 245, 220);
    } else if (state == L"New") {
        draw->clrText = RGB(0, 70, 150);
        draw->clrTextBk = RGB(220, 235, 255);
    } else if (state == L"Replaced -> New") {
        draw->clrText = RGB(150, 60, 0);
        draw->clrTextBk = RGB(255, 225, 190);
    }
}

void CB5CacheVisualizerDlg::OnApplyConfig() {
    b5cache::SimulationConfig config;
    if (!ReadConfiguration(config)) {
        return;
    }

    try {
        std::vector<b5cache::MemoryAccess> parsedTrace;
        if (!ParseTraceFromEditor(parsedTrace)) {
            return;
        }
        auto newSimulator = std::make_unique<b5cache::CacheSimulator>(config);
        simulator_ = std::move(newSimulator);
        trace_ = std::move(parsedTrace);
        ResetSession();
    } catch (const std::exception& error) {
        ShowUserError(error.what());
    }
}

void CB5CacheVisualizerDlg::OnImportTrace() {
    CFileDialog fileDialog(TRUE, L"txt", nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, L"Text Files (*.txt;*.trace)|*.txt;*.trace||");
    if (fileDialog.DoModal() != IDOK) {
        return;
    }

    try {
        const std::filesystem::path path(fileDialog.GetPathName().GetString());
        const auto importedTrace = b5cache::MemoryTraceParser::ParseFile(path);
        std::wstringstream traceText;
        for (const auto& access : importedTrace) {
            traceText << ToText(access) << L"\r\n";
        }
        SetDlgItemText(IDC_EDIT_TRACE, traceText.str().c_str());
        trace_ = importedTrace;
        ResetSession();
    } catch (const std::exception& error) {
        ShowUserError(error.what());
    }
}

void CB5CacheVisualizerDlg::OnClearTrace() {
    SetDlgItemText(IDC_EDIT_TRACE, L"");
    trace_.clear();
    ResetSession();
}

void CB5CacheVisualizerDlg::OnStepTrace() {
    if (simulator_ == nullptr) {
        ShowUserError("No simulator is available.");
        return;
    }

    try {
        if (!LoadTraceFromEditor(trace_)) {
            return;
        }
        ExecuteNextAccess();
    } catch (const std::exception& error) {
        ShowUserError(error.what());
    }
}

void CB5CacheVisualizerDlg::OnRunAllTrace() {
    if (simulator_ == nullptr) {
        ShowUserError("No simulator is available.");
        return;
    }

    try {
        if (!LoadTraceFromEditor(trace_)) {
            return;
        }

        if (trace_.empty()) {
            ShowUserError("Trace is empty. Add at least one access before running.");
            return;
        }

        while (nextIndex_ < trace_.size()) {
            lastResult_ = simulator_->Access(trace_[nextIndex_]);
            hasLastResult_ = true;
            ++nextIndex_;
            RecordStatisticsPoint();
            RefreshCacheViews(&lastResult_);
            RefreshStatistics();
        }

        RefreshTraceStatus();
        if (hasLastResult_) {
            RefreshLastResult(&lastResult_);
        }
    } catch (const std::exception& error) {
        ShowUserError(error.what());
    }
}

void CB5CacheVisualizerDlg::OnResetSimulation() {
    try {
        if (!LoadTraceFromEditor(trace_)) {
            return;
        }
        ResetSession();
    } catch (const std::exception& error) {
        ShowUserError(error.what());
    }
}
