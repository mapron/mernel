/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include <memory>

#include "IExecutor.hpp"

#include "MernelExecutionExport.hpp"

namespace Mernel {

class MERNELEXECUTION_EXPORT ParallelExecutor final : public IExecutor {
public:
    ParallelExecutor(size_t threadCount);
    ~ParallelExecutor();

    Result execQueue(ITaskQueue& taskQueue) override;

    void abort() override;

private:
    struct Impl;
    const std::unique_ptr<Impl> m_impl;
};

}
