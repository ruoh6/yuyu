#include "yuyu/yuyu.h"

yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

int count = 0;
//yuyu::RWMutex s_mutex;
yuyu::Mutex s_mutex;

void fun1(){
    YUYU_LOG_INFO(g_logger) << "name: " << yuyu::Thread::GetName()
                            << " this.name: " << yuyu::Thread::GetThis()->getName()
                            << " id: " << yuyu::GetThreadId()
                            << " this.id: " << yuyu::Thread::GetThis()->getId();
    for (int i = 0; i < 1000000; i++) {
        //yuyu::RWMutex::ReadLock lock(s_mutex);
        yuyu::Mutex::Lock lock(s_mutex);
        count++;
    }
}

void fun2(){
    while(true) 
        YUYU_LOG_INFO(g_logger) << "*****************************************";
}

void fun3(){
    while(true) 
        YUYU_LOG_INFO(g_logger) << "=========================================";
}

int main(int argc, char** argv) {
    YUYU_LOG_INFO(g_logger) << "Thread test begin";
    YAML::Node root = YAML::LoadFile("/home/hwl/yuyu/bin/conf/log2.yml");
    yuyu::Config::LoadFromYaml(root);
    std::vector<yuyu::Thread::ptr> thrs;
    for (int i = 0; i < 2; i++) {
        yuyu::Thread::ptr thr(new yuyu::Thread(&fun2, "name_" + std::to_string(i * 2)));
        yuyu::Thread::ptr thr2(new yuyu::Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for (int i = 0; i < 5; i++) {
        thrs[i]->join();
    }

    YUYU_LOG_INFO(g_logger) << "Thread test end";
    YUYU_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}
