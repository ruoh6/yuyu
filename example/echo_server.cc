#include "yuyu/tcp_server.h"
#include "yuyu/log.h"
#include "yuyu/iomanager.h"
#include "yuyu/bytearray.h"

static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

class EchoServer : public yuyu::TcpServer {
public:
    EchoServer(int type);
    void handleClient(yuyu::Socket::ptr client);
    
private:
    int m_type = 0;
};

int type = 1;

EchoServer::EchoServer(int type) : m_type(type) {}

void EchoServer::handleClient(yuyu::Socket::ptr client) {
    YUYU_LOG_INFO(g_logger) << "handleClient" << *client;
    yuyu::ByteArray::ptr ba(new yuyu::ByteArray);
    while (true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        YUYU_LOG_INFO(g_logger) << "recv result: rt=" << rt
            << " errno=" << errno << " errstr=" << strerror(errno);
        if (rt == 0) {
            YUYU_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if (rt < 0) {
            YUYU_LOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }

        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);

        // echo back to client
        std::vector<iovec> read_iovs;
        ba->getReadBuffers(read_iovs, rt);
        YUYU_LOG_INFO(g_logger) << "sending data: rt=" << rt
            << " iovs_size=" << read_iovs.size()
            << " data=[" << ba->toString() << "]";
        int send_rt = client->send(&read_iovs[0], read_iovs.size());
        YUYU_LOG_INFO(g_logger) << "send result: send_rt=" << send_rt
            << " errno=" << errno << " errstr=" << strerror(errno);
        if (send_rt <= 0) {
            break;
        }

        if (m_type == 1) {
            std::cout << ba->toString();
        } else {
            std::cout << ba->toHexString();
        }
        std::cout.flush();
    }
}

void run() {
    YUYU_LOG_INFO(g_logger) << "server type=" << type;
    EchoServer::ptr es(new EchoServer(type));
    auto addr = yuyu::Address::LookupAny("0.0.0.0:8020");
    while(!es->bind(addr)) {
        sleep(2);
    }
    es->start();
};

int main(int argc, char** argv) {
    if (argc < 2) {
        YUYU_LOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if (!strcmp(argv[1], "-b")) {
        type = 2;
    }

    yuyu::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
