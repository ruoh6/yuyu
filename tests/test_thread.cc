#include "yuyu/yuyu.h"

yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

int count = 0;

void fun1(){
    YUYU_LOG_INFO(g_logger) << "name: " << yuyu::Thread::GetName()
                            << " this.name: " << yuyu::Thread::GetThis()->getName()
                            << " id: " << yuyu::GetThreadId()
                            << " this.id: " << yuyu::Thread::GetThis()->getId();
    for (int i = 0; i < 100; i++) {
        count++;
    }
}

void fun2(){

}

int main(int argc, char** argv) {
    YUYU_LOG_INFO(g_logger) << "Thread test begin";
    std::vector<yuyu::Thread::ptr> thrs;
    for (int i = 0; i < 5; i++) {
        yuyu::Thread::ptr thr(new yuyu::Thread(&fun1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for (int i = 0; i < 5; i++) {
        thrs[i]->join();
    }

    YUYU_LOG_INFO(g_logger) << "Thread test end";
    YUYU_LOG_INFO(g_logger) << "count=" << count;
    return 0;
}
