#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "ValueRef.h"
#include "Effect.h"

using namespace boost::spirit;
using namespace phoenix;

rule<Scanner, EffectClosure::context_t> effect_p;

namespace {
    class EffectParserDefinition
    {
    public:
        typedef rule<Scanner, EffectClosure::context_t> Rule;

        EffectParserDefinition();

    private:
        struct SetMeterClosure : boost::spirit::closure<SetMeterClosure, Effect::EffectBase*, MeterType, ValueRef::ValueRefBase<double>*, bool>
        {
            member1 this_;
            member2 meter;
            member3 value;
            member4 max_meter;
        };

        struct SetOwnerStockpileClosure : boost::spirit::closure<SetOwnerStockpileClosure, Effect::EffectBase*, ResourceType, ValueRef::ValueRefBase<double>*>
        {
            member1 this_;
            member2 stockpile_type;
            member3 value;
        };

        struct SetPlanetTypeClosure : boost::spirit::closure<SetPlanetTypeClosure, Effect::EffectBase*, ValueRef::ValueRefBase< ::PlanetType>*>
        {
            member1 this_;
            member2 type;
        };

        struct SetPlanetSizeClosure : boost::spirit::closure<SetPlanetSizeClosure, Effect::EffectBase*, ValueRef::ValueRefBase< ::PlanetSize>*>
        {
            member1 this_;
            member2 size;
        };

        struct EmpireParamClosure : boost::spirit::closure<EmpireParamClosure, Effect::EffectBase*, ValueRef::ValueRefBase<int>*>
        {
            member1 this_;
            member2 empire;
        };

        struct ObjectIDParamClosure : boost::spirit::closure<ObjectIDParamClosure, Effect::EffectBase*, ValueRef::ValueRefBase<int>*>
        {
            member1 this_;
            member2 object_id;
        };

        struct NameParamClosure : boost::spirit::closure<NameParamClosure, Effect::EffectBase*, std::string>
        {
            member1 this_;
            member2 name;
        };

        struct SetStarTypeClosure : boost::spirit::closure<SetStarTypeClosure, Effect::EffectBase*, ValueRef::ValueRefBase< ::StarType>*>
        {
            member1 this_;
            member2 type;
        };

        struct SetTechAvailabilityClosure : boost::spirit::closure<SetTechAvailabilityClosure, Effect::EffectBase*, std::string, bool, bool>
        {
            member1 this_;
            member2 name;
            member3 available;
            member4 include_tech;
        };

        typedef rule<Scanner, SetMeterClosure::context_t>               SetMeterRule;
        typedef rule<Scanner, SetOwnerStockpileClosure::context_t>      SetOwnerStockpileRule;
        typedef rule<Scanner, SetPlanetTypeClosure::context_t>          SetPlanetTypeRule;
        typedef rule<Scanner, SetPlanetSizeClosure::context_t>          SetPlanetSizeRule;
        typedef rule<Scanner, EmpireParamClosure::context_t>            EmpireParamRule;
        typedef rule<Scanner, ObjectIDParamClosure::context_t>          ObjectIDParamRule;
        typedef rule<Scanner, NameParamClosure::context_t>              NameParamRule;
        typedef rule<Scanner, SetStarTypeClosure::context_t>            SetStarTypeRule;
        typedef rule<Scanner, SetTechAvailabilityClosure::context_t>    SetTechAvailabilityRule;

        SetMeterRule            set_meter;
        SetOwnerStockpileRule   set_owner_stockpile;
        SetPlanetTypeRule       set_planet_type;
        SetPlanetSizeRule       set_planet_size;
        EmpireParamRule         add_owner;
        EmpireParamRule         remove_owner;
        ObjectIDParamRule       move_to;
        Rule                    destroy;
        NameParamRule           add_special;
        NameParamRule           remove_special;
        SetStarTypeRule         set_star_type;
        SetTechAvailabilityRule set_tech_availability;

        ParamLabel              value_label;
        ParamLabel              type_label;
        ParamLabel              planetsize_label;
        ParamLabel              empire_label;
        ParamLabel              name_label;
        ParamLabel              object_id_label;
    };

    EffectParserDefinition::EffectParserDefinition() :
        value_label("value"),
        type_label("type"),
        planetsize_label("size"),
        empire_label("empire"),
        name_label("name"),
        object_id_label("objectid")
    {
        set_meter =
            ((str_p("setmax")[set_meter.max_meter = val(true)]
              | str_p("setcurrent")[set_meter.max_meter = val(false)])
             >> (str_p("population")[set_meter.meter = val(METER_POPULATION)]
                 | str_p("farming")[set_meter.meter = val(METER_FARMING)]
                 | str_p("industry")[set_meter.meter = val(METER_INDUSTRY)]
                 | str_p("research")[set_meter.meter = val(METER_RESEARCH)]
                 | str_p("trade")[set_meter.meter = val(METER_TRADE)]
                 | str_p("mining")[set_meter.meter = val(METER_MINING)]
                 | str_p("construction")[set_meter.meter = val(METER_CONSTRUCTION)]
                 | str_p("health")[set_meter.meter = val(METER_HEALTH)]
                 | str_p("fuel")[set_meter.meter = val(METER_FUEL)]
                 | str_p("supply")[set_meter.meter = val(METER_SUPPLY)]
                 | str_p("stealth")[set_meter.meter = val(METER_STEALTH)]
                 | str_p("detection")[set_meter.meter = val(METER_DETECTION)]
                 | str_p("shield")[set_meter.meter = val(METER_SHIELD)]
                 | str_p("defense")[set_meter.meter = val(METER_DEFENSE)])
             >> value_label >> double_expr_p[set_meter.value = arg1])
            [set_meter.this_ = new_<Effect::SetMeter>(set_meter.meter, set_meter.value, set_meter.max_meter)];

        set_owner_stockpile =
            ((str_p("setownerfoodstockpile")[set_owner_stockpile.stockpile_type = val(RE_FOOD)]
              | str_p("setownermineralstockpile")[set_owner_stockpile.stockpile_type = val(RE_MINERALS)]
              | str_p("setownertradestockpile")[set_owner_stockpile.stockpile_type = val(RE_TRADE)])
             >> value_label >> double_expr_p[set_owner_stockpile.value = arg1])
            [set_owner_stockpile.this_ = new_<Effect::SetEmpireStockpile>(set_owner_stockpile.stockpile_type, set_owner_stockpile.value)];

        set_planet_type =
            (str_p("setplanettype")
             >> type_label >> planettype_expr_p[set_planet_type.type = arg1])
            [set_planet_type.this_ = new_<Effect::SetPlanetType>(set_planet_type.type)];

        set_planet_size =
            (str_p("setplanetsize")
             >> planetsize_label >> planetsize_expr_p[set_planet_size.size = arg1])
            [set_planet_size.this_ = new_<Effect::SetPlanetSize>(set_planet_size.size)];

        add_owner =
            (str_p("addowner")
             >> empire_label >> int_expr_p[add_owner.empire = arg1])
            [add_owner.this_ = new_<Effect::AddOwner>(add_owner.empire)];

        remove_owner =
            (str_p("removeowner")
             >> empire_label >> int_expr_p[remove_owner.empire = arg1])
            [remove_owner.this_ = new_<Effect::RemoveOwner>(remove_owner.empire)];

        move_to =
            (str_p("moveto")
             >> object_id_label >> int_expr_p[move_to.object_id = arg1])
            [move_to.this_ = new_<Effect::MoveTo>(move_to.object_id)];

        destroy =
            str_p("destroy")
            [destroy.this_ = new_<Effect::Destroy>()];

        add_special =
            (str_p("addspecial")
             >> name_label >> name_p[add_special.name = arg1])
            [add_special.this_ = new_<Effect::AddSpecial>(add_special.name)];

        remove_special =
            (str_p("removespecial")
             >> name_label >> name_p[remove_special.name = arg1])
            [remove_special.this_ = new_<Effect::RemoveSpecial>(remove_special.name)];

        set_star_type =
            (str_p("setstartype")
             >> type_label >> startype_expr_p[set_star_type.type = arg1])
            [set_star_type.this_ = new_<Effect::SetStarType>(set_star_type.type)];

        set_tech_availability =
            ((str_p("givetechtoowner")[set_tech_availability.available = val(true), set_tech_availability.include_tech = val(true)]
              | str_p("revoketechfromowner")[set_tech_availability.available = val(false), set_tech_availability.include_tech = val(true)]
              | str_p("unlocktechitemsforowner")[set_tech_availability.available = val(true), set_tech_availability.include_tech = val(false)]
              | str_p("locktechitemsforowner")[set_tech_availability.available = val(false), set_tech_availability.include_tech = val(false)])
             >> name_label >> name_p[set_tech_availability.name = arg1])
            [set_tech_availability.this_ = new_<Effect::SetTechAvailability>(set_tech_availability.name, new_<ValueRef::Variable<int> >(false, "Owner"), set_tech_availability.available, set_tech_availability.include_tech)];

        effect_p =
            set_meter[effect_p.this_ = arg1]
            | set_owner_stockpile[effect_p.this_ = arg1]
            | set_planet_type[effect_p.this_ = arg1]
            | set_planet_size[effect_p.this_ = arg1]
            | add_owner[effect_p.this_ = arg1]
            | remove_owner[effect_p.this_ = arg1]
            | move_to[effect_p.this_ = arg1]
            | destroy[effect_p.this_ = arg1]
            | add_special[effect_p.this_ = arg1]
            | remove_special[effect_p.this_ = arg1]
            | set_star_type[effect_p.this_ = arg1]
            | set_tech_availability[effect_p.this_ = arg1];
    }
    EffectParserDefinition effect_parser_def;
}
