#ifndef __YUYU__FIBER_H__
#define __YUYU__FIBER_H__

#include <functional>
#include <ucontext.h>
#include <memory>
#include "thread.h"

namespace yuyu {

class Fiber : public std::enable_shared_from_this<Fiber>
{
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State {
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXCPT
    };

private:
    Fiber();
public:
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();

    // 重置协程函数，，并重置协程状态
    void reset(std::function<void()> cb);
    // 切换到当前执行协程
    void swapIn();
    // 切换到后台执行
    void swapOut();

    uint64_t getId() const { return m_id; }
public:
    // 设置当前协程
    void SetThis(Fiber* f);
    // 返回当前执行的协程
    static Fiber::ptr GetThis();
    // 协程切换到后台，并且设置为ready状态
    static void YieldToReady();
    // 协程切换到后台，并且设置为hold状态
    static void YieldToHold();
    // 总协程数量
    static uint64_t TotalFibers();

    static void MainFunc();
    static uint64_t GetFiberId();
private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;

    ucontext_t m_ctx;
    void* m_stack = nullptr;
    std::function<void()> m_cb;
};

} //end namespace

#endif