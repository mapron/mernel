/*
 * Copyright (C) 2022 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include "PropertyTree.hpp"

#include "MernelPlatformExport.hpp"

namespace Mernel {

/// @todo: rewrite noexcept version as wrappers over throwing.

MERNELPLATFORM_EXPORT bool readJsonFromBuffer(const std::string& buffer, PropertyTree& data) noexcept(true);
MERNELPLATFORM_EXPORT bool writeJsonToBuffer(std::string& buffer, const PropertyTree& data, bool pretty = false) noexcept(true);

MERNELPLATFORM_EXPORT PropertyTree readJsonFromBufferThrow(const std::string& buffer) noexcept(false);
MERNELPLATFORM_EXPORT std::string writeJsonToBufferThrow(const PropertyTree& data, bool pretty = false) noexcept(false);

}
