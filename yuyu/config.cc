//
// Created by hwl on 2023.06.21.
//

#include "config.h"

namespace yuyu {

static yuyu::Logger::ptr g_logger = YUYU_LOG_NAME("system");

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    //YUYU_LOG_INFO(YUYU_LOG_ROOT()) << "name = " << name;
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

static void ListAllMember(const std::string& prefix,
                          const YAML::Node& node,
                          std::list<std::pair<std::string, const YAML::Node> >& output) {
    if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos) {
        YUYU_LOG_ERROR(YUYU_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
        return;
    }
    // TODO: NEED FIX THIS BUG
    output.push_back(std::make_pair(prefix, node));
    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end() ; ++it) {
             ListAllMember(prefix.empty() ? it->first.Scalar()
             : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }
}
// TODO
// A:
//      B:10
//      C: str
void Config::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node> > all_nodes;
    ListAllMember("", root, all_nodes);

    for (auto& i : all_nodes) {
        std::string key = i.first;
        if (key.empty()) {
            continue;
        }
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var = LookupBase(key);
        if (var) {
            if (i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }

}

} // namespace end
