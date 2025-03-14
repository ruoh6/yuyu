#ifndef __YUYU_LOG_H__
#define __YUYU_LOG_H__

#include <string>
#include <stdint.h>
#include <stdarg.h>
#include <memory>
#include <list>
#include <vector>
#include <sstream>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include "util.h"
#include "singleton.h"

#define YUYU_LOG_LEVEL(logger, level) \
    if (logger->getLevel() <= level) \
        yuyu::LogEventWrap(yuyu::LogEvent::ptr(new yuyu::LogEvent(logger, level, __FILE__, __LINE__, 0, yuyu::GetThreadId(), yuyu::GetFiberId(), time(0)))).getSS()
#define YUYU_LOG_DEBUG(logger) YUYU_LOG_LEVEL(logger, yuyu::LogLevel::DEBUG)
#define YUYU_LOG_INFO(logger) YUYU_LOG_LEVEL(logger, yuyu::LogLevel::INFO)
#define YUYU_LOG_WARN(logger) YUYU_LOG_LEVEL(logger, yuyu::LogLevel::WARN)
#define YUYU_LOG_ERROR(logger) YUYU_LOG_LEVEL(logger, yuyu::LogLevel::ERROR)
#define YUYU_LOG_FATAL(logger) YUYU_LOG_LEVEL(logger, yuyu::LogLevel::FATAL)


#define YUYU_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if (logger->getLevel() <= level) \
        yuyu::LogEventWrap(yuyu::LogEvent::ptr(new yuyu::LogEvent(logger, level, __FILE__, __LINE__, 0, yuyu::GetThreadId(), yuyu::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)
#define YUYU_LOG_FMT_DEBUG(logger, fmt, ...) YUYU_LOG_FMT_LEVEL(logger, yuyu::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define YUYU_LOG_FMT_INFO(logger, fmt, ...) YUYU_LOG_FMT_LEVEL(logger, yuyu::LogLevel::INFO, fmt, __VA_ARGS__)
#define YUYU_LOG_FMT_WARN(logger, fmt, ...) YUYU_LOG_FMT_LEVEL(logger, yuyu::LogLevel::WARN, fmt, __VA_ARGS__)
#define YUYU_LOG_FMT_ERROR(logger, fmt, ...) YUYU_LOG_FMT_LEVEL(logger, yuyu::LogLevel::ERROR, fmt, __VA_ARGS__)
#define YUYU_LOG_FMT_FATAL(logger, fmt, ...) YUYU_LOG_FMT_LEVEL(logger, yuyu::LogLevel::FATAL, fmt, __VA_ARGS__)

#define YUYU_LOG_ROOT() yuyu::LoggerMgr::GetInstance()->getRoot()
#define YUYU_LOG_NAME(name) yuyu::LoggerMgr::GetInstance()->getLogger(name)

namespace yuyu {

class Logger;
class LoggerManager;

// 日志级别
class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL =5
    };
    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string& str);
};

class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time);

    const char* getFile() const { return m_file;}
    int32_t getLine() const { return m_line;} 
    uint32_t getElapse() const { return m_elapse;}
    uint32_t getThreadId() const { return m_threadId;}
    uint32_t getFiberId() const { return m_fiberId;}
    uint32_t getTime() const { return m_time;}
    const std::string& getThreadName() const { return m_threadName; }
    const std::string getContent() const { return m_ss.str();}
    std::stringstream& getSS()  { return m_ss;}
    std::shared_ptr<Logger> getLogger() { return m_logger;}
    LogLevel::Level getLevel() { return m_level;}
    void format(const char* fmt, ...);
    void formatIn(const char* fmt, va_list al);
private:
    const char* m_file = nullptr;       // 文件名
    int32_t m_line = 0;                 // 行号
    uint32_t m_elapse = 0;              // 程序启动开始到现在的毫秒数
    uint32_t m_threadId = 0;            // 线程ID
    uint32_t m_fiberId = 0;             // 协程ID
    uint64_t m_time = 0;                // 时间戳
    std::string m_threadName;           // 线程名称
    std::stringstream m_ss;             // 内容
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr event);
    ~LogEventWrap();
    LogEvent::ptr getEvent() { return m_event;}
    std::stringstream& getSS();
private:
    LogEvent::ptr m_event;
};

// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    /** 
     * @details
     *  %m 消息
     *  %p 日志级别
     *  %r 累积毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *  默认格式为：“%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n”
     */
    LogFormatter(const std::string& pattern);
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {};
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    bool isError() const { return m_error; };
    void init();
    const std::string getPattern() const { return m_pattern; }
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items; 
    bool m_error = false;
};

// 日志输出地
class LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){}
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;

    void setFormatter(LogFormatter::ptr val); 
    void setLevel(LogLevel::Level level) { m_level = level; }
    LogFormatter::ptr getFormatter() const { return m_formatter; }; 
    LogLevel::Level getLevel() const { return m_level; }
protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
    bool m_hasFormatter = false;
};


// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();
    void setLevel(LogLevel::Level val) { m_level = val;}
    LogLevel::Level getLevel() const { return m_level;} 
    const std::string getName() const { return m_name;}

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);

    LogFormatter::ptr getFormatter();
    std::string toYamlString();
private:
    std::string m_name;                     // 日志名称
    LogLevel::Level m_level;                // 日志级别
    std::list<LogAppender::ptr> m_appenders;// Appender集合
    LogFormatter::ptr m_formatter;
    Logger::ptr m_root;
};

// 输出到控制台
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
};

// 输出到文件
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
    // 重新打开文件，打开成功true，失败false
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime = 0;
};

class LoggerManager {
public:
    LoggerManager();
    void init();
    Logger::ptr getLogger(const std::string& name);
    Logger::ptr getRoot() const { return m_root;}
    std::string toYamlString();
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

// 使用单例设计模式
// 确保LoggerManager单一、全局访问（允许程序中的其他对象可以访问该单例实例）
// 延迟初始化（直到第一次需要使用对象或变量时才进行初始化）的特点
typedef yuyu::Singleton<LoggerManager> LoggerMgr;
} // namespace end

#endif
