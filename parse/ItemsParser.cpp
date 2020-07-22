#include "Parse.h"

#include <boost/filesystem/fstream.hpp>
#include <yaml-cpp/yaml.h>
#include "../universe/UnlockableItem.h"
#include "../util/Logger.h"


namespace parse {
    std::vector<UnlockableItem> items(const boost::filesystem::path& path) {
        std::vector<UnlockableItem> items_;

        YAML::Node doc;
        try {
            boost::filesystem::ifstream ifs(path);
            doc = YAML::Load(ifs);
            ifs.close();
        }
        catch(YAML::Exception& e) {
            ErrorLogger() << "parse::items: " << e.what();
            return items_;
        }

        if (doc["buildingtypes"])
            for (const auto& building_node : doc["buildingtypes"])
                items_.emplace_back(UIT_BUILDING, building_node.as<std::string>());
        if (doc["shiphulls"])
            for (const auto& shiphull_node : doc["shiphulls"])
                items_.emplace_back(UIT_SHIP_HULL, shiphull_node.as<std::string>());
        if (doc["shipparts"])
            for (const auto& shippart_node : doc["shipparts"])
                items_.emplace_back(UIT_SHIP_PART, shippart_node.as<std::string>());
        if (doc["techs"])
            for (const auto& tech_node : doc["techs"])
                items_.emplace_back(UIT_TECH, tech_node.as<std::string>());
        if (doc["policies"])
            for (const auto& tech_node : doc["policies"])
                items_.emplace_back(UIT_POLICY, tech_node.as<std::string>());

        return items_;
    }

    std::vector<UnlockableItem> starting_buildings(const boost::filesystem::path& path) {
        std::vector<UnlockableItem> starting_buildings_;

        YAML::Node doc;
        try {
            boost::filesystem::ifstream ifs(path);
            doc = YAML::Load(ifs);
            ifs.close();
        }
        catch(YAML::Exception& e) {
            ErrorLogger() << "parse::starting_buildings: " << e.what();
            return starting_buildings_;
        }

        if (!doc["buildings"]) {
            ErrorLogger() << "parse::parse::starting_buildings: buildings key not found, skipping.";
            return starting_buildings_;
        }

        for (const auto& building_node : doc["buildings"])
            starting_buildings_.emplace_back(UIT_BUILDING, building_node.as<std::string>());

        return starting_buildings_;
    }
}
