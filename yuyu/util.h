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
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <cxxabi.h>
#include <execinfo.h>

#include "log.h"

namespace yuyu {

pid_t GetThreadId();
uint64_t GetFiberId();
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

std::string ToUpper(const std::string& name);
std::string ToLower(const std::string& name);
std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

class FSUtil {
public:
    static void ListAllFile(std::vector<std::string>& files, const std::string& path, const std::string& subfix);
    static std::string Dirname(const std::string& filename);
    static bool Mkdir(const std::string& dirname);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& file_name, std::ios_base::openmode mode);
    static bool Unlink(const std::string& filename, bool exist = false);
};

template<class T>
const char* TypeToName() {
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}

template<class T>
void delete_array(T* v) {
    if(v) {
        delete[] v;
    }
}

class StringUtil {
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


    static std::string WStringToString(const std::wstring& ws);
    static std::wstring StringToWString(const std::string& s);

};

std::string GetHostName();
std::string GetIPv4();
} // namespace end

#endif
