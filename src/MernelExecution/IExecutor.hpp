/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include <atomic>

#include "ITaskQueue.hpp"

namespace Mernel {

/// Synchronous executor of task queues.
class IExecutor {
public:
    virtual ~IExecutor() = default;

    enum class Result
    {
        EmptyQueue,
        Completed,
    };

    /// Block execution until all tasks done
    virtual Result execQueue(ITaskQueue& taskQueue) = 0;

    virtual void abort() = 0;
};

class BlockingExecutor final : public IExecutor {
public:
    Result execQueue(ITaskQueue& taskQueue) override
    {
        Result result = Result::EmptyQueue;
        while (!m_isAborted && taskQueue.execOne())
            result = Result::Completed;
        return result;
    }
    void abort() override { m_isAborted = true; }

private:
    std::atomic_bool m_isAborted{ false };
};

}
