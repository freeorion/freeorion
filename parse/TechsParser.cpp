#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "EffectParser.h"
#include "ValueRefParser.h"
#include "MovableEnvelope.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include "../universe/Conditions.h"
#include "../universe/Effects.h"
#include "../universe/ValueRefs.h"

#include "../universe/Effect.h"
#include "../universe/Species.h"
#include "../universe/Tech.h"
#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi_as.hpp>

#include <boost/python/extract.hpp>
#include <boost/python/import.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<UnlockableItem>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::set<std::string>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const Tech::TechInfo&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<TechCategory>>&) { return os; }
}
#endif

namespace {
    struct py_grammar_techs;
    boost::python::object insert_game_rule_(const py_grammar_techs& g, const boost::python::tuple& args, const boost::python::dict& kw);
    template <ValueRef::OpType O>
    boost::python::object insert_minmaxoneof_(const py_grammar_techs& g, const boost::python::tuple& args, const boost::python::dict& kw);

    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    std::set<std::string>* g_categories_seen = nullptr;
    std::map<std::string, std::unique_ptr<TechCategory>>* g_categories = nullptr;

    /// Check if the tech will be unique.
    bool check_tech(TechManager::TechContainer& techs, const std::unique_ptr<Tech>& tech) {
        auto retval = true;
        if (techs.get<TechManager::NameIndex>().count(tech->Name())) {
            ErrorLogger() <<  "More than one tech has the name " << tech->Name();
            retval = false;
        }
        if (tech->Prerequisites().count(tech->Name())) {
            ErrorLogger() << "Tech " << tech->Name() << " depends on itself!";
            retval = false;
        }
        return retval;
    }

    void insert_tech(TechManager::TechContainer& techs,
                     parse::detail::MovableEnvelope<Tech::TechInfo>& tech_info,
                     boost::optional<parse::effects_group_payload>& effects,
                     boost::optional<std::set<std::string>>& prerequisites,
                     boost::optional<std::vector<UnlockableItem>>& unlocked_items,
                     boost::optional<std::string>& graphic,
                     bool& pass)
    {
        auto tech_ptr = std::make_unique<Tech>(
            std::move(*tech_info.OpenEnvelope(pass)),
            (effects ? parse::detail::OpenEnvelopes(*effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>{}),
            (prerequisites ? std::move(*prerequisites) : std::set<std::string>{}),
            (unlocked_items ? std::move(*unlocked_items) : std::vector<UnlockableItem>{}),
            (graphic ? std::move(*graphic) : std::string{}));

        if (check_tech(techs, tech_ptr)) {
            g_categories_seen->emplace(tech_ptr->Category());
            techs.emplace(std::move(tech_ptr));
        }
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_tech_, insert_tech, 7)

    void insert_category(std::map<std::string, std::unique_ptr<TechCategory>>& categories,
                         std::string& name, std::string& graphic, const std::array<unsigned char, 4>& color)
    {
        auto category_ptr = std::make_unique<TechCategory>(name, std::move(graphic), color);
        categories.emplace(std::move(name), std::move(category_ptr));
    }

    using start_rule_signature = void(TechManager::TechContainer&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            one_or_more_string_tokens(tok),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            castable_int_rules(tok, label, condition_parser, string_grammar),
            double_rules(tok, label, condition_parser, string_grammar),
            effects_group_grammar(tok, label, condition_parser, string_grammar),
            tags_parser(tok, label),
            unlockable_item_parser(tok, label),
            one_or_more_unlockable_items(unlockable_item_parser),
            color_parser(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::new_;
            using phoenix::construct;
            using phoenix::insert;
            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_7_type _7;
            qi::_8_type _8;
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::omit_type omit_;
            qi::as_string_type as_string_;
            const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            researchable =
                    tok.Unresearchable_ [ _val = false ]
                |   tok.Researchable_ [ _val = true ]
                |   eps [ _val = true ]
                ;

            tech_info
                = ( label(tok.name_)                > tok.string
                >   label(tok.description_)         > tok.string
                >   label(tok.short_description_)   > tok.string  // TODO: Get rid of underscore.
                >   label(tok.category_)            > tok.string
                >   label(tok.researchcost_)        > double_rules.expr
                >   label(tok.researchturns_)       > castable_int_rules.flexible_int
                >   researchable
                >   tags_parser
                ) [ _val = construct_movable_(new_<Tech::TechInfo>(
                    _1, _2, _3, _4, deconstruct_movable_(_5, _pass),
                    deconstruct_movable_(_6, _pass), _7, _8)) ]
                ;

            tech
                = ( omit_[tok.Tech_]
                >   tech_info
                >  -(label(tok.prerequisites_) > one_or_more_string_tokens)
                >  -(label(tok.unlock_)        > one_or_more_unlockable_items)
                >  -(label(tok.effectsgroups_) > effects_group_grammar)
                >  -as_string_[(label(tok.graphic_) > tok.string)]
                  ) [ insert_tech_(_r1, _1, _4, _2, _3, _5, _pass) ]
                ;

            start
                = +(tech(_r1));

            researchable.name("Researchable");
            tech_info.name("Tech info");
            tech.name("Tech");
            start.name("start");

#if DEBUG_PARSERS
            debug(tech_info_name_desc);
            debug(tech_info);
            debug(tech);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using tech_info_rule = parse::detail::rule<parse::detail::MovableEnvelope<Tech::TechInfo> ()>;
        using tech_rule = parse::detail::rule<void (TechManager::TechContainer&)>;
        using category_rule = parse::detail::rule<void (std::map<std::string, std::unique_ptr<TechCategory>>&)>;
        using start_rule = parse::detail::rule<void (TechManager::TechContainer&)>;

        parse::detail::Labeller                 label;
        parse::detail::single_or_repeated_string<std::set<std::string>>
                                                one_or_more_string_tokens;
        parse::conditions_parser_grammar        condition_parser;
        const parse::string_parser_grammar      string_grammar;
        parse::castable_as_int_parser_rules     castable_int_rules;
        parse::double_parser_rules              double_rules;
        parse::effects_group_grammar            effects_group_grammar;
        parse::detail::tags_grammar             tags_parser;
        parse::detail::unlockable_item_grammar  unlockable_item_parser;
        parse::detail::single_or_bracketed_repeat<parse::detail::unlockable_item_grammar>
                                                one_or_more_unlockable_items;
        parse::detail::color_parser_grammar     color_parser;
        parse::detail::rule<bool()>             researchable;
        tech_info_rule                          tech_info;
        tech_rule                               tech;
        start_rule                              start;
    };

    boost::python::object insert_category_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto graphic = boost::python::extract<std::string>(kw["graphic"])();
        auto colour = boost::python::extract<boost::python::tuple>(kw["colour"])();

        std::array<unsigned char, 4> color{0, 0, 0, 255};

        boost::python::stl_input_iterator<unsigned char> colour_begin(colour), colour_end;
        int colour_index = 0;
        for (auto it = colour_begin; it != colour_end; ++ it) {
            if (colour_index < 4) {
                color[colour_index] = *it;
            }
            ++ colour_index;
        }

        insert_category(*g_categories, name, graphic, color);

        return boost::python::object();
    }

    struct py_grammar_category {
        boost::python::dict operator()(TechManager::TechContainer& techs) const {
            boost::python::dict globals(boost::python::import("builtins").attr("__dict__"));
            globals["Category"] = boost::python::raw_function(insert_category_);
            return globals;
        }
    };

    condition_wrapper insert_owned_by_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        EmpireAffiliationType affiliation = EmpireAffiliationType::AFFIL_SELF;

        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        if (kw.has_key("affiliation")) {
            affiliation = boost::python::extract<enum_wrapper<EmpireAffiliationType>>(kw["affiliation"])().value;
        }

        return condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(std::move(empire), affiliation));
    }

    unlockable_item_wrapper insert_item_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto type = boost::python::extract<enum_wrapper<UnlockableItemType>>(kw["type"])();
        auto name = boost::python::extract<std::string>(kw["name"])();
        return unlockable_item_wrapper(UnlockableItem(type.value, std::move(name)));
    }

    effect_group_wrapper insert_effects_group_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto scope = boost::python::extract<condition_wrapper>(kw["scope"])();
        int priority = 100;
        if (kw.has_key("priority")) {
            priority = boost::python::extract<int>(kw["priority"])();
        }

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        auto effects_args = boost::python::extract<boost::python::list>(kw["effects"]);
        if (effects_args.check()) {
            boost::python::stl_input_iterator<effect_wrapper> effects_begin(effects_args), effects_end;
            for (auto it = effects_begin; it != effects_end; ++ it) {
                effects.push_back(ValueRef::CloneUnique(it->effect));
            }
        } else {
            effects.push_back(ValueRef::CloneUnique(boost::python::extract<effect_wrapper>(kw["effects"])().effect));
        }

        std::string stackinggroup = "";
        if (kw.has_key("stackinggroup")) {
            stackinggroup = boost::python::extract<std::string>(kw["stackinggroup"]);
        }

        std::unique_ptr<Condition::Condition> activation = nullptr;
        if (kw.has_key("activation")) {
           activation = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["activation"])().condition);
        }
        // ToDo: implement other arguments later

        return effect_group_wrapper(std::make_shared<Effect::EffectsGroup>(ValueRef::CloneUnique(scope.condition),
                                                      std::move(activation),
                                                      std::move(effects),
                                                      "",
                                                      stackinggroup,
                                                      priority,
                                                      "",
                                                      ""));
    }

    condition_wrapper insert_contained_by_(const condition_wrapper& cond) {
        return condition_wrapper(std::make_shared<Condition::ContainedBy>(ValueRef::CloneUnique(cond.condition)));
    }

    condition_wrapper insert_contains_(const condition_wrapper& cond) {
        return condition_wrapper(std::make_shared<Condition::Contains>(ValueRef::CloneUnique(cond.condition)));
    }

    template <MeterType M>
    effect_wrapper insert_set_meter_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto value = boost::python::extract<value_ref_wrapper<double>>(kw["value"])();

        boost::optional<std::string> accountinglabel = boost::none;
        if (kw.has_key("accountinglabel")) {
            accountinglabel = boost::python::extract<std::string>(kw["accountinglabel"])();
        }
        return effect_wrapper(std::make_shared<Effect::SetMeter>(M,
                                                                 ValueRef::CloneUnique(value.value_ref),
                                                                 accountinglabel));
    }

    template <MeterType M>
    condition_wrapper insert_meter_value_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<double>> low = nullptr;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<double>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<double>> high = nullptr;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<double>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["high"])());
            }
        }
        return condition_wrapper(std::make_shared<Condition::MeterValue>(M, std::move(low), std::move(high)));
    }

    template<typename T>
    value_ref_wrapper<T> insert_named_lookup_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();

        return value_ref_wrapper<T>(std::make_shared<ValueRef::NamedRef<T>>(name, true));
    }

    condition_wrapper insert_visible_to_empire_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        if (kw.has_key("turn")) {
            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        if (kw.has_key("visibility")) {
            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return condition_wrapper(std::make_shared<Condition::VisibleToEmpire>(std::move(empire)));
    }

    condition_wrapper insert_planet_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("type")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>> types;
            py_parse::detail::flatten_list<value_ref_wrapper< ::PlanetType>>(kw["type"], [](const value_ref_wrapper< ::PlanetType>& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>>& v) {
                v.push_back(ValueRef::CloneUnique(o.value_ref));
            }, types);
            return condition_wrapper(std::make_shared<Condition::PlanetType>(std::move(types)));
        } else if (kw.has_key("size")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>> sizes;
            py_parse::detail::flatten_list<value_ref_wrapper< ::PlanetSize>>(kw["size"], [](const value_ref_wrapper< ::PlanetSize>& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>>& v) {
                v.push_back(ValueRef::CloneUnique(o.value_ref));
            }, sizes);
            return condition_wrapper(std::make_shared<Condition::PlanetSize>(std::move(sizes)));

        } else if (kw.has_key("environment")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>> environments;
            py_parse::detail::flatten_list<value_ref_wrapper< ::PlanetEnvironment>>(kw["environment"], [](const value_ref_wrapper< ::PlanetEnvironment>& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>>& v) {
                v.push_back(ValueRef::CloneUnique(o.value_ref));
            }, environments);
            return condition_wrapper(std::make_shared<Condition::PlanetEnvironment>(std::move(environments)));
        }
        return condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_PLANET));
    }

    effect_wrapper insert_generate_sit_rep_message_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto message = boost::python::extract<std::string>(kw["message"])();
        auto label = boost::python::extract<std::string>(kw["label"])();

        bool stringtable_lookup = true;
        if (kw.has_key("NoStringtableLookup")) {
            stringtable_lookup = false;
        }

        std::string icon = "";
        if (kw.has_key("icon")) {
            icon = boost::python::extract<std::string>(kw["icon"])();
        }

        std::vector<std::pair<std::string, std::unique_ptr<ValueRef::ValueRef<std::string>>>> parameters;
        if (kw.has_key("parameters")) {
            boost::python::dict param_args = boost::python::extract<boost::python::dict>(kw["parameters"]);
            boost::python::stl_input_iterator<std::string> p_begin(param_args.keys()), p_end;
            for (auto it = p_begin; it != p_end; ++ it) {
                auto p_arg = boost::python::extract<value_ref_wrapper<std::string>>(param_args[*it]);
                if (p_arg.check()) {
                    parameters.push_back(std::make_pair(*it, ValueRef::CloneUnique(p_arg().value_ref)));
                } else {
                    auto p_arg_double = boost::python::extract<value_ref_wrapper<double>>(param_args[*it]);
                    if (p_arg_double.check()) {
                        parameters.push_back(std::make_pair(*it, std::make_unique<ValueRef::StringCast<double>>(ValueRef::CloneUnique(p_arg_double().value_ref))));
                    } else {
                        auto p_arg_int = boost::python::extract<value_ref_wrapper<int>>(param_args[*it]);
                        if (p_arg_int.check()) {
                            parameters.push_back(std::make_pair(*it, std::make_unique<ValueRef::StringCast<int>>(ValueRef::CloneUnique(p_arg_int().value_ref))));
                        } else {
                            parameters.push_back(std::make_pair(*it, std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(param_args[*it]))));
                        }
                    }
                }
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        EmpireAffiliationType affiliation = EmpireAffiliationType::AFFIL_SELF;
        if (kw.has_key("affiliation")) {
            affiliation = boost::python::extract<enum_wrapper<EmpireAffiliationType>>(kw["affiliation"])().value;
        }

        std::unique_ptr<Condition::Condition> condition;
        if (kw.has_key("condition")) {
            condition = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["condition"])().condition);
        }

        return effect_wrapper(std::make_shared<Effect::GenerateSitRepMessage>(
                                    message,
                                    icon,
                                    std::move(parameters),
                                    std::move(empire),
                                    affiliation,
                                    std::move(label),
                                    stringtable_lookup
                              ));
    }

    condition_wrapper insert_has_tag_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name = nullptr;
        if (kw.has_key("name")) {
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }
        return condition_wrapper(std::make_shared<Condition::HasTag>(std::move(name)));
    }

    boost::python::object py_insert_tech_(TechManager::TechContainer& techs, const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto description = boost::python::extract<std::string>(kw["description"])();
        auto short_description = boost::python::extract<std::string>(kw["short_description"])();
        auto category = boost::python::extract<std::string>(kw["category"])();

        std::unique_ptr<ValueRef::ValueRef<double>> researchcost;
        auto researchcost_args = boost::python::extract<value_ref_wrapper<double>>(kw["researchcost"]);
        if (researchcost_args.check()) {
            researchcost = ValueRef::CloneUnique(researchcost_args().value_ref);
        } else {
            researchcost = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["researchcost"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> researchturns;
        auto researchturns_args = boost::python::extract<value_ref_wrapper<int>>(kw["researchturns"]);
        if (researchturns_args.check()) {
            researchturns = ValueRef::CloneUnique(researchturns_args().value_ref);
        } else {
            researchturns = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["researchturns"])());
        }

        bool researchable = true;

        std::set<std::string> tags;
        auto tags_args = boost::python::extract<boost::python::list>(kw["tags"])();
        boost::python::stl_input_iterator<std::string> tags_begin(tags_args), tags_end;
        for (auto it = tags_begin; it != tags_end; ++ it) {
            tags.insert(*it);
        }

        std::vector<std::shared_ptr<Effect::EffectsGroup>> effectsgroups;
        if (kw.has_key("effectsgroups")) {
            py_parse::detail::flatten_list<effect_group_wrapper>(kw["effectsgroups"], [](const effect_group_wrapper& o, std::vector<std::shared_ptr<Effect::EffectsGroup>>& v) {
                v.push_back(o.effects_group);
            }, effectsgroups);
        }

        std::set<std::string> prerequisites;
        if (kw.has_key("prerequisites")) {
            py_parse::detail::flatten_list<std::string>(kw["prerequisites"], [](const std::string& o, std::set<std::string>& v) {
                v.insert(o);
            }, prerequisites);
        }

        std::vector<UnlockableItem> unlock;
        if (kw.has_key("unlock")) {
            auto unlock_args = boost::python::extract<boost::python::list>(kw["unlock"]);
            if (unlock_args.check()) {
                boost::python::stl_input_iterator<unlockable_item_wrapper> unlock_begin(unlock_args()), unlock_end;
                for (auto it = unlock_begin; it != unlock_end; ++ it) {
                    unlock.push_back(it->item);
                }
            } else {
                unlock.push_back(boost::python::extract<unlockable_item_wrapper>(kw["unlock"])().item);
            }
        }

        std::string graphic;
        if (kw.has_key("graphic")) {
            graphic = boost::python::extract<std::string>(kw["graphic"])();
        }

        auto tech_ptr = std::make_unique<Tech>(std::move(name), std::move(description),
                                               std::move(short_description), std::move(category),
                                               std::move(researchcost),
                                               std::move(researchturns),
                                               researchable,
                                               std::move(tags),
                                               std::move(effectsgroups),
                                               std::move(prerequisites),
                                               std::move(unlock),
                                               std::move(graphic));

        if (check_tech(techs, tech_ptr)) {
            g_categories_seen->emplace(tech_ptr->Category());
            techs.emplace(std::move(tech_ptr));
        }

        return boost::python::object();
    }

    struct py_grammar_techs {
        const PythonParser& m_parser;

        py_grammar_techs(const PythonParser& parser):m_parser(parser) {
        }

        boost::python::dict operator()(TechManager::TechContainer& techs) const {
            boost::python::dict globals(boost::python::import("builtins").attr("__dict__"));
            std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_game_rule = [this](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_game_rule_(*this, args, kw); };
            globals["GameRule"] = boost::python::raw_function(f_insert_game_rule);
            std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_tech = [&techs](const boost::python::tuple& args, const boost::python::dict& kw) { return py_insert_tech_(techs, args, kw); };
            globals["Tech"] = boost::python::raw_function(f_insert_tech);
            globals["EffectsGroup"] = boost::python::raw_function(insert_effects_group_);
            globals["Item"] = boost::python::raw_function(insert_item_);
            globals["Policy"] = enum_wrapper<UnlockableItemType>(UnlockableItemType::UIT_POLICY);
            globals["Building"] = enum_wrapper<UnlockableItemType>(UnlockableItemType::UIT_BUILDING);
            globals["Species"] = condition_wrapper(std::make_shared<Condition::Species>());
            globals["OwnedBy"] = boost::python::raw_function(insert_owned_by_);
            globals["Source"] = source_wrapper();
            globals["SetMaxShield"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_MAX_SHIELD>);
            globals["SetShield"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_SHIELD>);
            globals["SetTargetPopulation"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_TARGET_POPULATION>);
            globals["SetDefense"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_DEFENSE>);
            globals["SetTroops"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_TROOPS>);
            globals["SetStructure"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_STRUCTURE>);
            globals["Value"] = value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE));
            globals["Target"] = target_wrapper();
            globals["LocalCandidate"] = local_candidate_wrapper();
            globals["Planet"] = boost::python::raw_function(insert_planet_);
            globals["CurrentTurn"] = value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, "CurrentTurn"));
            std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_min = [this](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_minmaxoneof_<ValueRef::OpType::MINIMUM>(*this, args, kw); };
            globals["Min"] = boost::python::raw_function(f_insert_min, 3);
            std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_max = [this](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_minmaxoneof_<ValueRef::OpType::MAXIMUM>(*this, args, kw); };
            globals["Max"] = boost::python::raw_function(f_insert_max, 3);
            globals["NamedRealLookup"] = boost::python::raw_function(insert_named_lookup_<double>);
            globals["Ship"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_SHIP));
            globals["System"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_SYSTEM));
            globals["Fleet"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_FLEET));
            globals["ContainedBy"] = insert_contained_by_;
            globals["Contains"] = insert_contains_;
            globals["EnemyOf"] = enum_wrapper<EmpireAffiliationType>(EmpireAffiliationType::AFFIL_ENEMY);
            globals["Unowned"] = condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_NONE));
            globals["Human"] = condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_HUMAN));
            globals["Structure"] = boost::python::raw_function(insert_meter_value_<MeterType::METER_STRUCTURE>);
            globals["HasTag"] = boost::python::raw_function(insert_has_tag_);
            globals["Destroy"] = effect_wrapper(std::make_shared<Effect::Destroy>());
            globals["VisibleToEmpire"] = boost::python::raw_function(insert_visible_to_empire_);
            globals["GenerateSitRepMessage"] = boost::python::raw_function(insert_generate_sit_rep_message_);
            return globals;
        }

    };

    boost::python::object insert_game_rule_(const py_grammar_techs& g, const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto type_ = kw["type"];

        if (type_ == g.m_parser.type_int) {
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
                "GameRule",
                nullptr,
                nullptr,
                nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(name),
                nullptr)));
        } else if (type_ == g.m_parser.type_float) {
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
                "GameRule",
                nullptr,
                nullptr,
                nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(name),
                nullptr)));
        } else {
            ErrorLogger() << "Unsupported type for rule " << name << ": " << boost::python::extract<std::string>(boost::python::str(type_))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    template <ValueRef::OpType O>
    boost::python::object insert_minmaxoneof_(const py_grammar_techs& g, const boost::python::tuple& args, const boost::python::dict& kw) {
        if (args[0] == g.m_parser.type_int) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<int>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (size_t i = 1; i < boost::python::len(args); i++) {
                auto arg = boost::python::extract<value_ref_wrapper<int>>(args[i]);
                if (arg.check()) {
                    operands.push_back(ValueRef::CloneUnique(arg().value_ref));
                } else {
                    operands.push_back(std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(args[i])()));
                }
            }
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Operation<int>>(O, std::move(operands))));
        } else if (args[0] == g.m_parser.type_float) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<double>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (size_t i = 1; i < boost::python::len(args); i++) {
                auto arg = boost::python::extract<value_ref_wrapper<double>>(args[i]);
                if (arg.check()) {
                    operands.push_back(ValueRef::CloneUnique(arg().value_ref));
                } else {
                    operands.push_back(std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(args[i])()));
                }
            }
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Operation<double>>(O, std::move(operands))));
        } else {
            ErrorLogger() << "Unsupported type for min/max/oneof : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }
}

namespace parse {
    const lexer tech_lexer;

    template <typename T>
    T techs(const PythonParser& parser, const boost::filesystem::path& path) {
        TechManager::TechContainer techs_;
        std::map<std::string, std::unique_ptr<TechCategory>> categories;
        std::set<std::string> categories_seen;

        g_categories_seen = &categories_seen;
        g_categories = &categories;

        ScopedTimer timer("Techs Parsing", true);

        py_parse::detail::parse_file<py_grammar_category, TechManager::TechContainer>(parser, path / "Categories.inf.py", py_grammar_category(), techs_);

        py_grammar_techs p = py_grammar_techs(parser);

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, TechManager::TechContainer>(tech_lexer, file, techs_);
        for (const auto& file : ListDir(path, IsFOCPyScript))
            py_parse::detail::parse_file<py_grammar_techs, TechManager::TechContainer>(parser, file, p, techs_);

        return std::make_tuple(std::move(techs_), std::move(categories), categories_seen);
    }
}

// explicitly instantiate techs.
// This allows Tech.h to only be included in this .cpp file and not Parse.h
// which recompiles all parsers if Tech.h changes.
template FO_PARSE_API TechManager::TechParseTuple parse::techs<TechManager::TechParseTuple>(const PythonParser& parser, const boost::filesystem::path& path);
