#pragma once
#include <co/std.hpp>
#include <co/awaiter/task.hpp>

namespace co {
struct ReturnPreviousPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() {
        throw;
    }

    void return_value(std::coroutine_handle<> previous) noexcept {
        mPrevious = previous;
    }

    auto get_return_object() {
        return std::coroutine_handle<ReturnPreviousPromise>::from_promise(
            *this);
    }

    std::coroutine_handle<> mPrevious;
    ReturnPreviousPromise &operator=(ReturnPreviousPromise &&) = delete;
};

using ReturnPreviousTask = Task<void, ReturnPreviousPromise>;
} // namespace co
