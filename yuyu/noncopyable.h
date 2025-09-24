#ifndef __YUYU_NONCOPYABLE_H__
#define __YUYU_NONCOPYABLE_H__

namespace yuyu {

class Noncopyable
{
public:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};

} // namespace end
#endif
