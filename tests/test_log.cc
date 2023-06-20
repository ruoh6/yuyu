#include "../yuyu/log.h"
#include "../yuyu/util.h"

int main(int argc, char** argv) 
{
    yuyu::Logger::ptr logger(new yuyu::Logger);
    logger->addAppender(yuyu::LogAppender::ptr(new yuyu::StdoutLogAppender));
//    yuyu::LogEvent::ptr event(new yuyu::LogEvent(__FILE__, __LINE__, 0, yuyu::GetThreadId(), yuyu::GetFiberId(), time(0)));
    yuyu::FileLogAppender::ptr file_appender(new yuyu::FileLogAppender("./log.txt"));
    yuyu::LogFormatter::ptr fmt(new yuyu::LogFormatter("%d%T%m%n"));
    file_appender->setFormatter(fmt);
    logger->addAppender(file_appender);
//    event->getSS() << "yuyu";
//    logger->log(yuyu::LogLevel::DEBUG, event);

    // std::cout << "Test log" << std::endl;
    //const char* buf = "Test ";
    // const char* buf1 = "one ";
    YUYU_LOG_INFO(logger) << "test INFO macro";
    YUYU_LOG_DEBUG(logger) << "test DEBUG macro";
    YUYU_LOG_FMT_INFO(logger, "test info %s", "aa");
    //YUYU_LOG_FMT_INFO(logger, buf, buf1);
    auto l = yuyu::LoggerMgr::GetInstance()->getLogger("XX");
    YUYU_LOG_INFO(l) << "xxx";
    
    return 0;
}
