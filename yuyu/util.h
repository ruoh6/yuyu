#ifndef __YUYU_UTIL_H__
#define __YUYU_UTIL_H__

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <cstddef>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <cxxabi.h>
#include <execinfo.h>

#include "log.h"

namespace yuyu {

pid_t GetThreadId();
uint32_t GetFiberId();
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");


class FSUtil {
public:
    static void ListAllFile(std::vector<std::string>& files, const std::string& path, const std::string& subfix);
    static std::string Dirname(const std::string& filename);
    static bool Mkdir(const std::string& dirname);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& file_name, std::ios_base::openmode mode);
};

template<class T>
const char* TypeToName() {
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}

} // namespace end

#endif
