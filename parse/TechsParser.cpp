#define FUSION_MAX_VECTOR_SIZE 20

#include "Double.h"
#include "Int.h"
#include "Label.h"
#include "ParseImpl.h"
#include "../universe/Species.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ItemSpec>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::set<std::string>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const Tech::TechInfo&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, TechCategory*>&) { return os; }
}
#endif

namespace {

    std::set<std::string>* g_categories_seen = 0;
    std::map<std::string, TechCategory*>* g_categories = 0;

    struct insert_tech_
    {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(TechManager::TechContainer& techs, Tech* tech) const
            {
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

    struct insert_category_
    {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(std::map<std::string, TechCategory*>& categories, TechCategory* category) const
            {
                if (!categories.insert(std::make_pair(category->name, category)).second) {
                    std::string error_str = "ERROR: More than one tech category in techs.txt name " + category->name;
                    throw std::runtime_error(error_str.c_str());
                }
            }
    };
    const boost::phoenix::function<insert_category_> insert_category;

    struct rules
    {
        rules()
            {
                const parse::lexer& tok = parse::lexer::instance();

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
                qi::_r1_type _r2;
                qi::_val_type _val;
                qi::eps_type eps;
                using phoenix::construct;
                using phoenix::insert;
                using phoenix::new_;
                using phoenix::push_back;

                tech_info
                    =    parse::label(Name_name)              > tok.string [ _a = _1 ]
                    >    parse::label(Description_name)       > tok.string [ _b = _1 ]
                    >    parse::label(Short_Description_name) > tok.string [ _c = _1 ] // TODO: Get rid of underscore.
                    >>  -(
                              parse::label(TechType_name) >> parse::enum_parser<TechType>() [ _d = _1 ]
                          |   eps [ _d = TT_THEORY ]
                         )
                    >    parse::label(Category_name)          > tok.string [ _e = _1 ]
                    >>   (
                              parse::label(ResearchCost_name) >> parse::double_ [ _f = _1 ]
                          |   eps [ _f = 1.0 ]
                         )
                    >>   (
                              parse::label(ResearchTurns_name) >> parse::int_ [ _g = _1 ]
                          |   eps [ _g = 1 ]
                         )
                    >>   (
                              tok.Unresearchable_ [ _h = false ]
                          |   tok.Researchable_ [ _h = true ]
                          |   eps [ _h = true ]
                         )
                         [ _val = construct<Tech::TechInfo>(_a, _b, _c, _e, _d, _f, _g, _h) ]
                    ;

                tech
                    =    (
                              tok.Tech_
                         >    tech_info [ _a = _1 ]
                         >   -(
                                   parse::label(Prerequisites_name)
                               >>  (
                                        '[' > +tok.string [ insert(_b, _1) ] > ']'
                                    |   tok.string [ insert(_b, _1) ]
                                   )
                              )
                         >   -(
                                   parse::label(Unlock_name)
                               >>  (
                                        '[' > +parse::detail::item_spec_parser() [ push_back(_c, _1) ] > ']'
                                    |   parse::detail::item_spec_parser() [ push_back(_c, _1) ]
                                   )
                              )
                         >   -(
                                   parse::label(EffectsGroups_name) >> parse::detail::effects_group_parser() [ _d = _1 ]
                              )
                         >   -(
                                   parse::label(Graphic_name) >> tok.string [ _e = _1 ]
                              )
                         )
                         [ insert_tech(_r1, new_<Tech>(_a, _d, _b, _c, _e)) ]
                    ;

                category
                    =    tok.Category_
                    >    parse::label(Name_name)    > tok.string [ _a = _1 ]
                    >    parse::label(Graphic_name) > tok.string [ _b = _1 ]
                    >    parse::label(Colour_name)  > parse::detail::color_parser() [ insert_category(_r1, new_<TechCategory>(_a, _b, _1)) ]
                    ;

                start
                    =   +(
                              tech(_r1)
                          |   category(phoenix::ref(*g_categories)) // TODO: Using _r2 here as I would like to do seems to give GCC 4.6 fits.
                         )
                    ;

                tech_info.name("Tech info");
                tech.name("Tech");
                category.name("Category");
                start.name("start");

#if DEBUG_PARSERS
                debug(tech_info);
                debug(tech);
                debug(category);
                debug(start);
#endif

                qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
            }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Tech::TechInfo (),
            qi::locals<
                std::string,
                std::string,
                std::string,
                TechType,
                std::string,
                double,
                int,
                bool
            >,
            parse::skipper_type
        > tech_info_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (TechManager::TechContainer&),
            qi::locals<
                Tech::TechInfo,
                std::set<std::string>,
                std::vector<ItemSpec>,
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
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

        tech_info_rule tech_info;
        tech_rule tech;
        category_rule category;
        start_rule start;
    };

}

namespace parse {

    bool techs(const boost::filesystem::path& path,
               TechManager::TechContainer& techs_,
               std::map<std::string, TechCategory*>& categories,
               std::set<std::string>& categories_seen)
    {
        g_categories_seen = &categories_seen;
        g_categories = &categories;
        return detail::parse_file<rules, TechManager::TechContainer>(path, techs_);
    }

}
