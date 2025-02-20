#include "../util/GameRules.h"

#include "PythonParserImpl.h"

#include <boost/python/args.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/object_core.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/object_fwd.hpp>
#include <boost/python/str.hpp>
#include <boost/python/exec.hpp>
#include <boost/python/import.hpp>
#include <boost/python/stl_iterator.hpp>
#include <functional>
#include <string>

using namespace boost::python;

struct grammar;

object insert_rule_(const grammar& g, GameRulesTypeMap& game_rules, const tuple& args, const dict& kw);

struct grammar {
    const PythonParser& m_parser;

    grammar(const PythonParser& parser) :
        m_parser(parser)
    {}

    dict operator()(GameRulesTypeMap& game_rules) const {
        dict globals(import("builtins").attr("__dict__"));
        globals["GameRule"] = raw_function([this, &game_rules](const tuple& args, const dict& kw) { return insert_rule_(*this, game_rules, args, kw); });
        return globals;
    }
};

object insert_rule_(const grammar& g, GameRulesTypeMap& game_rules, const tuple& args, const dict& kw) {
    auto name{extract<std::string>(kw["name"])()};
    auto desc{extract<std::string>(kw["description"])()};
    auto category{extract<std::string>(kw["category"])()};
    auto type_ = kw["type"];
    uint32_t rank{extract<uint32_t>(kw["rank"])()};


    if (type_ == g.m_parser.type_int) {
        int default_value{extract<int>(kw["default"])()};
        int min{extract<int>(kw["min"])()};
        int max{extract<int>(kw["max"])()};
        DebugLogger() << "Adding Integer game rule with name: " << name
                      << ", desc: " << desc << ", default: " << default_value
                      << ", min: " << min << ", max: " << max;
        game_rules.insert_or_assign(name, GameRule{GameRule::Type::INT, name, default_value, default_value,
                                                   std::move(desc), std::make_unique<RangedValidator<int>>(min, max),
                                                   false, rank, std::move(category)});

    } else if (type_ == g.m_parser.type_float) {
        double default_value{extract<double>(kw["default"])()};
        double min{extract<double>(kw["min"])()};
        double max{extract<double>(kw["max"])()};
        DebugLogger() << "Adding Double game rule with name: " << name
                      << ", desc: " << desc << ", default: " << default_value
                      << ", min: " << min << ", max: " << max;
        game_rules.insert_or_assign(name, GameRule{GameRule::Type::DOUBLE, name, default_value, default_value,
                                                   std::move(desc), std::make_unique<RangedValidator<double>>(min, max),
                                                   false, rank, std::move(category)});

    } else if (type_ == g.m_parser.type_bool) {
        bool default_value{extract<bool>(kw["default"])()};
        DebugLogger() << "Adding Boolean game rule with name: " << name
                      << ", desc: " << desc << ", default: " << default_value;
        game_rules.insert_or_assign(name, GameRule{GameRule::Type::TOGGLE, name, default_value, default_value,
                                                   std::move(desc), std::make_unique<Validator<bool>>(),
                                                   false, rank, std::move(category)});

    } else if (type_ == g.m_parser.type_str) {
        auto default_value{extract<std::string>(kw["default"])()};
        std::vector<std::string> allowed{stl_input_iterator<std::string>(kw["allowed"]),
                                      stl_input_iterator<std::string>()};
        DebugLogger() << "Adding String game rule with name: " << name
                      << ", desc: " << desc << ", default: \"" << default_value
                      << "\", allowed: " << [&allowed](){
                std::string retval;
                for (const auto& e : allowed)
                    retval += "\"" + e + "\", ";
                return retval;
            }();

        game_rules.insert_or_assign(name, GameRule{GameRule::Type::STRING, name, default_value, default_value, std::move(desc),
                                                   allowed.empty() ?
                                                        nullptr :
                                                        std::make_unique<DiscreteValidator<std::string>>(std::move(allowed)),
                                                   false, rank, std::move(category)});

    } else {
        ErrorLogger() << "Unsupported type for rule " << name << ": " << extract<std::string>(str(type_))();
    }

    return object();
}

namespace parse {
    GameRulesTypeMap game_rules(const PythonParser& parser, const boost::filesystem::path& path, bool& success) {
        GameRulesTypeMap game_rules;
        py_parse::detail::parse_file<grammar, GameRulesTypeMap>(parser, path, grammar(parser), game_rules);
        return game_rules;
    }
}
