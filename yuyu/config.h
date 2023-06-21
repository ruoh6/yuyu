//
// Created by hwl on 2023.06.21.
//

#ifndef YUYU_CONFIG_H
#define YUYU_CONFIG_H

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "log.h"

namespace yuyu {
class  ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description) {
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

private:
    T m_val;
};

class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value
                                             , const std::string& description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "Lookup name=" << "exists";
            return tmp;
        }
        if (name.find_first_of("abcdefghikjlmnopqrstuvwxyzABCDEFGHIKJLMNOPQRSTUVWXYZ._0123456789")
            != std::string::npos) {
            YUYU_LOG_ERROR(YUYU_LOG_ROOT()) << "Lookup invalid name" << name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        s_datas[name] = v;

    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto it = s_datas.find(name);
        if (it == s_datas.end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }
private:
    static ConfigVarMap s_datas;
};
} // namespace end

#endif //YUYU_CONFIG_H
