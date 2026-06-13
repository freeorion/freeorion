#include "../util/GameRules.h"

#include "PythonParserImpl.h"

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/scope.hpp>

#include <functional>
#include <string>


using namespace boost::python;

extern "C" BOOST_SYMBOL_EXPORT PyObject* PyInit__game_rules();

namespace {
    DeclareThreadSafeLogger(parsing);

    struct py_grammar {
        const PythonParser& parser;
        boost::python::object module;
        GameRulesTypeMap& game_rules;
        PythonTypes types;

        py_grammar(const PythonParser& parser_, GameRulesTypeMap& game_rules_) :
            parser(parser_),
            module(parser_.LoadModule(&PyInit__game_rules)),
            game_rules(game_rules_)
        {
             module.attr("__grammar") = boost::cref(*this);           
        }
           
        ~py_grammar() {
            parser.UnloadModule(module);
        }
    };

    object insert_rule_(object scope, const tuple& args, const dict& kw) {
        auto name{extract<std::string>(kw["name"])()};
        auto desc{extract<std::string>(kw["description"])()};
        auto category{extract<std::string>(kw["category"])()};
        auto type_ = kw["type"];
        uint32_t rank{extract<uint32_t>(kw["rank"])()};

        py_grammar& p = boost::python::extract<py_grammar&>(scope.attr("__grammar"))();
    
        if (type_ == p.types.type_int) {
            int default_value{extract<int>(kw["default"])()};
            int min{extract<int>(kw["min"])()};
            int max{extract<int>(kw["max"])()};
            DebugLogger() << "Adding Integer game rule with name: " << name
                          << ", desc: " << desc << ", default: " << default_value
                          << ", min: " << min << ", max: " << max;
            p.game_rules.insert_or_assign(name, GameRule{GameRule::Type::INT, name, default_value, default_value,
                                                       std::move(desc), std::make_unique<RangedValidator<int>>(min, max),
                                                       false, rank, std::move(category)});
    
        } else if (type_ == p.types.type_float) {
            double default_value{extract<double>(kw["default"])()};
            double min{extract<double>(kw["min"])()};
            double max{extract<double>(kw["max"])()};
            DebugLogger() << "Adding Double game rule with name: " << name
                          << ", desc: " << desc << ", default: " << default_value
                          << ", min: " << min << ", max: " << max;
            p.game_rules.insert_or_assign(name, GameRule{GameRule::Type::DOUBLE, name, default_value, default_value,
                                                       std::move(desc), std::make_unique<RangedValidator<double>>(min, max),
                                                       false, rank, std::move(category)});
    
        } else if (type_ == p.types.type_bool) {
            bool default_value{extract<bool>(kw["default"])()};
            DebugLogger() << "Adding Boolean game rule with name: " << name
                          << ", desc: " << desc << ", default: " << default_value;
            p.game_rules.insert_or_assign(name, GameRule{GameRule::Type::TOGGLE, name, default_value, default_value,
                                                       std::move(desc), std::make_unique<Validator<bool>>(),
                                                       false, rank, std::move(category)});
    
        } else if (type_ == p.types.type_str) {
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
    
            p.game_rules.insert_or_assign(name, GameRule{GameRule::Type::STRING, name, default_value, default_value, std::move(desc),
                                                       allowed.empty() ?
                                                            nullptr :
                                                            std::make_unique<DiscreteValidator<std::string>>(std::move(allowed)),
                                                       false, rank, std::move(category)});
    
        } else {
            ErrorLogger() << "Unsupported type for rule " << name << ": " << extract<std::string>(str(type_))();
        }
    
        return object();
    }
    
}

BOOST_PYTHON_MODULE(_game_rules) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::class_<py_grammar, boost::python::bases<>, py_grammar, boost::noncopyable>("__Grammar", boost::python::no_init);

    boost::python::object current_module = boost::python::scope();

    boost::python::def("GameRule", raw_function([current_module](const tuple& args, const dict& kw) { return insert_rule_(current_module, args, kw); }));
}

namespace parse {
    GameRulesTypeMap game_rules(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        GameRulesTypeMap game_rules;
        success = py_parse::detail::parse_file<py_grammar>(parser, path, py_grammar(parser, game_rules));
        return game_rules;
    }
}
