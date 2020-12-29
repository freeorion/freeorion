#include "../util/GameRules.h"

#include "PythonParserImpl.h"

#include <boost/python/args.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/object_core.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/object_fwd.hpp>
#include <boost/python/str.hpp>
#include <boost/python/exec.hpp>
#include <boost/python/stl_iterator.hpp>
#include <functional>
#include <string>

using namespace boost::python;

struct grammar;

object insert_rule_(const grammar& g,
                    GameRules& game_rules,
                    const tuple& args,
                    const dict& kw);

struct grammar {
    object type_int;
    object type_float;
    object type_bool;
    object type_str;

    grammar() {
        type_int = eval("int", dict());
        type_float = eval("float", dict());
        type_bool = eval("bool", dict());
        type_str = eval("str", dict());
    }

    boost::python::dict operator()(GameRules& game_rules) {
        boost::python::dict globals;
        std::function<object(const tuple&, const dict&)> f = [this, &game_rules](const tuple& args,
                  const dict& kw) { return insert_rule_(*this, game_rules, args, kw); };
        globals["GameRule"] = raw_function(f);
        return globals;
    }
};

object insert_rule_(const grammar& g,
                    GameRules& game_rules,
                    const tuple& args,
                    const dict& kw)
{
    auto name = extract<std::string>(kw["name"])();
    auto desc = extract<std::string>(kw["description"])();
    auto category = extract<std::string>(kw["category"])();
    auto type_ = kw["type"];

    if (type_ == g.type_int) {
        int default_value = extract<int>(kw["default"])();
        int min = extract<int>(kw["min"])();
        int max = extract<int>(kw["max"])();
        DebugLogger() << "Adding Integer game rule with name: " << name
                      << ", desc: " << desc << ", default: " << default_value
                      << ", min: " << min << ", max: " << max;
        game_rules.Add<int>(std::move(name), std::move(desc), std::move(category),
                            default_value, false, RangedValidator<int>(min, max));
    } else if (type_ == g.type_float) {
        double default_value = extract<double>(kw["default"])();
        double min = extract<double>(kw["min"])();
        double max = extract<double>(kw["max"])();
        DebugLogger() << "Adding Double game rule with name: " << name
                      << ", desc: " << desc << ", default: " << default_value
                      << ", min: " << min << ", max: " << max;
        game_rules.Add<double>(std::move(name), std::move(desc), std::move(category),
                               default_value, false, RangedValidator<double>(min, max));
    } else if (type_ == g.type_bool) {
        bool default_value = extract<bool>(kw["default"])();
        DebugLogger() << "Adding Boolean game rule with name: " << name
                      << ", desc: " << desc << ", default: " << default_value;
        game_rules.Add<bool>(std::move(name), std::move(desc),
                             std::move(category), default_value, false);
    } else if (type_ == g.type_str) {
        std::string default_value = extract<std::string>(kw["default"])();
        std::set<std::string> allowed = std::set<std::string>(stl_input_iterator<std::string>(kw["allowed"]),
                                                              stl_input_iterator<std::string>());
        DebugLogger() << "Adding String game rule with name: " << name
                      << ", desc: " << desc << ", default: \"" << default_value
                      << "\", allowed: " << [&allowed](){
                std::string retval;
                for (const auto& e : allowed)
                    retval += "\"" + e + "\", ";
                return retval;
            }();

        if (allowed.empty()) {
            game_rules.Add<std::string>(std::move(name), std::move(desc), std::move(category),
                                        std::move(default_value), false);
        } else {
            game_rules.Add<std::string>(std::move(name), std::move(desc), std::move(category),
                                        std::move(default_value), false,
                                        DiscreteValidator<std::string>(std::move(allowed)));
        }
    } else {
        ErrorLogger() << "Unsupported type for rule " << name << ": " << extract<std::string>(str(type_))();
    }

    return object();
}

namespace parse {
    GameRules game_rules(const PythonParser& parser, const boost::filesystem::path& path) {
        GameRules game_rules;
        /*auto success =*/ py_parse::detail::parse_file<grammar, GameRules>(parser, path, game_rules);
        return game_rules;
    }
}
