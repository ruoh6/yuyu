#ifndef __YUYU_MACRO_H__
#define __YUYU_MACRO_H__

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

#if defined __GNUC__ || defined __llvm__
#   define YUYU_LIKELY(x)       __builtin_expect(!!(x), 1);
#   define YUYU_UNLIKELY(x)     __builtin_expect(!!(x), 0);
#else
#   define YUYU_LIKELY(x)   (x)
#   define YUYU_UNLIKELY(x) (x)
#endif

#define YUYU_ASSERT(x) \
    if (YUYU_UNLIKELY(!(x))) { \
        YUYU_LOG_ERROR(YUYU_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << yuyu::BacktraceToString(100, 2, "   "); \
        assert(x); \
    }

#define YUYU_ASSERT2(x, w) \
    if (YUYU_UNLIKELY(!(x))) { \
        YUYU_LOG_ERROR(YUYU_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << yuyu::BacktraceToString(100, 2, "   "); \
        assert(x); \
    }

#endif
