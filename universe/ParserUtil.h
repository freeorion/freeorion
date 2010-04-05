// -*- C++ -*-
#ifndef _ParserUtil_h_
#define _ParserUtil_h_

#include "Parser.h"

struct ParamLabel : public boost::spirit::classic::grammar<ParamLabel>
{
    ParamLabel(const std::string& param_name) : m_param_name(param_name) {}
    template <typename ScannerT>
    struct definition
    {
        definition(ParamLabel const& self) {
            r = !(boost::spirit::classic::str_p(self.m_param_name.c_str()) >> '=');
        }
        boost::spirit::classic::rule<ScannerT> r;
        boost::spirit::classic::rule<ScannerT> const& start() const {return r;}
    };
    const std::string m_param_name;
};

struct push_back_impl
{
    template <class Container, class Item>
    struct result {typedef void type;};
    template <class Container, class Item>
    void operator()(Container& c, const Item& item) const {c.push_back(item);}
};
extern const phoenix::function<push_back_impl> push_back_;

struct insert_impl
{
    template <class Container, class Item>
    struct result {typedef std::pair<typename Container::iterator, bool> type;};
    template <class Container, class Item>
    std::pair<typename Container::iterator, bool>
    operator()(Container& c, const Item& item) const {return c.insert(item);}
};
extern const phoenix::function<insert_impl> insert_;

extern boost::spirit::classic::rule<Scanner, NameClosure::context_t>     name_p;
extern boost::spirit::classic::rule<Scanner, NameClosure::context_t>     file_name_p;
extern boost::spirit::classic::rule<Scanner, ColourClosure::context_t>   colour_p;

extern boost::spirit::classic::symbols<bool>                     true_false_p;

extern boost::spirit::classic::symbols<PlanetSize>               planet_size_p;
extern boost::spirit::classic::symbols<PlanetType>               planet_type_p;
extern boost::spirit::classic::symbols<PlanetEnvironment>        planet_environment_type_p;
extern boost::spirit::classic::symbols<UniverseObjectType>       universe_object_type_p;
extern boost::spirit::classic::symbols<StarType>                 star_type_p;
extern boost::spirit::classic::symbols<FocusType>                focus_type_p;
extern boost::spirit::classic::symbols<EmpireAffiliationType>    affiliation_type_p;
extern boost::spirit::classic::symbols<UnlockableItemType>       unlockable_item_type_p;
extern boost::spirit::classic::symbols<TechType>                 tech_type_p;
extern boost::spirit::classic::symbols<CombatFighterType>        combat_fighter_type_p;
extern boost::spirit::classic::symbols<ShipPartClass>            part_class_p;
extern boost::spirit::classic::symbols<ShipSlotType>             slot_type_p;

void ReportError(const char* input, const boost::spirit::classic::parse_info<const char*>& result);

#endif // _ParserUtil_h_
