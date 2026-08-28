#include "pch.h"

#include "B5CacheVisualizer.h"
#include "B5CacheVisualizerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CB5CacheVisualizerApp, CWinApp)
END_MESSAGE_MAP()

CB5CacheVisualizerApp theApp;

BOOL CB5CacheVisualizerApp::InitInstance() {
    CWinApp::InitInstance();

    CB5CacheVisualizerDlg dialog;
    m_pMainWnd = &dialog;
    dialog.DoModal();
    return FALSE;
}
