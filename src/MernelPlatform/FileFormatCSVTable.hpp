/*
 * Copyright (C) 2024 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include <string>
#include <vector>
#include <deque>

#include "MernelPlatformExport.hpp"

namespace Mernel {

struct MERNELPLATFORM_EXPORT CSVTableCell {
    std::string str;

    CSVTableCell()                    = default;
    CSVTableCell(const CSVTableCell&) = default;
    CSVTableCell(std::string s)
        : str(std::move(s))
    {}

    int  toInt() const;
    void setInt(int value);
    bool isEmpty() const { return str.empty(); }
    void clear() { str.clear(); }

    std::string toLower() const;

    bool startsWith(const std::string& s) const;
    bool endsWith(const std::string& s) const;
    bool contains(const std::string& s) const;

    bool operator==(const CSVTableCell&) const = default;

    bool operator==(const std::string& s) const { return s == str; }
    bool operator!=(const std::string& s) const { return s != str; }
};

struct MERNELPLATFORM_EXPORT CSVTableRow {
    std::vector<CSVTableCell> data;
    CSVTableRow() = default;
    explicit CSVTableRow(int size)
    {
        data.resize(size);
    }
    explicit CSVTableRow(std::vector<CSVTableCell> data)
        : data(std::move(data))
    {
    }
    bool operator==(const CSVTableRow&) const = default;
};

struct MERNELPLATFORM_EXPORT CSVTable {
    std::deque<CSVTableRow>  rows;
    std::vector<std::string> columns;

    int indexOf(const std::string& col) const;
};

}
