/*
 * Copyright (C) 2022 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

namespace Mernel::details {

template<typename F>
struct ScopeExitFunctionWrapper {
    ScopeExitFunctionWrapper(const F& f)
        : f(f)
    {}

    ScopeExitFunctionWrapper(const ScopeExitFunctionWrapper&) = delete;
    ScopeExitFunctionWrapper(ScopeExitFunctionWrapper&&)      = default;

    ScopeExitFunctionWrapper& operator=(const ScopeExitFunctionWrapper&) = delete;
    ScopeExitFunctionWrapper& operator=(ScopeExitFunctionWrapper&&)      = default;

    ~ScopeExitFunctionWrapper() { f(); }
    F f;
};

template<typename F>
static constexpr ScopeExitFunctionWrapper<F> createScopeExitFunctionWrapper(const F& f)
{
    return ScopeExitFunctionWrapper<F>(f);
}

}
#define MERNEL_DO_STRING_JOIN2(arg1, arg2) arg1##arg2
#define MERNEL_STRING_JOIN2(arg1, arg2) MERNEL_DO_STRING_JOIN2(arg1, arg2) // concat hack

#define MERNEL_SCOPE_EXIT(...) \
    auto MERNEL_STRING_JOIN2(scope_exit_, __LINE__) = Mernel::details::createScopeExitFunctionWrapper(__VA_ARGS__); \
    (void) MERNEL_STRING_JOIN2(scope_exit_, __LINE__)
