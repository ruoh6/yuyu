//
// Created by hwl on 2023.06.21.
//

#ifndef YUYU_CONFIG_H
#define YUYU_CONFIG_H

#include <memory>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <list>
#include <utility>
#include "log.h"

namespace yuyu {
class  ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(),
                       [](unsigned char c) { return std::tolower(c);});
    }
    virtual ~ConfigVarBase() {}
    const std::string& getName() const { return m_name;}
    const std::string& getDescription() const { return m_description;};

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
private:
    std::string m_name;
    std::string m_description;
};

template<class T>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar(const std::string& name, const T& default_value, const std::string& description)
        : ConfigVarBase(name, description)
        , m_val(default_value) {
    }
    std::string toString() override {
        try {
            return boost::lexical_cast<std::string>(m_val);
        } catch (std::exception& e) {
            YUYU_LOG_ERROR(YUYU_LOG_ROOT()) << "ConfigVar:toString exception" << e.what() <<
            " convert: " << typeid(m_val).name() << " to String";
        }
        return "";
    }
    bool fromString(const std::string& val) override {
        try {
            m_val = boost::lexical_cast<T>(val);
        } catch (std::exception& e) {
            YUYU_LOG_ERROR(YUYU_LOG_ROOT()) << "ConfigVar:toString exception" << e.what() <<
            " convert: to string" << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() const { return m_val;}
    void setValue(const T& v) { m_val = v;}
private:
    T m_val;
};

class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    template<class T>
    // TODO
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value
                                             , const std::string& description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "Lookup name=" << "exists";
            return tmp;
        }
        if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789")
            != std::string::npos) {
            YUYU_LOG_ERROR(YUYU_LOG_ROOT()) << "Lookup invalid name " << name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        s_datas[name] = v;
        return v;
    }

    template<class T>
    // TODO
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto it = s_datas.find(name);
        if (it == s_datas.end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static ConfigVarMap s_datas;
};

} // namespace end

#endif //YUYU_CONFIG_H
