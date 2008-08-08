#include "Condition.h"

#include "../util/AppInterface.h"
#include "UniverseObject.h"
#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "Planet.h"
#include "Meter.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../util/Random.h"
#include "System.h"

#include <boost/format.hpp>

using namespace boost::io;
using boost::lexical_cast;
using boost::format;

extern int g_indent;

namespace {
    const Fleet* FleetFromObject(const UniverseObject* obj)
    {
        const Fleet* retval = universe_object_cast<const Fleet*>(obj);
        if (!retval) {
            if (const Ship* ship = universe_object_cast<const Ship*>(obj))
                retval = ship->GetFleet();
        }
        return retval;
    }
}

///////////////////////////////////////////////////////////
// Condition::ConditionBase                              //
///////////////////////////////////////////////////////////
Condition::ConditionBase::ConditionBase()
{}

Condition::ConditionBase::~ConditionBase()
{}

void Condition::ConditionBase::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain/* = NON_TARGETS*/) const
{
    ObjectSet& from_set = search_domain == TARGETS ? targets : non_targets;
    ObjectSet& to_set = search_domain == TARGETS ? non_targets : targets;
    ObjectSet::iterator it = from_set.begin();
    ObjectSet::iterator end_it = from_set.end();
    for ( ; it != end_it; ) {
        ObjectSet::iterator temp = it++;
        if (search_domain == TARGETS ? !Match(source, *temp) : Match(source, *temp)) {
            to_set.insert(*temp);
            from_set.erase(temp);
        }
    }
}

std::string Condition::ConditionBase::Description(bool negated/* = false*/) const
{
    return "";
}

std::string Condition::ConditionBase::Dump() const
{
    return "";
}

bool Condition::ConditionBase::Match(const UniverseObject* source, const UniverseObject* target) const
{
    return false;
}

///////////////////////////////////////////////////////////
// Number                                                //
///////////////////////////////////////////////////////////
Condition::Number::Number(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, const ConditionBase* condition) :
    m_low(low),
    m_high(high),
    m_condition(condition)
{
}

Condition::Number::~Number()
{
    delete m_low;
    delete m_high;
    delete m_condition;
}

std::string Condition::Number::Description(bool negated/* = false*/) const
{
    std::string low_str = ValueRef::ConstantExpr(m_low) ? lexical_cast<std::string>(m_low->Eval(0, 0)) : m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ? lexical_cast<std::string>(m_high->Eval(0, 0)) : m_high->Description();
    std::string description_str = "DESC_NUMBER";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str))
               % low_str
               % high_str
               % m_condition->Description());
}

std::string Condition::Number::Dump() const
{
    std::string retval = DumpIndent() + "Number low = " + m_low->Dump() + "Number high = " + m_high->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

void Condition::Number::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets,
                               SearchDomain search_domain/* = NON_TARGETS*/) const
{
    // get set of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator uit = universe.begin(); uit != universe.end(); ++uit) {
        condition_non_targets.insert(uit->second);
    }
    m_condition->Eval(source, condition_targets, condition_non_targets);

    // compare number of objects that satisfy m_condition to the acceptable range of such objects
    int matched = condition_targets.size();
    int low = m_low->Eval(source, source);
    int high = m_high->Eval(source, source);
    bool in_range = (low <= matched && matched < high);

    if (search_domain == TARGETS && !in_range) {
        non_targets.insert(targets.begin(), targets.end());
    }
    if (search_domain == NON_TARGETS && in_range) {
        targets.insert(non_targets.begin(), non_targets.end());
    }
}

///////////////////////////////////////////////////////////
// Turn                                                  //
///////////////////////////////////////////////////////////
Condition::Turn::Turn(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high) :
    m_low(low),
    m_high(high)
{}

Condition::Turn::~Turn()
{
    delete m_low;
    delete m_high;
}

std::string Condition::Turn::Description(bool negated/* = false*/) const
{
    std::string low_str = ValueRef::ConstantExpr(m_low) ? lexical_cast<std::string>(m_low->Eval(0, 0)) : m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ? lexical_cast<std::string>(m_high->Eval(0, 0)) : m_high->Description();
    std::string description_str = "DESC_TURN";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str))
               % low_str
               % high_str);
}

std::string Condition::Turn::Dump() const
{
    return DumpIndent() + "Turn low = " + m_low->Dump() + " high = " + m_high->Dump() + "\n";
}

bool Condition::Turn::Match(const UniverseObject* source, const UniverseObject* target) const
{
    double low = std::max(0, m_low->Eval(source, target));
    double high = std::min(m_high->Eval(source, target), IMPOSSIBLY_LARGE_TURN);
    int turn = CurrentTurn();
        
    return (low <= turn && turn < high);
}

///////////////////////////////////////////////////////////
// NumberOf                                              //
///////////////////////////////////////////////////////////
Condition::NumberOf::NumberOf(const ValueRef::ValueRefBase<int>* number, const ConditionBase* condition) :
    m_number(number),
    m_condition(condition)
{
}

Condition::NumberOf::~NumberOf()
{
    delete m_number;
    delete m_condition;
}

std::string Condition::NumberOf::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_number) ? lexical_cast<std::string>(m_number->Dump()) : m_number->Description();
    std::string description_str = "DESC_NUMBER_OF";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str))
               % value_str
               % m_condition->Description());
}

std::string Condition::NumberOf::Dump() const
{
    std::string retval = DumpIndent() + "NumberOf number = " + m_number->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

void Condition::NumberOf::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets,
                               SearchDomain search_domain/* = NON_TARGETS*/) const
{
    // get set of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator uit = universe.begin(); uit != universe.end(); ++uit) {
        condition_non_targets.insert(uit->second);
    }
    m_condition->Eval(source, condition_targets, condition_non_targets);

    // find the desired number of objects in condition_targets
    int number = m_number->Eval(source, source);
    std::vector<bool> inclusion_list(condition_targets.size());
    std::vector<int> selection_list(condition_targets.size());
    for (unsigned int i = 0; i < selection_list.size(); ++i) {
        selection_list[i] = i;
    }
    for (int i = 0; i < std::min(number, static_cast<int>(condition_targets.size())); ++i) {
        int temp = RandSmallInt(0, selection_list.size() - 1);
        int selection = selection_list[temp];
        inclusion_list[selection] = true;
        selection_list.erase(selection_list.begin() + temp);
    }

    int i = 0;
    for (ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it, ++i) {
        ObjectSet& from_set = search_domain == TARGETS ? targets : non_targets;
        ObjectSet& to_set = search_domain == TARGETS ? non_targets : targets;
        if (search_domain == TARGETS ? !inclusion_list[i] : inclusion_list[i]) {
            to_set.insert(*it);
            from_set.erase(*it);
        }
    }

    if (search_domain == TARGETS) {
        for (ObjectSet::const_iterator it = condition_non_targets.begin(); it != condition_non_targets.end(); ++it) {
            non_targets.insert(*it);
            targets.erase(*it);
        }
    }
}

///////////////////////////////////////////////////////////
// All                                                   //
///////////////////////////////////////////////////////////
Condition::All::All()
{}

void Condition::All::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain/* = NON_TARGETS*/) const
{
    if (search_domain == NON_TARGETS) {
        targets.insert(non_targets.begin(), non_targets.end());
        non_targets.clear();
    }
}

std::string Condition::All::Description(bool negated/* = false*/) const
{
    return UserString("DESC_ALL");
}

std::string Condition::All::Dump() const
{
    return DumpIndent() + "All\n";
}

///////////////////////////////////////////////////////////
// EmpireAffiliation                                     //
///////////////////////////////////////////////////////////
Condition::EmpireAffiliation::EmpireAffiliation(const ValueRef::ValueRefBase<int>* empire_id, EmpireAffiliationType affiliation, bool exclusive) :
    m_empire_id(empire_id),
    m_affiliation(affiliation),
    m_exclusive(exclusive)
{}

Condition::EmpireAffiliation::~EmpireAffiliation()
{
    delete m_empire_id;
}

std::string Condition::EmpireAffiliation::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_empire_id) ? Empires().Lookup(m_empire_id->Eval(0, 0))->Name() : m_empire_id->Description();
    if (m_affiliation == AFFIL_SELF) {
        std::string description_str = m_exclusive ? "DESC_EMPIRE_AFFILIATION_SELF_EXCLUSIVE" : "DESC_EMPIRE_AFFILIATION_SELF";
        if (negated)
            description_str += "_NOT";
        return str(format(UserString(description_str)) % value_str);
    } else {
        std::string description_str = m_exclusive ? "DESC_EMPIRE_AFFILIATION_EXCLUSIVE" : "DESC_EMPIRE_AFFILIATION";
        if (negated)
            description_str += "_NOT";
        return str(format(UserString(description_str))
                   % UserString(lexical_cast<std::string>(m_affiliation))
                   % value_str);
    }
}

std::string Condition::EmpireAffiliation::Dump() const
{
    std::string retval = DumpIndent() + (m_exclusive ? "OwnedExclusivelyBy" : "OwnedBy");
    retval += " affiliation = ";
    switch (m_affiliation) {
    case AFFIL_SELF:  retval += "TheEmpire"; break;
    case AFFIL_ENEMY: retval += "EnemyOf"; break;
    case AFFIL_ALLY:  retval += "AllyOf"; break;
    default: retval += "?"; break;
    }
    retval += " empire = " + m_empire_id->Dump() + "\n";
    return retval;
}

bool Condition::EmpireAffiliation::Match(const UniverseObject* source, const UniverseObject* target) const
{
    switch (m_affiliation) {
    case AFFIL_SELF:
        return m_exclusive ? target->WhollyOwnedBy(m_empire_id->Eval(source, target)) : target->OwnedBy(m_empire_id->Eval(source, target));
            break;
    case AFFIL_ENEMY:
        // TODO
        break;
    case AFFIL_ALLY:
        // TODO
        break;
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////
// Self                                                  //
///////////////////////////////////////////////////////////
Condition::Self::Self()
{}

std::string Condition::Self::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_SELF";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Self::Dump() const
{
    return DumpIndent() + "Source\n";
}

bool Condition::Self::Match(const UniverseObject* source, const UniverseObject* target) const
{
    return source == target;
}

///////////////////////////////////////////////////////////
// Type                                                  //
///////////////////////////////////////////////////////////
Condition::Type::Type(const ValueRef::ValueRefBase<UniverseObjectType>* type) :
    m_type(type)
{}

std::string Condition::Type::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_type) ? UserString(lexical_cast<std::string>(m_type->Eval(0, 0))) : m_type->Description();
    std::string description_str = "DESC_TYPE";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % value_str);
}

std::string Condition::Type::Dump() const
{
    std::string retval = DumpIndent();
    if (dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(m_type)) {
        switch (m_type->Eval(0, 0)) {
        case OBJ_BUILDING:    retval += "Building\n"; break;
        case OBJ_SHIP:        retval += "Ship\n"; break;
        case OBJ_FLEET:       retval += "Fleet\n"; break;
        case OBJ_PLANET:      retval += "Planet\n"; break;
        case OBJ_POP_CENTER:  retval += "PopulationCenter\n"; break;
        case OBJ_PROD_CENTER: retval += "ProductionCenter\n"; break;
        case OBJ_SYSTEM:      retval += "System\n"; break;
        default: retval += "?\n"; break;
        }
    } else {
        retval += "ObjectType type = " + m_type->Dump() + "\n";
    }
    return retval;
}

bool Condition::Type::Match(const UniverseObject* source, const UniverseObject* target) const
{
    switch (m_type->Eval(source, target)) {
    case OBJ_BUILDING:
        return universe_object_cast<const ::Building*>(target);
        break;
    case OBJ_SHIP:
        return universe_object_cast<const Ship*>(target);
        break;
    case OBJ_FLEET:
        return universe_object_cast<const Fleet*>(target);
        break;
    case OBJ_PLANET:
        return universe_object_cast<const Planet*>(target);
        break;
    case OBJ_POP_CENTER:
        return dynamic_cast<const PopCenter*>(target);
        break;
    case OBJ_PROD_CENTER:
        return dynamic_cast<const ResourceCenter*>(target);
        break;
    case OBJ_SYSTEM:
        return universe_object_cast<const System*>(target);
        break;
    default:
            break;
    }
    return false;
}

///////////////////////////////////////////////////////////
// Building                                              //
///////////////////////////////////////////////////////////
Condition::Building::Building(const std::string& name) :
    m_name(name)
{}

std::string Condition::Building::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_BUILDING";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % UserString(m_name));
}

std::string Condition::Building::Dump() const
{
    return DumpIndent() + "Building name = \"" + m_name + "\"\n";
}

bool Condition::Building::Match(const UniverseObject* source, const UniverseObject* target) const
{
    const ::Building* building = universe_object_cast<const ::Building*>(target);
    return building && building->BuildingTypeName() == m_name;
}

///////////////////////////////////////////////////////////
// HasSpecial                                            //
///////////////////////////////////////////////////////////
Condition::HasSpecial::HasSpecial(const std::string& name) :
    m_name(name)
{}

std::string Condition::HasSpecial::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_SPECIAL";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % UserString(m_name));
}

std::string Condition::HasSpecial::Dump() const
{
    return DumpIndent() + "HasSpecial name = \"" + m_name + "\"\n";
}

bool Condition::HasSpecial::Match(const UniverseObject* source, const UniverseObject* target) const
{
    return (m_name == "All" && !target->Specials().empty()) || target->Specials().find(m_name) != target->Specials().end();
}

///////////////////////////////////////////////////////////
// Contains                                              //
///////////////////////////////////////////////////////////
Condition::Contains::Contains(const ConditionBase* condition) :
    m_condition(condition)
{}

std::string Condition::Contains::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_CONTAINS";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % m_condition->Description());
}

std::string Condition::Contains::Dump() const
{
    std::string retval = DumpIndent() + "Contains condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

void Condition::Contains::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets,
                               SearchDomain search_domain/* = NON_TARGETS*/) const
{
    // get the list of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        condition_non_targets.insert(it->second);
    }
    m_condition->Eval(source, condition_targets, condition_non_targets);

    ObjectSet& from_set = search_domain == TARGETS ? targets : non_targets;
    ObjectSet& to_set = search_domain == TARGETS ? non_targets : targets;
    ObjectSet::iterator from_it = from_set.begin();
    ObjectSet::iterator from_end = from_set.end();

    for ( ; from_it != from_end; ) {
        ObjectSet::iterator container_it = from_it++;

        ObjectSet::const_iterator contained_it = condition_targets.begin();
        ObjectSet::const_iterator contained_end = condition_targets.end();
        
        if (search_domain == NON_TARGETS) {
            // non_targets (from_set) objects need to contain at least one condition_target to be transferred from
            // from_set to to_set.
            // As soon as one contained condition target is found, object may be transferred.
            for ( ; contained_it != contained_end; ++contained_it) {
                if ((*container_it)->Contains((*contained_it)->ID())) {
                    to_set.insert(*container_it);
                    from_set.erase(container_it);
                    break;  // don't need to check any more possible contained objects for this non_target set object
                }
            }
        } else {
            // targets (from_set) objects need to include no condition_targets to be transferred from from_set
            // to to_set.
            // As soon as one contained condition target is found, it is known that object need NOT
            // be trasferred.  Only after all condition targets are verified not to be contained can object
            // be transferred.
            for ( ; contained_it != contained_end; ++contained_it) {
                if ((*container_it)->Contains((*contained_it)->ID()))
                    break;
            }
            // if the end of the condition_targets was reached, no condition targets were contained, so 
            // this targets object can be transferred to the non_targets set
            if (contained_it == contained_end) {
                to_set.insert(*container_it);
                from_set.erase(container_it);
            }
        }
    }
}

///////////////////////////////////////////////////////////
// ContainedBy                                           //
///////////////////////////////////////////////////////////
Condition::ContainedBy::ContainedBy(const ConditionBase* condition) :
    m_condition(condition)
{}

std::string Condition::ContainedBy::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_CONTAINED_BY";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % m_condition->Description());
}

std::string Condition::ContainedBy::Dump() const
{
    std::string retval = DumpIndent() + "ContainedBy condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

void Condition::ContainedBy::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets,
                                  SearchDomain search_domain/* = NON_TARGETS*/) const
{
    // get the list of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        condition_non_targets.insert(it->second);
    }
    m_condition->Eval(source, condition_targets, condition_non_targets);

    ObjectSet& from_set = search_domain == TARGETS ? targets : non_targets;
    ObjectSet& to_set = search_domain == TARGETS ? non_targets : targets;
    ObjectSet::iterator from_it = from_set.begin();
    ObjectSet::iterator from_end = from_set.end();

    for ( ; from_it != from_end; ) {
        ObjectSet::iterator contained_it = from_it++;

        ObjectSet::const_iterator container_it = condition_targets.begin();
        ObjectSet::const_iterator container_end = condition_targets.end();

        if (search_domain == NON_TARGETS) {
            // non_targets (from_set) objects need to be contained by at least one condition_target to be 
            // transferred from from_set to to_set.
            // As soon as one containing condition target is found, object may be transferred.
            for ( ; container_it != container_end; ++container_it) {
                if ((*container_it)->Contains((*contained_it)->ID())) {
                    to_set.insert(*contained_it);
                    from_set.erase(contained_it);
                    break;  // don't need to check any more possible container objects for this non_target set object
                }
            }
        } else {
            // targets (from_set) objects need to be contained by no condition_targets to be transferred from from_set
            // to to_set.
            // As soon as one containing condition target is found, it is known that object need NOT
            // be trasferred.  Only after all condition targets are verified not to contain can object
            // be transferred.
            for ( ; container_it != container_end; ++container_it) {
                if ((*container_it)->Contains((*contained_it)->ID()))
                    break;
            }
            // if the end of the condition_targets was reached, no condition targets contained, so 
            // this targets object can be transferred to the non_targets set
            if (container_it == container_end) {
                to_set.insert(*contained_it);
                from_set.erase(contained_it);
            }
        }
    }
}

///////////////////////////////////////////////////////////
// PlanetType                                            //
///////////////////////////////////////////////////////////
Condition::PlanetType::PlanetType(const std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>& types) :
    m_types(types)
{}

Condition::PlanetType::~PlanetType()
{
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        delete m_types[i];
    }
}

std::string Condition::PlanetType::Description(bool negated/* = false*/) const
{
    std::string values_str;
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_types[i]) ? UserString(lexical_cast<std::string>(m_types[i]->Eval(0, 0))) : m_types[i]->Description();
        if (2 <= m_types.size() && i < m_types.size() - 2) {
            values_str += ", ";
        } else if (i == m_types.size() - 2) {
            values_str += m_types.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = "DESC_PLANET_TYPE";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % values_str);
}

std::string Condition::PlanetType::Dump() const
{
    std::string retval = DumpIndent() + "Planet type = ";
    if (m_types.size() == 1) {
        retval += m_types[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            retval += m_types[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::PlanetType::Match(const UniverseObject* source, const UniverseObject* target) const
{
    const Planet* planet = universe_object_cast<const Planet*>(target);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(target))) {
        planet = building->GetPlanet();
    }
    if (planet) {
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            if (m_types[i]->Eval(source, target) == planet->Type())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// PlanetSize                                            //
///////////////////////////////////////////////////////////
Condition::PlanetSize::PlanetSize(const std::vector<const ValueRef::ValueRefBase< ::PlanetSize>*>& sizes) :
    m_sizes(sizes)
{}

Condition::PlanetSize::~PlanetSize()
{
    for (unsigned int i = 0; i < m_sizes.size(); ++i) {
        delete m_sizes[i];
    }
}

std::string Condition::PlanetSize::Description(bool negated/* = false*/) const
{
    std::string values_str;
    for (unsigned int i = 0; i < m_sizes.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_sizes[i]) ? UserString(lexical_cast<std::string>(m_sizes[i]->Eval(0, 0))) : m_sizes[i]->Description();
        if (2 <= m_sizes.size() && i < m_sizes.size() - 2) {
            values_str += ", ";
        } else if (i == m_sizes.size() - 2) {
            values_str += m_sizes.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = "DESC_PLANET_SIZE";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % values_str);
}

std::string Condition::PlanetSize::Dump() const
{
    std::string retval = DumpIndent() + "Planet size = ";
    if (m_sizes.size() == 1) {
        retval += m_sizes[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_sizes.size(); ++i) {
            retval += m_sizes[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::PlanetSize::Match(const UniverseObject* source, const UniverseObject* target) const
{
    const Planet* planet = universe_object_cast<const Planet*>(target);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(target))) {
        planet = building->GetPlanet();
    }
    if (planet) {
        for (unsigned int i = 0; i < m_sizes.size(); ++i) {
            if (m_sizes[i]->Eval(source, target) == planet->Size())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// PlanetEnvironment                                     //
///////////////////////////////////////////////////////////
Condition::PlanetEnvironment::PlanetEnvironment(const std::vector<const ValueRef::ValueRefBase< ::PlanetEnvironment>*>& environments) :
    m_environments(environments)
{}

Condition::PlanetEnvironment::~PlanetEnvironment()
{
    for (unsigned int i = 0; i < m_environments.size(); ++i) {
        delete m_environments[i];
    }
}

std::string Condition::PlanetEnvironment::Description(bool negated/* = false*/) const
{
    std::string values_str;
    for (unsigned int i = 0; i < m_environments.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_environments[i]) ? UserString(lexical_cast<std::string>(m_environments[i]->Eval(0, 0))) : m_environments[i]->Description();
        if (2 <= m_environments.size() && i < m_environments.size() - 2) {
            values_str += ", ";
        } else if (i == m_environments.size() - 2) {
            values_str += m_environments.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = "DESC_PLANET_ENVIRONMENT";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % values_str);
}

std::string Condition::PlanetEnvironment::Dump() const
{
    std::string retval = DumpIndent() + "Planet environment = ";
    if (m_environments.size() == 1) {
        retval += m_environments[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_environments.size(); ++i) {
            retval += m_environments[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::PlanetEnvironment::Match(const UniverseObject* source, const UniverseObject* target) const
{
    const Planet* planet = universe_object_cast<const Planet*>(target);
    const ::Building* building = 0;
    if (!planet && (building = universe_object_cast<const ::Building*>(target))) {
        planet = building->GetPlanet();
    }
    if (planet) {
        for (unsigned int i = 0; i < m_environments.size(); ++i) {
            if (m_environments[i]->Eval(source, target) == planet->Environment())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// FocusType                                             //
///////////////////////////////////////////////////////////
Condition::FocusType::FocusType(const std::vector<const ValueRef::ValueRefBase< ::FocusType>*>& foci, bool primary) :
    m_foci(foci),
    m_primary(primary)
{}

Condition::FocusType::~FocusType()
{
    for (unsigned int i = 0; i < m_foci.size(); ++i) {
        delete m_foci[i];
    }
}

std::string Condition::FocusType::Description(bool negated/* = false*/) const
{
    std::string values_str;
    for (unsigned int i = 0; i < m_foci.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_foci[i]) ? UserString(lexical_cast<std::string>(m_foci[i]->Eval(0, 0))) : m_foci[i]->Description();
        if (2 <= m_foci.size() && i < m_foci.size() - 2) {
            values_str += ", ";
        } else if (i == m_foci.size() - 2) {
            values_str += m_foci.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = m_primary ? "DESC_FOCUS_TYPE_PRIMARY" : "DESC_FOCUS_TYPE_SECONDARY";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % values_str);
}

std::string Condition::FocusType::Dump() const
{
    std::string retval = DumpIndent() + (m_primary ? "Primary" : "Secondary");
    retval += "Focus focus = ";
    if (m_foci.size() == 1) {
        retval += m_foci[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_foci.size(); ++i) {
            retval += m_foci[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::FocusType::Match(const UniverseObject* source, const UniverseObject* target) const
{
    const ResourceCenter* prod_center = dynamic_cast<const ResourceCenter*>(target);
    const ::Building* building = 0;
    if (!prod_center && (building = universe_object_cast<const ::Building*>(target))) {
        prod_center = static_cast<const ResourceCenter*>(building->GetPlanet());
    }
    if (prod_center) {
        for (unsigned int i = 0; i < m_foci.size(); ++i) {
            if (m_foci[i]->Eval(source, target) == (m_primary ? prod_center->PrimaryFocus() : prod_center->SecondaryFocus()))
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// StarType                                              //
///////////////////////////////////////////////////////////
Condition::StarType::StarType(const std::vector<const ValueRef::ValueRefBase< ::StarType>*>& types) :
    m_types(types)
{}

Condition::StarType::~StarType()
{
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        delete m_types[i];
    }
}

std::string Condition::StarType::Description(bool negated/* = false*/) const
{
    std::string values_str;
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        values_str += ValueRef::ConstantExpr(m_types[i]) ? UserString(lexical_cast<std::string>(m_types[i]->Eval(0, 0))) : m_types[i]->Description();
        if (2 <= m_types.size() && i < m_types.size() - 2) {
            values_str += ", ";
        } else if (i == m_types.size() - 2) {
            values_str += m_types.size() < 3 ? " " : ", ";
            values_str += UserString("OR");
            values_str += " ";
        }
    }
    std::string description_str = "DESC_STAR_TYPE";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % values_str);
}

std::string Condition::StarType::Dump() const
{
    std::string retval = DumpIndent() + "Star type = ";
    if (m_types.size() == 1) {
        retval += m_types[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            retval += m_types[i]->Dump() + " ";
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::StarType::Match(const UniverseObject* source, const UniverseObject* target) const
{
    const System* system = target->GetSystem();
    if (system || (system = universe_object_cast<const System*>(target))) {
        for (unsigned int i = 0; i < m_types.size(); ++i) {
            if (m_types[i]->Eval(source, target) == system->Star())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// Chance                                                //
///////////////////////////////////////////////////////////
Condition::Chance::Chance(const ValueRef::ValueRefBase<double>* chance) :
    m_chance(chance)
{}

Condition::Chance::~Chance()
{
    delete m_chance;
}

std::string Condition::Chance::Description(bool negated/* = false*/) const
{
    std::string value_str;
    if (ValueRef::ConstantExpr(m_chance)) {
        std::string description_str = "DESC_CHANCE_PERCENTAGE";
        if (negated)
            description_str += "_NOT";
        return str(format(UserString(description_str)) % lexical_cast<std::string>(std::max(0.0, std::min(m_chance->Eval(0, 0), 1.0)) * 100));
    } else {
        std::string description_str = "DESC_CHANCE";
        if (negated)
            description_str += "_NOT";
        return str(format(UserString(description_str)) % m_chance->Description());
    }
}

std::string Condition::Chance::Dump() const
{
    return DumpIndent() + "Random probability = " + m_chance->Dump() + "\n";
}

bool Condition::Chance::Match(const UniverseObject* source, const UniverseObject* target) const
{
    double chance = std::max(0.0, std::min(m_chance->Eval(source, target), 1.0));
    return RandZeroToOne() <= chance;
}

///////////////////////////////////////////////////////////
// MeterValue                                            //
///////////////////////////////////////////////////////////
Condition::MeterValue::MeterValue(MeterType meter, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high, bool max_meter) :
    m_meter(meter),
    m_low(low),
    m_high(high),
    m_max_meter(max_meter)
{}

Condition::MeterValue::~MeterValue()
{
    delete m_low;
    delete m_high;
}

std::string Condition::MeterValue::Description(bool negated/* = false*/) const
{
    std::string low_str = ValueRef::ConstantExpr(m_low) ? lexical_cast<std::string>(m_low->Eval(0, 0)) : m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ? lexical_cast<std::string>(m_high->Eval(0, 0)) : m_high->Description();
    std::string description_str = m_max_meter ? "DESC_METER_VALUE_MAX" : "DESC_METER_VALUE_CURRENT";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str))
               % UserString(lexical_cast<std::string>(m_meter))
               % low_str
               % high_str);
}

std::string Condition::MeterValue::Dump() const
{
    std::string retval = DumpIndent() + (m_max_meter ? "Max" : "Current");
    switch (m_meter) {
    case METER_POPULATION:   retval += "Population"; break;
    case METER_FARMING:      retval += "Farming"; break;
    case METER_INDUSTRY:     retval += "Industry"; break;
    case METER_RESEARCH:     retval += "Research"; break;
    case METER_TRADE:        retval += "Trade"; break;
    case METER_MINING:       retval += "Mining"; break;
    case METER_CONSTRUCTION: retval += "Construction"; break;
    case METER_HEALTH:       retval += "Health"; break;
    default: retval += "?"; break;
    }
    retval += " low = " + m_low->Dump() + " high = " + m_high->Dump() + "\n";
    return retval;
}

bool Condition::MeterValue::Match(const UniverseObject* source, const UniverseObject* target) const
{
    double low = std::max(Meter::METER_MIN, m_low->Eval(source, target));
    double high = std::min(m_high->Eval(source, target), Meter::METER_MAX);
    if (const Meter* meter = target->GetMeter(m_meter)) {
        double value = m_max_meter ? meter->Max() : meter->Current();
        return low <= value && value < high;
    }
    return false;
}

///////////////////////////////////////////////////////////
// EmpireStockpileValue                                  //
///////////////////////////////////////////////////////////
Condition::EmpireStockpileValue::EmpireStockpileValue(ResourceType stockpile, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high) :
    m_stockpile(stockpile),
    m_low(low),
    m_high(high)
{}

Condition::EmpireStockpileValue::~EmpireStockpileValue()
{
    delete m_low;
    delete m_high;
}

std::string Condition::EmpireStockpileValue::Description(bool negated/* = false*/) const
{
    std::string low_str = ValueRef::ConstantExpr(m_low) ? lexical_cast<std::string>(m_low->Eval(0, 0)) : m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ? lexical_cast<std::string>(m_high->Eval(0, 0)) : m_high->Description();
    std::string description_str = "DESC_EMPIRE_STOCKPILE_VALUE";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str))
               % UserString(lexical_cast<std::string>(m_stockpile))
               % low_str
               % high_str);
}

std::string Condition::EmpireStockpileValue::Dump() const
{
    std::string retval = DumpIndent();
    switch (m_stockpile) {
    case RE_FOOD:       retval += "OwnerFoodStockpile";     break;
    case RE_MINERALS:   retval += "OwnerMineralStockpile";  break;
    case RE_TRADE:      retval += "OwnerTradeStockpile";    break;
    case RE_RESEARCH:   retval += "OwnerResearchStockpile"; break;
    case RE_INDUSTRY:   retval += "OwnerIndustryStockpile"; break;
    default: retval += "?"; break;
    }
    retval += " low = " + m_low->Dump() + " high = " + m_high->Dump() + "\n";
    return retval;
}

bool Condition::EmpireStockpileValue::Match(const UniverseObject* source, const UniverseObject* target) const
{
    if (target->Owners().size() != 1)
        return false;
    Empire* empire = Empires().Lookup(*target->Owners().begin());
    if (m_stockpile == RE_FOOD || m_stockpile == RE_MINERALS || m_stockpile == RE_TRADE) {
        double stockpile = empire->ResourceStockpile(m_stockpile);
        return (m_low->Eval(source, target) <= stockpile && stockpile <= m_high->Eval(source, target));
    }
    return false;
}

///////////////////////////////////////////////////////////
// OwnerHasTech                                          //
///////////////////////////////////////////////////////////
Condition::OwnerHasTech::OwnerHasTech(const std::string& name) :
    m_name(name)
{}

std::string Condition::OwnerHasTech::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_OWNER_HAS_TECH";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str)) % UserString(m_name));
}

std::string Condition::OwnerHasTech::Dump() const
{
    return DumpIndent() + "OwnerHasTech name = \"" + m_name + "\"\n";
}

bool Condition::OwnerHasTech::Match(const UniverseObject* source, const UniverseObject* target) const
{
    if (target->Owners().size() != 1)
        return false;
    Empire* empire = Empires().Lookup(*target->Owners().begin());
    return empire->TechResearched(m_name);
}

///////////////////////////////////////////////////////////
// VisibleToEmpire                                       //
///////////////////////////////////////////////////////////
Condition::VisibleToEmpire::VisibleToEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids) :
    m_empire_ids(empire_ids)
{}

Condition::VisibleToEmpire::~VisibleToEmpire()
{
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        delete m_empire_ids[i];
    }
}

std::string Condition::VisibleToEmpire::Description(bool negated/* = false*/) const
{
    if (m_empire_ids.size() == 1) {
        std::string value_str = ValueRef::ConstantExpr(m_empire_ids[0]) ? Empires().Lookup(m_empire_ids[0]->Eval(0, 0))->Name() : m_empire_ids[0]->Description();
        std::string description_str = "DESC_VISIBLE_TO_SINGLE_EMPIRE";
        if (negated)
            description_str += "_NOT";
        return str(format(UserString(description_str)) % value_str);
    } else {
        std::string values_str;
        for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
            values_str += ValueRef::ConstantExpr(m_empire_ids[i]) ? Empires().Lookup(m_empire_ids[i]->Eval(0, 0))->Name() : m_empire_ids[i]->Description();
            if (2 <= m_empire_ids.size() && i < m_empire_ids.size() - 2) {
                values_str += ", ";
            } else if (i == m_empire_ids.size() - 2) {
                values_str += m_empire_ids.size() < 3 ? " " : ", ";
                values_str += UserString("OR");
                values_str += " ";
            }
        }
        std::string description_str = "DESC_VISIBLE_TO_EMPIRES";
        if (negated)
            description_str += "_NOT";
        return str(format(UserString(description_str)) % values_str);
    }
}

std::string Condition::VisibleToEmpire::Dump() const
{
    std::string retval = DumpIndent() + "VisibleToEmpire empire = ";
    if (m_empire_ids.size() == 1) {
        retval += m_empire_ids[0]->Dump() + "\n";
    } else {
        retval += "[ ";
        for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
            retval += m_empire_ids[i]->Dump() + " " ;
        }
        retval += "]\n";
    }
    return retval;
}

bool Condition::VisibleToEmpire::Match(const UniverseObject* source, const UniverseObject* target) const
{
    bool retval = false;
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        if (target->GetVisibility(m_empire_ids[i]->Eval(source, target)) != VIS_NO_VISIBITY)
            return true;
    }
    return retval;
}

///////////////////////////////////////////////////////////
// WithinDistance                                        //
///////////////////////////////////////////////////////////
Condition::WithinDistance::WithinDistance(const ValueRef::ValueRefBase<double>* distance, const ConditionBase* condition) :
    m_distance(distance),
    m_condition(condition)
{}

Condition::WithinDistance::~WithinDistance()
{
    delete m_distance;
    delete m_condition;
}

void Condition::WithinDistance::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets,
                                     SearchDomain search_domain/* = NON_TARGETS*/) const
{
    // get the list of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        condition_non_targets.insert(it->second);
    }
    m_condition->Eval(source, condition_targets, condition_non_targets);

    // determine which objects in the Universe are within the specified distance from the objects in condition_targets
    for (ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it) {
        ObjectSet& from_set = search_domain == TARGETS ? targets : non_targets;
        ObjectSet& to_set = search_domain == TARGETS ? non_targets : targets;
        ObjectSet::iterator it2 = from_set.begin();
        ObjectSet::iterator end_it2 = from_set.end();
        for ( ; it2 != end_it2; ) {
            ObjectSet::iterator temp = it2++;
            if (search_domain == TARGETS ? !Match(*it, *temp) : Match(*it, *temp)) {
                to_set.insert(*temp);
                from_set.erase(temp);
            }
        }
    }
}

std::string Condition::WithinDistance::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_distance) ? lexical_cast<std::string>(m_distance->Eval(0, 0)) : m_distance->Description();
    std::string description_str = "DESC_WITHIN_DISTANCE";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str))
               % value_str
               % m_condition->Description());
}

std::string Condition::WithinDistance::Dump() const
{
    std::string retval = DumpIndent() + "WithinDistance distance = " + m_distance->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::WithinDistance::Match(const UniverseObject* source, const UniverseObject* target) const
{
    double dist = m_distance->Eval(source, target);
    double distance_squared = dist * dist;
    double delta_x = source->X() - target->X();
    double delta_y = source->Y() - target->Y();
    return (delta_x * delta_x + delta_y * delta_y) <= distance_squared;
}

///////////////////////////////////////////////////////////
// WithinStarlaneJumps                                   //
///////////////////////////////////////////////////////////
Condition::WithinStarlaneJumps::WithinStarlaneJumps(const ValueRef::ValueRefBase<int>* jumps, const ConditionBase* condition) :
    m_jumps(jumps),
    m_condition(condition)
{}

Condition::WithinStarlaneJumps::~WithinStarlaneJumps()
{
    delete m_jumps;
    delete m_condition;
}

void Condition::WithinStarlaneJumps::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets,
                                          SearchDomain search_domain/* = NON_TARGETS*/) const
{
    // get the list of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        condition_non_targets.insert(it->second);
    }
    m_condition->Eval(source, condition_targets, condition_non_targets);

    // determine which objects in the Universe are within the specified distance from the objects in condition_targets
    for (ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it) {
        ObjectSet& from_set = search_domain == TARGETS ? targets : non_targets;
        ObjectSet& to_set = search_domain == TARGETS ? non_targets : targets;
        ObjectSet::iterator it2 = from_set.begin();
        ObjectSet::iterator end_it2 = from_set.end();
        for ( ; it2 != end_it2; ) {
            ObjectSet::iterator temp = it2++;
            if (search_domain == TARGETS ? !Match(*it, *temp) : Match(*it, *temp)) {
                to_set.insert(*temp);
                from_set.erase(temp);
            }
        }
    }
}

std::string Condition::WithinStarlaneJumps::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_jumps) ? lexical_cast<std::string>(m_jumps->Eval(0, 0)) : m_jumps->Description();
    std::string description_str = "DESC_WITHIN_STARLANE_JUMPS";
    if (negated)
        description_str += "_NOT";
    return str(format(UserString(description_str))
               % value_str
               % m_condition->Description());
}

std::string Condition::WithinStarlaneJumps::Dump() const
{
    std::string retval = DumpIndent() + "WithinStarlaneJumps jumps = " + m_jumps->Dump() + " condition =\n";
    ++g_indent;
    retval += m_condition->Dump();
    --g_indent;
    return retval;
}

bool Condition::WithinStarlaneJumps::Match(const UniverseObject* source, const UniverseObject* target) const
{
    int jump_limit = m_jumps->Eval(source, target);
    if (jump_limit == 0) { // special case, since ShortestPath() doesn't expect the start point to be the end point
        double delta_x = source->X() - target->X();
        double delta_y = source->Y() - target->Y();
        return !(delta_x * delta_x + delta_y * delta_y);
    } else {
        const System* source_system = source->GetSystem();
        if (!source_system)
            source_system = universe_object_cast<const System*>(source);
        const System* target_system = target->GetSystem();
        if (!target_system)
            target_system = universe_object_cast<const System*>(target);
        if (source_system && target_system) {
            std::pair<std::list<System*>, double> path = GetUniverse().ShortestPath(source_system->ID(), target_system->ID());
            if (!path.first.empty()) { // if path.first is empty, no path exists between the systems
                return (static_cast<int>(path.first.size()) - 1) <= jump_limit;
            }
        } else if (source_system) {
            if (const Fleet* target_fleet = FleetFromObject(target)) {
                std::pair<std::list<System*>, double> path1 = GetUniverse().ShortestPath(source_system->ID(), target_fleet->PreviousSystemID());
                std::pair<std::list<System*>, double> path2 = GetUniverse().ShortestPath(source_system->ID(), target_fleet->NextSystemID());
                if (int jumps = static_cast<int>(std::max(path1.first.size(), path2.first.size())) - 1)
                    return jumps <= jump_limit;
            }
        } else if (target_system) {
           if (const Fleet* source_fleet = FleetFromObject(source)) {
                std::pair<std::list<System*>, double> path1 = GetUniverse().ShortestPath(source_fleet->PreviousSystemID(), target_system->ID());
                std::pair<std::list<System*>, double> path2 = GetUniverse().ShortestPath(source_fleet->NextSystemID(), target_system->ID());
                if (int jumps = static_cast<int>(std::max(path1.first.size(), path2.first.size())))
                    return jumps - 1 <= jump_limit;
            }
        } else {
            const Fleet* source_fleet = FleetFromObject(source);
            const Fleet* target_fleet = FleetFromObject(target);
            if (source_fleet && target_fleet) {
                int source_fleet_prev_system_id = source_fleet->PreviousSystemID();
                int source_fleet_next_system_id = source_fleet->NextSystemID();
                int target_fleet_prev_system_id = target_fleet->PreviousSystemID();
                int target_fleet_next_system_id = target_fleet->NextSystemID();
                std::pair<std::list<System*>, int> path1 = GetUniverse().LeastJumpsPath(source_fleet_prev_system_id, target_fleet_prev_system_id);
                std::pair<std::list<System*>, int> path2 = GetUniverse().LeastJumpsPath(source_fleet_prev_system_id, target_fleet_next_system_id);
                std::pair<std::list<System*>, int> path3 = GetUniverse().LeastJumpsPath(source_fleet_next_system_id, target_fleet_prev_system_id);
                std::pair<std::list<System*>, int> path4 = GetUniverse().LeastJumpsPath(source_fleet_next_system_id, target_fleet_next_system_id);
                if (int jumps = static_cast<int>(std::max(std::max(path1.second, path2.second),
                                                          std::max(path3.second, path4.second))))
                    return jumps - 1 <= jump_limit;
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////
// EffectTarget                                          //
///////////////////////////////////////////////////////////
Condition::EffectTarget::EffectTarget()
{}

std::string Condition::EffectTarget::Description(bool negated/* = false*/) const
{
    std::string retval;
    // TODO
    return retval;
}

std::string Condition::EffectTarget::Dump() const
{
    return "";
}

bool Condition::EffectTarget::Match(const UniverseObject* source, const UniverseObject* target) const
{
    return false/*TODO*/;
}

///////////////////////////////////////////////////////////
// And                                                   //
///////////////////////////////////////////////////////////
Condition::And::And(const std::vector<const ConditionBase*>& operands) :
    m_operands(operands)
{
    assert(!m_operands.empty());
}

Condition::And::~And()
{
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        delete m_operands[i];
    }
}

void Condition::And::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain/* = NON_TARGETS*/) const
{
    // if search domain is non targets, evalucate first operand condition on non targets, to assemble an
    // initial targets set.  if search domain is targets, evaluate first (and all other) operand condition
    // on existing targets set
    if (search_domain == NON_TARGETS)
        m_operands[0]->Eval(source, targets, non_targets);
    else
        m_operands[0]->Eval(source, targets, non_targets, TARGETS);

    // regardless of whether search domain is TARGET or NON_TARGETS, evaluate remaining operand conditions
    // on targets set, removing any targets that don't meet additional conditions, and stopping early if all
    // targets are removed
    for (unsigned int i = 1; i < m_operands.size(); ++i) {
        if (targets.empty()) break;
        m_operands[i]->Eval(source, targets, non_targets, TARGETS);
    }
}

std::string Condition::And::Description(bool negated/* = false*/) const
{
    if (m_operands.size() == 1) {
        return m_operands[0]->Description();
    } else {
        // TODO: use per-operand-type connecting language
        std::string values_str;
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            values_str += m_operands[i]->Description();
            if (i != m_operands.size() - 1) {
                values_str += UserString("DESC_AND_BETWEEN_OPERANDS");
            }
        }
        return values_str;
    }
}

std::string Condition::And::Dump() const
{
    std::string retval = DumpIndent() + "And [\n";
    ++g_indent;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        retval += m_operands[i]->Dump();
    }
    --g_indent;
    retval += DumpIndent() + "]\n";
    return retval;
}

///////////////////////////////////////////////////////////
// Or                                                    //
///////////////////////////////////////////////////////////
Condition::Or::Or(const std::vector<const ConditionBase*>& operands) :
    m_operands(operands)
{
    assert(!m_operands.empty());
}

Condition::Or::~Or()
{
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        delete m_operands[i];
    }
}

void Condition::Or::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain/* = NON_TARGETS*/) const
{
    if (search_domain == NON_TARGETS) {
        // if search domain is non targets, evalucate with search domain non_targets on each operand condition, using
        // the remaining non_targets from the previous operand condition as the non_targets for the next operand condition
        
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            if (non_targets.empty()) break;
            m_operands[i]->Eval(source, targets, non_targets);
        }
    }
    else
    {
        // if search domain is targets, create a temporary empty new_targets set, and use the targets set as the
        // effective non_targets set while evaluating each condition on the effective non_targets set.  this way,
        // if a target set object matches any conditions, it will be added to the new_targets set.  after evaluating
        // all conditions on the effective non_targets set, add the remaining objects to the real non_targets set,
        // and set the real targets set equal to the new_targets set
        
        ObjectSet new_targets;  // new empty targets set
        ObjectSet& temp_non_targets = targets;
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            if (temp_non_targets.empty()) break;
            m_operands[i]->Eval(source, new_targets, temp_non_targets);
        }
        non_targets.insert(temp_non_targets.begin(), temp_non_targets.end()); // move targets set object that didn't match any conditions to non_targets set
        targets = new_targets;  // set targets set equal to set of objects that matched at least one condition
    }
}

std::string Condition::Or::Description(bool negated/* = false*/) const
{
    if (m_operands.size() == 1) {
        return m_operands[0]->Description();
    } else {
        // TODO: use per-operand-type connecting language
        std::string values_str;
        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            values_str += m_operands[i]->Description();
            if (i != m_operands.size() - 1) {
                values_str += UserString("DESC_OR_BETWEEN_OPERANDS");
            }
        }
        return values_str;
    }
}

std::string Condition::Or::Dump() const
{
    std::string retval = DumpIndent() + "Or [\n";
    ++g_indent;
    for (unsigned int i = 0; i < m_operands.size(); ++i) {
        retval += m_operands[i]->Dump();
    }
    --g_indent;
    retval += DumpIndent() + "]\n";
    return retval;
}

///////////////////////////////////////////////////////////
// Not                                                   //
///////////////////////////////////////////////////////////
Condition::Not::Not(const ConditionBase* operand) :
    m_operand(operand)
{
    assert(m_operand);
}

Condition::Not::~Not()
{
    delete m_operand;
}

void Condition::Not::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain/* = NON_TARGETS*/) const
{
    m_operand->Eval(source, non_targets, targets, search_domain == TARGETS ? NON_TARGETS : TARGETS);
}

std::string Condition::Not::Description(bool negated/* = false*/) const
{
    return m_operand->Description(true);
}

std::string Condition::Not::Dump() const
{
    std::string retval = DumpIndent() + "Not\n";
    ++g_indent;
    retval += m_operand->Dump();
    --g_indent;
    return retval;
}
