#pragma once

#include "common/CacheTypes.h"

#include <filesystem>
#include <string>
#include <vector>

namespace b5cache {

class MemoryTraceParser {
public:
    static std::vector<MemoryAccess> ParseText(const std::string& text);
    static std::vector<MemoryAccess> ParseFile(const std::filesystem::path& path);
};

}  // namespace b5cache
