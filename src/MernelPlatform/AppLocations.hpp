/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include "MernelPlatformExport.hpp"

#include "FsUtils.hpp"

namespace Mernel {

class MERNELPLATFORM_EXPORT AppLocations {
public:
    AppLocations(std::string orgName, std::string appName = {});

    std_path getTempDir() const;
    std_path getBinDir() const;
    std_path getHomeDir() const;
    std_path getAppdataDir() const;

private:
    const std::string m_orgName;
    const std::string m_appName;
    mutable std_path  m_tempDir;
    mutable std_path  m_exePath;
    mutable std_path  m_homeDir;
    mutable std_path  m_appdataDir;
};

}
