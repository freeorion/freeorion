#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "Double.h"
#include "Label.h"
#include "Parse.h"
#include "ParseImpl.h"
#include "ValueRefParser.h"
#include "CommonParams.h"
#include "../universe/ShipDesign.h"
#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipSlotType>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, PartType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, PartType*>&) { return os; }
}
#endif

namespace {
    struct insert_part_type_ {
#if BOOST_VERSION < 105600
        template <typename Arg1, typename Arg2> // Phoenix v2
        struct result
        { typedef void type; };
#else
        typedef void result_type;
#endif

        void operator()(std::map<std::string, PartType*>& part_types, PartType* part_type) const {
            if (!part_types.insert(std::make_pair(part_type->Name(), part_type)).second) {
                std::string error_str = "ERROR: More than one ship part in ship_parts.txt has the name " + part_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_part_type_> insert_part_type;


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
            qi::eps_type eps;
            using phoenix::construct;
            using phoenix::new_;
            using phoenix::push_back;

            slots
                =  -(
                        parse::label(MountableSlotTypes_token)
                    >   (
                            '[' > +parse::enum_parser<ShipSlotType>() [ push_back(_r1, _1) ] > ']'
                        |   parse::enum_parser<ShipSlotType>() [ push_back(_r1, _1) ]
                        )
                     )
                ;

            part_type
                = ( tok.Part_
                >   parse::detail::more_common_params_parser()       [ _a = _1 ]
                >   parse::label(Class_token)       > parse::enum_parser<ShipPartClass>() [ _c = _1 ]
                > (  (parse::label(Capacity_token)  > parse::double_ [ _d = _1 ])
                   | (parse::label(Damage_token)    > parse::double_ [ _d = _1 ])
                   |  eps [ _d = 0.0 ]
                  )
                > (  (parse::label(Damage_token)    > parse::double_ [ _h = _1 ])   // damage is secondary for fighters
                   | (parse::label(Shots_token)     > parse::double_ [ _h = _1 ])   // shots is secondary for direct fire weapons
                   |  eps [ _h = 1.0 ]
                  )
                > (   tok.NoDefaultCapacityEffect_ [ _g = false ]
                   |  eps [ _g = true ]
                  )
                >   slots(_f)
                >   parse::detail::common_params_parser()           [ _e = _1 ]
                >   parse::label(Icon_token)        > tok.string    [ _b = _1 ]
                  ) [ insert_part_type(_r1, new_<PartType>(_c, _d, _h, _e, _a, _f, _b, _g)) ]
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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<ShipSlotType>&),
            parse::skipper_type
        > slots_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, PartType*>&),
            qi::locals<
                MoreCommonParams,
                std::string,
                ShipPartClass,
                double,
                CommonParams,
                std::vector<ShipSlotType>,
                bool,
                double
            >,
            parse::skipper_type
        > part_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, PartType*>&),
            parse::skipper_type
        > start_rule;

        slots_rule                                  slots;
        part_type_rule                              part_type;
        start_rule                                  start;
    };

}

namespace parse {
    bool ship_parts(std::map<std::string, PartType*>& parts) {
        bool result = true;

        std::vector<boost::filesystem::path> file_list = ListScripts("scripting/ship_parts");

        for(std::vector<boost::filesystem::path>::iterator file_it = file_list.begin();
            file_it != file_list.end(); ++file_it)
        {
            result &= detail::parse_file<rules, std::map<std::string, PartType*> >(*file_it, parts);
        }

        return result;
    }
}
