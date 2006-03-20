// -*- C++ -*-
#ifndef _Parser_h_
#define _Parser_h_

#define PHOENIX_LIMIT 12
#define BOOST_SPIRIT_CLOSURE_LIMIT PHOENIX_LIMIT

#include "Enums.h"
#include "Tech.h"

#include <boost/spirit.hpp>
#include <boost/spirit/attribute.hpp>
#include <boost/spirit/phoenix.hpp>
#include <boost/tuple/tuple.hpp>

#include <stdexcept>
#include <set>
#include <string>


////////////////////////////////////////////////////////////
// Forward Declarations                                   //
////////////////////////////////////////////////////////////
namespace Condition {
    class ConditionBase;
}
namespace Effect {
    class EffectsGroup;
    class EffectBase;
}
class Special;
class BuildingType;


////////////////////////////////////////////////////////////
// Scanner                                                //
////////////////////////////////////////////////////////////
struct Skip : boost::spirit::grammar<Skip>
{
    template <class ScannerT>
    struct definition
    {
        definition(const Skip&)
        {
            using namespace boost::spirit;
            skip = space_p | comment_p("//") | comment_p("/*", "*/");
        }
        boost::spirit::rule<ScannerT> skip;
        const boost::spirit::rule<ScannerT>& start() const {return skip;}
    };
};
extern const Skip skip_p;

typedef boost::spirit::scanner<const char*, boost::spirit::scanner_policies<boost::spirit::skip_parser_iteration_policy<Skip> > > ScannerBase;
typedef boost::spirit::as_lower_scanner<ScannerBase>::type Scanner;

struct NameClosure : boost::spirit::closure<NameClosure, std::string>
{
    member1 this_;
};


////////////////////////////////////////////////////////////
// Condition Parser                                       //
////////////////////////////////////////////////////////////
struct ConditionClosure : boost::spirit::closure<ConditionClosure, Condition::ConditionBase*>
{
    member1 this_;
};

extern boost::spirit::rule<Scanner, ConditionClosure::context_t> condition_p;


////////////////////////////////////////////////////////////
// Effect Parser                                          //
////////////////////////////////////////////////////////////
struct EffectClosure : boost::spirit::closure<EffectClosure, Effect::EffectBase*>
{
    member1 this_;
};

extern boost::spirit::rule<Scanner, EffectClosure::context_t> effect_p;


////////////////////////////////////////////////////////////
// Top Level Parsers                                      //
////////////////////////////////////////////////////////////
struct BuildingTypeClosure : boost::spirit::closure<BuildingTypeClosure, BuildingType*, std::string,
                                                    std::string, double, int, double,
                                                    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
                                                    std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 build_cost;
    member5 build_time;
    member6 maintenance_cost;
    member7 effects_groups;
    member8 graphic;
};

struct SpecialClosure : boost::spirit::closure<SpecialClosure, Special*, std::string, std::string,
                                               std::vector<boost::shared_ptr<const Effect::EffectsGroup> > >
{
    member1 this_;
    member2 name;
    member3 description;
    member4 effects_groups;
};

struct TechClosure : boost::spirit::closure<TechClosure, Tech*,
                                            std::string, std::string, std::string, TechType, double, int,
                                            std::vector<boost::shared_ptr<const Effect::EffectsGroup> >, std::set<std::string>,
                                            std::vector<Tech::ItemSpec>, std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 category;
    member5 tech_type;
    member6 research_cost;
    member7 research_turns;
    member8 effects_groups;
    member9 prerequisites;
    member10 unlocked_items;
    member11 graphic;
};

extern boost::spirit::rule<Scanner, BuildingTypeClosure::context_t> building_type_p;
extern boost::spirit::rule<Scanner, SpecialClosure::context_t> special_p;
extern boost::spirit::rule<Scanner, NameClosure::context_t> tech_category_p;
extern boost::spirit::rule<Scanner, TechClosure::context_t> tech_p;

#endif // _Parser_h_
