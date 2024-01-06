/*
 * Copyright (C) 2024 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */

#include "FileFormatCSVTable.hpp"

#include "StringUtils.hpp"

namespace Mernel {

int CSVTableCell::toInt() const
{
    return std::atoi(str.c_str());
}

void CSVTableCell::setInt(int value)
{
    str = std::to_string(value);
}

std::string CSVTableCell::toLower() const
{
    return strToLower(str);
}

bool CSVTableCell::startsWith(const std::string& s) const
{
    return str.starts_with(s);
}

bool CSVTableCell::endsWith(const std::string& s) const
{
    return str.ends_with(s);
}

bool CSVTableCell::contains(const std::string& s) const
{
    return str.find(s) != std::string::npos;
}

int CSVTable::indexOf(const std::string& col) const
{
    auto it = std::find(columns.cbegin(), columns.cend(), col);
    if (it == columns.cend())
        return -1;
    return it - columns.cbegin();
}

}
