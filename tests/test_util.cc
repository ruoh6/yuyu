#include "yuyu/yuyu.h"

yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

void test_assert() {
    YUYU_LOG_INFO(g_logger) << yuyu::BacktraceToString(10);
    //assert(0);
    YUYU_ASSERT2(0 == 1, "abcsxx");
}
int main(int argc, char** argv) {
    test_assert();
    return 0;
}
