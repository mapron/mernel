/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */

#include "TaskQueue.hpp"

#include <list>
#include <map>
#include <mutex>
#include <set>

namespace Mernel {

struct TaskQueue::Impl {
    std::mutex                        m_mutex;
    std::list<Task>                   m_queue;
    bool                              m_readyState{ false };
    std::set<ITaskQueueEventHandler*> m_eventProcessors;

    bool takeFirst(Task& task)
    {
        std::unique_lock lock(m_mutex);

        if (m_queue.empty()) {
            markReady(false);
            return false;
        }

        task = std::move(m_queue.front());
        m_queue.pop_front();

        for (auto* eventProcessor : m_eventProcessors)
            eventProcessor->handleProgress();
        return true;
    }

    void markReady(bool ready)
    {
        const bool isChanged = m_readyState != ready;
        m_readyState         = ready;
        if (isChanged) {
            for (auto* eventProcessor : m_eventProcessors)
                eventProcessor->handleChangedState(m_readyState ? ITaskQueueEventHandler::State::Ready
                                                                : ITaskQueueEventHandler::State::Empty);
        }
    }
};

TaskQueue::TaskQueue()
    : m_impl(new Impl)
{}

TaskQueue::~TaskQueue()
{
    std::unique_lock lock(m_impl->m_mutex);
    m_impl->m_queue.clear();
}

bool TaskQueue::execOne()
{
    Task task;
    if (!m_impl->takeFirst(task))
        return false;
    task();
    return true;
}

void TaskQueue::addTask(Task task)
{
    std::unique_lock lock(m_impl->m_mutex);
    m_impl->m_queue.push_back(std::move(task));
    m_impl->markReady(true);
}

size_t TaskQueue::queueSize() const
{
    std::unique_lock lock(m_impl->m_mutex);
    return m_impl->m_queue.size();
}

void TaskQueue::registerHandler(ITaskQueueEventHandler* eventProcessor)
{
    std::unique_lock lock(m_impl->m_mutex);
    m_impl->m_eventProcessors.insert(eventProcessor);
}

void TaskQueue::unregisterHandler(ITaskQueueEventHandler* eventProcessor)
{
    std::unique_lock lock(m_impl->m_mutex);
    m_impl->m_eventProcessors.erase(eventProcessor);
}

}
