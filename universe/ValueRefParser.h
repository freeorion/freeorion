// -*- C++ -*-
#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

namespace ValueRef {
    template <class T>
    struct ValueRefBase;
}

template <class T>
struct ValueRefRule
{
    typedef ValueRef::ValueRefBase<T> RefBase;
    struct Closure : boost::spirit::closure<Closure, RefBase*, RefBase*, RefBase*>
    {
        typedef boost::spirit::closure<Closure, RefBase*, RefBase*, RefBase*> BaseClass;
        typename BaseClass::member1 this_;
        typename BaseClass::member2 operand1;
        typename BaseClass::member3 operand2;
    };
    typedef boost::spirit::rule<Scanner, typename Closure::context_t> type;
};

typedef ValueRefRule<int>::type IntValueRefRule;
typedef ValueRefRule<double>::type DoubleValueRefRule;
typedef ValueRefRule<PlanetSize>::type PlanetSizeValueRefRule;
typedef ValueRefRule<PlanetType>::type PlanetTypeValueRefRule;
typedef ValueRefRule<PlanetEnvironment>::type PlanetEnvironmentValueRefRule;
typedef ValueRefRule<UniverseObjectType>::type UniverseObjectTypeValueRefRule;
typedef ValueRefRule<StarType>::type StarTypeValueRefRule;
typedef ValueRefRule<FocusType>::type FocusTypeValueRefRule;

extern IntValueRefRule int_expr_p;
extern DoubleValueRefRule double_expr_p;
extern PlanetSizeValueRefRule planetsize_expr_p;
extern PlanetTypeValueRefRule planettype_expr_p;
extern PlanetEnvironmentValueRefRule planetenvironment_expr_p;
extern UniverseObjectTypeValueRefRule universeobjecttype_expr_p;
extern StarTypeValueRefRule startype_expr_p;
extern FocusTypeValueRefRule focustype_expr_p;

#endif // _ValueRefParser_h_
