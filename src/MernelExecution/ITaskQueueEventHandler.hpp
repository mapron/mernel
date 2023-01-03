/*
 * Copyright (C) 2023 Smirnov Vladimir / mapron1@gmail.com
 * SPDX-License-Identifier: MIT
 * See LICENSE file for details.
 */
#pragma once

namespace Mernel {

class ITaskQueueEventHandler {
public:
    virtual ~ITaskQueueEventHandler() = default;

    enum class State
    {
        Empty,
        Ready,
    };

    virtual void handleChangedState(State state) = 0;
    virtual void handleProgress()                = 0;
};

}
