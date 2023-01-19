/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include "MacroUtils.hpp"
#include "MetaInfo.hpp"

#define MAKE_FIELD_PAIR(typeBase, str, value) Field(str, &typeBase ::value),
#define MAKE_FIELD_SINGLE(typeBase, value) Field(#value, &typeBase ::value),

#define MAKE_FIELD_SINGLE_OFFSET_1(typeBase, value) Field(1, #value, &typeBase ::value),
#define MAKE_FIELD_SINGLE_OFFSET_2(typeBase, value) Field(2, #value, &typeBase ::value),

#define MAKE_FIELD_LIST_PAIRED(typeBase, ...) \
    INTERNAL_EXPAND(FOR_EACH_BY_PAIR(MAKE_FIELD_PAIR, typeBase, __VA_ARGS__))

#define MAKE_FIELD_LIST_STRINGIFY(typeBase, ...) \
    INTERNAL_EXPAND(FOR_EACH_BY_SINGLE(MAKE_FIELD_SINGLE, typeBase, __VA_ARGS__))

#define MAKE_FIELD_LIST_STRINGIFY_OFFSET_1(typeBase, ...) \
    INTERNAL_EXPAND(FOR_EACH_BY_SINGLE(MAKE_FIELD_SINGLE_OFFSET_1, typeBase, __VA_ARGS__))
#define MAKE_FIELD_LIST_STRINGIFY_OFFSET_2(typeBase, ...) \
    INTERNAL_EXPAND(FOR_EACH_BY_SINGLE(MAKE_FIELD_SINGLE_OFFSET_2, typeBase, __VA_ARGS__))

// clang-format off
#define STRUCT_REFLECTION_PREFIX(typeBase)                                 \
    template <>                                                            \
    struct MetaInfo::MetaFields<typeBase>                                  \
    {                                                                      \
        [[maybe_unused]] static inline constexpr const std::tuple s_fields \
        {

#define STRUCT_REFLECTION_SUFFIX() \
    };                             \
    }                              \
    ;
// clang-format on

#define STRUCT_REFLECTION_PAIRED(typeBase, ...) \
    STRUCT_REFLECTION_PREFIX(typeBase) \
    INTERNAL_EXPAND(MAKE_FIELD_LIST_PAIRED(typeBase, __VA_ARGS__)) \
    STRUCT_REFLECTION_SUFFIX()
#define STRUCT_REFLECTION_STRINGIFY(typeBase, ...) \
    STRUCT_REFLECTION_PREFIX(typeBase) \
    INTERNAL_EXPAND(MAKE_FIELD_LIST_STRINGIFY(typeBase, __VA_ARGS__)) \
    STRUCT_REFLECTION_SUFFIX()

// these macros are handy if you want to strigify "m_field" and strip "m_" part for Field reflection name.
#define STRUCT_REFLECTION_STRINGIFY_OFFSET_1(typeBase, ...) \
    STRUCT_REFLECTION_PREFIX(typeBase) \
    INTERNAL_EXPAND(MAKE_FIELD_LIST_STRINGIFY_OFFSET_1(typeBase, __VA_ARGS__)) \
    STRUCT_REFLECTION_SUFFIX()

#define STRUCT_REFLECTION_STRINGIFY_OFFSET_2(typeBase, ...) \
    STRUCT_REFLECTION_PREFIX(typeBase) \
    INTERNAL_EXPAND(MAKE_FIELD_LIST_STRINGIFY_OFFSET_2(typeBase, __VA_ARGS__)) \
    STRUCT_REFLECTION_SUFFIX()

#define FIELD_SET_GET_LAMBDA(typeBase, fieldType, propertyName, setterName, getterName) \
    SetGetLambda<typeBase, fieldType>( \
        propertyName, \
        [](typeBase& obj, fieldType val) { obj.setterName(val); }, \
        [](const typeBase& obj) { return obj.getterName(); })
