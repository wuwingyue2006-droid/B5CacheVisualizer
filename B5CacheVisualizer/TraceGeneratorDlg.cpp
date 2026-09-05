#include "pch.h"

#include "TraceGeneratorDlg.h"

#include <atlconv.h>
#include <cerrno>
#include <limits>
#include <stdexcept>
#include <string>

BEGIN_MESSAGE_MAP(CTraceGeneratorDlg, CDialogEx)
    ON_CBN_SELCHANGE(IDC_COMBO_GENERATOR_MODE, &CTraceGeneratorDlg::OnModeChanged)
    ON_CBN_SELCHANGE(IDC_COMBO_GENERATOR_PRESET, &CTraceGeneratorDlg::OnPresetChanged)
END_MESSAGE_MAP()

CTraceGeneratorDlg::CTraceGeneratorDlg(CWnd* parent)
    : CDialogEx(IDD_TRACE_GENERATOR_DIALOG, parent) {}

const std::vector<b5cache::MemoryAccess>& CTraceGeneratorDlg::GeneratedTrace() const noexcept {
    return generatedTrace_;
}

BOOL CTraceGeneratorDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    auto* mode = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_GENERATOR_MODE));
    mode->AddString(L"Sequential");
    mode->AddString(L"Loop");
    mode->AddString(L"Random");
    mode->AddString(L"Hot Set");
    mode->AddString(L"Mixed R/W");
    mode->SetCurSel(0);

    auto* preset = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_GENERATOR_PRESET));
    preset->AddString(L"Custom");
    preset->AddString(L"Sequential locality");
    preset->AddString(L"Loop working set");
    preset->AddString(L"Hot access");
    preset->AddString(L"Mixed reads/writes");
    preset->SetCurSel(0);

    SetDlgItemText(IDC_EDIT_GENERATOR_COUNT, L"32");
    SetDlgItemText(IDC_EDIT_GENERATOR_START, L"0x0");
    SetDlgItemText(IDC_EDIT_GENERATOR_RANGE, L"512");
    SetDlgItemText(IDC_EDIT_GENERATOR_STEP, L"16");
    SetDlgItemText(IDC_EDIT_GENERATOR_LOOP, L"4");
    SetDlgItemText(IDC_EDIT_GENERATOR_HOT_COUNT, L"2");
    SetDlgItemText(IDC_EDIT_GENERATOR_HOT_PERCENT, L"80");
    SetDlgItemText(IDC_EDIT_GENERATOR_WRITE_PERCENT, L"0");
    SetDlgItemText(IDC_EDIT_GENERATOR_SEED, L"20260903");
    UpdateControlStates();
    return TRUE;
}

b5cache::TraceGenerationConfig CTraceGeneratorDlg::ReadConfig() const {
    auto readUnsigned = [&](const int controlId, const char* fieldName) -> std::uint64_t {
        CString text;
        GetDlgItemText(controlId, text);
        text.Trim();
        if (text.IsEmpty() || text[0] == L'+' || text[0] == L'-') {
            throw std::invalid_argument(std::string(fieldName) + " must be a non-negative integer.");
        }

        wchar_t* endPointer = nullptr;
        errno = 0;
        const auto parsed = std::wcstoull(text.GetString(), &endPointer, 0);
        if (errno != 0 || endPointer == text.GetString() || *endPointer != L'\0') {
            throw std::invalid_argument(
                std::string(fieldName) + " must be a valid decimal or 0x hexadecimal integer.");
        }
        return parsed;
    };
    auto readSize = [&](const int controlId, const char* fieldName) -> std::size_t {
        const auto parsed = readUnsigned(controlId, fieldName);
        if (parsed > static_cast<std::uint64_t>((std::numeric_limits<std::size_t>::max)())) {
            throw std::invalid_argument(std::string(fieldName) + " is too large for this computer.");
        }
        return static_cast<std::size_t>(parsed);
    };
    auto readPercent = [&](const int controlId, const char* fieldName) -> double {
        CString text;
        GetDlgItemText(controlId, text);
        text.Trim();
        wchar_t* endPointer = nullptr;
        errno = 0;
        const auto parsed = std::wcstod(text.GetString(), &endPointer);
        if (text.IsEmpty() || errno != 0 || endPointer == text.GetString() || *endPointer != L'\0' ||
            parsed < 0.0 || parsed > 100.0) {
            throw std::invalid_argument(std::string(fieldName) + " must be a percentage between 0 and 100.");
        }
        return parsed / 100.0;
    };

    b5cache::TraceGenerationConfig config;
    const auto selection = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_GENERATOR_MODE))->GetCurSel();
    switch (selection) {
    case 0:
        config.mode = b5cache::TraceGenerationMode::Sequential;
        break;
    case 1:
        config.mode = b5cache::TraceGenerationMode::Loop;
        break;
    case 2:
        config.mode = b5cache::TraceGenerationMode::Random;
        break;
    case 3:
        config.mode = b5cache::TraceGenerationMode::HotSet;
        break;
    case 4:
        config.mode = b5cache::TraceGenerationMode::MixedReadWrite;
        break;
    default:
        throw std::invalid_argument("Select a trace generation mode.");
    }

    config.requestCount = readSize(IDC_EDIT_GENERATOR_COUNT, "Request count");
    config.startAddress = readUnsigned(IDC_EDIT_GENERATOR_START, "Start address");
    config.addressRangeBytes = readUnsigned(IDC_EDIT_GENERATOR_RANGE, "Address range");
    config.stepBytes = readUnsigned(IDC_EDIT_GENERATOR_STEP, "Step");

    if (config.mode == b5cache::TraceGenerationMode::Loop) {
        config.loopLength = readSize(IDC_EDIT_GENERATOR_LOOP, "Loop length");
    }
    if (config.mode == b5cache::TraceGenerationMode::HotSet) {
        config.hotSetSize = readSize(IDC_EDIT_GENERATOR_HOT_COUNT, "Hot-set size");
        config.hotProbability = readPercent(IDC_EDIT_GENERATOR_HOT_PERCENT, "Hot-set probability");
    }
    if (config.mode == b5cache::TraceGenerationMode::MixedReadWrite) {
        config.writeProbability = readPercent(IDC_EDIT_GENERATOR_WRITE_PERCENT, "Write probability");
    }
    if (config.mode == b5cache::TraceGenerationMode::Random ||
        config.mode == b5cache::TraceGenerationMode::HotSet ||
        config.mode == b5cache::TraceGenerationMode::MixedReadWrite) {
        config.randomSeed = readUnsigned(IDC_EDIT_GENERATOR_SEED, "Random seed");
    }
    return config;
}

void CTraceGeneratorDlg::OnOK() {
    try {
        generatedTrace_ = b5cache::TraceGenerator::Generate(ReadConfig());
        CDialogEx::OnOK();
    } catch (const std::exception& error) {
        const CA2W message(error.what(), CP_UTF8);
        AfxMessageBox(message, MB_OK | MB_ICONERROR);
    }
}

void CTraceGeneratorDlg::OnModeChanged() {
    if (auto* preset = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_GENERATOR_PRESET)); preset != nullptr) {
        preset->SetCurSel(0);
    }
    UpdateControlStates();
}

void CTraceGeneratorDlg::OnPresetChanged() {
    ApplyPreset();
}

void CTraceGeneratorDlg::UpdateControlStates() {
    const auto* mode = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_GENERATOR_MODE));
    const int selection = mode == nullptr ? -1 : mode->GetCurSel();
    const bool loopMode = selection == 1;
    const bool seededMode = selection == 2 || selection == 3 || selection == 4;
    const bool hotSetMode = selection == 3;
    const bool mixedMode = selection == 4;

    GetDlgItem(IDC_EDIT_GENERATOR_LOOP)->EnableWindow(loopMode);
    GetDlgItem(IDC_EDIT_GENERATOR_HOT_COUNT)->EnableWindow(hotSetMode);
    GetDlgItem(IDC_EDIT_GENERATOR_HOT_PERCENT)->EnableWindow(hotSetMode);
    GetDlgItem(IDC_EDIT_GENERATOR_WRITE_PERCENT)->EnableWindow(mixedMode);
    GetDlgItem(IDC_EDIT_GENERATOR_SEED)->EnableWindow(seededMode);
}

void CTraceGeneratorDlg::ApplyPreset() {
    auto* preset = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_GENERATOR_PRESET));
    auto* mode = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_GENERATOR_MODE));
    if (preset == nullptr || mode == nullptr || preset->GetCurSel() <= 0) {
        UpdateControlStates();
        return;
    }

    SetDlgItemText(IDC_EDIT_GENERATOR_START, L"0x0");
    SetDlgItemText(IDC_EDIT_GENERATOR_STEP, L"16");
    SetDlgItemText(IDC_EDIT_GENERATOR_LOOP, L"4");
    SetDlgItemText(IDC_EDIT_GENERATOR_HOT_COUNT, L"2");
    SetDlgItemText(IDC_EDIT_GENERATOR_HOT_PERCENT, L"80");
    SetDlgItemText(IDC_EDIT_GENERATOR_WRITE_PERCENT, L"0");
    SetDlgItemText(IDC_EDIT_GENERATOR_SEED, L"20260903");

    switch (preset->GetCurSel()) {
    case 1:
        mode->SetCurSel(0);
        SetDlgItemText(IDC_EDIT_GENERATOR_COUNT, L"32");
        SetDlgItemText(IDC_EDIT_GENERATOR_RANGE, L"512");
        break;
    case 2:
        mode->SetCurSel(1);
        SetDlgItemText(IDC_EDIT_GENERATOR_COUNT, L"48");
        SetDlgItemText(IDC_EDIT_GENERATOR_RANGE, L"64");
        break;
    case 3:
        mode->SetCurSel(3);
        SetDlgItemText(IDC_EDIT_GENERATOR_COUNT, L"64");
        SetDlgItemText(IDC_EDIT_GENERATOR_RANGE, L"256");
        SetDlgItemText(IDC_EDIT_GENERATOR_HOT_PERCENT, L"85");
        break;
    case 4:
        mode->SetCurSel(4);
        SetDlgItemText(IDC_EDIT_GENERATOR_COUNT, L"48");
        SetDlgItemText(IDC_EDIT_GENERATOR_RANGE, L"128");
        SetDlgItemText(IDC_EDIT_GENERATOR_WRITE_PERCENT, L"35");
        break;
    default:
        break;
    }
    UpdateControlStates();
}
