#pragma once
#include <filesystem>
#include <string>

namespace ObSL {
    inline std::string canonicalize_module_path(const std::filesystem::path &root, const std::string &raw_path) {
        std::filesystem::path p = root / raw_path;
        return p.lexically_normal().generic_string();
    }
}
