#include "../yuyu/log.h"
#include "../yuyu/util.h"

int main(int argc, char** argv) 
{
    yuyu::Logger::ptr logger(new yuyu::Logger);
    logger->addAppender(yuyu::LogAppender::ptr(new yuyu::StdoutLogAppender));
//    yuyu::LogEvent::ptr event(new yuyu::LogEvent(__FILE__, __LINE__, 0, yuyu::GetThreadId(), yuyu::GetFiberId(), time(0)));
//    event->getSS() << "yuyu";
//    logger->log(yuyu::LogLevel::DEBUG, event);

    // std::cout << "Test log" << std::endl;
    YUYU_LOG_INFO(logger) << "test INFO macro";
    YUYU_LOG_DEBUG(logger) << "test DEBUG macro";
    return 0;
}
