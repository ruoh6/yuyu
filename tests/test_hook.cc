#include "yuyu/hook.h"
#include "yuyu/log.h"
#include "yuyu/iomanager.h"

yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

void test_sleep() {
    yuyu::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        YUYU_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        YUYU_LOG_INFO(g_logger) << "sleep 3";
    });
    YUYU_LOG_INFO(g_logger) << "test_sleep";
}

int main(int argc, char** argv) {
    test_sleep();
    return 0;
}
