#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "EffectParser.h"
#include "ValueRefParser.h"
#include "MovableEnvelope.h"

#include "../universe/Effect.h"
#include "../universe/Species.h"
#include "../universe/Tech.h"
#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi_as.hpp>


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

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_category_, insert_category, 4)


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

            prerequisites
                %=   label(tok.Prerequisites_)
                >  one_or_more_string_tokens
                ;

            unlocks
                %=   label(tok.Unlock_)
                >  one_or_more_unlockable_items
                ;

            tech
                = ( omit_[tok.Tech_]
                >   tech_info
                >  -prerequisites
                >  -unlocks
                >  -(label(tok.effectsgroups_) > effects_group_grammar)
                >  -as_string_[(label(tok.graphic_) > tok.string)]
                  ) [ insert_tech_(_r1, _1, _4, _2, _3, _5, _pass) ]
                ;

            category
                = ( tok.Category_
                    >   label(tok.name_)    > tok.string
                    >   label(tok.graphic_) > tok.string
                    >   label(tok.Colour_)  > color_parser
                  ) [ _pass = is_unique_(_r1, _1, _2),
                      insert_category_(_r1, _2, _3, _4) ]
                ;

            start
                = +(tech(_r1)
                |   category(phoenix::ref(*g_categories)) // TODO: Using _r2 here as I would like to do seems to give GCC 4.6 fits.
                   )
                ;

            researchable.name("Researchable");
            tech_info.name("Tech info");
            prerequisites.name("Prerequisites");
            unlocks.name("Unlock");
            tech.name("Tech");
            category.name("Category");
            start.name("start");

#if DEBUG_PARSERS
            debug(tech_info_name_desc);
            debug(tech_info);
            debug(prerequisites);
            debug(unlocks);
            debug(tech);
            debug(category);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using tech_info_rule = parse::detail::rule<parse::detail::MovableEnvelope<Tech::TechInfo> ()>;
        using prerequisites_rule = parse::detail::rule<std::set<std::string> ()>;
        using unlocks_rule = parse::detail::rule<std::vector<UnlockableItem> ()>;
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
        prerequisites_rule                      prerequisites;
        unlocks_rule                            unlocks;
        tech_rule                               tech;
        category_rule                           category;
        start_rule                              start;
    };
}

namespace parse {
    template <typename T>
    T techs(const boost::filesystem::path& path) {
        const lexer lexer;
        TechManager::TechContainer techs_;
        std::map<std::string, std::unique_ptr<TechCategory>> categories;
        std::set<std::string> categories_seen;

        g_categories_seen = &categories_seen;
        g_categories = &categories;

        ScopedTimer timer("Techs Parsing", true);

        detail::parse_file<grammar, TechManager::TechContainer>(lexer, path / "Categories.inf", techs_);

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, TechManager::TechContainer>(lexer, file, techs_);

        return std::make_tuple(std::move(techs_), std::move(categories), categories_seen);
    }
}

// explicitly instantiate techs.
// This allows Tech.h to only be included in this .cpp file and not Parse.h
// which recompiles all parsers if Tech.h changes.
template FO_PARSE_API TechManager::TechParseTuple parse::techs<TechManager::TechParseTuple>(const boost::filesystem::path& path);
