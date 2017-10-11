#include "EffectParser4.h"

#include "ValueRefParserImpl.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    effect_parser_rules_4::effect_parser_rules_4(
        const parse::lexer& tok,
        const effect_parser_grammar& effect_parser,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        effect_parser_rules_4::base_type(start, "effect_parser_rules_4"),
        int_rules(tok, labeller, condition_parser, string_grammar),
        double_rules(tok, labeller, condition_parser, string_grammar),
        star_type_rules(tok, labeller, condition_parser),
        planet_type_rules(tok, labeller, condition_parser),
        planet_size_rules(tok, labeller, condition_parser)
    {
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

        create_planet
            =   (       tok.CreatePlanet_
                        >   labeller.rule(Type_token)        >   planet_type_rules.expr [ _a = _1 ]
                        >   labeller.rule(PlanetSize_token)  >   planet_size_rules.expr [ _b = _1 ]
                        > -(labeller.rule(Name_token)        >   string_grammar      [ _c = _1 ])
                        > -(labeller.rule(Effects_token)
                            >   (
                                ('[' > +effect_parser [ push_back(_d, _1) ] > ']')
                                |    effect_parser [ push_back(_d, _1) ]
                            )
                           )
                ) [ _val = new_<Effect::CreatePlanet>(_a, _b, _c, _d) ]
            ;

        create_building
            =   (       tok.CreateBuilding_
                        >   labeller.rule(Type_token)        >   string_grammar [ _a = _1 ]
                        > -(labeller.rule(Name_token)        >   string_grammar [ _b = _1 ])
                        > -(labeller.rule(Effects_token)
                            >   (
                                ('[' > +effect_parser [ push_back(_c, _1) ] > ']')
                                |    effect_parser [ push_back(_c, _1) ]
                            )
                           )
                ) [ _val = new_<Effect::CreateBuilding>(_a, _b, _c) ]
            ;

        create_ship_1
            =   ((       tok.CreateShip_
                         >>  labeller.rule(DesignID_token)
                 )  >   int_rules.expr    [ _b = _1 ]
                 > -(labeller.rule(Empire_token)      >   int_rules.expr    [ _c = _1 ])
                 > -(labeller.rule(Species_token)     >   string_grammar [ _d = _1 ])
                 > -(labeller.rule(Name_token)        >   string_grammar [ _e = _1 ])
                 > -(labeller.rule(Effects_token)
                     >   (
                         ('[' > +effect_parser [ push_back(_f, _1) ] > ']')
                         |    effect_parser [ push_back(_f, _1) ]
                     )
                    )
                ) [ _val = new_<Effect::CreateShip>(_b, _c, _d, _e, _f) ]
            ;

        create_ship_2
            =   ((       tok.CreateShip_
                         >>  labeller.rule(DesignName_token)  >>  string_grammar [ _a = _1 ]
                 )
                 > -(labeller.rule(Empire_token)      >   int_rules.expr    [ _c = _1 ])
                 > -(labeller.rule(Species_token)     >   string_grammar [ _d = _1 ])
                 > -(labeller.rule(Name_token)        >   string_grammar [ _e = _1 ])
                 > -(labeller.rule(Effects_token)
                     >   (
                         ('[' > +effect_parser [ push_back(_f, _1) ] > ']')
                         |    effect_parser [ push_back(_f, _1) ]
                     )
                    )
                ) [ _val = new_<Effect::CreateShip>(_a, _c, _d, _e, _f) ]
            ;

        create_field_1
            =   ((       tok.CreateField_
                         >>  labeller.rule(Type_token)    >>  string_grammar [ _a = _1 ]
                         >>  labeller.rule(Size_token)
                 )
                 >   double_rules.expr [ _b = _1 ]
                 > -(labeller.rule(Name_token)    >   string_grammar [ _d = _1 ])
                 > -(labeller.rule(Effects_token)
                     >
                     (
                         ('[' > +effect_parser [ push_back(_f, _1) ] > ']')
                         |    effect_parser [ push_back(_f, _1) ]
                     )
                    )
                ) [ _val = new_<Effect::CreateField>(_a, _b, _d, _f) ]
            ;

        create_field_2
            =   ((       tok.CreateField_
                         >>  labeller.rule(Type_token)    >>  string_grammar [ _a = _1 ]
                         >>  labeller.rule(X_token)
                 )
                 >   double_rules.expr [ _b = _1 ]
                 >   labeller.rule(Y_token)       >   double_rules.expr [ _c = _1 ]
                 >   labeller.rule(Size_token)    >   double_rules.expr [ _e = _1 ]
                 > -(labeller.rule(Name_token)    >   string_grammar [ _d = _1 ])
                 > -(labeller.rule(Effects_token)
                     >
                     (
                         ('[' > +effect_parser [ push_back(_f, _1) ] > ']')
                         |    effect_parser [ push_back(_f, _1) ]
                     )
                    )
                ) [ _val = new_<Effect::CreateField>(_a, _b, _c, _e, _d, _f) ]
            ;

        create_system_1
            =   ((       tok.CreateSystem_
                         >>  labeller.rule(Type_token)
                 )
                 >   star_type_rules.expr [ _a = _1 ]
                 >   labeller.rule(X_token)       >   double_rules.expr    [ _b = _1 ]
                 >   labeller.rule(Y_token)       >   double_rules.expr    [ _c = _1 ]
                 > -(labeller.rule(Name_token)    >   string_grammar    [ _d = _1 ])
                 > -(labeller.rule(Effects_token)
                     >
                     (
                         ('[' > +effect_parser [ push_back(_e, _1) ] > ']')
                         |    effect_parser [ push_back(_e, _1) ]
                     )
                    )
                ) [ _val = new_<Effect::CreateSystem>(_a, _b, _c, _d, _e) ]
            ;

        create_system_2
            =   ((       tok.CreateSystem_
                         >>  labeller.rule(X_token)
                 )
                 >   double_rules.expr [ _b = _1 ]
                 >   labeller.rule(Y_token)       >   double_rules.expr [ _c = _1 ]
                 > -(labeller.rule(Name_token)    >   string_grammar [ _d = _1 ])
                 > -(labeller.rule(Effects_token)
                     >
                     (
                         ('[' > +effect_parser [ push_back(_e, _1) ] > ']')
                         |    effect_parser [ push_back(_e, _1) ]
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

} }
