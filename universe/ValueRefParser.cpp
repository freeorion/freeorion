#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "ValueRef.h"
#include "Condition.h"

using namespace boost::spirit::classic;
using namespace phoenix;

// final variable tokens: object property names that have the appropriate type
// for the ValueRef return type.
SimpleRule int_variable_final =
    str_p("owner") | "id" | "creationturn" | "age" | "producedbyempireid"
    | "designid" | "fleetid" | "planetid" | "systemid" | "finaldestinationid"
    | "nextsystemid" | "previoussystemid" | "numships";

SimpleRule double_variable_final =
    str_p("farming") | "targetfarming" | "industry" | "targetindustry"
    | "research" | "targetresearch" | "trade" | "targettrade"
    | "mining" | "targetmining" | "construction" | "targetconstruction"
    | "population" | "targetpopulation" | "health" | "targethealth"
    | "maxfuel" | "fuel" | "maxshield" | "shield"
    | "maxdefense" | "defense" | "maxstructure" | "structure"
    | "supply" | "stealth" | "detection" | "foodconsumption"
    | "battlespeed" | "starlanespeed"
    | "tradestockpile" | "mineralstockpile" | "foodstockpile"
    | "distancetosource";

SimpleRule string_variable_final =
    str_p("name") | "species" | "buildingtype" | "focus";

SimpleRule variable_container =
    str_p("planet") | "system" | "fleet";

// complete rule for all possible ValueRef return types
StringValueRefRule              string_expr_p;
IntValueRefRule                 int_expr_p;
DoubleValueRefRule              double_expr_p;
PlanetSizeValueRefRule          planetsize_expr_p;
PlanetTypeValueRefRule          planettype_expr_p;
PlanetEnvironmentValueRefRule   planetenvironment_expr_p;
UniverseObjectTypeValueRefRule  universeobjecttype_expr_p;
StarTypeValueRefRule            startype_expr_p;

// statistic parsers: require more complex parser than single-property valueref
template <class T>
struct ValueRefStatisticRule
{
    typedef ValueRef::ValueRefBase<T> RefBase;
    struct ValueRefStatisticClosure : boost::spirit::classic::closure<ValueRefStatisticClosure, RefBase*,
                                                                      ValueRef::StatisticType, std::string,
                                                                      const Condition::ConditionBase*>
    {
        typedef boost::spirit::classic::closure<
            ValueRefStatisticClosure,
            RefBase*,
            ValueRef::StatisticType,
            std::string,
            const Condition::ConditionBase*
        > BaseType;

        typename BaseType::member1 this_;
        typename BaseType::member2 stat_type;
        typename BaseType::member3 property_name;
        typename BaseType::member4 sampling_condition;
    };
    typedef boost::spirit::classic::rule<Scanner, typename ValueRefStatisticClosure::context_t> type;
};

namespace {
    template <class T>
    class ValueRefParserDefinition
    {
    public:
        typedef ValueRef::Constant<T>       RefConst;
        typedef ValueRef::Variable<T>       RefVar;
        typedef ValueRef::Variable<int>     IntRefVar;
        typedef ValueRef::Variable<double>  DoubleRefVar;
        typedef ValueRef::Statistic<T>      RefStat;
        typedef ValueRef::Operation<T>      RefOp;

        typedef typename ValueRefRule<T>::type  Rule;
        ValueRefParserDefinition(Rule& expr);

    private:
        void SpecializedConstantAndVariableFinalDefinition();
        void SpecializedVariableDefinition();
        void SpecializedVariableStatisticDefinition();

        typedef typename ValueRefStatisticRule<T>::type StatisticRule;
        StatisticRule   statistic;

        SimpleRule      variable_final;
        Rule            constant;
        Rule            variable;

        Rule            primary_expr;
        Rule            negative_expr;
        Rule            times_expr;
        Rule            divides_expr;
        Rule            plus_expr;
        Rule            minus_expr;

        ParamLabel      property_label;
        ParamLabel      condition_label;
    };

    ValueRefParserDefinition<std::string>           string_value_ref_def(string_expr_p);
    ValueRefParserDefinition<int>                   int_value_ref_def(int_expr_p);
    ValueRefParserDefinition<double>                double_value_ref_def(double_expr_p);
    ValueRefParserDefinition<PlanetSize>            planetsize_value_ref_def(planetsize_expr_p);
    ValueRefParserDefinition<PlanetType>            planettype_value_ref_def(planettype_expr_p);
    ValueRefParserDefinition<PlanetEnvironment>     planetenvironment_value_ref_def(planetenvironment_expr_p);
    ValueRefParserDefinition<UniverseObjectType>    universeobjecttype_value_ref_def(universeobjecttype_expr_p);
    ValueRefParserDefinition<StarType>              startype_value_ref_def(startype_expr_p);

    template <class T>
    ValueRefParserDefinition<T>::ValueRefParserDefinition(Rule& expr) :
        property_label("property"),
        condition_label("condition")
    {
        SpecializedConstantAndVariableFinalDefinition();
        SpecializedVariableDefinition();
        SpecializedVariableStatisticDefinition();

        // basic expression: constant, variable or statistical variable; input to further calculations
        primary_expr =
            constant[primary_expr.this_ = arg1]
            | variable[primary_expr.this_ = arg1]
            | statistic[primary_expr.this_ = arg1]
            | '(' >> expr[primary_expr.this_ = arg1] >> ')';

        // compound expressions, comprising one or more primary expressions and mathematical operations.
        // defined recursively according to order of operations when evaluating.
        negative_expr =
            primary_expr[negative_expr.this_ = arg1]
            | (ch_p('-') >> primary_expr[negative_expr.operand1 = arg1])[negative_expr.this_ = new_<RefOp>(val(ValueRef::NEGATE), negative_expr.operand1)];

        times_expr =
            (negative_expr[times_expr.operand1 = arg1] >> ch_p('*') >> times_expr[times_expr.operand2 = arg1])[times_expr.this_ = new_<RefOp>(val(ValueRef::TIMES), times_expr.operand1, times_expr.operand2)]
            | negative_expr[times_expr.this_ = arg1];

        divides_expr =
            (times_expr[divides_expr.operand1 = arg1] >> ch_p('/') >> divides_expr[divides_expr.operand2 = arg1])[divides_expr.this_ = new_<RefOp>(val(ValueRef::DIVIDES), divides_expr.operand1, divides_expr.operand2)]
            | times_expr[divides_expr.this_ = arg1];

        plus_expr =
            (divides_expr[plus_expr.operand1 = arg1] >> ch_p('+') >> plus_expr[plus_expr.operand2 = arg1])[plus_expr.this_ = new_<RefOp>(val(ValueRef::PLUS), plus_expr.operand1, plus_expr.operand2)]
            | divides_expr[plus_expr.this_ = arg1];

        minus_expr =
            (plus_expr[minus_expr.operand1 = arg1] >> ch_p('-') >> minus_expr[minus_expr.operand2 = arg1])[minus_expr.this_ = new_<RefOp>(val(ValueRef::MINUS), minus_expr.operand1, minus_expr.operand2)]
            | plus_expr[minus_expr.this_ = arg1];

        expr = minus_expr[expr.this_ = arg1];
    }

    template <>
    void ValueRefParserDefinition<std::string>::SpecializedConstantAndVariableFinalDefinition()
    {
        constant =
            // quote-enclosed string
            name_p[constant.this_ = new_<RefConst>(arg1)]

            // parser enum names, not in quotes, like Swamp, Terran, Asteroids,
            // which are converted to enum values, and then converted to text
            // representations of the enums, like PT_SWAMP, PT_TERRAN,
            // PT_ASTEROIDS, which can be looked up in the stringtable
            | planet_size_p[constant.this_ =                new_<RefConst>(enum_to_string_(static_cast_<PlanetSize>(arg1)))]
            | planet_type_p[constant.this_ =                new_<RefConst>(enum_to_string_(static_cast_<PlanetType>(arg1)))]
            | planet_environment_type_p[constant.this_ =    new_<RefConst>(enum_to_string_(static_cast_<PlanetEnvironment>(arg1)))]
            | universe_object_type_p[constant.this_ =       new_<RefConst>(enum_to_string_(static_cast_<UniverseObjectType>(arg1)))]
            | star_type_p[constant.this_ =                  new_<RefConst>(enum_to_string_(static_cast_<StarType>(arg1)))]

            // raw constant number, not in quotes, which are left as the raw
            // text (due to the eps_p), just as if they had been written
            // enclosed in quotes.  this is done to maintain the requirement
            // that an int is always a valid ValueRef value in the parser
            | (real_p >> eps_p) [constant.this_ =           new_<RefConst>(construct_<std::string>(arg1, arg2))]
            | (int_p >> eps_p)[constant.this_ =             new_<RefConst>(construct_<std::string>(arg1, arg2))];

        variable_final = string_variable_final;
    }

    template <>
    void ValueRefParserDefinition<int>::SpecializedConstantAndVariableFinalDefinition()
    {
        constant =
            real_p[constant.this_ = new_<RefConst>(static_cast_<int>(arg1))]
            | int_p[constant.this_ = new_<RefConst>(arg1)];

        variable_final = int_variable_final;
    }

    template <>
    void ValueRefParserDefinition<double>::SpecializedConstantAndVariableFinalDefinition()
    {
        constant =
            real_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<double>(arg1))];

        variable_final = double_variable_final;
    }

    template <>
    void ValueRefParserDefinition<PlanetSize>::SpecializedConstantAndVariableFinalDefinition()
    {
        constant =
            planet_size_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<PlanetSize>(arg1))];

        variable_final = str_p("planetsize");
    }

    template <>
    void ValueRefParserDefinition<PlanetType>::SpecializedConstantAndVariableFinalDefinition()
    {
        constant =
            planet_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<PlanetType>(arg1))];

        variable_final = str_p("planettype");
    }

    template <>
    void ValueRefParserDefinition<PlanetEnvironment>::SpecializedConstantAndVariableFinalDefinition()
    {
        constant =
            planet_environment_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<PlanetEnvironment>(arg1))];

        variable_final = str_p("planetenvironment");
    }

    template <>
    void ValueRefParserDefinition<UniverseObjectType>::SpecializedConstantAndVariableFinalDefinition()
    {
        constant =
            universe_object_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<UniverseObjectType>(arg1))];

        variable_final = str_p("objecttype");
    }

    template <>
    void ValueRefParserDefinition<StarType>::SpecializedConstantAndVariableFinalDefinition()
    {
        constant =
            star_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<StarType>(arg1))];

        variable_final = str_p("startype");
    }

    template <class T>
    void ValueRefParserDefinition<T>::SpecializedVariableDefinition()
    {
        variable =
            str_p("source") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::SOURCE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("localcandidate") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("rootcandidate") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("value")
            [variable.this_ = new_<RefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2))];
    }

    template <>
    void ValueRefParserDefinition<std::string>::SpecializedVariableDefinition()
    {
        typedef ValueRef::StringCast<int>       CastIntRefVar;
        typedef ValueRef::StringCast<double>    CastDoubleRefVar;

        variable =
            str_p("source") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::SOURCE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("source") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastIntRefVar>(new_<IntRefVar>(val(ValueRef::SOURCE_REFERENCE), construct_<std::string>(arg1, arg2)))]
            | str_p("source") >> '.' >> (!(variable_container >> ".") >> double_variable_final)
            [variable.this_ = new_<CastDoubleRefVar>(new_<DoubleRefVar>(val(ValueRef::SOURCE_REFERENCE), construct_<std::string>(arg1, arg2)))]

            | str_p("target") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastIntRefVar>(new_<IntRefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2)))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> double_variable_final)
            [variable.this_ = new_<CastDoubleRefVar>(new_<DoubleRefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2)))]

            | str_p("localcandidate") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("localcandidate") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastIntRefVar>(new_<IntRefVar>(val(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2)))]
            | str_p("localcandidate") >> '.' >> (!(variable_container >> ".") >> double_variable_final)
            [variable.this_ = new_<CastDoubleRefVar>(new_<DoubleRefVar>(val(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2)))]

            | str_p("rootcandidate") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("rootcandidate") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastIntRefVar>(new_<IntRefVar>(val(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2)))]
            | str_p("rootcandidate") >> '.' >> (!(variable_container >> ".") >> double_variable_final)
            [variable.this_ = new_<CastDoubleRefVar>(new_<DoubleRefVar>(val(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2)))]

            | str_p("currentturn")
            [variable.this_ = new_<CastIntRefVar>(new_<IntRefVar>(val(ValueRef::NON_OBJECT_REFERENCE), construct_<std::string>(arg1, arg2)))]
            | str_p("value")
            [variable.this_ = new_<RefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2))];
    }

    template <>
    void ValueRefParserDefinition<int>::SpecializedVariableDefinition()
    {
        variable =
            str_p("source") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::SOURCE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("localcandidate") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("rootcandidate") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2))]

            | str_p("currentturn")
            [variable.this_ = new_<RefVar>(val(ValueRef::NON_OBJECT_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("value")
            [variable.this_ = new_<RefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2))];
    }

    template <>
    void ValueRefParserDefinition<double>::SpecializedVariableDefinition()
    {
        typedef ValueRef::StaticCast<int, double> CastRefVar;
        variable =
            str_p("source") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::SOURCE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("source") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastRefVar>(new_<IntRefVar>(val(ValueRef::SOURCE_REFERENCE), construct_<std::string>(arg1, arg2)))]

            | str_p("target") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastRefVar>(new_<IntRefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2)))]

            | str_p("localcandidate") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("localcandidate") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastRefVar>(new_<IntRefVar>(val(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2)))]

            | str_p("rootcandidate") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2))]
            | str_p("rootcandidate") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastRefVar>(new_<IntRefVar>(val(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE), construct_<std::string>(arg1, arg2)))]

            | str_p("currentturn")
            [variable.this_ = new_<CastRefVar>(new_<IntRefVar>(val(ValueRef::NON_OBJECT_REFERENCE), construct_<std::string>(arg1, arg2)))]
            | str_p("value")
            [variable.this_ = new_<RefVar>(val(ValueRef::EFFECT_TARGET_REFERENCE), construct_<std::string>(arg1, arg2))];
    }

    template <class T>
    void ValueRefParserDefinition<T>::SpecializedVariableStatisticDefinition()
    {
        // enumerated types only support the MODE StatisticType
        statistic =
            (str_p("mode")
             >> property_label >> (!(variable_container >> ".") >> variable_final)
                                  [statistic.property_name = construct_<std::string>(arg1, arg2)]
             >> condition_label >> condition_p[statistic.sampling_condition = arg1])
            [statistic.this_ = new_<RefStat>(statistic.property_name, val(ValueRef::MODE), statistic.sampling_condition)];
    }

    template <>
    void ValueRefParserDefinition<int>::SpecializedVariableStatisticDefinition()
    {
        statistic =
            ((str_p("number") >> condition_label >> condition_p[statistic.sampling_condition = arg1])
             [statistic.this_ = new_<RefStat>(val(""), val(ValueRef::COUNT), statistic.sampling_condition)])
             | (((str_p("sum")[statistic.stat_type = val(ValueRef::SUM)]
                  | str_p("mean")[statistic.stat_type = val(ValueRef::MEAN)]
                  | str_p("rms")[statistic.stat_type = val(ValueRef::RMS)]
                  | str_p("mode")[statistic.stat_type = val(ValueRef::MODE)]
                  | str_p("max")[statistic.stat_type = val(ValueRef::MAX)]
                  | str_p("min")[statistic.stat_type = val(ValueRef::MIN)]
                  | str_p("spread")[statistic.stat_type = val(ValueRef::SPREAD)]
                  | str_p("stdev")[statistic.stat_type = val(ValueRef::STDEV)]
                  | str_p("product")[statistic.stat_type = val(ValueRef::PRODUCT)])
                 >> property_label >> (!(variable_container >> ".") >> variable_final)
                                      [statistic.property_name = construct_<std::string>(arg1, arg2)]
                 >> condition_label >> condition_p[statistic.sampling_condition = arg1])
                [statistic.this_ = new_<RefStat>(statistic.property_name, statistic.stat_type, statistic.sampling_condition)]);
    }

    template <>
    void ValueRefParserDefinition<double>::SpecializedVariableStatisticDefinition()
    {
        statistic =
            ((str_p("number") >> condition_label >> condition_p[statistic.sampling_condition = arg1])
             [statistic.this_ = new_<RefStat>(val("dummy"), val(ValueRef::COUNT), statistic.sampling_condition)])
             | (((str_p("sum")[statistic.stat_type = val(ValueRef::SUM)]
                  | str_p("mean")[statistic.stat_type = val(ValueRef::MEAN)]
                  | str_p("rms")[statistic.stat_type = val(ValueRef::RMS)]
                  | str_p("mode")[statistic.stat_type = val(ValueRef::MODE)]
                  | str_p("max")[statistic.stat_type = val(ValueRef::MAX)]
                  | str_p("min")[statistic.stat_type = val(ValueRef::MIN)]
                  | str_p("spread")[statistic.stat_type = val(ValueRef::SPREAD)]
                  | str_p("stdev")[statistic.stat_type = val(ValueRef::STDEV)]
                  | str_p("product")[statistic.stat_type = val(ValueRef::PRODUCT)])
                 >> property_label >> (!(variable_container >> ".") >> variable_final)
                                      [statistic.property_name = construct_<std::string>(arg1, arg2)]
                 >> condition_label >> condition_p[statistic.sampling_condition = arg1])
                [statistic.this_ = new_<RefStat>(statistic.property_name, statistic.stat_type, statistic.sampling_condition)]);
    }
}
