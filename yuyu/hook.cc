#include "hook.h"
#include <time.h>
#include <dlfcn.h>
#include <fcntl.h>

#include "log.h"
#include "fiber.h"
#include "iomanager.h"
#include "fd_manager.h"

yuyu::Logger::ptr g_logger = YUYU_LOG_NAME("system");

namespace yuyu {

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep)   \
    XX(usleep)  \
    XX(nanosleep)   \
    XX(socket)      \
    XX(connect)     \
    XX(accept)      \
    XX(fcntl)

void hook_init() {
    static bool is_inited = false;
    if (is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}
struct _HookIniter {
    _HookIniter() {
        hook_init();
    }
};
static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
}
void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

} // namespace end


extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

// sleep
unsigned int sleep(unsigned int seconds) {
    if (!yuyu::t_hook_enable) {
        return sleep_f(seconds);
    }

    yuyu::Fiber::ptr fiber = yuyu::Fiber::GetThis();
    yuyu::IOManager* iom = yuyu::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(yuyu::Scheduler::*)
                (yuyu::Fiber::ptr, int thread))&yuyu::IOManager::schedule
                ,iom, fiber, -1));
    yuyu::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if (!yuyu::t_hook_enable) {
        return usleep_f(usec);
    }

    yuyu::Fiber::ptr fiber = yuyu::Fiber::GetThis();
    yuyu::IOManager* iom = yuyu::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(yuyu::Scheduler::*)
                (yuyu::Fiber::ptr, int thread))&yuyu::IOManager::schedule
                ,iom, fiber, -1));
    yuyu::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec* req, struct timespec* rem) {
    if  (!yuyu::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
    yuyu::Fiber::ptr fiber = yuyu::Fiber::GetThis();
    yuyu::IOManager* iom = yuyu::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(yuyu::Scheduler::*)
                (yuyu::Fiber::ptr, int thread))&yuyu::IOManager::schedule
                ,iom, fiber, -1));
    yuyu::Fiber::YieldToHold();
    return 0;
}

// socket
int socket(int domain, int type, int protocol) {
    if (!yuyu::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if (fd == -1) {
        return fd;
    }
    // fd manager process
    return 0;
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return 0;
}

int accept(int s, struct sockaddr* addr, socklen_t* addrlen) {
    return 0;
}

// control
int fcntl(int fd, int cmd, ... /* arg */ ) {
        va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                yuyu::FdCtx::ptr ctx = yuyu::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                yuyu::FdCtx::ptr ctx = yuyu::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    } 
}
} // extern "C" end
