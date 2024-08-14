#pragma once

#include <co/std.hpp>
#include <filesystem>
#include <unistd.h>
#include <fcntl.h>
#include "co/awaiter/task.hpp"
#include "co/epoll_loop.hpp"
#include "co/platform/error_handle.hpp"

namespace co {

enum class OpenMode : int {
    Read = O_RDONLY,
    Write = O_WRONLY | O_TRUNC | O_CREAT,
    ReadWrite = O_RDWR | O_CREAT,
    Append = O_WRONLY | O_APPEND | O_CREAT,
};

inline Task<AsyncFile> open_fs_file(EpollLoop &loop, std::filesystem::path path, OpenMode mode, mode_t access = 0644) {
    int oflags = (int)mode;
    /* oflags |= O_NONBLOCK; */ // TODO: nonblockfsfilesviaiouring!
    int res = checkError(open(path.c_str(), oflags, access));
    AsyncFile file(res);
    co_return file;
}

}
