#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "Condition.h"

using namespace boost::spirit;
using namespace phoenix;

ParamLabel low_label("low");
ParamLabel high_label("high");
ParamLabel number_label("number");
ParamLabel condition_label("condition");
ParamLabel empire_label("empire");
ParamLabel affiliation_label("affiliation");
ParamLabel name_label("name");
ParamLabel type_label("type");
ParamLabel planetsize_label("size");
ParamLabel focus_label("focus");
ParamLabel environment_label("environment");
ParamLabel probability_label("probability");
ParamLabel distance_label("distance");
ParamLabel jumps_label("jumps");

rule<Scanner, ConditionClosure::context_t> condition1_p;

namespace Condition {
    class ConditionBase;
}

namespace {
    class ConditionParser1Definition
    {
    public:
        typedef rule<Scanner, ConditionClosure::context_t> Rule;

        ConditionParser1Definition();

    private:
        struct OwnedByClosure : boost::spirit::closure<OwnedByClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, EmpireAffiliationType, bool>
        {
            member1 this_;
            member2 empire;
            member3 affiliation;
            member4 exclusive;
        };

        struct NameParamClosure : boost::spirit::closure<NameParamClosure, Condition::ConditionBase*, std::string>
        {
            member1 this_;
            member2 name;
        };

        struct PlanetTypeClosure : boost::spirit::closure<PlanetTypeClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase< ::PlanetType>*> >
        {
            member1 this_;
            member2 types;
        };

        struct PlanetSizeClosure : boost::spirit::closure<PlanetSizeClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*> >
        {
            member1 this_;
            member2 sizes;
        };

        struct PlanetEnvironmentClosure : boost::spirit::closure<PlanetEnvironmentClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*> >
        {
            member1 this_;
            member2 environments;
        };

        struct ObjectTypeClosure : boost::spirit::closure<ObjectTypeClosure, Condition::ConditionBase*, ValueRef::ValueRefBase< ::UniverseObjectType>*>
        {
            member1 this_;
            member2 type;
        };

        struct FocusTypeClosure : boost::spirit::closure<FocusTypeClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase< ::FocusType>*>, bool>
        {
            member1 this_;
            member2 foci;
            member3 primary;
        };

        struct MeterValueClosure : boost::spirit::closure<MeterValueClosure, Condition::ConditionBase*, MeterType, ValueRef::ValueRefBase<double>*, ValueRef::ValueRefBase<double>*, bool>
        {
            member1 this_;
            member2 meter;
            member3 low;
            member4 high;
            member5 max_meter;
        };

        struct AndOrClosure : boost::spirit::closure<AndOrClosure, Condition::ConditionBase*, std::vector<const Condition::ConditionBase*> >
        {
            member1 this_;
            member2 conditions;
        };

        struct NotClosure : boost::spirit::closure<NotClosure, Condition::ConditionBase*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 condition;
        };

        typedef rule<Scanner, OwnedByClosure::context_t>            OwnedByRule;
        typedef rule<Scanner, NameParamClosure::context_t>          NameParamRule;
        typedef rule<Scanner, PlanetTypeClosure::context_t>         PlanetTypeRule;
        typedef rule<Scanner, PlanetSizeClosure::context_t>         PlanetSizeRule;
        typedef rule<Scanner, PlanetEnvironmentClosure::context_t>  PlanetEnvironmentRule;
        typedef rule<Scanner, ObjectTypeClosure::context_t>         ObjectTypeRule;
        typedef rule<Scanner, FocusTypeClosure::context_t>          FocusTypeRule;
        typedef rule<Scanner, MeterValueClosure::context_t>         MeterValueRule;
        typedef rule<Scanner, AndOrClosure::context_t>              AndOrRule;
        typedef rule<Scanner, NotClosure::context_t>                NotRule;

        Rule                    all;
        OwnedByRule             owned_by;
        Rule                    source;
        NameParamRule           building;
        PlanetTypeRule          planet_type;
        PlanetSizeRule          planet_size;
        PlanetEnvironmentRule   planet_environment;
        ObjectTypeRule          object_type;
        FocusTypeRule           focus_type;
        MeterValueRule          meter_value;
        AndOrRule               and_;
        AndOrRule               or_;
        NotRule                 not_;
    };

    ConditionParser1Definition::ConditionParser1Definition()
    {
        all =
            str_p("all")
            [all.this_ = new_<Condition::All>()];

        owned_by =
            ((str_p("ownedby")[owned_by.exclusive = val(false)] | str_p("ownedexclusivelyby")[owned_by.exclusive = val(true)])
             >> affiliation_label >> affiliation_type_p[owned_by.affiliation = arg1]
             >> empire_label >> int_expr_p[owned_by.empire = arg1])
            [owned_by.this_ = new_<Condition::EmpireAffiliation>(owned_by.empire, owned_by.affiliation, owned_by.exclusive)];

        source =
            str_p("source")
            [source.this_ = new_<Condition::Self>()];

        building =
            (str_p("building")
             >> name_label >> name_p[building.name = arg1])
            [building.this_ = new_<Condition::Building>(building.name)];

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
            ((str_p("primaryfocus")[focus_type.primary = val(true)] | str_p("secondaryfocus")[focus_type.primary = val(false)])
             >> focus_label
             >> (focustype_expr_p[push_back_(focus_type.foci, arg1)]
                 | ('[' >> +(focustype_expr_p[push_back_(focus_type.foci, arg1)]) >> ']')))
            [focus_type.this_ = new_<Condition::FocusType>(focus_type.foci, focus_type.primary)];

        meter_value =
            (((str_p("max")[meter_value.max_meter = val(true)]
               | str_p("current")[meter_value.max_meter = val(false)])
              >> (str_p("population")[meter_value.meter = val(METER_POPULATION)]
                  | str_p("farming")[meter_value.meter = val(METER_FARMING)]
                  | str_p("industry")[meter_value.meter = val(METER_INDUSTRY)]
                  | str_p("research")[meter_value.meter = val(METER_RESEARCH)]
                  | str_p("trade")[meter_value.meter = val(METER_TRADE)]
                  | str_p("mining")[meter_value.meter = val(METER_MINING)]
                  | str_p("construction")[meter_value.meter = val(METER_CONSTRUCTION)]
                  | str_p("health")[meter_value.meter = val(METER_HEALTH)]))
             >> low_label >> double_expr_p[meter_value.low = arg1]
             >> high_label >> double_expr_p[meter_value.high = arg1])
            [meter_value.this_ = new_<Condition::MeterValue>(meter_value.meter, meter_value.low, meter_value.high, meter_value.max_meter)];

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
            | building[condition1_p.this_ = arg1]
            | planet_type[condition1_p.this_ = arg1]
            | planet_size[condition1_p.this_ = arg1]
            | planet_environment[condition1_p.this_ = arg1]
            | object_type[condition1_p.this_ = arg1]
            | focus_type[condition1_p.this_ = arg1]
            | meter_value[condition1_p.this_ = arg1]
            | owned_by[condition1_p.this_ = arg1]
            | and_[condition1_p.this_ = arg1]
            | or_[condition1_p.this_ = arg1]
            | not_[condition1_p.this_ = arg1];
    }
    ConditionParser1Definition condition1_def;
}
