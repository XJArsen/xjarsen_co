#include "co/awaiter/task.hpp"
#include "co/epoll_loop.hpp"
#include "co/sleep_loop.hpp"
#include "co/std.hpp"
#include "debug.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "co/std.hpp"
using namespace co;
using namespace std::chrono_literals;

co::EpollLoop epollLoop;
co::TimerLoop timerLoop;

co::Task<std::string> read_string(co::AsyncFile &file) {
    co_await wait_file_event(epollLoop, file, EPOLLIN);
    std::string s;
    size_t chunk = 8;
    while (true) {
        char c;
        std::size_t exist = s.size();
        s.resize(exist + chunk);
        std::span<char> buffer(s.data() + exist, chunk);
        auto len = readFileSync(file, buffer);
        if (len != chunk) {
            s.resize(exist + len);
            break;
        }
        if (chunk < 65536) {
            chunk *= 4;
        }
    }
    co_return s;
}

co::Task<void> async_main() {
    co::AsyncFile file(STDIN_FILENO);
    while (true) {
        auto s = co_await read_string(file);
        debug(), "读到了", s;
        if (s == "quit\n") {
            break;
        }
    }
}

int main() {
    int attr = 1;
    ioctl(0, FIONBIO, &attr);

    auto t = async_main();
    t.mCoroutine.resume();
    while (!t.mCoroutine.done()) {
        auto timeout = timerLoop.run();
        epollLoop.run(timeout);
    }
    return 0;
}
