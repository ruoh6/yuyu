#include "util.h"

namespace yuyu {
pid_t GetThreadId() {
    return syscall(SYS_gettid);
}


uint32_t GetFiberId() {
    return 0;
}

} // namespace end
