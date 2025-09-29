#include "yuyu/yuyu.h"

static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

void test_fiber() {
    static int s_count = 4;
    YUYU_LOG_INFO(g_logger) << "s_count=" << s_count;
    sleep(1);
    if (--s_count >= 0) {
        yuyu::Scheduler::GetThis()->schedule(&test_fiber, yuyu::GetThreadId());
    }
}
int main(int argc, char** argv)
{
    YUYU_LOG_INFO(g_logger) << "main";
    //yuyu::Scheduler sc(4, false, "test");
    yuyu::Scheduler sc;
    sc.start();
    YUYU_LOG_INFO(g_logger) << "stop";
    sc.schedule(&test_fiber);
    sc.stop();
    YUYU_LOG_INFO(g_logger) << "over";
    return 0;
}
