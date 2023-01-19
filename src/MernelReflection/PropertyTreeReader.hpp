/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include "EnumTraits.hpp"
#include "MetaInfo.hpp"

#include "MernelPlatform/PropertyTree.hpp"

#include <set>

namespace Mernel::Reflection {

template<class T>
concept HasFromJsonRead = requires(T t, const PropertyTree& data)
{
    t.convertFromJson(data);
};
template<class T>
concept HasFromJsonReadGlobal = !PropertyTreeScalarHeld<T> && requires(T t, const PropertyTree& data)
{
    convertFromJson(data, t);
};
template<class T>
concept HasFromStringRead = requires(T t, const std::string& data)
{
    t.fromString(data);
};
template<typename T>
concept HasFieldsForRead = HasFields<T> && !HasCustomTransformRead<T> && !HasFromStringRead<T> && !HasFromJsonRead<T>;

template<class CustomReader>
class PropertyTreeReaderBase {
public:
    template<class T>
    void jsonToValueUsingMeta(const PropertyTree& json, T& value)
    {
        const auto& jsonMap = json.getMap();

        auto visitor = [&value, &jsonMap, this](auto&& field) {
            if (jsonMap.contains(field.name())) {
                auto writer = field.makeValueWriter(value);
                this->jsonToValue(jsonMap.at(field.name()), writer.getRef());
            } else if (m_resetToDefault) {
                if constexpr (std::is_default_constructible_v<T>) {
                    const T& defParent = MetaInfo::getDefaultConstructed<T>();
                    auto     defValue  = field.get(defParent);
                    auto     writer    = field.makeValueWriter(value);
                    writer.getRef()    = std::move(defValue);
                }
            }
        };
        std::apply([&visitor](auto&&... field) { ((visitor(field)), ...); }, MetaInfo::MetaFields<T>::s_fields);
    }

    template<HasFieldsForRead T>
    void jsonToValue(const PropertyTree& json, T& value)
    {
        jsonToValueUsingMeta(json, value);
    }

    void jsonToValue(const PropertyTree& json, PropertyTreeScalarHeld auto& value)
    {
        json.getScalar().convertTo(value);
    }

    template<IsEnum Enum>
    void jsonToValue(const PropertyTree& json, Enum& value)
    {
        const auto str = json.isScalar() ? json.getScalar().toString() : "";
        value          = EnumTraits::stringToEnum<Enum>({ str.c_str(), str.size() });
    }

    template<HasCustomTransformRead T>
    void jsonToValue(const PropertyTree& json, T& value)
    {
        PropertyTree tmp;
        if (MetaInfo::transformTreeRead<T>(json, tmp))
            jsonToValueUsingMeta(tmp, value);
        else
            jsonToValueUsingMeta(json, value);
    }

    void jsonToValue(const PropertyTree& json, HasFromStringRead auto& value)
    {
        if (!json.isScalar() || !json.getScalar().isString())
            return jsonToValueUsingMeta(json, value);

        std::string tmp = json.getScalar().toString();
        value.fromString(std::move(tmp));
    }

    void jsonToValue(const PropertyTree& json, HasFromJsonRead auto& value)
    {
        value.convertFromJson(value);
    }

    void jsonToValue(const PropertyTree& json, HasFromJsonReadGlobal auto& value)
    {
        convertFromJson(json, value);
    }

    template<NonAssociative Container>
    void jsonToValue(const PropertyTree& json, Container& container)
    {
        if (!json.isList())
            return;
        container.clear();
        if constexpr (IsStdVector<Container>) {
            container.reserve(json.getList().size());
        }
        auto inserter = std::inserter(container, container.end());
        for (const PropertyTree& child : json.getList()) {
            typename Container::value_type value;
            jsonToValue(child, value);
            *inserter = std::move(value);
        }
    }

    template<IsStdArray Container>
    void jsonToValue(const PropertyTree& json, Container& container)
    {
        if (!json.isList())
            return;
        assert(container.size() <= json.getList().size());
        size_t index = 0;
        for (const PropertyTree& child : json.getList()) {
            typename Container::value_type value;
            jsonToValue(child, value);
            container[index++] = std::move(value);
        }
    }

    template<IsStdOptional Container>
    void jsonToValue(const PropertyTree& json, Container& container)
    {
        if (!json.isList())
            return;
        assert(json.getList().size() <= 1);
        for (const PropertyTree& child : json.getList()) {
            typename Container::value_type value;
            jsonToValue(child, value);
            container = std::move(value);
            //break;
        }
    }

    template<IsMap Container>
    void jsonToValue(const PropertyTree& json, Container& container)
    {
        if (!json.isList())
            return;

        for (const PropertyTree& child : json.getList()) {
            typename Container::mapped_type value;
            typename Container::key_type    key;
            jsonToValue(child["key"], key);
            jsonToValue(child["value"], value);
            container[key] = std::move(value);
        }
    }

    template<IsStringMap Container>
    void jsonToValue(const PropertyTree& json, Container& container)
    {
        if (!json.isMap())
            return;

        for (const auto& [keyString, childJson] : json.getMap()) {
            typename Container::key_type    key;
            typename Container::mapped_type value;
            const PropertyTreeScalar        keyScalar(keyString);
            jsonToValue(childJson, value);
            jsonToValue(keyScalar, key);
            container[key] = value;
        }
    }

    void jsonToValue(const PropertyTree& json, IsEmptyType auto& container)
    {
    }

    template<class T>
    void jsonToValue(const PropertyTree& json, T& value)
    {
        static_cast<CustomReader*>(this)->jsonToValueImpl(json, value);
    }

    bool m_resetToDefault = true;
};

class PropertyTreeReader : public PropertyTreeReaderBase<PropertyTreeReader> {};

}
