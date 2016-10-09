#include "Label.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "ParseImpl.h"
#include "Parse.h"
#include "../universe/Species.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ItemSpec>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::set<std::string>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const Tech::TechInfo&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, TechCategory*>&) { return os; }
}
#endif

namespace {
    std::set<std::string>* g_categories_seen = 0;
    std::map<std::string, TechCategory*>* g_categories = 0;

    struct insert_tech_ {
        typedef void result_type;

        void operator()(TechManager::TechContainer& techs, Tech* tech) const {
            g_categories_seen->insert(tech->Category());
            if (techs.get<TechManager::NameIndex>().find(tech->Name()) != techs.get<TechManager::NameIndex>().end()) {
                std::string error_str = "ERROR: More than one tech in techs.txt has the name " + tech->Name();
                throw std::runtime_error(error_str.c_str());
            }
            if (tech->Prerequisites().find(tech->Name()) != tech->Prerequisites().end()) {
                std::string error_str = "ERROR: Tech " + tech->Name() + " depends on itself!";
                throw std::runtime_error(error_str.c_str());
            }
            techs.insert(tech);
        }
    };
    const boost::phoenix::function<insert_tech_> insert_tech;

    struct insert_category_ {
        typedef void result_type;

        void operator()(std::map<std::string, TechCategory*>& categories, TechCategory* category) const {
            if (!categories.insert(std::make_pair(category->name, category)).second) {
                std::string error_str = "ERROR: More than one tech category in techs.txt name " + category->name;
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_category_> insert_category;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<double>::type& double_value_ref =    parse::value_ref_parser<double>();
            const parse::value_ref_parser_rule< int >::type& flexible_int_ref =     parse::value_ref_parser_flexible_int();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_g_type _g;
            qi::_h_type _h;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::construct;
            using phoenix::insert;
            using phoenix::new_;
            using phoenix::push_back;

            tech_info_name_desc
                =   parse::label(Name_token)              > tok.string [ _r1 = _1 ]
                >   parse::label(Description_token)       > tok.string [ _r2 = _1 ]
                >   parse::label(Short_Description_token) > tok.string [ _r3 = _1 ] // TODO: Get rid of underscore.
                ;

            tech_info
                =   tech_info_name_desc(_a, _b, _c)
                >   parse::label(Category_token)      > tok.string      [ _e = _1 ]
                >   parse::label(ResearchCost_token)  > double_value_ref[ _f = _1 ]
                >   parse::label(ResearchTurns_token) > flexible_int_ref[ _g = _1 ]
                >  (    tok.Unresearchable_ [ _h = false ]
                    |   tok.Researchable_ [ _h = true ]
                    |   eps [ _h = true ]
                   )
                >   parse::detail::tags_parser()(_d)
                [ _val = construct<Tech::TechInfo>(_a, _b, _c, _e, _f, _g, _h, _d) ]
                ;

            prerequisites
                =   parse::label(Prerequisites_token)
                >  (    '[' > +tok.string [ insert(_r1, _1) ] > ']'
                    |   tok.string [ insert(_r1, _1) ]
                   )
                ;

            unlocks
                =   parse::label(Unlock_token)
                >  (    '[' > +parse::detail::item_spec_parser() [ push_back(_r1, _1) ] > ']'
                    |   parse::detail::item_spec_parser() [ push_back(_r1, _1) ]
                   )
                ;

            tech
                =  (tok.Tech_
                >   tech_info [ _a = _1 ]
                >  -prerequisites(_b)
                >  -unlocks(_c)
                > -(parse::label(EffectsGroups_token) > parse::detail::effects_group_parser() [ _d = _1 ])
                > -(parse::label(Graphic_token) > tok.string [ _e = _1 ])
                   )
                [ insert_tech(_r1, new_<Tech>(_a, _d, _b, _c, _e)) ]
                ;

            category
                =   tok.Category_
                >   parse::label(Name_token)    > tok.string [ _a = _1 ]
                >   parse::label(Graphic_token) > tok.string [ _b = _1 ]
                >   parse::label(Colour_token)  > parse::detail::color_parser() [ insert_category(_r1, new_<TechCategory>(_a, _b, _1)) ]
                ;

            start
                = +(tech(_r1)
                |   category(phoenix::ref(*g_categories)) // TODO: Using _r2 here as I would like to do seems to give GCC 4.6 fits.
                   )
                ;

            tech_info_name_desc.name("tech name");
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

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Tech::TechInfo (std::string&, std::string&, std::string&),
            parse::skipper_type
        > tech_info_name_desc_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Tech::TechInfo (),
            qi::locals<
                std::string,
                std::string,
                std::string,
                std::set<std::string>,
                std::string,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<int>*,
                bool
            >,
            parse::skipper_type
        > tech_info_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Tech::TechInfo (std::set<std::string>&),
            parse::skipper_type
        > prerequisites_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Tech::TechInfo (std::vector<ItemSpec>&),
            parse::skipper_type
        > unlocks_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (TechManager::TechContainer&),
            qi::locals<
                Tech::TechInfo,
                std::set<std::string>,
                std::vector<ItemSpec>,
                std::vector<boost::shared_ptr<Effect::EffectsGroup> >,
                std::string
            >,
            parse::skipper_type
        > tech_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, TechCategory*>&),
            qi::locals<
                std::string,
                std::string
            >,
            parse::skipper_type
        > category_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (TechManager::TechContainer&),
            parse::skipper_type
        > start_rule;

        tech_info_name_desc_rule    tech_info_name_desc;
        tech_info_rule              tech_info;
        prerequisites_rule          prerequisites;
        unlocks_rule                unlocks;
        tech_rule                   tech;
        category_rule               category;
        start_rule                  start;
    };
}

namespace parse {
    bool techs(TechManager::TechContainer& techs_,
               std::map<std::string, TechCategory*>& categories,
               std::set<std::string>& categories_seen)
    {
        bool result = true;

        std::vector<boost::filesystem::path> file_list = ListScripts("scripting/techs");

        g_categories_seen = &categories_seen;
        g_categories = &categories;

        result &= detail::parse_file<rules, TechManager::TechContainer>(GetResourceDir() / "scripting/techs/Categories.inf", techs_);

        for (std::vector<boost::filesystem::path>::iterator file_it = file_list.begin();
             file_it != file_list.end(); ++file_it)
        {
            boost::filesystem::path path = *file_it;

            result &= detail::parse_file<rules, TechManager::TechContainer>(path, techs_);
        }

        return result;
    }
}
