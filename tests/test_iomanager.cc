#include "yuyu/yuyu.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

int sock = 0;

void test_fiber() {
    YUYU_LOG_INFO(g_logger) << "test_fiber";
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.70.37", &addr.sin_addr.s_addr);

    if (!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if (errno == EINPROGRESS) {
        yuyu::IOManager::GetThis()->addEvent(sock, yuyu::IOManager::READ, [](){
            YUYU_LOG_INFO(g_logger) << "read callback";
            });
        yuyu::IOManager::GetThis()->addEvent(sock, yuyu::IOManager::WRITE, [](){
            YUYU_LOG_INFO(g_logger) << "write callback";
            yuyu::IOManager::GetThis()->cancelEvent(sock, yuyu::IOManager::READ);
            close(sock);
            });
    }
}

void test1() {
    yuyu::IOManager iom(2, false, "iom");
    iom.schedule(&test_fiber);
}
int main(int argc, char** agrv){
    test1();
    return 0;
}
