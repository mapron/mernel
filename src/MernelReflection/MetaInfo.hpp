/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include <type_traits>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <vector>
#include <list>
#include <deque>
#include <array>
#include <string>

#include <frozen/string.h>

namespace Mernel {
class PropertyTree;
}
namespace Mernel::Reflection {

namespace details {
template<class T>
struct is_array : std::is_array<T> {};
template<class T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {};
}

template<typename T>
concept IsMap = std::same_as<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type>> //
                || std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

template<typename T>
concept IsSet = std::same_as<T, std::set<typename T::key_type, typename T::key_compare, typename T::allocator_type>> //
                || std::same_as<T, std::unordered_set<typename T::key_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

template<typename T>
concept IsStdArray = details::is_array<T>::value;

template<typename T>
concept IsStdOptional = std::same_as<T, std::optional<typename T::value_type>>;

template<typename T>
concept IsStdVector = std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type>>;

template<typename T>
concept IsSequentalContainer = std::same_as<T, std::deque<typename T::value_type, typename T::allocator_type>>   //
                               || std::same_as<T, std::list<typename T::value_type, typename T::allocator_type>> //
                               || IsStdVector<T> || IsStdArray<T>;

template<typename T>
concept NonAssociative = IsSequentalContainer<T> || IsSet<T>;

namespace details {
template<class T>
static inline constexpr bool is_comparable() noexcept
{
    return false;
}
template<std::equality_comparable T>
    requires(!(IsMap<T> || IsSet<T> || IsStdOptional<T> || IsSequentalContainer<T>) )
static inline constexpr bool is_comparable() noexcept
{
    return true;
}
template<IsSequentalContainer T>
static inline constexpr bool is_comparable() noexcept
{
    return is_comparable<typename T::value_type>();
}
template<IsStdOptional T>
static inline constexpr bool is_comparable() noexcept
{
    return is_comparable<typename T::value_type>();
}
template<IsSet T>
static inline constexpr bool is_comparable() noexcept
{
    return is_comparable<typename T::key_type>();
}
template<IsMap T>
static inline constexpr bool is_comparable() noexcept
{
    return is_comparable<typename T::key_type>() && is_comparable<typename T::mapped_type>();
}
}

struct MetaInfo {
    template<class Parent>
    static inline const Parent& getDefaultConstructed()
    {
        static const Parent s_inst{};
        return s_inst;
    }

    template<class Parent, class FieldType>
    struct Field {
        constexpr Field(frozen::string name, FieldType Parent::*f)
            : m_name(name)
            , m_f(f)
        {}
        template<std::size_t N>
        constexpr Field(size_t offset, char const (&data)[N], FieldType Parent::*f)
            : Field(frozen::string(data + offset, N - offset - 1), f)
        {}

        struct ValueWriter {
            const Field& m_field;

            Parent& m_parent;

            constexpr FieldType& getRef() { return m_parent.*(m_field.m_f); }
        };

        constexpr ValueWriter makeValueWriter(Parent& parent) const noexcept
        {
            return { *this, parent };
        }
        constexpr const FieldType& get(const Parent& parent) const noexcept
        {
            return parent.*m_f;
        }

        [[nodiscard]] std::string name() const noexcept
        {
            return std::string(m_name.begin(), m_name.end()); // if this throws, we don't care, abort is fine.
        }

        frozen::string m_name;
        FieldType Parent::*m_f;
    };

    /**
     * usage:
     * SetGetLambda<QPoint, int>("x", [](QPoint& obj, int val){ obj.setX(val); }, [](const QPoint& obj){ return obj.x(); } );
     * 
     * important to have getter with second argument taking value (not const reference).
     */

    template<class Parent, typename FieldType>
    struct SetGetLambda {
        using ArgTypeSetter = void (*)(Parent&, FieldType);
        using ArgTypeGetter = FieldType (*)(const Parent&);

        constexpr SetGetLambda(frozen::string name, ArgTypeSetter setter, ArgTypeGetter getter)
            : m_name(name)
            , m_setter(setter)
            , m_getter(getter)
        {}

        struct ValueWriter {
            const SetGetLambda& m_field;
            Parent&             m_parent;
            FieldType           m_tmp{};

            constexpr FieldType& getRef() { return m_tmp; }

            constexpr ~ValueWriter()
            {
                m_field.set(m_parent, std::move(m_tmp));
            }
        };

        constexpr ValueWriter makeValueWriter(Parent& parent) const noexcept
        {
            return { *this, parent };
        }

        constexpr FieldType get(const Parent& parent) const
        {
            return m_getter(parent);
        }
        constexpr void set(Parent& parent, FieldType value) const
        {
            m_setter(parent, std::move(value));
        }

        [[nodiscard]] std::string name() const noexcept
        {
            return std::string(m_name.begin(), m_name.end()); // if this throws, we don't care, abort is fine.
        }

        frozen::string m_name;
        ArgTypeSetter  m_setter;
        ArgTypeGetter  m_getter;
    };

    /**
     * usage:
     * SetGet("property", &MyType::setProp, &MyType::getProperty)
     */
    template<class Parent, typename ArgTypeSetter, typename ArgTypeGetter>
    struct SetGet {
        using FieldType = std::remove_reference_t<std::remove_cv_t<ArgTypeGetter>>;

        constexpr SetGet(frozen::string name, void (Parent::*setter)(ArgTypeSetter), ArgTypeGetter (Parent::*getter)() const)
            : m_name(name)
            , m_setter(setter)
            , m_getter(getter)
        {}

        struct ValueWriter {
            const SetGet& m_field;
            Parent&       m_parent;
            FieldType     m_tmp{};

            constexpr FieldType& getRef() { return m_tmp; }

            constexpr ~ValueWriter() { m_field.set(m_parent, std::move(m_tmp)); }
        };

        constexpr ValueWriter makeValueWriter(Parent& parent) const noexcept { return { *this, parent }; }

        constexpr ArgTypeGetter get(const Parent& parent) const noexcept { return (parent.*m_getter)(); }
        constexpr void          set(Parent& parent, ArgTypeSetter value) const noexcept { (parent.*m_setter)(std::move(value)); }

        [[nodiscard]] std::string name() const noexcept
        {
            return std::string(m_name.begin(), m_name.end()); // if this throws, we don't care, abort is fine.
        }

        frozen::string m_name;
        void (Parent::*m_setter)(ArgTypeSetter);
        ArgTypeGetter (Parent::*m_getter)() const;
    };

    /**
     * usage:
     * SetGetNoexcept("property", &MyType::setProp, &MyType::getProperty)
     */
    template<class Parent, typename ArgTypeSetter, typename ArgTypeGetter>
    struct SetGetNoexcept {
        using FieldType = std::remove_reference_t<std::remove_cv_t<ArgTypeGetter>>;

        constexpr SetGetNoexcept(frozen::string name, void (Parent::*setter)(ArgTypeSetter) noexcept, ArgTypeGetter (Parent::*getter)() const noexcept)
            : m_name(name)
            , m_setter(setter)
            , m_getter(getter)
        {}

        struct ValueWriter {
            const SetGetNoexcept& m_field;
            Parent&               m_parent;
            FieldType             m_tmp{};

            constexpr FieldType& getRef() { return m_tmp; }

            constexpr ~ValueWriter()
            {
                m_field.set(m_parent, std::move(m_tmp));
            }
        };

        constexpr ValueWriter makeValueWriter(Parent& parent) const noexcept
        {
            return { *this, parent };
        }

        constexpr ArgTypeGetter get(const Parent& parent) const noexcept
        {
            return (parent.*m_getter)();
        }
        constexpr void set(Parent& parent, ArgTypeSetter value) const noexcept
        {
            (parent.*m_setter)(std::move(value));
        }

        [[nodiscard]] std::string name() const noexcept
        {
            return std::string(m_name.begin(), m_name.end()); // if this throws, we don't care, abort is fine.
        }

        frozen::string m_name;
        void (Parent::*m_setter)(ArgTypeSetter) noexcept;
        ArgTypeGetter (Parent::*m_getter)() const noexcept;
    };

    template<class Parent>
    struct MetaFields {
        static inline constexpr const bool s_fields{ false };
    };

    template<class Parent>
    static inline bool transformTreeRead(const PropertyTree& treeIn, PropertyTree& treeOut);

    template<class Parent>
    static inline bool transformTreeWrite(const PropertyTree& treeIn, PropertyTree& treeOut);
};

template<class Parent>
[[maybe_unused]] static inline constexpr const bool s_useCustomTransformRead{ false };

template<class Parent>
[[maybe_unused]] static inline constexpr const bool s_useCustomTransformWrite{ false };

template<class Parent>
[[maybe_unused]] static inline constexpr const bool s_isStringMap{ false };

template<class Parent>
[[maybe_unused]] static inline constexpr const bool s_isEmpty{ false };

template<typename T>
concept HasCustomTransformRead = s_useCustomTransformRead<T>;

template<typename T>
concept HasCustomTransformWrite = s_useCustomTransformWrite<T>;

template<typename T>
concept HasFields = !
std::is_same_v<std::remove_cvref_t<decltype(MetaInfo::MetaFields<T>::s_fields)>, bool>;

template<typename T>
concept IsStringMap = IsMap<T> && s_isStringMap<T>;

template<typename T>
concept IsEmptyType = s_isEmpty<T>;

}
