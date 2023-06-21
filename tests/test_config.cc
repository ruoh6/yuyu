//
// Created by hwl on 2023.06.21.
//
#include "yuyu/config.h"
#include "yuyu/log.h"
#include <yaml-cpp/yaml.h>

yuyu::ConfigVar<int>::ptr g_int_value_config =
        yuyu::Config::Lookup("system.port", (int)8080, "system port");

yuyu::ConfigVar<float>::ptr g_float_value_config =
        yuyu::Config::Lookup("system.value", (float)10.2f, "system value");
void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        YUYU_LOG_INFO(YUYU_LOG_ROOT()) << std::string(level * 4, ' ')
        << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        YUYU_LOG_INFO(YUYU_LOG_ROOT()) << std::string(level * 4, ' ')
        << "NULL - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            YUYU_LOG_INFO(YUYU_LOG_ROOT()) << std::string(level * 4, ' ')
            << it->first << " - " << node.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            YUYU_LOG_INFO(YUYU_LOG_ROOT()) << std::string(level * 4, ' ')
            << i << " - " << node.Type() << " - " << level;
            print_yaml(node[i], level +  1);
        }
    }
}
void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/hwl/sylar-yin/yuyu/bin/conf/log.yml");
    print_yaml(root, 0);
    //YUYU_LOG_INFO(YUYU_LOG_ROOT()) << root;
}
int main(int argc, char** argv) {
    // TODO
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << g_int_value_config->getValue();
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << g_int_value_config->toString();

    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << g_float_value_config->getValue();
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << g_float_value_config->toString();
    test_yaml();
    return 0;
}
