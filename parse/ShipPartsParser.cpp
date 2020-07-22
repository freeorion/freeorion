#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include "Parse.h"
#include "ParseImpl.h"
#include "../universe/ConditionAll.h"
#include "../universe/Effect.h"
#include "../universe/Enums.h"
#include "../universe/ShipPart.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"
#include <yaml-cpp/yaml.h>


namespace YAML {
    template <>
    struct convert<ShipSlotType> {
        static bool decode(const Node& node, ShipSlotType& rhs);
    };

    template <>
    struct convert<ShipPartClass> {
        static bool decode(const Node& node, ShipPartClass& rhs);
    };

    template <>
    struct convert<MeterType> {
        static bool decode(const Node& node, MeterType& rhs);
    };

    bool convert<ShipSlotType>::decode(const Node& node, ShipSlotType& rhs) {
        if (!node.IsScalar())
            return false;

        try {
            // TODO: maybe fixme
            rhs = boost::lexical_cast<ShipSlotType>("SL_" + boost::to_upper_copy(node.Scalar()));
        }
        catch(const boost::bad_lexical_cast&) {
            return false;
        }

        if (GG::EnumMap<ShipSlotType>::BAD_VALUE == rhs)
            return false;

        return true;
    }

    bool convert<ShipPartClass>::decode(const Node& node, ShipPartClass& rhs) {
        if (!node.IsScalar())
            return false;

        try {
            // TODO: maybe fixme
            std::string class_str{boost::to_upper_copy(node.Scalar())};
            if ("FIGHTERBAY" == class_str) {
                rhs = PC_FIGHTER_BAY;
                return true;
            }
            if ("FIGHTERHANGAR" == class_str) {
                rhs = PC_FIGHTER_HANGAR;
                return true;
            }
            if ("SHORTRANGE" == class_str) {
                rhs = PC_DIRECT_WEAPON;
                return true;
            }
            rhs = boost::lexical_cast<ShipPartClass>("PC_" + class_str);
        }
        catch(const boost::bad_lexical_cast&) {
            return false;
        }

        if (GG::EnumMap<ShipPartClass>::BAD_VALUE == rhs)
            return false;

        return true;
    }

    bool convert<MeterType>::decode(const Node& node, MeterType& rhs) {
        if (!node.IsScalar())
            return false;

        try {
            // TODO: maybe fixme
            rhs = boost::lexical_cast<MeterType>("METER_" + boost::to_upper_copy(node.Scalar()));
        }
        catch(const boost::bad_lexical_cast&) {
            return false;
        }

        if (GG::EnumMap<MeterType>::BAD_VALUE == rhs)
            return false;

        return true;
    }

}


namespace parse {
    std::map<std::string, std::unique_ptr<ShipPart>> ship_parts(const boost::filesystem::path& path) {
        const lexer lexer;
        std::map<std::string, std::unique_ptr<ShipPart>> parts;

        for (const auto& file_path : ListDir(path, IsYAML)) {
            YAML::Node doc;

            try {
                boost::filesystem::ifstream ifs(file_path);
                doc = YAML::Load(ifs);
                ifs.close();

                if (!doc["ship_parts"])
                    continue;

                resolve_macro_includes(doc, file_path);

                auto macros = doc["macros"].as<std::map<std::string, std::string>>(std::map<std::string, std::string>{});
                preprocess_macros(doc, file_path, macros);

                for (const auto& ship_part_node : doc["ship_parts"]) {
                    std::unique_ptr<Condition::Condition> location{std::make_unique<Condition::All>()};
                    std::unique_ptr<Condition::Condition> enqueue_location{std::make_unique<Condition::All>()};
                    std::unique_ptr<Condition::Condition> combat_targets{};
                    std::vector<std::unique_ptr<Effect::EffectsGroup>> effects{};
                    ConsumptionMap<MeterType> consumption;
                    ConsumptionMap<std::string> buildconsumption;

                    if (ship_part_node["location"])
                        location = ship_part_node["location"].as<std::unique_ptr<Condition::Condition>>();

                    if (ship_part_node["enqueue_location"])
                        enqueue_location = ship_part_node["enqueue_location"].as<std::unique_ptr<Condition::Condition>>();

                    if (ship_part_node["combat_targets"])
                        combat_targets = ship_part_node["combat_targets"].as<std::unique_ptr<Condition::Condition>>();
                    if (ship_part_node["effects"])
                        effects = ship_part_node["effects"].as<std::vector<std::unique_ptr<Effect::EffectsGroup>>>();
                    if (ship_part_node["consumption"])
                        consumption = ship_part_node["consumption"].as<ConsumptionMap<MeterType>>();
                    if (ship_part_node["buildconsumption"])
                        buildconsumption = ship_part_node["buildconsumption"].as<ConsumptionMap<std::string>>();

                    CommonParams common_params{
                        ship_part_node["buildcost"].as<std::unique_ptr<ValueRef::ValueRef<double>>>(),
                        ship_part_node["buildtime"].as<std::unique_ptr<ValueRef::ValueRef<int>>>(),
                        ship_part_node["producible"].as<bool>(true),
                        ship_part_node["tags"].as<std::set<std::string>>(std::set<std::string>{}),
                        std::move(location),
                        std::move(effects),
                        std::move(consumption),
                        std::move(buildconsumption),
                        std::move(enqueue_location)
                    };

                    auto part_class = ship_part_node["class"].as<ShipPartClass>();
                    std::string primary_stat{"capacity"}; // damage
                    std::string secondary_stat{"damage"}; // shots

                    auto ship_part = std::make_unique<ShipPart>(
                        part_class,
                        ship_part_node[primary_stat].as<double>(0.0),
                        ship_part_node[secondary_stat].as<double>(1.0),
                        common_params,
                        ship_part_node["name"].as<std::string>(),
                        ship_part_node["description"].as<std::string>(),
                        ship_part_node["exclusions"].as<std::set<std::string>>(std::set<std::string>{}),
                        ship_part_node["slot_types"].as<std::vector<ShipSlotType>>(),
                        ship_part_node["icon"].as<std::string>(),
                        !ship_part_node["no_default_capacity_effect"].as<bool>(false),
                        std::move(combat_targets));

                    parts.insert(std::make_pair(ship_part->Name(), std::move(ship_part)));
                }
            }
            catch(YAML::Exception& e) {
                ErrorLogger() << boost::format(
                    "parse::ship_parts: %1%:%2%:%3%: %4%")
                    % file_path % e.mark.line % e.mark.column % e.what();

                InfoLogger() << doc;
            }
        }

        return parts;
    }
}
