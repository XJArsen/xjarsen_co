#pragma once

#include <coroutine>
#include <chrono>
#include <cstdint>
#include <utility>
#include <optional>
#include <string_view>
#include <span>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include "awaiter/task.hpp"
#include "co/platform/error_handle.hpp"

namespace co {

using EpollEventMask = std::uint32_t;

struct EpollFilePromise : Promise<uint32_t> {
    auto get_return_object() {
        return std::coroutine_handle<EpollFilePromise>::from_promise(*this);
    }

    inline ~EpollFilePromise();

    struct EpollFileAwaiter *mAwaiter;
};

struct EpollLoop {
    bool addListener(EpollFilePromise &promise, int ctl);
    void removeListener(int fileNo);
    bool run(std::optional<std::chrono::system_clock::duration> timeout);

    bool hasEvent() const noexcept {
        return mCount != 0;
    }

    EpollLoop &operator=(EpollLoop &&) = delete;

    ~EpollLoop() {
        close(mEpoll);
    }

    int mEpoll = checkError(epoll_create1(0));
    std::size_t mCount = 0;
    struct epoll_event mEventBuf[64];
};

struct EpollFileAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<EpollFilePromise> coroutine) {
        auto &promise = coroutine.promise();
        promise.mAwaiter = this;
        mLoop.addListener(promise, mCtlCode);
    }

    uint32_t await_resume() const noexcept {
        return mResumeEvents;
    }

    using ClockType = std::chrono::system_clock;

    EpollLoop &mLoop;
    int mFileNo;
    uint32_t mEvents;
    uint32_t mResumeEvents;
    int mCtlCode = EPOLL_CTL_ADD;
};

EpollFilePromise::~EpollFilePromise() {
    if (mAwaiter) {
        mAwaiter->mLoop.removeListener(mAwaiter->mFileNo);
    }
}


struct AsyncFile {
    AsyncFile() : mFileNo(-1) {};

    explicit AsyncFile(int fileNo) noexcept : mFileNo(fileNo) {}

    AsyncFile(AsyncFile &&that) noexcept : mFileNo(that.mFileNo) {
        that.mFileNo = -1;
    }

    AsyncFile &operator=(AsyncFile &&that) noexcept {
        std::swap(mFileNo, that.mFileNo);
        return *this;
    }

    ~AsyncFile() {
        if (mFileNo != -1) {
            close(mFileNo);
        }
    }

    int fileNo() const noexcept {
        return mFileNo;
    }

    int releaseOwnership() noexcept {
        int ret = mFileNo;
        mFileNo = -1;
        return ret;
    }

    void setNonblock() const {
        int attr = 1;
        checkError(ioctl(fileNo(), FIONBIO, &attr));
    }

private:
    int mFileNo;
};

inline Task<EpollEventMask, EpollFilePromise>
wait_file_event(EpollLoop &loop, AsyncFile &file, EpollEventMask events) {
    co_return co_await EpollFileAwaiter(loop, file.fileNo(), events);
}

inline std::size_t readFileSync(AsyncFile &file, std::span<char> buffer) {
    return checkErrorNonBlock(
        read(file.fileNo(), buffer.data(), buffer.size()));
}

inline std::size_t writeFileSync(AsyncFile &file,
                                 std::span<char const> buffer) {
    return checkErrorNonBlock(
        write(file.fileNo(), buffer.data(), buffer.size()));
}

inline Task<std::size_t> read_file(EpollLoop &loop, AsyncFile &file,
                                   std::span<char> buffer) {
    co_await wait_file_event(loop, file, EPOLLIN | EPOLLRDHUP);
    auto len = readFileSync(file, buffer);
    co_return len;
}

inline Task<std::size_t> write_file(EpollLoop &loop, AsyncFile &file,
                                    std::span<char const> buffer) {
    co_await wait_file_event(loop, file, EPOLLOUT | EPOLLHUP);
    auto len = writeFileSync(file, buffer);
    co_return len;
}

} // namespace co
