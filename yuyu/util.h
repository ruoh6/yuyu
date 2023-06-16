#ifndef __YUYU_UTIL_H__
#define __YUYU_UTIL_H__

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>

namespace yuyu {

pid_t GetThreadId();
uint32_t GetFiberId();

} // namespace end

#endif
