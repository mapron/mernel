/*
 * Copyright (C) 2026 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: BSL-1.0
 * See LICENSE file for details.
 */
#pragma once

#include <algorithm>
#include <filesystem>

namespace Mernel {

namespace std_fs = std::filesystem;
using std_path   = std_fs::path;

inline std::string path2string(const std_path& path)
{
    auto        str = path.u8string();
    std::string result;
    result.resize(str.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char>(str[i]);
    }
    return result;
}

inline std_path string2path(const std::string& str)
{
    std::u8string result;
    result.resize(str.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char8_t>(str[i]);
    }
    return result;
}

inline std::string pathToLower(const std_path& path)
{
    std::string str = path2string(path);
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

inline bool isExistingDirectory(const std_path& path)
{
    std::error_code ec;
    return std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec);
}

inline bool isExistingFile(const std_path& path)
{
    std::error_code ec;
    return std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec);
}

}
