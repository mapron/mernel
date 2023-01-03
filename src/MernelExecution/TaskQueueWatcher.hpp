/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

#include "ITaskQueue.hpp"

#include <functional>
#include <mutex>
#include <condition_variable>

namespace Mernel {

class TaskQueueWatcher : public ITaskQueueEventHandler {
public:
    using Progress = std::function<void(int, int)>;

public:
    TaskQueueWatcher(ITaskQueue& queue)
        : m_queue(queue)
    {
        m_queue.registerHandler(this);
        m_total = m_queue.queueSize();
    }

    ~TaskQueueWatcher() { m_queue.unregisterHandler(this); }

    void setProgressHandler(Progress progress)
    {
        m_progress = std::move(progress);
    }

    void handleChangedState(State state) override
    {
        std::scoped_lock lock(m_mutex);
        m_currentState = state;
        m_cv.notify_one();
    }

    void handleProgress() override
    {
        std::scoped_lock lock(m_mutex);
        m_current++;
        if (m_progress)
            m_progress(static_cast<int>(m_current), static_cast<int>(m_total));
    }

    void waitForState(State state, std::chrono::seconds timeout = std::chrono::seconds(0))
    {
        std::unique_lock lock(m_mutex);
        m_total = m_queue.queueSize();

        auto predicate = [this, state] { return !m_running || m_currentState == state; };
        if (timeout.count() > 0)
            m_cv.wait_for(lock, timeout, predicate);
        else
            m_cv.wait(lock, predicate);
    }

    void stop()
    {
        std::scoped_lock lock(m_mutex);
        m_running = false;
        m_cv.notify_one();
    }

    void start()
    {
        std::scoped_lock lock(m_mutex);
        m_running = true;
    }

    bool isRunning()
    {
        std::scoped_lock lock(m_mutex);
        return m_running;
    }

private:
    State                   m_currentState{ State::Empty };
    std::mutex              m_mutex;
    std::condition_variable m_cv;
    bool                    m_running{ true };
    ITaskQueue&             m_queue;
    size_t                  m_total   = 0;
    size_t                  m_current = 0;
    Progress                m_progress;
};

}
