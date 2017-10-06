#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"

#include "../universe/Species.h"
#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ItemSpec>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::set<std::string>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::shared_ptr<Effect::EffectsGroup>>&) { return os; }
    inline ostream& operator<<(ostream& os, const Tech::TechInfo&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, TechCategory*>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    std::set<std::string>* g_categories_seen = nullptr;
    std::map<std::string, TechCategory*>* g_categories = nullptr;

    /// Check if the tech will be unique.
    struct check_tech {
        typedef bool result_type;

        result_type operator()(TechManager::TechContainer& techs, Tech* tech) const {
            auto retval = true;
            if (techs.get<TechManager::NameIndex>().find(tech->Name()) != techs.get<TechManager::NameIndex>().end()) {
                ErrorLogger() <<  "More than one tech has the name " << tech->Name();
                retval = false;
            }
            if (tech->Prerequisites().find(tech->Name()) != tech->Prerequisites().end()) {
                ErrorLogger() << "Tech " << tech->Name() << " depends on itself!";
                retval = false;
            }
            return retval;
        }
    };

    struct insert_tech {
        typedef void result_type;

        result_type operator()(TechManager::TechContainer& techs, Tech* tech) const {
            g_categories_seen->insert(tech->Category());
            techs.insert(tech);
        }
    };

    const boost::phoenix::function<check_tech> check_tech_;
    const boost::phoenix::function<insert_tech> insert_tech_;

    struct insert_category {
        typedef void result_type;

        void operator()(std::map<std::string, TechCategory*>& categories, TechCategory* category) const {
            categories.insert(std::make_pair(category->name, category));
        }
    };
    const boost::phoenix::function<insert_category> insert_category_;


    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;
            using phoenix::insert;
            using phoenix::new_;
            using phoenix::push_back;

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
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            qi::_val_type _val;
            qi::eps_type eps;

            const parse::lexer& tok = parse::lexer::instance();

            tech_info_name_desc
                =   parse::detail::label(Name_token)              > tok.string [ _r1 = _1 ]
                >   parse::detail::label(Description_token)       > tok.string [ _r2 = _1 ]
                >   parse::detail::label(Short_Description_token) > tok.string [ _r3 = _1 ] // TODO: Get rid of underscore.
                ;

            tech_info
                =   tech_info_name_desc(_a, _b, _c)
                >   parse::detail::label(Category_token)      > tok.string      [ _e = _1 ]
                >   parse::detail::label(ResearchCost_token)  > parse::double_value_ref() [ _f = _1 ]
                >   parse::detail::label(ResearchTurns_token) > parse::flexible_int_value_ref() [ _g = _1 ]
                >  (    tok.Unresearchable_ [ _h = false ]
                    |   tok.Researchable_ [ _h = true ]
                    |   eps [ _h = true ]
                   )
                >   parse::detail::tags_parser()(_d)
                [ _val = construct<Tech::TechInfo>(_a, _b, _c, _e, _f, _g, _h, _d) ]
                ;

            prerequisites
                =   parse::detail::label(Prerequisites_token)
                >  (    ('[' > +tok.string [ insert(_r1, _1) ] > ']')
                    |    tok.string [ insert(_r1, _1) ]
                   )
                ;

            unlocks
                =   parse::detail::label(Unlock_token)
                >  (    ('[' > +parse::detail::item_spec_parser() [ push_back(_r1, _1) ] > ']')
                    |    parse::detail::item_spec_parser() [ push_back(_r1, _1) ]
                   )
                ;

            tech
                =  (tok.Tech_
                >   tech_info [ _a = _1 ]
                >  -prerequisites(_b)
                >  -unlocks(_c)
                > -(parse::detail::label(EffectsGroups_token) > parse::detail::effects_group_parser() [ _d = _1 ])
                > -(parse::detail::label(Graphic_token) > tok.string [ _e = _1 ])
                   )
                [ _f = new_<Tech>(_a, _d, _b, _c, _e), _pass = check_tech_(_r1, _f), insert_tech_(_r1, _f) ]
                ;

            category
                =   tok.Category_
                >   parse::detail::label(Name_token)    > tok.string [ _pass = is_unique_(_r1, Category_token, _1), _a = _1 ]
                >   parse::detail::label(Graphic_token) > tok.string [ _b = _1 ]
                >   parse::detail::label(Colour_token)  > parse::detail::color_parser() [ insert_category_(_r1, new_<TechCategory>(_a, _b, _1)) ]
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

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            Tech::TechInfo (std::string&, std::string&, std::string&)
        > tech_info_name_desc_rule;

        typedef parse::detail::rule<
            Tech::TechInfo (),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::string,
                std::set<std::string>,
                std::string,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<int>*,
                bool
            >
        > tech_info_rule;

        typedef parse::detail::rule<
            Tech::TechInfo (std::set<std::string>&)
        > prerequisites_rule;

        typedef parse::detail::rule<
            Tech::TechInfo (std::vector<ItemSpec>&)
        > unlocks_rule;

        typedef parse::detail::rule<
            void (TechManager::TechContainer&),
            boost::spirit::qi::locals<
                Tech::TechInfo,
                std::set<std::string>,
                std::vector<ItemSpec>,
                std::vector<std::shared_ptr<Effect::EffectsGroup>>,
                std::string,
                Tech*
            >
        > tech_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, TechCategory*>&),
            boost::spirit::qi::locals<
                std::string,
                std::string
            >
        > category_rule;

        typedef parse::detail::rule<
            void (TechManager::TechContainer&)
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

        g_categories_seen = &categories_seen;
        g_categories = &categories;

        result &= detail::parse_file<rules, TechManager::TechContainer>(GetResourceDir() / "scripting/techs/Categories.inf", techs_);

        for (const boost::filesystem::path& file : ListScripts("scripting/techs")) {
            result &= detail::parse_file<rules, TechManager::TechContainer>(file, techs_);
        }

        return result;
    }
}
