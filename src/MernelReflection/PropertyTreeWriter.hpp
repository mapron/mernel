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
concept HasToJsonWrite = requires(T t, PropertyTree& data)
{
    t.convertToJson(data);
};

template<class T>
concept HasToJsonWriteGlobal = !PropertyTreeScalarHeld<T> && requires(T t, PropertyTree& data)
{
    convertToJson(t, data);
};

template<class T>
concept HasToStringWrite = requires(T t, std::string& data)
{
    data = t.toString();
};
template<typename T>
concept HasFieldsForWrite = HasFields<T> && !HasCustomTransformWrite<T> && !HasToStringWrite<T> && !HasToJsonWrite<T>;

template<class CustomWriter>
class PropertyTreeWriterBase {
public:
    template<class T>
    void valueToJsonUsingMeta(const T& value, PropertyTree& result)
    {
        if (m_clearMaps)
            result = {};
        result.convertToMap();
        auto& jsonMap = result.getMap();

        auto visitor = [&value, &jsonMap, this](auto&& field) {
            const auto& fieldVal = field.get(value);
            using FieldType      = std::remove_cvref_t<decltype(fieldVal)>;

            if (m_skipDefault) {
                if constexpr (std::is_default_constructible_v<T> && details::is_comparable<FieldType>()) {
                    const T&         defParent = MetaInfo::getDefaultConstructed<T>();
                    const FieldType& defValue  = field.get(defParent);
                    if (fieldVal == defValue)
                        return;
                }
            }

            this->valueToJson(fieldVal, jsonMap[field.name()]);
        };

        std::apply([&visitor](auto&&... field) { ((visitor(field)), ...); }, MetaInfo::MetaFields<T>::s_fields);
    }

    void valueToJson(const HasFieldsForWrite auto& value, PropertyTree& result)
    {
        valueToJsonUsingMeta(value, result);
    }

    void valueToJson(const PropertyTreeScalarHeld auto& value, PropertyTree& result)
    {
        result = PropertyTreeScalar(value);
    }

    void valueToJson(const IsEnum auto& value, PropertyTree& result)
    {
        const auto str = EnumTraits::enumToString(value);
        result         = PropertyTreeScalar(std::string(str.begin(), str.end()));
    }

    template<HasCustomTransformWrite T>
    void valueToJson(const T& value, PropertyTree& result)
    {
        valueToJsonUsingMeta(value, result);
        PropertyTree tmp;
        if (MetaInfo::transformTreeWrite<T>(result, tmp))
            result = std::move(tmp);
    }

    void valueToJson(const HasToStringWrite auto& value, PropertyTree& result)
    {
        result = PropertyTreeScalar(value.toString());
    }
    void valueToJson(const HasToJsonWrite auto& value, PropertyTree& result)
    {
        value.convertToJson(result);
    }
    void valueToJson(const HasToJsonWriteGlobal auto& value, PropertyTree& result)
    {
        convertToJson(value, result);
    }

    void valueToJson(const NonAssociative auto& container, PropertyTree& result)
    {
        result = {};
        result.convertToList();
        result.getList().resize(std::size(container));
        size_t i = 0;
        for (const auto& value : container) {
            PropertyTree& child = result.getList()[i++];
            valueToJson(value, child);
        }
    }

    void valueToJson(const IsStdOptional auto& container, PropertyTree& result)
    {
        result = {};
        result.convertToList();
        if (!container.has_value())
            return;

        result.getList().resize(1);

        PropertyTree& child = result.getList()[0];
        valueToJson(container.value(), child);
    }

    void valueToJson(const IsMap auto& container, PropertyTree& result)
    {
        result = {};
        result.convertToList();
        for (const auto& [key, value] : container) {
            PropertyTree pair;
            valueToJson(key, pair["key"]);
            valueToJson(value, pair["value"]);
            result.append(std::move(pair));
        }
    }

    void valueToJson(const IsStringMap auto& container, PropertyTree& result)
    {
        if (m_clearMaps)
            result = {};
        result.convertToMap();
        for (const auto& [key, value] : container) {
            PropertyTree childKey;
            valueToJson(key, childKey);
            assert(childKey.isScalar() && childKey.getScalar().isString());

            valueToJson(value, result[childKey.getScalar().toString()]);
        }
    }
    void valueToJson(const IsEmptyType auto& container, PropertyTree& result)
    {
    }

    template<class T>
    void valueToJson(const T& value, PropertyTree& result)
    {
        static_cast<CustomWriter*>(this)->valueToJsonImpl(value, result);
    }

    bool m_clearMaps   = true;
    bool m_skipDefault = true;
};
class PropertyTreeWriter : public PropertyTreeWriterBase<PropertyTreeWriter> {};

}
