#include "yuyu/log.h"
#include "yuyu/tcp_server.h"
#include "yuyu/iomanager.h"

static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

void run() {
    auto addr = yuyu::Address::LookupAny("0.0.0.0:8033");
    //auto addr2 = yuyu::UnixAddress::ptr(new yuyu::UnixAddress("/tmp/unix_addr"));
    std::vector<yuyu::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    yuyu::TcpServer::ptr tcp_server(new yuyu::TcpServer);
    std::vector<yuyu::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
}

int main(int argc, char** argv) {
    yuyu::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
