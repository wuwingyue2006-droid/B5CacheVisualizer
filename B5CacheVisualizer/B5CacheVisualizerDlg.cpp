#include "pch.h"

#include "B5CacheVisualizer.h"
#include "B5CacheVisualizerDlg.h"
#include "trace/MemoryTraceParser.h"

#include <atlconv.h>
#include <iomanip>
#include <sstream>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CB5CacheVisualizerDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CB5CacheVisualizerDlg::OnRunTrace)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CB5CacheVisualizerDlg::OnResetSimulation)
END_MESSAGE_MAP()

CB5CacheVisualizerDlg::CB5CacheVisualizerDlg(CWnd* parent)
    : CDialogEx(IDD_B5CACHEVISUALIZER_DIALOG, parent) {}

void CB5CacheVisualizerDlg::DoDataExchange(CDataExchange* dataExchange) {
    CDialogEx::DoDataExchange(dataExchange);
}

BOOL CB5CacheVisualizerDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // TODO(D): replace the baseline summary dialog with configuration controls, step mode and cache grids.
    SetDlgItemText(
        IDC_EDIT_TRACE,
        L"# R/W + decimal or hexadecimal address\r\n"
        L"R 0x00\r\n"
        L"R 0x00\r\n"
        L"W 0x40\r\n"
        L"R 0x00");
    ShowSummary();
    return TRUE;
}

void CB5CacheVisualizerDlg::OnRunTrace() {
    CString traceText;
    GetDlgItemText(IDC_EDIT_TRACE, traceText);

    try {
        const CW2A narrowText(traceText, CP_UTF8);
        const auto accesses = b5cache::MemoryTraceParser::ParseText(narrowText.m_psz);
        simulator_.Reset();
        simulator_.Run(accesses);
        ShowSummary();
    } catch (const std::exception& error) {
        const CA2W wideMessage(error.what(), CP_UTF8);
        AfxMessageBox(wideMessage, MB_OK | MB_ICONERROR);
    }
}

void CB5CacheVisualizerDlg::OnResetSimulation() {
    simulator_.Reset();
    ShowSummary();
}

void CB5CacheVisualizerDlg::ShowSummary() {
    const auto snapshot = simulator_.Statistics();
    std::ostringstream output;
    output << "Framework status: Direct Mapping + FIFO baseline is available.\r\n"
           << "Fully/Set Associative, LRU and visual cache grids are assigned TODO modules.\r\n\r\n"
           << "Accesses: " << snapshot.accesses << "\r\n"
           << "Reads: " << snapshot.reads << "\r\n"
           << "Writes: " << snapshot.writes << "\r\n"
           << "L1 Hits: " << snapshot.l1Hits << "\r\n"
           << "L2 Hits: " << snapshot.l2Hits << "\r\n"
           << "Memory Misses: " << snapshot.memoryMisses << "\r\n"
           << std::fixed << std::setprecision(2)
           << "Overall Hit Rate: " << snapshot.OverallHitRate() * 100.0 << "%";

    const CA2W wideOutput(output.str().c_str(), CP_UTF8);
    SetDlgItemText(IDC_EDIT_RESULT, wideOutput);

    CString summary;
    summary.Format(L"Current requests: %llu", static_cast<unsigned long long>(snapshot.accesses));
    SetDlgItemText(IDC_STATIC_SUMMARY, summary);
}
