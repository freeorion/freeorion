#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "ValueRef.h"

using namespace boost::spirit;
using namespace phoenix;

IntValueRefRule                 int_expr_p;
DoubleValueRefRule              double_expr_p;
PlanetSizeValueRefRule          planetsize_expr_p;
PlanetTypeValueRefRule          planettype_expr_p;
PlanetEnvironmentValueRefRule   planetenvironment_expr_p;
UniverseObjectTypeValueRefRule  universeobjecttype_expr_p;
StarTypeValueRefRule            startype_expr_p;
FocusTypeValueRefRule           focustype_expr_p;

namespace {
    template <class T>
    class ValueRefParserDefinition
    {
    public:
        typedef ValueRef::ValueRefBase<T> RefBase;
        typedef ValueRef::Constant<T> RefConst;
        typedef ValueRef::Variable<T> RefVar;
        typedef ValueRef::Variable<int> IntRefVar;
        typedef ValueRef::Operation<T> RefOp;

        typedef typename ValueRefRule<T>::type Rule;

        ValueRefParserDefinition(Rule& expr);

    private:
        void SpecializedInit();
        void SpecializedVarDefinition();

        Rule constant;
        Rule variable_container;
        Rule variable_final;
        Rule int_variable_final;
        Rule variable;
        Rule primary_expr;
        Rule negative_expr;
        Rule times_expr;
        Rule divides_expr;
        Rule plus_expr;
        Rule minus_expr;
    };

    ValueRefParserDefinition<int> int_value_ref_def(int_expr_p);
    ValueRefParserDefinition<double> double_value_ref_def(double_expr_p);
    ValueRefParserDefinition<PlanetSize> planetsize_value_ref_def(planetsize_expr_p);
    ValueRefParserDefinition<PlanetType> planettype_value_ref_def(planettype_expr_p);
    ValueRefParserDefinition<PlanetEnvironment> planetenvironment_value_ref_def(planetenvironment_expr_p);
    ValueRefParserDefinition<UniverseObjectType> universeobjecttype_value_ref_def(universeobjecttype_expr_p);
    ValueRefParserDefinition<StarType> startype_value_ref_def(startype_expr_p);
    ValueRefParserDefinition<FocusType> focustype_value_ref_def(focustype_expr_p);

    template <class T>
    ValueRefParserDefinition<T>::ValueRefParserDefinition(Rule& expr)
    {
        int_variable_final =
            str_p("owner")
            | "id"
            | "creationturn"
            | "age";

        SpecializedInit();

        variable_container =
            str_p("planet")
            | "system"
            | "fleet";

        SpecializedVarDefinition();

        primary_expr =
            constant[primary_expr.this_ = arg1]
            | variable[primary_expr.this_ = arg1]
            | '(' >> expr[primary_expr.this_ = arg1] >> ')';

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
    void ValueRefParserDefinition<int>::SpecializedInit()
    {
        constant =
            real_p[constant.this_ = new_<RefConst>(static_cast_<int>(arg1))]
            | int_p[constant.this_ = new_<RefConst>(arg1)];

        variable_final = int_variable_final;
    }

    template <>
    void ValueRefParserDefinition<double>::SpecializedInit()
    {
        constant =
            real_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<double>(arg1))];

        variable_final =
            str_p("currentfarming")
            | "maxfarming"
            | "currentindustry"
            | "maxindustry"
            | "currentresearch"
            | "maxresearch"
            | "currenttrade"
            | "maxtrade"
            | "currentmining"
            | "maxmining"
            | "currentconstruction"
            | "maxconstruction"
            | "currenthealth"
            | "maxhealth"
            | "currentpopulation"
            | "maxpopulation"
            | "maxfuel"
            | "currentfuel"
            | "maxsupply"
            | "currentsupply"
            | "maxstealth"
            | "currentstealth"
            | "maxdetection"
            | "currentdetection"
            | "maxshield"
            | "currentshield"
            | "maxdefense"
            | "currentdefense"
            | "mineralstockpile"
            | "foodstockpile"
            | "tradeproduction"
            | "foodproduction"
            | "mineralproduction"
            | "industryproduction"
            | "researchproduction";
    }

    template <>
    void ValueRefParserDefinition<PlanetSize>::SpecializedInit()
    {
        constant =
            planet_size_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<PlanetSize>(arg1))];

        variable_final = str_p("planetsize");
    }

    template <>
    void ValueRefParserDefinition<PlanetType>::SpecializedInit()
    {
        constant =
            planet_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<PlanetType>(arg1))];

        variable_final = str_p("planettype");
    }

    template <>
    void ValueRefParserDefinition<PlanetEnvironment>::SpecializedInit()
    {
        constant =
            planet_environment_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<PlanetEnvironment>(arg1))];

        variable_final = str_p("planetenvironment");
    }

    template <>
    void ValueRefParserDefinition<UniverseObjectType>::SpecializedInit()
    {
        constant =
            universe_object_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<UniverseObjectType>(arg1))];

        variable_final = str_p("objecttype");
    }

    template <>
    void ValueRefParserDefinition<StarType>::SpecializedInit()
    {
        constant =
            star_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<StarType>(arg1))];

        variable_final = str_p("startype");
    }

    template <>
    void ValueRefParserDefinition<FocusType>::SpecializedInit()
    {
        constant =
            focus_type_p[constant.this_ = new_<RefConst>(arg1)]
            | int_p[constant.this_ = new_<RefConst>(static_cast_<FocusType>(arg1))];

        variable_final = str_p("primaryfocus") | "secondaryfocus";
    }

    template <class T>
    void ValueRefParserDefinition<T>::SpecializedVarDefinition()
    {
        variable =
            str_p("source") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(true), construct_<std::string>(arg1, arg2))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(false), construct_<std::string>(arg1, arg2))];
    }

    template <>
    void ValueRefParserDefinition<int>::SpecializedVarDefinition()
    {
        variable =
            str_p("source") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(true), construct_<std::string>(arg1, arg2))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(false), construct_<std::string>(arg1, arg2))]
            | str_p("currentturn")
            [variable.this_ = new_<RefVar>(val(false), construct_<std::string>(arg1, arg2))];
    }

    template <>
    void ValueRefParserDefinition<double>::SpecializedVarDefinition()
    {
        typedef ValueRef::StaticCast<int, double> CastRefVar;
        variable =
            str_p("source") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(true), construct_<std::string>(arg1, arg2))]
            | str_p("source") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastRefVar>(new_<IntRefVar>(val(true), construct_<std::string>(arg1, arg2)))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> variable_final)
            [variable.this_ = new_<RefVar>(val(false), construct_<std::string>(arg1, arg2))]
            | str_p("target") >> '.' >> (!(variable_container >> ".") >> int_variable_final)
            [variable.this_ = new_<CastRefVar>(new_<IntRefVar>(val(false), construct_<std::string>(arg1, arg2)))]
            | str_p("currentturn")
            [variable.this_ = new_<CastRefVar>(new_<IntRefVar>(val(false), construct_<std::string>(arg1, arg2)))];
    }
}
