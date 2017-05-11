#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "ConditionParserImpl.h"
#include "CommonParams.h"

#include "../universe/ShipDesign.h"
#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipSlotType>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::shared_ptr<Effect::EffectsGroup>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, PartType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, PartType*>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    const boost::phoenix::function<parse::detail::insert> insert_;

    struct rules {
        rules() {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;
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
            qi::eps_type eps;

            const parse::lexer& tok = parse::lexer::instance();

            slots
                =  -(
                        parse::detail::label(MountableSlotTypes_token)
                    >   (
                            ('[' > +parse::ship_slot_type_enum() [ push_back(_r1, _1) ] > ']')
                        |    parse::ship_slot_type_enum() [ push_back(_r1, _1) ]
                        )
                     )
                ;

            part_type
                = ( tok.Part_
                >   parse::detail::more_common_params_parser()
                    [_pass = is_unique_(_r1, "PartType", phoenix::bind(&MoreCommonParams::name, _1)), _a = _1 ]
                >   parse::detail::label(Class_token)       > parse::ship_part_class_enum() [ _c = _1 ]
                > (  (parse::detail::label(Capacity_token)  > parse::detail::double_ [ _d = _1 ])
                   | (parse::detail::label(Damage_token)    > parse::detail::double_ [ _d = _1 ])
                   |  eps [ _d = 0.0 ]
                  )
                > (  (parse::detail::label(Damage_token)    > parse::detail::double_ [ _h = _1 ])   // damage is secondary for fighters
                   | (parse::detail::label(Shots_token)     > parse::detail::double_ [ _h = _1 ])   // shots is secondary for direct fire weapons
                   |  eps [ _h = 1.0 ]
                  )
                > (   tok.NoDefaultCapacityEffect_ [ _g = false ]
                   |  eps [ _g = true ]
                  )
                >   slots(_f)
                >   parse::detail::common_params_parser()           [ _e = _1 ]
                >   parse::detail::label(Icon_token)        > tok.string    [ _b = _1 ]
                  ) [ insert_(_r1, phoenix::bind(&MoreCommonParams::name, _a),
                              new_<PartType>(_c, _d, _h, _e, _a, _f, _b, _g)) ]
                ;

            start
                =   +part_type(_r1)
                ;

            slots.name("mountable slot types");
            part_type.name("Part");

#if DEBUG_PARSERS
            debug(slots);
            debug(part_type);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (std::vector<ShipSlotType>&)
        > slots_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, PartType*>&),
            boost::spirit::qi::locals<
                MoreCommonParams,
                std::string,
                ShipPartClass,
                double,
                CommonParams,
                std::vector<ShipSlotType>,
                bool,
                double
            >
        > part_type_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, PartType*>&)
        > start_rule;

        slots_rule                                  slots;
        part_type_rule                              part_type;
        start_rule                                  start;
    };

}

namespace parse {
    bool ship_parts(std::map<std::string, PartType*>& parts) {
        bool result = true;

        for(const boost::filesystem::path& file : ListScripts("scripting/ship_parts")) {
            result &= detail::parse_file<rules, std::map<std::string, PartType*>>(file, parts);
        }

        return result;
    }
}
