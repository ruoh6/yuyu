#ifndef __YUYU_ENDIAN_H__
#define __YUYU_ENDIAN_H__

#define YUYU_LITTLE_ENDIAN 1
#define YUYU_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace yuyu {

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define YUYU_BYTE_ORDER YUYU_BIG_ENDIAN
#else
#define YUYU_BYTE_ORDER YUYU_LITTLE_ENDIAN
#endif

#if YUYU_BYTE_ORDER == YUYU_BIG_ENDIAN

template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else

template<class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

} // namespace end
#endif
