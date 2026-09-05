#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "Parse.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParamsParser.h"

#include "../universe/Condition.h"
#include "../universe/ShipHull.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"

#include <boost/phoenix.hpp>

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/make_function.hpp>
#include <boost/python/module.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/scope.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipHull::Slot>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<ShipHull>>&) { return os; }
    inline ostream& operator<<(ostream& os, const ShipHull::Slot&) { return os; }
}
#endif

extern "C" BOOST_SYMBOL_EXPORT PyObject* PyInit__ship_hulls();

namespace {
    struct ShipHullStats {
        ShipHullStats() = default;

        ShipHullStats(float fuel_,
                    float speed_,
                    float stealth_,
                    float structure_,
                    bool default_fuel_effects_,
                    bool default_speed_effects_,
                    bool default_stealth_effects_,
                    bool default_structure_effects_) :
            fuel(fuel_),
            speed(speed_),
            stealth(stealth_),
            structure(structure_),
            default_fuel_effects(default_fuel_effects_),
            default_speed_effects(default_speed_effects_),
            default_stealth_effects(default_stealth_effects_),
            default_structure_effects(default_structure_effects_)
        {}

        float   fuel = 0.0f;
        float   speed = 0.0f;
        float   stealth = 0.0f;
        float   structure = 0.0f;
        bool    default_fuel_effects = true;
        bool    default_speed_effects = true;
        bool    default_stealth_effects = true;
        bool    default_structure_effects = true;
    };

    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_shiphull(std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>& shiphulls,
                         ShipHullStats& stats,
                         parse::detail::MovableEnvelope<CommonParams>& common_params,
                         parse::detail::MoreCommonParams& more_common_params,
                         boost::optional<std::vector<ShipHull::Slot>>& slots,
                         std::string& icon, std::string& graphic,
                         bool& pass)
    {
        auto shiphull = std::make_unique<ShipHull>(
            stats.fuel,
            stats.speed,
            stats.stealth,
            stats.structure,
            stats.default_fuel_effects,
            stats.default_speed_effects,
            stats.default_stealth_effects,
            stats.default_structure_effects,
            std::move(*common_params.OpenEnvelope(pass)),
            std::move(more_common_params.name),
            std::move(more_common_params.description),
            std::move(more_common_params.exclusions),
            (slots ? std::move(*slots) : std::vector<ShipHull::Slot>{}),
            std::move(icon),
            std::move(graphic));

        auto& hull_name{shiphull->Name()};
        shiphulls.emplace(hull_name, std::move(shiphull));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_shiphull_, insert_shiphull, 8)

    using start_rule_payload = std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator first, const parse::text_iterator last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            tags_parser(tok, label),
            common_rules(tok, label, condition_parser, string_grammar, tags_parser),
            ship_slot_type_enum(tok),
            double_rule(tok),
            one_or_more_slots(slot)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_7_type _7;
            qi::_8_type _8;
            qi::_r1_type _r1;
            qi::matches_type matches_;
            qi::_pass_type _pass;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::lit_type lit;
            qi::omit_type omit_;

            hull_stats
                =  (label(tok.speed_)       >   double_rule // _1
                >   matches_[tok.NoDefaultSpeedEffect_]     // _2
                >   label(tok.fuel_)        >   double_rule // _3
                >   matches_[tok.NoDefaultFuelEffect_]      // _4
                >   label(tok.stealth_)     >   double_rule // _5
                >   matches_[tok.NoDefaultStealthEffect_]   // _6
                >   label(tok.structure_)   >   double_rule // _7
                >   matches_[tok.NoDefaultStructureEffect_])// _8
                    [ _val = construct<ShipHullStats>(_3, _1, _5, _7, !_4, !_2, !_6, !_8) ]
                ;

            slot
                =  (omit_[tok.Slot_]
                >   label(tok.type_) > ship_slot_type_enum
                >   label(tok.position_)
                >   '(' > double_rule > ',' > double_rule > lit(')'))
                    [ _val = construct<ShipHull::Slot>(_1, _2, _3) ]
                ;

            hull
                =   (tok.Hull_                              // _1
                >   common_rules.more_common                // _2
                >   hull_stats                              // _3
                >  -(label(tok.slots_) > one_or_more_slots) // _4
                >   common_rules.common                     // _5
                >   label(tok.icon_)    > tok.string        // _6
                >   label(tok.graphic_) > tok.string)       // _7
                [ ( _pass = is_unique_(_r1, _1, phoenix::bind(&parse::detail::MoreCommonParams::name, _2)),
                    insert_shiphull_(_r1, _3, _5, _2, _4, _6, _7, _pass) ) ]
                ;

            start
                =   +hull(_r1)
                ;

            hull_stats.name("Hull stats");
            slot.name("Slot");
            hull.name("Hull");

#if DEBUG_PARSERS
            debug(cost);
            debug(hull_stats);
            debug(slot);
            debug(hull);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using hull_stats_rule = parse::detail::rule<ShipHullStats ()>;

        using slot_rule =  parse::detail::rule<ShipHull::Slot ()>;

        using hull_rule = parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>&)
        >;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller             label;
        parse::conditions_parser_grammar    condition_parser;
        const parse::string_parser_grammar  string_grammar;
        parse::detail::tags_grammar         tags_parser;
        parse::detail::common_params_rules  common_rules;
        parse::ship_slot_enum_grammar       ship_slot_type_enum;
        parse::detail::double_grammar       double_rule;
        hull_stats_rule                     hull_stats;
        slot_rule                           slot;
        parse::detail::single_or_bracketed_repeat<slot_rule> one_or_more_slots;
        hull_rule                           hull;
        start_rule                          start;
    };

    struct py_grammar {
        const PythonParser& parser;
        boost::python::object module;
        start_rule_payload& hulls;

        py_grammar(const PythonParser& parser_, start_rule_payload& hulls_) :
            parser(parser_),
            module(parser_.LoadModule(&PyInit__ship_hulls)),
            hulls(hulls_)
        {
            parser.LoadValueRefsModule();
            parser.LoadEffectsModule();
            parser.LoadConditionsModule();
            parser.LoadSourcesModule();
            parser.LoadEnumsModule();

            module.attr("__grammar") = boost::cref(*this);
        }

        ~py_grammar() {
            parser.UnloadModule(module);
        }
    };

    struct ship_slot_wrapper {
        ship_slot_wrapper(ShipHull::Slot&& slot_) : slot(std::move(slot_)) {}
        ship_slot_wrapper(const ShipHull::Slot& slot_) : slot(slot_) {}
        const ShipHull::Slot slot;
    };

    ship_slot_wrapper py_insert_slot_(const boost::python::tuple& args,
                                      const boost::python::dict& kw)
    {
        auto type = boost::python::extract<enum_wrapper<ShipSlotType>>(kw["type"])().value;

        auto position = boost::python::extract<boost::python::tuple>(kw["position"])();
        auto x = boost::python::extract<double>(position[0])();
        auto y = boost::python::extract<double>(position[1])();

        return ship_slot_wrapper(ShipHull::Slot(type, x, y));
    }

    boost::python::object py_insert_hull_(boost::python::object scope, const boost::python::tuple& args,
                                          const boost::python::dict& kw)
    {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto description = boost::python::extract<std::string>(kw["description"])();

        std::set<std::string> exclusions;
        if (kw.has_key("exclusions")) {
            boost::python::stl_input_iterator<std::string> exclusions_begin(kw["exclusions"]), exclusions_end;
            exclusions = std::set<std::string>(exclusions_begin, exclusions_end);
        }

        auto fuel = boost::python::extract<float>(kw["fuel"])();
        auto speed = boost::python::extract<float>(kw["speed"])();
        auto stealth = boost::python::extract<float>(kw["stealth"])();
        auto structure = boost::python::extract<float>(kw["structure"])();

        bool default_fuel_effects = true;
        if (kw.has_key("NoDefaultFuelEffect"))
            default_fuel_effects = !boost::python::extract<bool>(kw["NoDefaultFuelEffect"])();

        bool default_speed_effects = true;
        if (kw.has_key("NoDefaultSpeedEffect"))
            default_speed_effects = !boost::python::extract<bool>(kw["NoDefaultSpeedEffect"])();

        bool default_stealth_effects = true;
        if (kw.has_key("NoDefaultStealthEffect"))
            default_stealth_effects = !boost::python::extract<bool>(kw["NoDefaultStealthEffect"])();

        bool default_structure_effects = true;
        if (kw.has_key("NoDefaultStructureEffect"))
            default_structure_effects = !boost::python::extract<bool>(kw["NoDefaultStructureEffect"])();

        std::vector<ShipHull::Slot> slots;
        if (kw.has_key("slots")) {
            boost::python::stl_input_iterator<ship_slot_wrapper> slots_begin(kw["slots"]), slots_end;
            for (auto it = slots_begin; it != slots_end; ++it)
                slots.push_back(it->slot);
        }

        auto production_cost = pyobject_to_vref_or_cast<double, int>(kw["buildcost"]);
        auto production_time = pyobject_to_vref_or_cast<int, double>(kw["buildtime"]);

        bool producible = true;
        if (kw.has_key("producible"))
            producible = boost::python::extract<bool>(kw["producible"])();

        std::set<std::string> tags;
        if (kw.has_key("tags")) {
            boost::python::stl_input_iterator<std::string> tags_begin(kw["tags"]), tags_end;
            tags = std::set<std::string>(tags_begin, tags_end);
        }

        std::unique_ptr<Condition::Condition> location;
        if (kw.has_key("location"))
            location = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["location"])().condition);
        else
            location = std::make_unique<Condition::All>();

        std::unique_ptr<Condition::Condition> enqueue_location;
        if (kw.has_key("enqueuelocation"))
            enqueue_location = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["enqueuelocation"])().condition);
        else
            enqueue_location = std::make_unique<Condition::All>();

        std::vector<std::unique_ptr<Effect::EffectsGroup>> effectsgroups;
        boost::python::stl_input_iterator<effect_group_wrapper> effectsgroups_begin(kw["effectsgroups"]), effectsgroups_end;
        for (auto it = effectsgroups_begin; it != effectsgroups_end; ++it) {
            const auto& effects_group = *it->effects_group;
            effectsgroups.push_back(std::make_unique<Effect::EffectsGroup>(
                ValueRef::CloneUnique(effects_group.Scope()),
                ValueRef::CloneUnique(effects_group.Activation()),
                ValueRef::CloneUnique(effects_group.Effects()),
                effects_group.AccountingLabel(),
                effects_group.StackingGroup(),
                effects_group.Priority(),
                effects_group.GetDescription(),
                effects_group.TopLevelContent()
            ));
        }

        auto icon = boost::python::extract<std::string>(kw["icon"])();
        auto graphic = boost::python::extract<std::string>(kw["graphic"])();

        auto shiphull = std::make_unique<ShipHull>(
            fuel,
            speed,
            stealth,
            structure,
            default_fuel_effects,
            default_speed_effects,
            default_stealth_effects,
            default_structure_effects,
            CommonParams{
                std::move(production_cost),
                std::move(production_time),
                producible,
                tags, // TODO: make this parameter by value and move?
                std::move(location),
                std::move(effectsgroups),
                {},
                {},
                std::move(enqueue_location)
            },
            std::move(name),
            std::move(description),
            std::move(exclusions),
            std::move(slots),
            std::move(icon),
            std::move(graphic));

        py_grammar& p = boost::python::extract<py_grammar&>(scope.attr("__grammar"))();

        auto& hull_name{shiphull->Name()};
        p.hulls.emplace(hull_name, std::move(shiphull));

        return boost::python::object();
    }
}

BOOST_PYTHON_MODULE(_ship_hulls) {
    boost::python::docstring_options doc_options(true, true, false);

    boost::python::class_<py_grammar, boost::python::bases<>, py_grammar, boost::noncopyable>("__Grammar", boost::python::no_init);
    boost::python::class_<ship_slot_wrapper, boost::python::bases<>, ship_slot_wrapper, boost::noncopyable>("_ShipSlot", boost::python::no_init);

    boost::python::def("Slot", boost::python::raw_function(py_insert_slot_));

    boost::python::object current_module = boost::python::scope();

    boost::python::def("Hull", boost::python::raw_function(
        [current_module](const boost::python::tuple& args, const boost::python::dict& kw)
        { return py_insert_hull_(current_module, args, kw); }));
}

namespace parse {
    start_rule_payload ship_hulls(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        start_rule_payload hulls;

        ScopedTimer timer("Ship Hulls Parsing");

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(GetLexer(), file, hulls);
        
        bool file_success = true;
        py_grammar p = py_grammar(parser, hulls);
        for (const auto& file : ListDir(path, IsFOCPyScript))
            file_success = py_parse::detail::parse_file(parser, file) && file_success;

        success = file_success;
        return hulls;
    }
}
