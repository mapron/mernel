/*
 * Copyright (C) 2026 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: BSL-1.0
 * See LICENSE file for details.
 */
#include "ShellUtils.hpp"

#ifdef _WIN32
#include "ScopeExit.hpp"

#include <Objbase.h>
#include <Shlobj.h>

namespace Mernel {

namespace {
class CoInitializer {
public:
    CoInitializer() { ::CoInitialize(NULL); }

    ~CoInitializer() { ::CoUninitialize(); }
};

const std::string g_linkExtension = ".lnk";

}

bool showFolderInFileManager(const std_path& path)
{
    CoInitializer coInit;
    return ::ShellExecuteW(nullptr, nullptr, path.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL) > HINSTANCE(32);
}

bool createShortCut(const std_path&    fromDir,
                    const std::string& fromName,
                    const std_path&    toDir,
                    const std::string& toFilename,
                    const std::string& extraArgs)
{
    CoInitializer coInit;
    auto          linkPath = fromDir / (fromName + g_linkExtension);

    const std::wstring extraArgsW(extraArgs.cbegin(), extraArgs.cend());

    const std_path to = toDir / toFilename;

    IShellLink* psl          = nullptr;
    bool        neededCoInit = false;

    HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**) &psl);

    if (hres == CO_E_NOTINITIALIZED) { // COM was not initialized
        neededCoInit = true;
        CoInitialize(NULL);
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**) &psl);
    }
    MERNEL_SCOPE_EXIT([&psl, neededCoInit] {
        if (psl)
            psl->Release();
        if (neededCoInit)
            CoUninitialize();
    });
    if (!SUCCEEDED(hres))
        return false;
    if (!SUCCEEDED(psl->SetPath(to.wstring().c_str())))
        return false;
    if (!SUCCEEDED(psl->SetWorkingDirectory(toDir.wstring().c_str())))
        return false;

    if (!extraArgs.empty() && !SUCCEEDED(psl->SetArguments(extraArgsW.c_str())))
        return false;

    IPersistFile* ppf = nullptr;
    hres              = psl->QueryInterface(IID_IPersistFile, (void**) &ppf);
    MERNEL_SCOPE_EXIT([&ppf] {
        if (ppf)
            ppf->Release();
    });
    if (!SUCCEEDED(hres))
        return false;

    hres = ppf->Save(linkPath.wstring().c_str(), TRUE);
    if (!SUCCEEDED(hres)) {
        return false;
    }

    return true;
}

bool createDesktopEntry(const std_path&,
                        const std::string&,
                        const std::string&,
                        const std::string&,
                        const std_path&,
                        const std_path&)
{
    return false;
}

}
#elif defined(__linux__)
#include "FileIOUtils.hpp"

#include <cerrno>

#include <sys/wait.h>
#include <unistd.h>

namespace Mernel {

namespace {

// Escaping for "string" values of the Desktop Entry spec.
std::string escapeDesktopValue(const std::string& value)
{
    std::string result;
    result.reserve(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (c == '\\')
            result += "\\\\";
        else if (c == '\n')
            result += "\\n";
        else if (c == '\t')
            result += "\\t";
        else if (c == '\r')
            result += "\\r";
        else if (c == ' ' && i == 0)
            result += "\\s";
        else
            result += c;
    }
    return result;
}

}

bool showFolderInFileManager(const std_path& path)
{
    const std::string pathStr = path2string(path);

    // Double fork, so xdg-open never blocks the caller and leaves no zombie.
    const pid_t child = fork();
    if (child < 0)
        return false;
    if (child == 0) {
        setsid();
        const pid_t grandchild = fork();
        if (grandchild == 0) {
            execlp("xdg-open", "xdg-open", pathStr.c_str(), nullptr);
            _exit(127);
        }
        _exit(grandchild < 0 ? 1 : 0);
    }

    int status = 0;
    while (waitpid(child, &status, 0) < 0) {
        if (errno != EINTR)
            return false;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

bool createShortCut(const std_path&    fromDir,
                    const std::string& fromName,
                    const std_path&    toDir,
                    const std::string& toFilename,
                    const std::string& extraArgs)
{
    return false;
}

bool createDesktopEntry(const std_path&    toDir,
                        const std::string& toFilename,
                        const std::string& name,
                        const std::string& execLine,
                        const std_path&    workingDir,
                        const std_path&    iconPath)
{
    std::error_code ec;
    std_fs::create_directories(toDir, ec);
    if (ec)
        return false;

    std::string content = "[Desktop Entry]\nType=Application\n";
    content += "Name=" + escapeDesktopValue(name) + "\n";
    content += "Exec=" + execLine + "\n";
    if (!workingDir.empty())
        content += "Path=" + escapeDesktopValue(path2string(workingDir)) + "\n";
    if (!iconPath.empty())
        content += "Icon=" + escapeDesktopValue(path2string(iconPath)) + "\n";

    const std_path file = toDir / toFilename;
    if (!writeFileFromBufferNoexcept(file, content))
        return false;

    using std_fs::perms;
    std_fs::permissions(file, perms::owner_all | perms::group_read | perms::group_exec | perms::others_read | perms::others_exec, ec);
    return !ec;
}

}
#else
namespace Mernel {

bool showFolderInFileManager(const std_path&)
{
    return false;
}

bool createShortCut(const std_path&    fromDir,
                    const std::string& fromName,
                    const std_path&    toDir,
                    const std::string& toFilename,
                    const std::string& extraArgs)
{
    return false;
}

bool createDesktopEntry(const std_path&,
                        const std::string&,
                        const std::string&,
                        const std::string&,
                        const std_path&,
                        const std_path&)
{
    return false;
}

}
#endif
