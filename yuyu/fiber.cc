#include "fiber.h"
#include "config.h"
#include "macro.h"
#include <atomic>

namespace yuyu {

static Logger::ptr g_logger = YUYU_LOG_NAME("system");
static std::atomic<uint64_t> s_fiber_id{0};
static std::atomic<uint64_t> s_fiber_count{0};
static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;
static ConfigVar<uint32_t>::ptr g_config_fiber_stacksize = 
    Config::Lookup<uint32_t>("fiber.stacksize", 1024 * 1024, "fiber stack size");


class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }
    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if (getcontext(&m_ctx)) {
        YUYU_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize) 
    : m_id(++s_fiber_count)
    , m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_config_fiber_stacksize->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) {
        YUYU_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
}

Fiber::~Fiber() {
    --s_fiber_count;
    if (m_stack) {
        YUYU_ASSERT(m_state == INIT 
                || m_state == EXCPT
                || m_state == TERM);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else { 
        YUYU_ASSERT(!m_cb); 
        YUYU_ASSERT(m_state == EXEC); 
        
        Fiber* cur = t_fiber;
        if (cur == this) {
            SetThis(nullptr);
        }
    }
}

// 重置协程函数，，并重置协程状态
void Fiber::reset(std::function<void()> cb) {
    YUYU_ASSERT(m_stack);
    YUYU_ASSERT(m_state == INIT
            || m_state == EXCPT
            || m_state == TERM);
    m_cb = cb;

    if (getcontext(&m_ctx)) {
        YUYU_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

// 切换到当前执行协程
void Fiber::swapIn() {
    SetThis(this);
    YUYU_ASSERT(m_state != EXEC);

    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        YUYU_ASSERT2(false, "swapcontext");
    }
}

// 切换到后台执行
void Fiber::swapOut() {
    SetThis(t_threadFiber.get());

    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        YUYU_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

// 返回当前执行的协程
Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }

    Fiber::ptr main_fiber(new Fiber);
    YUYU_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

// 协程切换到后台，并且设置为ready状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}

// 协程切换到后台，并且设置为hold状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
}

// 总协程数量
uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    YUYU_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch(std::exception& ex){
        cur->m_state = EXCPT;
        YUYU_LOG_ERROR(g_logger) << "Fiber Execption: " << ex.what();
    } catch(...) {
        cur->m_state = EXCPT;
        YUYU_LOG_ERROR(g_logger) << "Fiber Execption";
    }
}

} // namespace end