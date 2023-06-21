//
// Created by hwl on 2023.06.21.
//
#include "yuyu/config.h"
#include "yuyu/log.h"

yuyu::ConfigVar<int>::ptr g_int_value_config =
        yuyu::Config::Lookup("system.port", (int)8080, "system port");

yuyu::ConfigVar<float>::ptr g_float_value_config =
        yuyu::Config::Lookup("system.value", (float)10.2f, "system value");
int main(int argc, char** argv) {
    // TODO
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << g_int_value_config->getValue();
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << g_int_value_config->toString();

    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << g_float_value_config->getValue();
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << g_float_value_config->toString();
    return 0;
}