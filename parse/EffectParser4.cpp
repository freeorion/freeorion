#include "EffectParserImpl.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "ValueRefParserImpl.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace {
    struct effect_parser_rules_4 {
        effect_parser_rules_4() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::construct;
            using phoenix::push_back;

            const parse::lexer& tok = parse::lexer::instance();

            create_planet
                =   (       tok.CreatePlanet_
                        >   parse::detail::label(Type_token)        >   parse::detail::planet_type_rules().expr [ _a = _1 ]
                        >   parse::detail::label(PlanetSize_token)  >   parse::detail::planet_size_rules().expr [ _b = _1 ]
                        > -(parse::detail::label(Name_token)        >   parse::string_value_ref()      [ _c = _1 ])
                        > -(parse::detail::label(Effects_token)
                        >   (
                                ('[' > +parse::effect_parser() [ push_back(_d, _1) ] > ']')
                            |    parse::effect_parser() [ push_back(_d, _1) ]
                            )
                           )
                    ) [ _val = new_<Effect::CreatePlanet>(_a, _b, _c, _d) ]
                ;

            create_building
                =   (       tok.CreateBuilding_
                        >   parse::detail::label(Type_token)        >   parse::string_value_ref() [ _a = _1 ]
                        > -(parse::detail::label(Name_token)        >   parse::string_value_ref() [ _b = _1 ])
                        > -(parse::detail::label(Effects_token)
                        >   (
                                ('[' > +parse::effect_parser() [ push_back(_c, _1) ] > ']')
                            |    parse::effect_parser() [ push_back(_c, _1) ]
                            )
                           )
                    ) [ _val = new_<Effect::CreateBuilding>(_a, _b, _c) ]
                ;

            create_ship_1
                =   ((       tok.CreateShip_
                     >>  parse::detail::label(DesignID_token)
                     )  >   parse::int_value_ref()    [ _b = _1 ]
                        > -(parse::detail::label(Empire_token)      >   parse::int_value_ref()    [ _c = _1 ])
                        > -(parse::detail::label(Species_token)     >   parse::string_value_ref() [ _d = _1 ])
                        > -(parse::detail::label(Name_token)        >   parse::string_value_ref() [ _e = _1 ])
                        > -(parse::detail::label(Effects_token)
                        >   (
                                ('[' > +parse::effect_parser() [ push_back(_f, _1) ] > ']')
                            |    parse::effect_parser() [ push_back(_f, _1) ]
                            )
                           )
                    ) [ _val = new_<Effect::CreateShip>(_b, _c, _d, _e, _f) ]
                ;

            create_ship_2
                =   ((       tok.CreateShip_
                     >>  parse::detail::label(DesignName_token)  >>  parse::string_value_ref() [ _a = _1 ]
                     )
                        > -(parse::detail::label(Empire_token)      >   parse::int_value_ref()    [ _c = _1 ])
                        > -(parse::detail::label(Species_token)     >   parse::string_value_ref() [ _d = _1 ])
                        > -(parse::detail::label(Name_token)        >   parse::string_value_ref() [ _e = _1 ])
                        > -(parse::detail::label(Effects_token)
                        >   (
                                ('[' > +parse::effect_parser() [ push_back(_f, _1) ] > ']')
                            |    parse::effect_parser() [ push_back(_f, _1) ]
                            )
                           )
                    ) [ _val = new_<Effect::CreateShip>(_a, _c, _d, _e, _f) ]
                ;

            create_field_1
                =   ((       tok.CreateField_
                     >>  parse::detail::label(Type_token)    >>  parse::string_value_ref() [ _a = _1 ]
                     >>  parse::detail::label(Size_token)
                     )
                     >   parse::double_value_ref() [ _b = _1 ]
                     > -(parse::detail::label(Name_token)    >   parse::string_value_ref() [ _d = _1 ])
                     > -(parse::detail::label(Effects_token)
                         >
                         (
                             ('[' > +parse::effect_parser() [ push_back(_f, _1) ] > ']')
                             |    parse::effect_parser() [ push_back(_f, _1) ]
                         )
                        )
                    ) [ _val = new_<Effect::CreateField>(_a, _b, _d, _f) ]
                ;

            create_field_2
                =   ((       tok.CreateField_
                     >>  parse::detail::label(Type_token)    >>  parse::string_value_ref() [ _a = _1 ]
                     >>  parse::detail::label(X_token)
                     )
                     >   parse::double_value_ref() [ _b = _1 ]
                     >   parse::detail::label(Y_token)       >   parse::double_value_ref() [ _c = _1 ]
                     >   parse::detail::label(Size_token)    >   parse::double_value_ref() [ _e = _1 ]
                     > -(parse::detail::label(Name_token)    >   parse::string_value_ref() [ _d = _1 ])
                     > -(parse::detail::label(Effects_token)
                         >
                         (
                             ('[' > +parse::effect_parser() [ push_back(_f, _1) ] > ']')
                             |    parse::effect_parser() [ push_back(_f, _1) ]
                         )
                        )
                    ) [ _val = new_<Effect::CreateField>(_a, _b, _c, _e, _d, _f) ]
                ;

            create_system_1
                =   ((       tok.CreateSystem_
                     >>  parse::detail::label(Type_token)
                     )
                     >   parse::detail::star_type_rules().expr [ _a = _1 ]
                     >   parse::detail::label(X_token)       >   parse::double_value_ref()    [ _b = _1 ]
                     >   parse::detail::label(Y_token)       >   parse::double_value_ref()    [ _c = _1 ]
                     > -(parse::detail::label(Name_token)    >   parse::string_value_ref()    [ _d = _1 ])
                     > -(parse::detail::label(Effects_token)
                         >
                         (
                             ('[' > +parse::effect_parser() [ push_back(_e, _1) ] > ']')
                             |    parse::effect_parser() [ push_back(_e, _1) ]
                         )
                        )
                    ) [ _val = new_<Effect::CreateSystem>(_a, _b, _c, _d, _e) ]
                ;

            create_system_2
                =   ((       tok.CreateSystem_
                     >>  parse::detail::label(X_token)
                     )
                     >   parse::double_value_ref() [ _b = _1 ]
                     >   parse::detail::label(Y_token)       >   parse::double_value_ref() [ _c = _1 ]
                     > -(parse::detail::label(Name_token)    >   parse::string_value_ref() [ _d = _1 ])
                     > -(parse::detail::label(Effects_token)
                         >
                         (
                             ('[' > +parse::effect_parser() [ push_back(_e, _1) ] > ']')
                             |    parse::effect_parser() [ push_back(_e, _1) ]
                         )
                        )
                    ) [ _val = new_<Effect::CreateSystem>(_b, _c, _d, _e) ]
                ;

            start
                =   create_planet
                |   create_building
                |   create_ship_1
                |   create_ship_2
                |   create_field_1
                |   create_field_2
                |   create_system_1
                |   create_system_2
                ;

            create_planet.name("CreatePlanet");
            create_building.name("CreateBuilding");
            create_ship_1.name("CreateShip");
            create_ship_2.name("CreateShip");
            create_field_1.name("CreateField");
            create_field_2.name("CreateField");
            create_system_1.name("CreateSystem");
            create_system_2.name("CreateSystem");

#if DEBUG_EFFECT_PARSERS
            debug(create_planet);
            debug(create_building);
            debug(create_ship_1);
            debug(create_ship_2);
            debug(create_field_1);
            debug(create_field_2);
            debug(create_system_1);
            debug(create_system_2);
#endif
        }

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase< ::PlanetType>*,
                ValueRef::ValueRefBase< ::PlanetSize>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_planet_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_building_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase< ::StarType>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_system_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_ship_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                std::vector<Effect::EffectBase*>
            >
        > create_field_rule;

        create_planet_rule              create_planet;
        create_building_rule            create_building;
        create_ship_rule                create_ship_1;
        create_ship_rule                create_ship_2;
        create_field_rule               create_field_1;
        create_field_rule               create_field_2;
        create_system_rule              create_system_1;
        create_system_rule              create_system_2;
        parse::effect_parser_rule       start;
    };
}

namespace parse { namespace detail {
    const effect_parser_rule& effect_parser_4() {
        static effect_parser_rules_4 retval;
        return retval.start;
    }
} }
