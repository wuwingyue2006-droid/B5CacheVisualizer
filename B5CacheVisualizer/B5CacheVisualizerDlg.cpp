#include "pch.h"

#include "B5CacheVisualizer.h"
#include "B5CacheVisualizerDlg.h"
#include "trace/MemoryTraceParser.h"

#include <atlconv.h>
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

}  // namespace

BEGIN_MESSAGE_MAP(CB5CacheVisualizerDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CB5CacheVisualizerDlg::OnApplyConfig)
    ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CB5CacheVisualizerDlg::OnImportTrace)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_TRACE, &CB5CacheVisualizerDlg::OnClearTrace)
    ON_BN_CLICKED(IDC_BUTTON_STEP, &CB5CacheVisualizerDlg::OnStepTrace)
    ON_BN_CLICKED(IDC_BUTTON_RUN_ALL, &CB5CacheVisualizerDlg::OnRunAllTrace)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CB5CacheVisualizerDlg::OnResetSimulation)
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
        L"# R/W + decimal or hexadecimal address\r\n"
        L"R 0x00\r\n"
        L"R 0x10\r\n"
        L"W 0x20\r\n"
        L"R 0x10\r\n"
        L"R 0x40");

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

bool CB5CacheVisualizerDlg::LoadTraceFromEditor(std::vector<b5cache::MemoryAccess>& accesses) const {
    CString traceText;
    GetDlgItemText(IDC_EDIT_TRACE, traceText);

    try {
        const CW2A narrowText(traceText, CP_UTF8);
        accesses = b5cache::MemoryTraceParser::ParseText(narrowText.m_psz);
        return true;
    } catch (const std::exception& error) {
        ShowUserError(error.what());
        return false;
    }
}

void CB5CacheVisualizerDlg::ResetSession() {
    if (simulator_ == nullptr) {
        simulator_ = std::make_unique<b5cache::CacheSimulator>(b5cache::CacheSimulator::DefaultConfig());
    }

    simulator_->Reset();
    nextIndex_ = 0;
    hasLastResult_ = false;
    lastResult_ = {};
    RefreshTraceStatus();
    RefreshStatistics();
    RefreshCacheViews();
    SetDlgItemText(IDC_STATIC_LAST_RESULT, L"Last Result: none");
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
    RefreshTraceStatus();
    RefreshStatistics();
    RefreshCacheViews(&lastResult_);

    std::wstringstream summary;
    summary << L"Last Result: " << OutcomeText(lastResult_.outcome)
            << L" | L1 set " << lastResult_.l1.setIndex << L" line " << lastResult_.l1.lineIndex
            << L" | L2 set " << lastResult_.l2.setIndex << L" line " << lastResult_.l2.lineIndex;
    SetDlgItemText(IDC_STATIC_LAST_RESULT, summary.str().c_str());
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
                        if (latest->outcome == b5cache::AccessOutcome::L1Hit && latest->l1.setIndex == setIndex && latest->l1.lineIndex == lineIndex) {
                            state = L"L1 Hit";
                        } else if (latest->l1.setIndex == setIndex && latest->l1.lineIndex == lineIndex && !latest->l1.hit) {
                            state = L"New";
                        } else if (latest->l1.evicted && latest->l1.setIndex == setIndex && latest->l1.lineIndex == lineIndex) {
                            state = L"Replaced";
                        }
                    } else {
                        if (latest->outcome == b5cache::AccessOutcome::L2Hit && latest->l2.setIndex == setIndex && latest->l2.lineIndex == lineIndex) {
                            state = L"L2 Hit";
                        } else if (latest->l2.setIndex == setIndex && latest->l2.lineIndex == lineIndex && !latest->l2.hit) {
                            state = L"New";
                        } else if (latest->l2.evicted && latest->l2.setIndex == setIndex && latest->l2.lineIndex == lineIndex) {
                            state = L"Replaced";
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

                if (state != L"Idle") {
                    view.SetItemState(itemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                }
            }
        }
    };

    renderLevel(l1CacheView_, simulator_->L1(), true);
    renderLevel(l2CacheView_, simulator_->L2(), false);
}

void CB5CacheVisualizerDlg::RefreshStatistics() {
    if (simulator_ == nullptr) {
        SetDlgItemText(IDC_EDIT_RESULT, L"No simulator available.");
        SetDlgItemText(IDC_STATIC_SUMMARY, L"Current requests: 0");
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

    CString summary;
    summary.Format(L"Current requests: %llu", static_cast<unsigned long long>(snapshot.accesses));
    SetDlgItemText(IDC_STATIC_SUMMARY, summary);
}

void CB5CacheVisualizerDlg::RefreshTraceStatus() {
    const auto total = trace_.size();
    CString summary;
    summary.Format(L"Current: %llu / %llu", static_cast<unsigned long long>(nextIndex_), static_cast<unsigned long long>(total));
    SetDlgItemText(IDC_STATIC_TRACE_INDEX, summary);
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
    view.InsertColumn(0, L"Set", LVCFMT_LEFT, 55);
    view.InsertColumn(1, L"Line", LVCFMT_LEFT, 50);
    view.InsertColumn(2, L"Valid", LVCFMT_LEFT, 60);
    view.InsertColumn(3, L"Dirty", LVCFMT_LEFT, 60);
    view.InsertColumn(4, L"Tag", LVCFMT_LEFT, 80);
    view.InsertColumn(5, L"Block", LVCFMT_LEFT, 90);
    view.InsertColumn(6, L"Inserted At", LVCFMT_LEFT, 90);
    view.InsertColumn(7, L"Last Used At", LVCFMT_LEFT, 90);
    view.InsertColumn(8, L"State", LVCFMT_LEFT, 80);
}

void CB5CacheVisualizerDlg::OnApplyConfig() {
    b5cache::SimulationConfig config;
    if (!ReadConfiguration(config)) {
        return;
    }

    try {
        simulator_ = std::make_unique<b5cache::CacheSimulator>(config);
        if (!LoadTraceFromEditor(trace_)) {
            return;
        }
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
        const CStringA narrowPath(CW2A(fileDialog.GetPathName(), CP_UTF8));
        std::ifstream input(narrowPath, std::ios::in | std::ios::binary);
        if (!input) {
            ShowUserError("Unable to open trace file.");
            return;
        }

        std::ostringstream stream;
        stream << input.rdbuf();
        if (input.bad()) {
            ShowUserError("Unable to read trace file.");
            return;
        }

        const std::string text = stream.str();
        const CA2W wideText(text.c_str(), CP_UTF8);
        SetDlgItemText(IDC_EDIT_TRACE, wideText);

        if (!LoadTraceFromEditor(trace_)) {
            return;
        }
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

        while (nextIndex_ < trace_.size()) {
            lastResult_ = simulator_->Access(trace_[nextIndex_]);
            hasLastResult_ = true;
            ++nextIndex_;
            RefreshCacheViews(&lastResult_);
            RefreshStatistics();
        }

        RefreshTraceStatus();
        std::wstringstream summary;
        summary << L"Last Result: " << OutcomeText(lastResult_.outcome)
                << L" | L1 set " << lastResult_.l1.setIndex << L" line " << lastResult_.l1.lineIndex
                << L" | L2 set " << lastResult_.l2.setIndex << L" line " << lastResult_.l2.lineIndex;
        SetDlgItemText(IDC_STATIC_LAST_RESULT, summary.str().c_str());
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
