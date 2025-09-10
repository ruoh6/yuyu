#include "yuyu/yuyu.h"

static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

void running_in_fiber() {
    YUYU_LOG_INFO(g_logger) << "running_in_fiber begin";
    yuyu::Fiber::YieldToHold();
    YUYU_LOG_INFO(g_logger) << "running_in_fiber end";
    yuyu::Fiber::YieldToHold();

}

void test_fiber() {
    YUYU_LOG_INFO(g_logger) << "main begin -1";
    {
        yuyu::Fiber::GetThis();
        YUYU_LOG_INFO(g_logger) << "main begin";
        yuyu::Fiber::ptr fiber(new yuyu::Fiber(running_in_fiber));
        fiber->swapIn();
        YUYU_LOG_INFO(g_logger) << "main after swap in";
        fiber->swapIn();
        YUYU_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    YUYU_LOG_INFO(g_logger) << "main end -1";
}

int main(int argc, char** agrv)
{
    yuyu::Thread::SetName("main");

    std::vector<yuyu::Thread::ptr> thrs;
    for (int i = 0; i < 3;i++) {
        thrs.push_back(yuyu::Thread::ptr(
                    new yuyu::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for (auto i : thrs) {
        i->join();
    }
    //test_fiber();
    return 0;
}
