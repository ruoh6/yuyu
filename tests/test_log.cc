#include "../yuyu/log.h"

int main(int argc, char** argv) 
{
    yuyu::Logger::ptr logger(new yuyu::Logger);
    logger->addAppender(yuyu::LogAppender::ptr(new yuyu::StdoutLogAppender));
    yuyu::LogEvent::ptr event(new yuyu::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));
    logger->log(yuyu::LogLevel::DEBUG, event);

    std::cout << "Test log" << std::endl;
    return 0;
}
