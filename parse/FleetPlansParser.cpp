#include "Parse.h"

#include <memory>
#include <boost/filesystem/fstream.hpp>
#include <yaml-cpp/yaml.h>
#include "../universe/FleetPlan.h"
#include "../util/Logger.h"


namespace parse {
    std::vector<std::unique_ptr<FleetPlan>> fleet_plans(const boost::filesystem::path& path) {
        std::vector<std::unique_ptr<FleetPlan>> fleet_plans_;

        YAML::Node doc;
        try {
            boost::filesystem::ifstream ifs(path);
            doc = YAML::Load(ifs);
            ifs.close();
            fleet_plans_ = doc["fleets"].as<std::vector<std::unique_ptr<FleetPlan>>>();
        }
        catch(YAML::Exception& e) {
            ErrorLogger() << "parse::fleet_plans: " << e.what();
        }

        return fleet_plans_;
    }
}

namespace YAML {
    template <>
    struct convert<std::unique_ptr<FleetPlan>> {
        static bool decode(const Node& node, std::unique_ptr<FleetPlan>& rhs);
    };

    bool convert<std::unique_ptr<FleetPlan>>::decode(const Node& node, std::unique_ptr<FleetPlan>& rhs) {
        if (!node.IsMap())
            return false;

        if (!node["name"])
            throw YAML::KeyNotFound(node.Mark(), std::string{"name"});

        if (!node["ships"])
            throw YAML::KeyNotFound(node.Mark(), std::string{"ships"});

        if (3 < node.size())
            throw YAML::RepresentationException(node.Mark(), "unexpected attributes in fleet plan aside from: [name, ships]");

        rhs = std::make_unique<FleetPlan>(
            node["name"].as<std::string>(),
            node["ships"].as<std::vector<std::string>>(),
            true
        );

        return true;
    }
}
