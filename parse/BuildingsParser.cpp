#define FUSION_MAX_VECTOR_SIZE 20

#include "Double.h"
#include "Int.h"
#include "Label.h"
#include "ParseImpl.h"
#include "../universe/Building.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, BuildingType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, BuildingType*>&) { return os; }
}
#endif

namespace {

    struct insert_
    {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(std::map<std::string, BuildingType*>& building_types, BuildingType* building_type) const
            {
                if (!building_types.insert(std::make_pair(building_type->Name(), building_type)).second) {
                    std::string error_str = "ERROR: More than one building type in buildings.txt has the name " + building_type->Name();
                    throw std::runtime_error(error_str.c_str());
                }
            }
    };
    const boost::phoenix::function<insert_> insert;

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
                qi::_val_type _val;
                qi::eps_type eps;
                using phoenix::new_;

                building_type
                    =    tok.BuildingType_
                    >    parse::label(Name_name)        > tok.string [ _a = _1 ]
                    >    parse::label(Description_name) > tok.string [ _b = _1 ]
                    >    parse::label(BuildCost_name)   > parse::double_ [ _c = _1 ]
                    >    parse::label(BuildTime_name)   > parse::int_ [ _d = _1 ]
                    >    (
                             tok.Unproducible_ [ _e = false ]
                          |  tok.Producible_ [ _e = true ]
                          |  eps [ _e = true ]
                         )
                    >    parse::label(Location_name) > parse::detail::condition_parser [ _f = _1 ]
                    >    (
                              parse::label(CaptureResult_name) >> parse::enum_parser<CaptureResult>() [ _g = _1 ]
                          |   eps [ _g = CR_CAPTURE ]
                         )
                    >    parse::label(EffectsGroups_name) > -parse::detail::effects_group_parser() [ _h = _1 ]
                    >    parse::label(Graphic_name)       >  tok.string [ insert(_r1, new_<BuildingType>(_a, _b, _c, _d, _e, _g, _f, _h, _1)) ]
                    ;

                start
                    =   +building_type(_r1)
                    ;

                building_type.name("BuildingType");

#if DEBUG_PARSERS
                debug(building_type);
#endif

                qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
            }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, BuildingType*>&),
            qi::locals<
                std::string,
                std::string,
                double,
                int,
                bool,
                Condition::ConditionBase*,
                CaptureResult,
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
            >,
            parse::skipper_type
        > building_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, BuildingType*>&),
            parse::skipper_type
        > start_rule;

        building_type_rule building_type;
        start_rule start;
    };

}

namespace parse {

    bool buildings(const boost::filesystem::path& path, std::map<std::string, BuildingType*>& building_types)
    { return detail::parse_file<rules, std::map<std::string, BuildingType*> >(path, building_types); }

}
