/*
 * Copyright (C) 2026 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: BSL-1.0
 * See LICENSE file for details.
 */
#pragma once

#include "FsUtils.hpp"

#include "MernelPlatformExport.hpp"

namespace Mernel {

class MERNELPLATFORM_EXPORT SharedLibLoader {
public:
    SharedLibLoader() = default;
    ~SharedLibLoader() { close(); }

    bool        open(const std_path& path);
    void        close();
    void*       getSymbol(const std::string& name) const;
    std::string getLastError() const;

private:
    void* m_handle = nullptr;
};

}
