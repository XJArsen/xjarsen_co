#include "epoll_loop.hpp"

namespace co {

bool EpollLoop::addListener(EpollFilePromise &promise, int ctl) {
    struct epoll_event event;
    event.events = promise.mAwaiter->mEvents;
    event.data.ptr = &promise;
    int res = epoll_ctl(mEpoll, ctl, promise.mAwaiter->mFileNo, &event);
    if (res == -1) {
        return false;
    }
    if (ctl == EPOLL_CTL_ADD) {
        ++mCount;
    }
    return true;
}

void EpollLoop::removeListener(int fileNo) {
    checkError(epoll_ctl(mEpoll, EPOLL_CTL_DEL, fileNo, NULL));
}

bool EpollLoop::run(
    std::optional<std::chrono::system_clock::duration> timeout) {
    if (mCount == 0) {
        return false;
    }
    int timeoutInMs = -1;
    if (timeout) {
        timeoutInMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(*timeout)
                .count();
    }

    int res = checkError(
        epoll_wait(mEpoll, mEventBuf, std::size(mEventBuf), timeoutInMs));
    for (int i = 0; i < res; i++) {
        auto &event = mEventBuf[i];
        auto &promise = *(EpollFilePromise *)event.data.ptr;
        promise.mAwaiter->mResumeEvents = event.events;
    }
    for (int i = 0; i < res; i++) {
        auto &event = mEventBuf[i];
        auto &promise = *(EpollFilePromise *)event.data.ptr;
        std::coroutine_handle<EpollFilePromise>::from_promise(promise).resume();
    }
    return true;
}

}
