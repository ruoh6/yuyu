#include "yuyu/address.h"
#include "yuyu/log.h"

static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

void test() {
    std::vector<yuyu::Address::ptr> addrs;

    YUYU_LOG_INFO(g_logger) << "begin";
    //bool v = yuyu::Address::Lookup(addrs, "localhost:3080");
    bool v = yuyu::Address::Lookup(addrs, "www.baidu.com:ftp");
    YUYU_LOG_INFO(g_logger) << "end";
    if (!v) {
        YUYU_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for (size_t i = 0; i < addrs.size(); ++i) {
        YUYU_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = yuyu::Address::LookupAny("localhost:4080");
    if (addr) {
        YUYU_LOG_INFO(g_logger) << *addr;
    } else {
        YUYU_LOG_INFO(g_logger) << "error";
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<yuyu::Address::ptr, uint32_t> > results;
    bool v = yuyu::Address::GetInterfaceAddresses(results);
    if (!v) {
        YUYU_LOG_ERROR(g_logger) << "GetInterfaceAddress fail";
        return;
    }

    for (auto& i : results) {
        YUYU_LOG_INFO(g_logger) << i.first << " - " 
            << i.second.first->toString() << " - " << i.second.second;
    }
}

void test_ipv4() {
    auto addr = yuyu::IPAddress::Create("127.0.0.8");
    if (addr) {
        YUYU_LOG_INFO(g_logger) << addr->toString();
    }
}
int main(int argc, char** argv) {
    //test_ipv4();
    //test_iface();
    test();
    return 0;
}
