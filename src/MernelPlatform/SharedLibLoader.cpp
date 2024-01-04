/*
 * Copyright (C) 2024 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#include "SharedLibLoader.hpp"

#ifdef _WIN32

#include <Windows.h>

namespace Mernel {

bool SharedLibLoader::open(const std_path& path)
{
    HMODULE handle = ::LoadLibraryW(path.wstring().c_str());
    m_handle       = reinterpret_cast<void*>(handle);

    return m_handle != nullptr;
}

void SharedLibLoader::close()
{
    if (m_handle)
        ::FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
    m_handle = nullptr;
}

void* SharedLibLoader::getSymbol(const std::string& name) const
{
    return reinterpret_cast<void*>(::GetProcAddress(reinterpret_cast<HMODULE>(m_handle), name.c_str()));
}

std::string SharedLibLoader::getLastError() const
{
    DWORD errCode = ::GetLastError();
    return "GetLastError=" + std::to_string(errCode);
}

}

#else

#include <dlfcn.h>

namespace Mernel {

bool SharedLibLoader::open(const std_path& path)
{
    m_handle = dlopen(path2string(path).c_str(), RTLD_NOW);
    return m_handle != nullptr;
}

void SharedLibLoader::close()
{
    if (m_handle)
        dlclose(m_handle);
    m_handle = nullptr;
}

void* SharedLibLoader::getSymbol(const std::string& name) const
{
    return dlsym(m_handle, name.c_str());
}

std::string SharedLibLoader::getLastError() const
{
    return dlerror();
}

}

#endif
