#include "hook.h"
#include <time.h>
#include <dlfcn.h>

#include "log.h"
#include "fiber.h"
#include "iomanager.h"

yuyu::Logger::ptr g_logger = YUYU_LOG_NAME("system");

namespace yuyu {

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep)   \
    XX(usleep)  \
    XX(nanosleep)   \
    XX(socket)      \
    XX(connect)     \
    XX(accept)

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

} // extern "C" end
