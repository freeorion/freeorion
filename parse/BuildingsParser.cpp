#include "Double.h"
#include "Int.h"
#include "Label.h"
#include "ParseImpl.h"
#include "../universe/Building.h"

//#include "ReportParseError.h"

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, BuildingType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, BuildingType*>&) { return os; }
}
#endif

namespace {
    struct insert_ {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(std::map<std::string, BuildingType*>& building_types, BuildingType* building_type) const {
            if (!building_types.insert(std::make_pair(building_type->Name(), building_type)).second) {
                std::string error_str = "ERROR: More than one building type in buildings.txt has the name " + building_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules {
        rules() {
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
            qi::_i_type _i;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::push_back;

            building_prefix
                =    tok.BuildingType_
                >    parse::label(Name_name)        > tok.string [ _r1 = _1 ]
                >    parse::label(Description_name) > tok.string [ _r2 = _1 ]
                ;

            cost
                =    parse::label(BuildCost_name)   > parse::double_ [ _r1 = _1 ]
                >    parse::label(BuildTime_name)   > parse::int_ [ _r2 = _1 ]
                ;

            producible
                =    tok.Unproducible_ [ _val = false ]
                |    tok.Producible_ [ _val = true ]
                |    eps [ _val = true ]
                ;

            tags
                =  -(
                        parse::label(Tags_name)
                    >>  (
                            '[' > +tok.string [ push_back(_r1, _1) ] > ']'
                            |   tok.string [ push_back(_r1, _1) ]
                        )
                    )
                ;

            building_type
                =    building_prefix(_a, _b)
                >    cost(_c, _d)
                >    producible [ _e = _1 ]
                >    (
                            parse::label(CaptureResult_name)    >> parse::enum_parser<CaptureResult>() [ _f = _1 ]
                        |   eps [ _f = CR_CAPTURE ]
                     )
                >    tags(_g)
                >    parse::label(Location_name)                > parse::detail::condition_parser [ _h = _1 ]
                >    parse::label(EffectsGroups_name)           > -parse::detail::effects_group_parser() [ _i = _1 ]
                >    parse::label(Icon_name)                    >  tok.string
                    [ insert(_r1, new_<BuildingType>(_a, _b, _c, _d, _e, _f, _g, _h, _i, _1)) ]
                ;

            start
                =   +building_type(_r1)
                ;

            building_prefix.name("BuildingType");
            cost.name("cost");
            producible.name("Producible or Unproducible");
            tags.name("Tags");
            building_type.name("BuildingType");

#if DEBUG_PARSERS
            debug(building_prefix);
            debug(cost);
            debug(producible);
            debug(tags);
            debug(building_type);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::string&, std::string&),
            parse::skipper_type
        > building_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (double&, int&),
            parse::skipper_type
        > cost_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            bool (),
            parse::skipper_type
        > producible_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<std::string>&),
            parse::skipper_type
        > tags_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, BuildingType*>&),
            qi::locals<
                std::string,
                std::string,
                double,
                int,
                bool,
                CaptureResult,
                std::vector<std::string>,
                Condition::ConditionBase*,
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
            >,
            parse::skipper_type
        > building_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, BuildingType*>&),
            parse::skipper_type
        > start_rule;

        building_prefix_rule    building_prefix;
        cost_rule               cost;
        producible_rule         producible;
        tags_rule               tags;
        building_type_rule      building_type;
        start_rule              start;
    };
}

namespace parse {
    bool buildings(const boost::filesystem::path& path, std::map<std::string, BuildingType*>& building_types)
    { return detail::parse_file<rules, std::map<std::string, BuildingType*> >(path, building_types); }
}
