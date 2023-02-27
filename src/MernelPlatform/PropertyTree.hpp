/*
 * Copyright (C) 2022 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include "MernelPlatformExport.hpp"

#include <cassert>
#include <iosfwd>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace Mernel {

template<class T>
concept PropertyTreeIntegral = std::is_integral_v<T>;
template<class T>
concept PropertyTreeFloating = std::is_floating_point_v<T>;

template<class T>
concept PropertyTreeScalarHeld = PropertyTreeIntegral<T> || PropertyTreeFloating<T> || std::is_same_v<T, std::string>;

class MERNELPLATFORM_EXPORT PropertyTreeScalar {
public:
    PropertyTreeScalar()                                     = default;
    PropertyTreeScalar(const PropertyTreeScalar&)            = default;
    PropertyTreeScalar& operator=(const PropertyTreeScalar&) = default;

    PropertyTreeScalar(PropertyTreeScalar&&) noexcept               = default;
    PropertyTreeScalar& operator=(PropertyTreeScalar&& rh) noexcept = default;

    explicit PropertyTreeScalar(PropertyTreeIntegral auto value)
        : m_data(static_cast<int64_t>(value))
    {}
    explicit PropertyTreeScalar(PropertyTreeFloating auto value)
        : m_data(static_cast<double>(value))
    {}
    explicit PropertyTreeScalar(bool value)
        : m_data(value)
    {}
    explicit PropertyTreeScalar(std::string value)
        : m_data(std::move(value))
    {}
    explicit PropertyTreeScalar(const char* value)
        : m_data(std::string(value))
    {}

    bool operator==(const PropertyTreeScalar& rh) const noexcept = default;

    // convert value to standard scalar types. If conversion cannot be made, returns default value.
    bool               toBool() const noexcept;
    const std::string& toString() const noexcept;
    const char*        toCString() const noexcept;
    int64_t            toInt() const noexcept;
    double             toDouble() const noexcept;

    template<PropertyTreeIntegral T>
    void convertTo(T& value) const noexcept
    {
        value = static_cast<T>(toInt());
    }
    void convertTo(bool& value) const noexcept { value = toBool(); }
    template<PropertyTreeFloating T>
    void convertTo(T& value) const noexcept
    {
        value = static_cast<T>(toDouble());
    }
    void convertTo(std::string& value) const noexcept { value = toString(); }

    // print any possible value as a string.
    [[nodiscard]] std::string dump() const noexcept;
    void                      print(std::ostream& os, bool addQuotes = true, bool escapeString = true) const noexcept;

    [[nodiscard]] bool isNull() const noexcept { return m_data.index() == 0; }
    [[nodiscard]] bool isBool() const noexcept { return m_data.index() == 1; }
    [[nodiscard]] bool isInt() const noexcept { return m_data.index() == 2; }
    [[nodiscard]] bool isDouble() const noexcept { return m_data.index() == 3; }
    [[nodiscard]] bool isString() const noexcept { return m_data.index() == 4; }

private:
    std::variant<std::monostate, bool, int64_t, double, std::string> m_data;
};

class PropertyTree;

using PropertyTreeList      = std::vector<PropertyTree>;
using PropertyTreeMap       = std::map<std::string, PropertyTree>;
using PropertyTreeScalarMap = std::map<std::string, PropertyTreeScalar>;

class MERNELPLATFORM_EXPORT PropertyTree {
public:
    PropertyTree()                               = default;
    PropertyTree(const PropertyTree&)            = default;
    PropertyTree& operator=(const PropertyTree&) = default;

    PropertyTree(PropertyTree&&) noexcept               = default;
    PropertyTree& operator=(PropertyTree&& rh) noexcept = default;

    // making this explicit will be very inconvenient.
    PropertyTree(PropertyTreeScalar scalar)
        : m_data(std::move(scalar))
    {}
    explicit PropertyTree(const PropertyTreeScalarMap& scmap)
    {
        PropertyTreeMap copy;
        for (const auto& p : scmap)
            copy[p.first] = p.second;
        m_data = std::move(copy);
    }
    explicit PropertyTree(PropertyTreeMap tmap)
        : m_data(std::move(tmap))
    {}

    // check what is inside
    [[nodiscard]] bool isNull() const noexcept { return m_data.index() == 0; }
    [[nodiscard]] bool isScalar() const noexcept { return m_data.index() == 1; }
    [[nodiscard]] bool isList() const noexcept { return m_data.index() == 2; }
    [[nodiscard]] bool isMap() const noexcept { return m_data.index() == 3; }

    // const access to scalar and containers. May throw.
    const auto& getScalar() const noexcept(false)
    {
        if (const auto* val = std::get_if<PropertyTreeScalar>(&m_data); val)
            return *val;
        throw std::runtime_error("Invalid variant access, scalar expected");
    }
    const auto& getList() const noexcept(false)
    {
        if (const auto* val = std::get_if<PropertyTreeList>(&m_data); val)
            return *val;
        throw std::runtime_error("Invalid variant access, list expected");
    }
    const auto& getMap() const noexcept(false)
    {
        if (const auto* val = std::get_if<PropertyTreeMap>(&m_data); val)
            return *val;
        throw std::runtime_error("Invalid variant access, map expected");
    }

    // non-const access. May throw.
    auto& getScalar() noexcept(false)
    {
        if (auto* val = std::get_if<PropertyTreeScalar>(&m_data); val)
            return *val;
        throw std::runtime_error("Invalid variant access, scalar expected");
    }
    auto& getList() noexcept(false)
    {
        if (auto* val = std::get_if<PropertyTreeList>(&m_data); val)
            return *val;
        throw std::runtime_error("Invalid variant access, list expected");
    }
    auto& getMap() noexcept(false)
    {
        if (auto* val = std::get_if<PropertyTreeMap>(&m_data); val)
            return *val;
        throw std::runtime_error("Invalid variant access, map expected");
    }

    // checks if container has child object with provided key. returns false if property is not a map.
    bool contains(const std::string& key) const noexcept
    {
        if (!isMap())
            return false;
        return getMap().contains(key);
    }

    // get direct access to child value by key. Will throw if no key exists or variant is not a map.
    const PropertyTree& operator[](const std::string& key) const noexcept(false) { return getMap().at(key); }
    // add new key into map or modify existing one. property will be automatically converted to map type.
    PropertyTree& operator[](const std::string& key) noexcept(false)
    {
        if (m_data.index() == 0)
            m_data = PropertyTreeMap{};
        return getMap()[key];
    }
    PropertyTreeScalar value(const std::string& key, PropertyTreeScalar defaultValue) const noexcept(false)
    {
        if (!isMap())
            return defaultValue;
        const auto& map = getMap();
        auto        it  = map.find(key);
        if (it == map.cend())
            return defaultValue;
        return it->second.getScalar();
    }
    bool operator==(const PropertyTree& rh) const noexcept = default;

    void append(PropertyTree child) noexcept(false);
    void insert(const std::string& key, PropertyTree child) noexcept(false);

    // may throw if current value is not null or list. You only need this if you want to make an empty list.
    void convertToList() noexcept(false);

    // may throw if current value is not null or map. You only need this if you want to make an empty map.
    void convertToMap() noexcept(false);

    struct DumpParams {
        int    m_indentWidth    = 2;
        size_t m_smallArraySize = 10;
        bool   m_isDump         = true;
        bool   m_quoteKeys      = false;
        bool   m_quoteValues    = false;
        bool   m_escapeStrings  = false;
        bool   m_compactArrays  = true;
        bool   m_compactMaps    = true;
    };

    void dump(std::ostream& stream, const DumpParams& params, int level = 0) const noexcept;

public:
    static void mergePatch(PropertyTree& dest, const PropertyTree& source) noexcept(false);
    static void removeEqualValues(PropertyTree& one, PropertyTree& two) noexcept(false);

    // that function is not utf-safe! Only for debugging purpose (or if you sure that no Unicode string exist)
    static void printReadableJson(std::ostream& stream, const PropertyTree& source) noexcept;

    MERNELPLATFORM_EXPORT friend std::ostream& operator<<(std::ostream& stream, const PropertyTree& tree);

private:
    using Variant = std::variant<std::monostate, PropertyTreeScalar, PropertyTreeList, PropertyTreeMap>;
    Variant m_data;
};

}
