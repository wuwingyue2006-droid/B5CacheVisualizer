#pragma once

#include "resource.h"

class CB5CacheVisualizerApp final : public CWinApp {
public:
    BOOL InitInstance() override;

    DECLARE_MESSAGE_MAP()
};

extern CB5CacheVisualizerApp theApp;
