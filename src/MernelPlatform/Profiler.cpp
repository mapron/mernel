/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#include "Profiler.hpp"
#include "Logger.hpp"

#include <chrono>
#include <iostream>
#include <sstream>
#include <map>
#include <deque>
#include <array>
#include <compare>

#define FH_ENABLE_GLOBAL_PROFILER_CONTEXT 1

namespace Mernel {

bool ProfilerContext::s_enableLogging = false;

struct LightStringList {
    std::array<std::string_view, 8> m_data;
    size_t                          m_size = 0;

    static constexpr const std::string_view s_undefined = "???";

    constexpr void push(std::string_view value)
    {
        if (m_size < m_data.size() - 1) {
            m_data[m_size] = value;
        }
        m_size++;
    }
    constexpr void pop()
    {
        if (!m_size)
            return;
        m_size--;
        m_data[m_size] = {};
    }

    std::string toString(const std::string& glue) const
    {
        if (!m_size)
            return {};
        std::string result(m_data[0]);
        for (size_t i = 1; i < m_size; ++i)
            result = result + glue + std::string(i < m_data.size() ? m_data[i] : s_undefined);
        return result;
    }

    constexpr auto operator<=>(const LightStringList&) const noexcept = default;
};

struct ProfilerContext::Impl {
    struct Rec {
        int64_t ns    = 0;
        int     calls = 0;
        void    add(int64_t valueNS)
        {
            ns += valueNS;
            calls++;
        }
    };

    void printTo(std::ostream& os)
    {
        for (auto& p : all) {
            os << p.first.toString("->") << "=" << (p.second.ns / 1000000) << " #" << (p.second.calls) << "\n";
        }
    }

    void pushPrefix(std::string_view key)
    {
        stackPrefix.push(key);
        if (s_enableLogging)
            Logger(Logger::Debug) << stackPrefix.toString("->");
    }
    void addRecord(LightStringList key, int64_t valueNS)
    {
        if (lastRecord && lastKey == key) {
            lastRecord->add(valueNS);
            return;
        }
        lastRecord = &all[key];
        lastRecord->add(valueNS);
        lastKey = key;
    }
    void addRecord(std::string_view key, int64_t valueNS)
    {
        LightStringList list;
        list.push(key);
        addRecord(list, valueNS);
    }

    void pop(int64_t value)
    {
        LightStringList key = stackPrefix;

        if (s_enableLogging)
            Logger(Logger::Debug) << "/ " << key.toString("->");
        stackPrefix.pop();
        addRecord(key, value);
    }

    using RecordMap = std::map<LightStringList, Rec>;

    RecordMap       all{};
    Rec*            lastRecord = nullptr;
    LightStringList lastKey;
    LightStringList stackPrefix;
};

ProfilerContext::ProfilerContext()
    : m_impl(std::make_unique<ProfilerContext::Impl>())
{
}

ProfilerContext::~ProfilerContext() = default;

void ProfilerContext::addRecord(std::string_view key, int64_t ms)
{
    m_impl->addRecord(key, ms * 1000000);
}
void ProfilerContext::clearAll()
{
    m_impl->all.clear();
    m_impl->lastRecord         = nullptr;
    m_impl->stackPrefix.m_size = 0;
}

void ProfilerContext::printToStdErr() const
{
    m_impl->printTo(std::cerr);
}

std::string ProfilerContext::printToStr() const
{
    std::ostringstream ss;
    m_impl->printTo(ss);
    return ss.str();
}

namespace {

int64_t curNS()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

ProfilerContext* defaultContext = nullptr;

ProfilerContext* getGlobalProfilerContext()
{
#ifdef FH_ENABLE_GLOBAL_PROFILER_CONTEXT
    static ProfilerContext globalProfilerContext{};
    return &globalProfilerContext;
#else
    return nullptr;
#endif
}

ProfilerContext* getDefaultContext()
{
    return defaultContext ? defaultContext : getGlobalProfilerContext();
}

}

ScopeTimer::ScopeTimer()
    : start(curNS())
    , out(nullptr)
{
}

ScopeTimer::ScopeTimer(int64_t& out)
    : start(curNS())
    , out(&out)
{
}

ScopeTimer::~ScopeTimer()
{
    if (out)
        *out = elapsedUS();
}

void ScopeTimer::reset()
{
    start = curNS();
}

int64_t ScopeTimer::elapsedUS() const noexcept
{
    return (curNS() - start) / 1000;
}

int64_t ScopeTimer::elapsedNano() const noexcept
{
    return (curNS() - start);
}

ProfilerScope::ProfilerScope(std::string_view key, bool nested)
    : key(key)
    , nested(nested)
    , context(getDefaultContext())
{
    if (!context || !nested)
        return;
    context->m_impl->pushPrefix(key);
}

ProfilerScope::ProfilerScope(std::string_view key, ProfilerContext& customContext, bool nested)
    : key(key)
    , context(&customContext)
{
    if (nested)
        context->m_impl->pushPrefix(key);
}

ProfilerScope::~ProfilerScope()
{
    if (!context)
        return;

    if (nested)
        context->m_impl->pop(timer.elapsedNano());
    else
        context->m_impl->addRecord(key, timer.elapsedNano());
}

void ProfilerScope::printToStdErr()
{
    auto* context = getDefaultContext();
    if (!context)
        return;
    context->printToStdErr();
}

void ProfilerScope::clearAll()
{
    auto* context = getDefaultContext();
    if (!context)
        return;
    context->clearAll();
}

std::string ProfilerScope::printToStr()
{
    auto* context = getDefaultContext();
    if (!context)
        return {};
    return context->printToStr();
}
void ProfilerScope::addRecord(std::string_view str, int64_t ms)
{
    auto* context = getDefaultContext();
    if (!context)
        return;
    context->addRecord(str, ms);
}

ProfilerDefaultContextSwitcher::ProfilerDefaultContextSwitcher(ProfilerContext& customContext) noexcept
{
    isTrivial = &customContext == defaultContext;
    if (isTrivial)
        return;

    contextPrev    = defaultContext;
    defaultContext = &customContext;
}

ProfilerDefaultContextSwitcher::~ProfilerDefaultContextSwitcher()
{
    if (!isTrivial)
        defaultContext = contextPrev;
}

}
