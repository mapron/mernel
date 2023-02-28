/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#include "FileIOUtils.hpp"

#include <fstream>
#include <cstring>

namespace Mernel {

bool readFileIntoBufferNoexcept(const std_path& filename, std::string& buffer) noexcept(true)
{
    std::ifstream ifs((filename), std::ios_base::in | std::ios_base::binary);
    if (!ifs)
        return false;

    ifs.seekg(0, std::ios::end);

    buffer.resize(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    ifs.read(buffer.data(), buffer.size());
    return true;
}

bool writeFileFromBufferNoexcept(const std_path& filename, const std::string& buffer) noexcept(true)
{
    std::ofstream ofs((filename), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if (!ofs)
        return false;

    if (buffer.empty())
        return true;

    ofs.write(buffer.data(), buffer.size());
    if (!ofs)
        return false;
    return true;
}

bool readFileIntoHolderNoexcept(const std_path& filename, ByteArrayHolder& holder) noexcept(true)
{
    std::string buffer;
    if (!readFileIntoBufferNoexcept(filename, buffer))
        return false;
    holder.resize(buffer.size());
    memcpy(holder.data(), buffer.data(), holder.size());
    return true;
}

bool writeFileFromHolderNoexcept(const std_path& filename, const ByteArrayHolder& holder) noexcept(true)
{
    std::string buffer;
    buffer.resize(holder.size());
    memcpy(buffer.data(), holder.data(), holder.size());
    return writeFileFromBufferNoexcept(filename, buffer);
}

std::string readFileIntoBuffer(const std_path& filename) noexcept(false)
{
    std::string buffer;
    if (!readFileIntoBufferNoexcept(filename, buffer))
        throw std::runtime_error("Failed to read file: " + path2string(filename));
    return buffer;
}

void writeFileFromBuffer(const std_path& filename, const std::string& buffer) noexcept(false)
{
    if (!writeFileFromBufferNoexcept(filename, buffer))
        throw std::runtime_error("Failed to write file: " + path2string(filename));
}

ByteArrayHolder readFileIntoHolder(const std_path& filename) noexcept(false)
{
    ByteArrayHolder holder;
    if (!readFileIntoHolderNoexcept(filename, holder))
        throw std::runtime_error("Failed to read file: " + path2string(filename));
    return holder;
}

void writeFileFromHolder(const std_path& filename, const ByteArrayHolder& holder) noexcept(false)
{
    if (!writeFileFromHolderNoexcept(filename, holder))
        throw std::runtime_error("Failed to write file: " + path2string(filename));
}

}
