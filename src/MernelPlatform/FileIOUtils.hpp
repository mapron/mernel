/*
 * Copyright (C) 2022 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include "MernelPlatformExport.hpp"
#include "FsUtils.hpp"
#include "ByteBuffer.hpp"

#include <filesystem>

namespace Mernel {

MERNELPLATFORM_EXPORT bool readFileIntoBufferNoexcept(const std_path& filename, std::string& buffer) noexcept(true);
MERNELPLATFORM_EXPORT bool writeFileFromBufferNoexcept(const std_path& filename, const std::string& buffer) noexcept(true);

MERNELPLATFORM_EXPORT bool readFileIntoHolderNoexcept(const std_path& filename, ByteArrayHolder& holder) noexcept(true);
MERNELPLATFORM_EXPORT bool writeFileFromHolderNoexcept(const std_path& filename, const ByteArrayHolder& holder) noexcept(true);

MERNELPLATFORM_EXPORT std::string readFileIntoBuffer(const std_path& filename) noexcept(false);
MERNELPLATFORM_EXPORT void        writeFileFromBuffer(const std_path& filename, const std::string& buffer) noexcept(false);

MERNELPLATFORM_EXPORT ByteArrayHolder readFileIntoHolder(const std_path& filename) noexcept(false);
MERNELPLATFORM_EXPORT void            writeFileFromHolder(const std_path& filename, const ByteArrayHolder& holder) noexcept(false);
}
