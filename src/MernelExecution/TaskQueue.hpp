/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include <memory>

#include "ITaskQueue.hpp"

#include "MernelExecutionExport.hpp"

namespace Mernel {

class MERNELEXECUTION_EXPORT TaskQueue final : public ITaskQueue {
public:
    TaskQueue();
    ~TaskQueue();

    bool execOne() override;

    void addTask(Task task) override;

    size_t queueSize() const override;

    void registerHandler(ITaskQueueEventHandler* eventProcessor) override;
    void unregisterHandler(ITaskQueueEventHandler* eventProcessor) override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}
