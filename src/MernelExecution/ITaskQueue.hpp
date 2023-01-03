/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include <functional>

#include "ITaskQueueEventHandler.hpp"

namespace Mernel {

class ITaskQueue {
public:
    using Task = std::function<void()>;

public:
    virtual ~ITaskQueue() = default;

    /// process first task in a queue.
    virtual bool execOne() = 0;

    /// add task and return immediately.
    virtual void addTask(Task task) = 0;

    virtual size_t queueSize() const = 0;

    virtual void registerHandler(ITaskQueueEventHandler* eventProcessor)   = 0;
    virtual void unregisterHandler(ITaskQueueEventHandler* eventProcessor) = 0;
};

class TaskQueueImmediate final : public ITaskQueue {
public:
    bool   execOne() override { return false; }
    void   addTask(Task task) override { task(); }
    size_t queueSize() const override { return 0; }
    void   registerHandler(ITaskQueueEventHandler* eventProcessor) override {}
    void   unregisterHandler(ITaskQueueEventHandler* eventProcessor) override {}
};

}
