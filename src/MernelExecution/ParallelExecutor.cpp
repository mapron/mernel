/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */

#include "ParallelExecutor.hpp"

#include <atomic>
#include <cassert>
#include <mutex>
#include <thread>
#include <vector>

#include <condition_variable>

namespace Mernel {

struct ParallelExecutor::Impl : public ITaskQueueEventHandler {
    Impl(size_t threadCount)
        : m_threads(threadCount)
    {
        assert(threadCount > 0);
    }

    void handleChangedState(State state) override
    {
        if (state != State::Ready)
            return;

        std::unique_lock lock(m_activeThreadsMutex);
        m_isReady = true;
        m_activeThreadsCV.notify_all();
    }

    void handleProgress() override
    {
    }

    bool                    m_isReady{ false };
    std::mutex              m_activeThreadsMutex;
    std::condition_variable m_activeThreadsCV;
    size_t                  m_activeThreads{ 0 };

    std::vector<std::thread> m_threads;
    std::mutex               m_execMutex;
    std::atomic_bool         m_terminate{ false };
};

ParallelExecutor::ParallelExecutor(size_t threadCount)
    : m_impl(std::make_unique<Impl>(threadCount))
{}

ParallelExecutor::~ParallelExecutor()
{
    abort();
}

IExecutor::Result ParallelExecutor::execQueue(ITaskQueue& taskQueue)
{
    Result result = Result::EmptyQueue;

    std::unique_lock lock(m_impl->m_execMutex);

    m_impl->m_activeThreads = m_impl->m_threads.size();
    taskQueue.registerHandler(m_impl.get());

    auto threadFun = [this, &taskQueue, &result] {
        while (!m_impl->m_terminate) {
            if (taskQueue.execOne()) {
                result = Result::Completed;
            } else {
                std::unique_lock lock(m_impl->m_activeThreadsMutex);
                m_impl->m_activeThreads--;
                m_impl->m_isReady = false;
                m_impl->m_activeThreadsCV.notify_all();
                m_impl->m_activeThreadsCV.wait(lock, [this]() -> bool {
                    return m_impl->m_terminate || m_impl->m_activeThreads == 0 || m_impl->m_isReady;
                });
                if (!m_impl->m_isReady)
                    break;
                m_impl->m_activeThreads++;
            }
        }
    };
    for (auto& thread : m_impl->m_threads)
        thread = std::thread(threadFun);
    for (auto& thread : m_impl->m_threads)
        thread.join();
    taskQueue.unregisterHandler(m_impl.get());
    return result;
}

void ParallelExecutor::abort()
{
    m_impl->m_terminate = true;
    m_impl->m_activeThreadsCV.notify_all();
    std::unique_lock lock(m_impl->m_execMutex);
}

}
