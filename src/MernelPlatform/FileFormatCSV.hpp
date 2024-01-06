/*
 * Copyright (C) 2024 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include "FileFormatCSVTable.hpp"

#include "MernelPlatformExport.hpp"

namespace Mernel {

MERNELPLATFORM_EXPORT bool writeCSVToBuffer(std::string& csvData, const CSVTable& table);
MERNELPLATFORM_EXPORT bool readCSVFromBuffer(const std::string& csvData, CSVTable& table);

}
