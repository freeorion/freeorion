#include "Parse.h"

#include <boost/filesystem/fstream.hpp>
#include <yaml-cpp/yaml.h>
#include "MovableEnvelope.h"
#include "ConditionParser.h"
#include "../universe/Condition.h"
#include "../universe/FleetPlan.h"


namespace YAML {
    template <>
    struct convert<std::unique_ptr<Condition::Condition>> {
        static bool decode(const Node& node, std::unique_ptr<Condition::Condition>& rhs) {
            if (!node.IsScalar())
                return false;

            std::string data{node.as<std::string>()};

            const parse::lexer lexer;
            parse::detail::Labeller label;
            parse::conditions_parser_grammar condition_parser(lexer, label);
            boost::spirit::qi::in_state_type in_state;
            parse::text_iterator begin = data.begin();
            parse::text_iterator end = data.end();
            parse::token_iterator it = lexer.begin(begin, end);

            parse::detail::MovableEnvelope<Condition::Condition> location;

            bool success = boost::spirit::qi::phrase_parse(
                it, lexer.end(),
                condition_parser, in_state("WS")[lexer.self],
                location);

            if (!success) {
                ErrorLogger() << "monster_fleet_plans: failed to parse location attribute";
                return false;
            }

            bool passed{true};

            rhs = location.OpenEnvelope(passed);

            return passed;
        }
    };
}


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
