#pragma once

#include <co/std.hpp>
#include <co/awaiter/task.hpp>

#if defined(__unix__) && __has_include(<cxxabi.h>)
# include <cxxabi.h>
#endif

namespace co {
template <class FinalAwaiter = std::suspend_always>
struct IgnoreReturnPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return FinalAwaiter();
    }

    void unhandled_exception() noexcept {
        try {
            throw;
        } catch (std::exception const &e) {
            auto name = typeid(e).name();
#if defined(__unix__) && __has_include(<cxxabi.h>)
            int status;
            char *p = abi::__cxa_demangle(name, 0, 0, &status);
            std::string s = p ? p : name;
            std::free(p);
#else
            std::string s = name;
#endif
            std::cerr
                << "co_spawn coroutine terminated after thrown exception '" +
                       s + "'\n  e.what(): " + std::string(e.what()) + "\n";
        } catch (...) {
            std::cerr
                << "co_spawn coroutine terminated after thrown exception\n";
        }
    }

    void result() noexcept {}

    void return_void() noexcept {}

    auto get_return_object() {
        return std::coroutine_handle<IgnoreReturnPromise>::from_promise(*this);
    }

    IgnoreReturnPromise &operator=(IgnoreReturnPromise &&) = delete;

    [[maybe_unused]] TaskAwaiter<void> *mAwaiter{};
};

struct AutoDestroyFinalAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        coroutine.destroy();
    }

    void await_resume() const noexcept {}
};
} // namespace co
