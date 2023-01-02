/*
 * Copyright (C) 2022 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */

#pragma once
#include "ByteBuffer.hpp"
#include "MernelPlatformExport.hpp"

#include <vector>
#include <string>

namespace Mernel {

enum class CompressionType
{
    None,
    Gzip,
    Zlib, // only differs in window
    ZStd, // unsupported yet.
};

struct CompressionInfo {
    CompressionType m_type    = CompressionType::None;
    int             m_level   = 5;
    bool            m_skipCRC = false;
};

MERNELPLATFORM_EXPORT void uncompressDataBuffer(const ByteArrayHolder& input, ByteArrayHolder& output, CompressionInfo compressionInfo);
MERNELPLATFORM_EXPORT void compressDataBuffer(const ByteArrayHolder& input, ByteArrayHolder& output, CompressionInfo compressionInfo);

}
