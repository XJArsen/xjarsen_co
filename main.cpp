#include <iostream>
#include "co/task.hpp"
#include "debug.hpp"
#include "co/sleep_loop.hpp"
#include "co/when_any.hpp"

using namespace co;
using namespace std::chrono_literals;

TimerLoop &getTimerLoop() {
    static TimerLoop loop;
    return loop;
}
Task<void, SleepUntilPromise> hello1() {
    debug(), "hello1开始睡1秒";
    co_await sleep_for(getTimerLoop(), 1s); 
    debug(), "hello1睡醒了";
    co_return ;
}

Task<void, SleepUntilPromise> hello2() {
    debug(), "hello2开始睡2秒";
    co_await sleep_for(getTimerLoop(), 2s); 
    debug(), "hello2睡醒了";
    co_return ;
}

Task<void, SleepUntilPromise> hello() {
    debug(), "hello开始等1和2";
    auto v = co_await when_any(hello1(), hello2());
    /* co_await hello1(); */
    /* co_await hello2(); */
    debug(), "hello看到", (int)v.index() + 1, "睡醒了";
    // co_return std::get<0>(v);
}


int main() {
    std::cout << "Hello, from co_web!\n";
    auto t = hello();
    getTimerLoop().run();
    debug(), "debug";
    debug(), "主函数中得到hello结果:", t.mCoroutine.promise().result();
    return 0;
}
