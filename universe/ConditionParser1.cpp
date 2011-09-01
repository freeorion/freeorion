#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "Condition.h"
#include "ValueRef.h"

using namespace boost::spirit::classic;
using namespace phoenix;

ParamLabel low_label("low");
ParamLabel high_label("high");
ParamLabel number_label("number");
ParamLabel condition_label("condition");
ParamLabel empire_label("empire");
ParamLabel affiliation_label("affiliation");
ParamLabel name_label("name");
ParamLabel type_label("type");
ParamLabel class_label("class");
ParamLabel planetsize_label("size");
ParamLabel environment_label("environment");
ParamLabel probability_label("probability");
ParamLabel distance_label("distance");
ParamLabel jumps_label("jumps");
ParamLabel sort_key_label("sortby");
ParamLabel design_label("design");

rule<Scanner, ConditionClosure::context_t> condition1_p;

namespace Condition {
    struct ConditionBase;
}

namespace {
    class ConditionParser1Definition
    {
    public:
        typedef rule<Scanner, ConditionClosure::context_t> Rule;

        ConditionParser1Definition();

    private:
        struct OwnedByClosure : boost::spirit::classic::closure<OwnedByClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, EmpireAffiliationType>
        {
            member1 this_;
            member2 empire;
            member3 affiliation;
        };

        struct StringRefVecClosure : boost::spirit::classic::closure<StringRefVecClosure, Condition::ConditionBase*,
                                                                     std::vector<const ValueRef::ValueRefBase<std::string>*> >
        {
            member1 this_;
            member2 names;
        };

        struct PlanetTypeClosure : boost::spirit::classic::closure<PlanetTypeClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase< ::PlanetType>*> >
        {
            member1 this_;
            member2 types;
        };

        struct PlanetSizeClosure : boost::spirit::classic::closure<PlanetSizeClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*> >
        {
            member1 this_;
            member2 sizes;
        };

        struct PlanetEnvironmentClosure : boost::spirit::classic::closure<PlanetEnvironmentClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*> >
        {
            member1 this_;
            member2 environments;
        };

        struct ObjectTypeClosure : boost::spirit::classic::closure<ObjectTypeClosure, Condition::ConditionBase*, ValueRef::ValueRefBase< ::UniverseObjectType>*>
        {
            member1 this_;
            member2 type;
        };

        struct MeterValueClosure : boost::spirit::classic::closure<MeterValueClosure, Condition::ConditionBase*, MeterType, ValueRef::ValueRefBase<double>*, ValueRef::ValueRefBase<double>*>
        {
            member1 this_;
            member2 meter;
            member3 low;
            member4 high;
        };

        struct AndOrClosure : boost::spirit::classic::closure<AndOrClosure, Condition::ConditionBase*, std::vector<const Condition::ConditionBase*> >
        {
            member1 this_;
            member2 conditions;
        };

        struct NotClosure : boost::spirit::classic::closure<NotClosure, Condition::ConditionBase*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 condition;
        };

        struct StringRefVectorClosure : boost::spirit::classic::closure<StringRefVectorClosure,
                                                                        std::vector<const ValueRef::ValueRefBase<std::string>*> >
        {
            member1 this_;
        };

        typedef rule<Scanner, OwnedByClosure::context_t>            OwnedByRule;
        typedef rule<Scanner, StringRefVecClosure::context_t>       StringRefVecRule;
        typedef rule<Scanner, PlanetTypeClosure::context_t>         PlanetTypeRule;
        typedef rule<Scanner, PlanetSizeClosure::context_t>         PlanetSizeRule;
        typedef rule<Scanner, PlanetEnvironmentClosure::context_t>  PlanetEnvironmentRule;
        typedef rule<Scanner, ObjectTypeClosure::context_t>         ObjectTypeRule;
        typedef rule<Scanner, MeterValueClosure::context_t>         MeterValueRule;
        typedef rule<Scanner, AndOrClosure::context_t>              AndOrRule;
        typedef rule<Scanner, NotClosure::context_t>                NotRule;
        typedef rule<Scanner, StringRefVectorClosure::context_t>    StringRefVectorRule;

        Rule                    all;
        Rule                    source;
        Rule                    target;
        Rule                    stationary;
        Rule                    capital;
        Rule                    monster;
        Rule                    armed;
        OwnedByRule             owned_by;
        StringRefVecRule        homeworld;
        StringRefVecRule        building;
        StringRefVecRule        species;
        StringRefVecRule        focus_type;
        PlanetTypeRule          planet_type;
        PlanetSizeRule          planet_size;
        PlanetEnvironmentRule   planet_environment;
        ObjectTypeRule          object_type;
        MeterValueRule          meter_value;
        AndOrRule               and_;
        AndOrRule               or_;
        NotRule                 not_;
        StringRefVectorRule     string_ref_vector;
    };

    ConditionParser1Definition::ConditionParser1Definition()
    {
        // not a condition parser, but a utility function for parsing a list of ValueRef
        string_ref_vector =
            (string_expr_p[push_back_(string_ref_vector.this_, arg1)])
            | ('[' >> +(string_expr_p[push_back_(string_ref_vector.this_, arg1)]) >> ']');

        all =
            str_p("all")
            [all.this_ = new_<Condition::All>()];

        owned_by =
            (str_p("ownedby")
             >> affiliation_label >> affiliation_type_p[owned_by.affiliation = arg1]
             >> empire_label >> int_expr_p[owned_by.empire = arg1]
             [owned_by.this_ = new_<Condition::EmpireAffiliation>(owned_by.empire, owned_by.affiliation)])
            | (str_p("ownedby")
              >> affiliation_label >> affiliation_type_p[owned_by.affiliation = arg1]
              [owned_by.this_ = new_<Condition::EmpireAffiliation>(owned_by.affiliation)]);

        source =
            str_p("source")
            [source.this_ = new_<Condition::Source>()];

        target =
            str_p("target")
            [target.this_ = new_<Condition::Target>()];

        stationary =
            str_p("stationary")
            [stationary.this_ = new_<Condition::Stationary>()];

        homeworld =
            str_p("homeworld")
            >> !(name_label >> string_ref_vector[homeworld.names = arg1])
            [homeworld.this_ = new_<Condition::Homeworld>(homeworld.names)];

        capital =
            str_p("capital")
            [capital.this_ = new_<Condition::Capital>()];

        monster =
            str_p("monster")
            [monster.this_ = new_<Condition::Monster>()];

        armed =
            str_p("armed")
            [armed.this_ = new_<Condition::Armed>()];

        building =
            str_p("building")
            >> (name_label >> string_ref_vector[building.names = arg1])
            [building.this_ = new_<Condition::Building>(building.names)];

        species =
            str_p("species")
            >> !(name_label >> string_ref_vector[species.names = arg1])
            [species.this_ = new_<Condition::Species>(species.names)];

        planet_type =
            (str_p("planet")
             >> type_label
             >> (planettype_expr_p[push_back_(planet_type.types, arg1)]
                 | ('[' >> +(planettype_expr_p[push_back_(planet_type.types, arg1)]) >> ']')))
            [planet_type.this_ = new_<Condition::PlanetType>(planet_type.types)];

        planet_size =
            (str_p("planet")
             >> planetsize_label
             >> (planetsize_expr_p[push_back_(planet_size.sizes, arg1)]
                 | ('[' >> +(planetsize_expr_p[push_back_(planet_size.sizes, arg1)]) >> ']')))
            [planet_size.this_ = new_<Condition::PlanetSize>(planet_size.sizes)];

        planet_environment =
            (str_p("planet")
             >> environment_label
             >> (planetenvironment_expr_p[push_back_(planet_environment.environments, arg1)]
                 | ('[' >> +(planetenvironment_expr_p[push_back_(planet_environment.environments, arg1)]) >> ']')))
            [planet_environment.this_ = new_<Condition::PlanetEnvironment>(planet_environment.environments)];

        object_type =
            universe_object_type_p[object_type.this_ = new_<Condition::Type>(new_<ValueRef::Constant<UniverseObjectType> >(arg1))]
            | (str_p("objecttype")
               >> type_label >> universeobjecttype_expr_p[object_type.type = arg1])
            [object_type.this_ = new_<Condition::Type>(object_type.type)];

        focus_type =
            str_p("focus")
            >> !(type_label >> string_ref_vector[focus_type.names = arg1])
            [focus_type.this_ = new_<Condition::FocusType>(focus_type.names)];

        // non-vectorized string valueref version of focus_type
        //focus_type =
        //    str_p("focus")
        //    >> !(type_label >> string_expr_p[push_back_(focus_type.names, arg1)])
        //    [focus_type.this_ = new_<Condition::FocusType>(focus_type.names)];

        meter_value =
           ((str_p("targetpopulation")[meter_value.meter =  val(METER_TARGET_POPULATION)]
           | str_p("targethealth")[meter_value.meter =      val(METER_TARGET_HEALTH)]
           | str_p("targetfarming")[meter_value.meter =     val(METER_TARGET_FARMING)]
           | str_p("targetindustry")[meter_value.meter =    val(METER_TARGET_INDUSTRY)]
           | str_p("targetresearch")[meter_value.meter =    val(METER_TARGET_RESEARCH)]
           | str_p("targettrade")[meter_value.meter =       val(METER_TARGET_TRADE)]
           | str_p("targetmining")[meter_value.meter =      val(METER_TARGET_MINING)]
           | str_p("targetconstruction")[meter_value.meter =val(METER_TARGET_CONSTRUCTION)]
           | str_p("maxfuel")[meter_value.meter =           val(METER_MAX_FUEL)]
           | str_p("maxshield")[meter_value.meter =         val(METER_MAX_SHIELD)]
           | str_p("maxstructure")[meter_value.meter =      val(METER_MAX_STRUCTURE)]
           | str_p("maxdefense")[meter_value.meter =        val(METER_MAX_DEFENSE)]
           | str_p("population")[meter_value.meter =        val(METER_POPULATION)]
           | str_p("health")[meter_value.meter =            val(METER_HEALTH)]
           | str_p("farming")[meter_value.meter =           val(METER_FARMING)]
           | str_p("industry")[meter_value.meter =          val(METER_INDUSTRY)]
           | str_p("research")[meter_value.meter =          val(METER_RESEARCH)]
           | str_p("trade")[meter_value.meter =             val(METER_TRADE)]
           | str_p("mining")[meter_value.meter =            val(METER_MINING)]
           | str_p("construction")[meter_value.meter =      val(METER_CONSTRUCTION)]
           | str_p("fuel")[meter_value.meter =              val(METER_FUEL)]
           | str_p("shield")[meter_value.meter =            val(METER_SHIELD)]
           | str_p("structure")[meter_value.meter =         val(METER_STRUCTURE)]
           | str_p("defense")[meter_value.meter =           val(METER_DEFENSE)]
           | str_p("foodconsumption")[meter_value.meter =   val(METER_FOOD_CONSUMPTION)]
           | str_p("supply")[meter_value.meter =            val(METER_SUPPLY)]
           | str_p("stealth")[meter_value.meter =           val(METER_STEALTH)]
           | str_p("detection")[meter_value.meter =         val(METER_DETECTION)]
           | str_p("battlespeed")[meter_value.meter =       val(METER_BATTLE_SPEED)]
           | str_p("starlanespeed")[meter_value.meter =     val(METER_STARLANE_SPEED)])
             >> low_label >> double_expr_p[meter_value.low = arg1]
             >> high_label >> double_expr_p[meter_value.high = arg1])
            [meter_value.this_ = new_<Condition::MeterValue>(meter_value.meter, meter_value.low, meter_value.high)];

        and_ =
            (str_p("and") >> '[' >> +(condition_p[push_back_(and_.conditions, arg1)]) >> ']')
            [and_.this_ = new_<Condition::And>(and_.conditions)];

        or_ =
            (str_p("or") >> '[' >> +(condition_p[push_back_(or_.conditions, arg1)]) >> ']')
            [or_.this_ = new_<Condition::Or>(or_.conditions)];

        not_ =
            (str_p("not") >> condition_p[not_.condition = arg1])
            [not_.this_ = new_<Condition::Not>(not_.condition)];

        condition1_p =
            all[condition1_p.this_ = arg1]
            | source[condition1_p.this_ = arg1]
            | focus_type[condition1_p.this_ = arg1]
            | stationary[condition1_p.this_ = arg1]
            | homeworld[condition1_p.this_ = arg1]
            | capital[condition1_p.this_ = arg1]
            | monster[condition1_p.this_ = arg1]
            | armed[condition1_p.this_ = arg1]
            | building[condition1_p.this_ = arg1]
            | species[condition1_p.this_ = arg1]
            | planet_type[condition1_p.this_ = arg1]
            | planet_size[condition1_p.this_ = arg1]
            | planet_environment[condition1_p.this_ = arg1]
            | object_type[condition1_p.this_ = arg1]
            | meter_value[condition1_p.this_ = arg1]
            | owned_by[condition1_p.this_ = arg1]
            | target[condition1_p.this_ = arg1]
            | and_[condition1_p.this_ = arg1]
            | or_[condition1_p.this_ = arg1]
            | not_[condition1_p.this_ = arg1];
    }
    ConditionParser1Definition condition1_def;
}
