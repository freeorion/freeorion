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
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipSlotType>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<PartType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<PartType>>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_parttype(std::map<std::string, std::unique_ptr<PartType>>& part_types,
                         ShipPartClass part_class,
                         std::pair<double, double> capacity_and_stat2,
                         const parse::detail::MovableEnvelope<CommonParams>& common_params,
                         const MoreCommonParams& more_common_params,
                         std::vector<ShipSlotType> mountable_slot_types,
                         const std::string& icon,
                         bool add_standard_capacity_effect,
                         bool& pass)
    {
        auto part_type = boost::make_unique<PartType>(
            part_class, capacity_and_stat2.first, capacity_and_stat2.second,
            *common_params.OpenEnvelope(pass), more_common_params, mountable_slot_types, icon,
            add_standard_capacity_effect);

        part_types.insert(std::make_pair(part_type->Name(), std::move(part_type)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_parttype_, insert_parttype, 9)

    using start_rule_payload = std::map<std::string, std::unique_ptr<PartType>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            labeller(tok),
            condition_parser(tok, labeller),
            string_grammar(tok, labeller, condition_parser),
            tags_parser(tok, labeller),
            common_rules(tok, labeller, condition_parser, string_grammar, tags_parser),
            ship_slot_type_enum(tok),
            ship_part_class_enum(tok),
            double_rule(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::push_back;
            using phoenix::construct;

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
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::eps_type eps;

            slots
                =  -(
                        labeller.rule(MountableSlotTypes_token)
                    >   (
                            ('[' > +ship_slot_type_enum [ push_back(_r1, _1) ] > ']')
                        |    ship_slot_type_enum [ push_back(_r1, _1) ]
                        )
                     )
                ;

            part_type
                = ( tok.Part_
                >   common_rules.more_common
                    [_pass = is_unique_(_r1, PartType_token, phoenix::bind(&MoreCommonParams::name, _1)), _a = _1 ]
                >   labeller.rule(Class_token)       > ship_part_class_enum [ _c = _1 ]
                > (  (labeller.rule(Capacity_token)  > double_rule [ _d = _1 ])
                   | (labeller.rule(Damage_token)    > double_rule [ _d = _1 ])
                   |  eps [ _d = 0.0 ]
                  )
                > (  (labeller.rule(Damage_token)    > double_rule [ _h = _1 ])   // damage is secondary for fighters
                   | (labeller.rule(Shots_token)     > double_rule [ _h = _1 ])   // shots is secondary for direct fire weapons
                   |  eps [ _h = 1.0 ]
                  )
                > (   tok.NoDefaultCapacityEffect_ [ _g = false ]
                   |  eps [ _g = true ]
                  )
                >   slots(_f)
                >   common_rules.common           [ _e = _1 ]
                >   labeller.rule(Icon_token)        > tok.string    [ _b = _1 ]
                  ) [ _i = construct<std::pair<double, double>>(_d, _h),
                      insert_parttype_(_r1, _c, _i, _e, _a, _f, _b, _g, _pass) ]
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

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (std::vector<ShipSlotType>&)
        > slots_rule;

        typedef parse::detail::rule<
            void (start_rule_payload&),
            boost::spirit::qi::locals<
                MoreCommonParams,
                std::string,
                ShipPartClass,
                double,
                parse::detail::MovableEnvelope<CommonParams>,
                std::vector<ShipSlotType>,
                bool,
                double,
                std::pair<double, double>
            >
        > part_type_rule;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller labeller;
        const parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::detail::tags_grammar tags_parser;
        parse::detail::common_params_rules common_rules;
        parse::ship_slot_enum_grammar  ship_slot_type_enum;
        parse::ship_part_class_enum_grammar ship_part_class_enum;
        parse::detail::double_grammar double_rule;
        slots_rule                         slots;
        part_type_rule                     part_type;
        start_rule                         start;
    };

}

namespace parse {
    start_rule_payload ship_parts(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload parts;

        for (const auto& file : ListScripts(path)) {
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, parts);
        }

        return parts;
    }
}
