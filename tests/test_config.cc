//
// Created by hwl on 2023.06.21.
//
#include "yuyu/config.h"
#include "yuyu/log.h"
#include <yaml-cpp/yaml.h>

yuyu::ConfigVar<int>::ptr g_int_value_config =
        yuyu::Config::Lookup("system.port", (int)9090, "system port");

yuyu::ConfigVar<float>::ptr g_int_valuem_config =
        yuyu::Config::Lookup("system.port", (float)9090, "system port");

yuyu::ConfigVar<float>::ptr g_float_value_config =
        yuyu::Config::Lookup("system.value", (float)10.2f, "system value");

yuyu::ConfigVar<std::vector<int> >::ptr g_vector_value_config =
        yuyu::Config::Lookup("system.int_vec", std::vector<int>{1, 3, 14}, "system value vec");

yuyu::ConfigVar<std::list<int> >::ptr g_list_value_config =
        yuyu::Config::Lookup("system.int_list", std::list<int>{1, 2, 14}, "system value list");

yuyu::ConfigVar<std::set<int> >::ptr g_set_value_config =
        yuyu::Config::Lookup("system.int_set", std::set<int>{1, 78, 14}, "system value set");

yuyu::ConfigVar<std::unordered_set<int> >::ptr g_unordered_set_value_config =
        yuyu::Config::Lookup("system.int_unordered_set", std::unordered_set<int>{5, 6, 1, 2, 14}, "system value unordered_set");

yuyu::ConfigVar<std::map<std::string, int> >::ptr g_map_value_config =
        yuyu::Config::Lookup("system.int_map", std::map<std::string, int>{ {"ke", 14} }, "system value map");

yuyu::ConfigVar<std::unordered_map<std::string, int> >::ptr g_umap_value_config =
        yuyu::Config::Lookup("system.int_umap", std::unordered_map<std::string, int>{ {"ke", 0000} }, "system value umap");

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
    YAML::Node root = YAML::LoadFile("./conf/log.yml");
    print_yaml(root, 0);
    //YUYU_LOG_INFO(YUYU_LOG_ROOT()) << root;
}

void test_config() {
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) <<"Before: " << g_int_value_config->getValue();
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) <<"Before: " << g_float_value_config->toString();

#define XX(g_var, name, prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto& i : v) { \
            YUYU_LOG_INFO(YUYU_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        YUYU_LOG_INFO(YUYU_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_MAP(g_var, name, prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto& i : v) { \
        YUYU_LOG_INFO(YUYU_LOG_ROOT()) << #prefix " " #name ": { " << i.first << " - " << i.second << " }"; \
        } \
    }

    XX(g_list_value_config, int_list, before)
    XX(g_vector_value_config, int_vec, before)
    XX(g_set_value_config, int_set, before)
    XX(g_unordered_set_value_config, int_unordered_set, before)
    XX_MAP(g_map_value_config, int_map, before)
    XX_MAP(g_umap_value_config, int_umap, before)
    YAML::Node root = YAML::LoadFile("./conf/log.yml");
    yuyu::Config::LoadFromYaml(root);

    XX(g_list_value_config, int_list, after)
    XX(g_vector_value_config, int_vec, after)
    XX(g_set_value_config, int_set, after)
    XX(g_unordered_set_value_config, int_unordered_set, after)
    XX_MAP(g_map_value_config, int_map, after)
    XX_MAP(g_umap_value_config, int_umap, after)
    
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) <<"After: " << g_map_value_config->toString();
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) <<"After: " << g_int_value_config->getValue();
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) <<"After: " << g_float_value_config->toString();

}

class Person {
public:
    Person() {};
    std::string m_name = "";
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age
           << " sex=" << m_sex << "]";
        return ss.str();
    }
    
    bool operator==(const Person& oth) const {
        return m_name == oth.m_name
            && m_age == oth.m_age
            && m_sex == oth.m_sex;
    }

};

namespace yuyu {

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator() (const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

yuyu::ConfigVar<Person>::ptr g_person =
        yuyu::Config::Lookup("class.person", Person(), "system person");

yuyu::ConfigVar<std::map<std::string, Person> >::ptr g_map_person =
        yuyu::Config::Lookup("class.map", std::map<std::string, Person>(), "system map");

yuyu::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_map_vec_person =
        yuyu::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person> >(), "system map");

void test_class(){
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();

#define XX_MP(g_var, prefix) \
    { \
        auto m = g_var->getValue(); \
        for (auto& i : m) { \
            YUYU_LOG_INFO(YUYU_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
        YUYU_LOG_INFO(YUYU_LOG_ROOT()) << prefix << ": size=" << m.size(); \
    }

    g_person->addListener(10, [](const Person& old_value, const Person& new_value) {
        YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "old_value=" << old_value.toString()
            << " new_value=" << new_value.toString();
    });

    XX_MP(g_map_person, "class.map before");
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "before: " << g_map_vec_person->toString();
    YAML::Node root = YAML::LoadFile("./conf/log.yml");
    yuyu::Config::LoadFromYaml(root);

    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "after: " << g_map_person->toString(); 
    XX_MP(g_map_person, "class.map after");
    YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "after: " << g_map_vec_person->toString();
}

int main(int argc, char** argv) {
    // TODO
    // test_yaml();
    // test_config();
    test_class();
    return 0;
}
