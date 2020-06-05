#include "Parse.h"

#include <boost/filesystem/fstream.hpp>
#include <yaml-cpp/yaml.h>
#include "ParseImpl.h"
#include "../universe/Condition.h"
#include "../universe/FleetPlan.h"


namespace parse {
    std::vector<std::unique_ptr<MonsterFleetPlan>> monster_fleet_plans(const boost::filesystem::path& path) {
        std::vector<std::unique_ptr<MonsterFleetPlan>> monster_fleet_plans_;

        YAML::Node doc;
        try {
            boost::filesystem::ifstream ifs(path);
            doc = YAML::Load(ifs);
            ifs.close();
        }
        catch(YAML::Exception& e) {
            ErrorLogger() << "parse::monster_fleet_plans: " << e.what();
            return monster_fleet_plans_;
        }

        if (!doc["monsters"]) {
            ErrorLogger() << "parse::monster_fleet_plans: monsters key not found, skipping.";
            return monster_fleet_plans_;
        }

        for (const auto& monster_fleet_node : doc["monsters"]) {
            monster_fleet_plans_.push_back(std::make_unique<MonsterFleetPlan>(
                monster_fleet_node["name"].as<std::string>(),
                monster_fleet_node["ships"].as<std::vector<std::string>>(),
                monster_fleet_node["spawnrate"].as<double>(1.0),
                monster_fleet_node["spawnlimit"].as<int>(9999),
                monster_fleet_node["location"].as<std::unique_ptr<Condition::Condition>>()
            ));
        }

        return monster_fleet_plans_;
    }
}
