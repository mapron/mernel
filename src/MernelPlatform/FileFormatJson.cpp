/*
 * Copyright (C) 2026 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: BSL-1.0
 * See LICENSE file for details.
 */
#include "FileFormatJson.hpp"

#include "Logger.hpp"
#include "Profiler.hpp"

#include <sstream>
#include <iterator>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/filestream.h>

#include <jsoncons/json.hpp>

namespace Mernel {

namespace {

class JsonStreamOut {
public:
    JsonStreamOut(std::string& output)
        : m_output(output)
    {}

    char   Peek() const { return 0; }
    char   Take() { return 0; }
    size_t Tell() const { return 0; }

    void Put(char c) { m_output += c; }

    char*  PutBegin() { return nullptr; }
    size_t PutEnd(char*) { return 0; }

private:
    std::string& m_output;
};

void jsonToPropery(PropertyTree& data, rapidjson::Value& input)
{
    switch (input.GetType()) {
        case rapidjson::kNullType:
        {
            data = {};
        } break;
        case rapidjson::kFalseType:
        {
            data = PropertyTreeScalar(false);
        } break;
        case rapidjson::kTrueType:
        {
            data = PropertyTreeScalar(true);
        } break;
        case rapidjson::kObjectType:
        {
            data = {};
            data.convertToMap();
            auto& m = data.getMap();
            for (auto keyIt = input.MemberBegin(); keyIt != input.MemberEnd(); ++keyIt) {
                std::string key(keyIt->name.GetString(), keyIt->name.GetStringLength());
                jsonToPropery(m[key], keyIt->value);
            }
        } break;
        case rapidjson::kArrayType:
        {
            data.convertToList();
            auto& l = data.getList();
            l.resize(std::distance(input.Begin(), input.End()));
            size_t index = 0;
            for (auto nodeIt = input.Begin(); nodeIt != input.End(); ++nodeIt) {
                jsonToPropery(l[index++], *nodeIt);
            }
        } break;
        case rapidjson::kStringType:
        {
            std::string inputString(input.GetString(), input.GetStringLength());
            data = PropertyTreeScalar(std::move(inputString));
        } break;
        case rapidjson::kNumberType:
        {
            if (input.IsDouble())
                data = PropertyTreeScalar(input.GetDouble());
            else if (input.IsInt())
                data = PropertyTreeScalar(std::int64_t(input.GetInt()));
            else if (input.IsInt64())
                data = PropertyTreeScalar(std::int64_t(input.GetInt64()));
            else if (input.IsUint())
                data = PropertyTreeScalar(std::int64_t(input.GetUint()));
            else if (input.IsUint64())
                data = PropertyTreeScalar(std::int64_t(input.GetUint64()));
        } break;
    }
}

void propertyToJson(const PropertyTree& data, rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
{
    if (data.isNull()) {
    } else if (data.isList()) {
        json.SetArray();
        for (const PropertyTree& child : data.getList()) {
            rapidjson::Value tempValue;
            propertyToJson(child, tempValue, allocator);
            json.PushBack(tempValue, allocator);
        }
    } else if (data.isMap()) {
        json.SetObject();
        for (const auto& p : data.getMap()) {
            const PropertyTree& child = p.second;
            rapidjson::Value    tempValue;
            propertyToJson(child, tempValue, allocator);
            json.AddMember(p.first.c_str(), allocator, tempValue, allocator);
        }
    } else if (data.isScalar()) {
        const auto& scalar = data.getScalar();
        if (scalar.isBool())
            json.SetBool(scalar.toBool());
        if (scalar.isInt())
            json.SetInt64(scalar.toInt());
        if (scalar.isDouble())
            json.SetDouble(scalar.toDouble());
        if (scalar.isString()) {
            const auto s = scalar.toString();
            json.SetString(s.data(), static_cast<rapidjson::SizeType>(s.size()), allocator);
        }
    }
}

void jsonconsToProperty(PropertyTree& data, const jsoncons::json& input)
{
    switch (input.type()) {
        case jsoncons::json_type::null:
        {
            data = {};
        } break;
        case jsoncons::json_type::boolean:
        {
            data = PropertyTreeScalar(input.as<bool>());
        } break;
        case jsoncons::json_type::object:
        {
            data.convertToMap();
            auto& m = data.getMap();
            for (const auto& member : input.object_range()) {
                std::string key(member.key().data(), member.key().size());
                jsonconsToProperty(m[key], member.value());
            }
        } break;
        case jsoncons::json_type::array:
        {
            data.convertToList();
            auto& l = data.getList();
            l.resize(input.size());
            size_t index = 0;
            for (const auto& element : input.array_range()) {
                jsonconsToProperty(l[index++], element);
            }
        } break;
        case jsoncons::json_type::string:
        case jsoncons::json_type::byte_string:
        {
            std::string inputString(input.as_string_view().data(), input.as_string_view().size());
            data = PropertyTreeScalar(std::move(inputString));
        } break;
        case jsoncons::json_type::uint64:
        {
            data = PropertyTreeScalar(static_cast<std::int64_t>(input.as<std::uint64_t>()));
        } break;
        case jsoncons::json_type::int64:
        {
            data = PropertyTreeScalar(input.as<std::int64_t>());
        } break;
        case jsoncons::json_type::float16:
        case jsoncons::json_type::float64:
        {
            data = PropertyTreeScalar(input.as<double>());
        } break;
    }
}

}

bool readJsonFromBufferNoexcept(const std::string& buffer, PropertyTree& data) noexcept(true)
{
    rapidjson::Document input;
    const char*         dataPtr = buffer.data();
    if (buffer.starts_with(std::string_view("\xef\xbb\xbf", 3)))
        dataPtr += 3;

    auto& res = input.Parse<0>(dataPtr);
    if (res.HasParseError()) {
        Logger(Logger::Err) << res.GetParseError() << " (" << res.GetErrorOffset() << ")";
        return false;
    }

    if (!input.IsObject() && !input.IsArray())
        return false;

    data = PropertyTree{};
    jsonToPropery(data, input);

    return true;
}

bool writeJsonToBufferNoexcept(std::string& buffer, const PropertyTree& data, bool pretty) noexcept(true)
{
    if (pretty) {
        std::ostringstream os;
        PropertyTree::printReadableJson(os, data);
        buffer = os.str();
        return true;
    }
    if (data.isNull()) {
        return false;
    }
    rapidjson::Document json;
    propertyToJson(data, json, json.GetAllocator());

    JsonStreamOut                    outStream(buffer);
    rapidjson::Writer<JsonStreamOut> writer(outStream);
    json.Accept(writer);

    return true;
}

PropertyTree readJsonFromBuffer(const std::string& buffer) noexcept(false)
{
    PropertyTree result;
    if (!readJsonFromBufferNoexcept(buffer, result))
        throw std::runtime_error("Failed to read JSON");
    return result;
}

std::string writeJsonToBuffer(const PropertyTree& data, bool pretty) noexcept(false)
{
    std::string buffer;
    if (!writeJsonToBufferNoexcept(buffer, data, pretty))
        throw std::runtime_error("Failed to write JSON");
    return buffer;
}

bool readDirtyJsonFromBufferNoexcept(const std::string& buffer, PropertyTree& data) noexcept(true)
{
    using namespace jsoncons;
    try {
        auto options = json_options{}
                           .allow_comments(true)
                           .allow_trailing_comma(true);

        json val = jsoncons::json::parse(buffer, options);
        jsonconsToProperty(data, val);

        return true;
    }
    catch (const ser_error& e) {
        Logger(Logger::Err) << "Failed to parse input: " << e.what() << '\n';
    }
    return false;
}

}
