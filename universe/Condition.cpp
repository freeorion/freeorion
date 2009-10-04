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

using boost::io::str;
using boost::lexical_cast;

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

    std::string GetTypeName(const UniverseObject* obj) {
        if (universe_object_cast<const Fleet*>(obj))
            return "Fleet";
        if (universe_object_cast<const Ship*>(obj))
            return "Ship";
        if (universe_object_cast<const Planet*>(obj))
            return "Planet";
        if (universe_object_cast<const System*>(obj))
            return "System";
        if (universe_object_cast<const Building*>(obj))
            return "Building";
        return "UniverseObject";
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
    return str(FlexibleFormat(UserString(description_str))
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

void Condition::Number::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain/* = NON_TARGETS*/) const
{
    // get set of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator uit = universe.begin(); uit != universe.end(); ++uit) {
        condition_non_targets.insert(uit->second);
    }
    m_condition->Eval(source, condition_targets, condition_non_targets, NON_TARGETS);

    // compare number of objects that satisfy m_condition to the acceptable range of such objects
    int matched = condition_targets.size();
    int low = m_low->Eval(source, source);
    int high = m_high->Eval(source, source);
    bool in_range = (low <= matched && matched < high);

    // transfer objects to or from target set, according to whether number of matches was within
    // the requested range.
    if (search_domain == TARGETS && !in_range) {
        non_targets.insert(targets.begin(), targets.end());
        targets.clear();
    }
    if (search_domain == NON_TARGETS && in_range) {
        targets.insert(non_targets.begin(), non_targets.end());
        non_targets.clear();
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
    return str(FlexibleFormat(UserString(description_str))
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
namespace {
    /** Random number genrator function to use with random_shuffle */
    int CustomRandInt(int max_plus_one) {
        return RandSmallInt(0, max_plus_one - 1);
    }
    int (*CRI)(int) = CustomRandInt;


    /** Transfers the indicated \a number of objects from from_set to to_set */
    void TransferObjects(unsigned int number, Condition::ObjectSet& from_set, Condition::ObjectSet& to_set) {
        // ensure number of objects to be moved is within reasonable range
        number = std::min<unsigned int>(number, from_set.size());
        if (number == 0)
            return;

        // create list of bool flags to indicate whether each item in from_set
        // with corresponding place in iteration order should be transfered
        std::vector<bool> transfer_flags(from_set.size(), false);   // initialized to all false

        // set first  number  flags to true
        std::fill_n(transfer_flags.begin(), number, true);

        // shuffle flags to randomize which flags are set
        std::random_shuffle(transfer_flags.begin(), transfer_flags.end(), CRI);

        // transfer objects that have been flagged
        int i = 0;
        for (Condition::ObjectSet::iterator it = from_set.begin() ; it != from_set.end(); ++i) {
            Condition::ObjectSet::iterator temp = it++;
            if (transfer_flags[i]) {
                to_set.insert(*temp);
                from_set.erase(temp);
            }
        }
    }
}

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
    return str(FlexibleFormat(UserString(description_str))
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
    int number = m_number->Eval(source, source);

    if (search_domain == NON_TARGETS) {
        // find items in non_targets set that match the test condition
        ObjectSet matched_non_targets;
        m_condition->Eval(source, matched_non_targets, non_targets, NON_TARGETS);

        // transfer the indicated number of matched_non_targets to targets
        TransferObjects(number, matched_non_targets, targets);

        // move remainder of matched_non_targets back to non_targets
        non_targets.insert(matched_non_targets.begin(), matched_non_targets.end());
        matched_non_targets.clear();

    } else {
        // find items in targets set that don't match test condition, and move
        // directly to non_targets
        m_condition->Eval(source, targets, non_targets, TARGETS);

        // move all matched targets to matched_targets set
        ObjectSet matched_targets = targets;
        targets.clear();

        // transfer desired number of matched_targets back to targets set
        TransferObjects(number, matched_targets, targets);

        // move remainder to non_targets set
        non_targets.insert(matched_targets.begin(), matched_targets.end());
        matched_targets.clear();
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
        // move all objects from non_targets to targets
        targets.insert(non_targets.begin(), non_targets.end());
        non_targets.clear();
    }
    // if search_comain is TARGETS, do nothing: all objects in target set
    // match this condition, so should remain in target set
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
        return str(FlexibleFormat(UserString(description_str)) % value_str);
    } else {
        std::string description_str = m_exclusive ? "DESC_EMPIRE_AFFILIATION_EXCLUSIVE" : "DESC_EMPIRE_AFFILIATION";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str))
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
    int empire_id = m_empire_id->Eval(source, target);

    switch (m_affiliation) {
    case AFFIL_SELF:
        if (m_exclusive) {
            // target object owned only by specified empire
            return target->WhollyOwnedBy(empire_id);
        } else {
            // target object owned by specified empire, and possibly others
            return target->OwnedBy(empire_id);
        }
        break;
    case AFFIL_ENEMY:
        if (m_exclusive) {
            // target has an owner, but isn't owned by specified empire
            return (!target->Owners().empty() && !target->OwnedBy(empire_id));
        } else {
            // at least one of target's owners is not specified empire, but specified empire may also own target
            return (target->Owners().size() > 1 ||
                    (!target->Owners().empty() && !target->OwnedBy(empire_id))
                   );
        }
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
// Homeworld                                             //
///////////////////////////////////////////////////////////
Condition::Homeworld::Homeworld()
{}

std::string Condition::Homeworld::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_HOMEWORLD";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Homeworld::Dump() const
{
    return DumpIndent() + "Homeworld\n";
}

bool Condition::Homeworld::Match(const UniverseObject* source, const UniverseObject* target) const
{
    // check if any empire's homeworld's ID is that target object's id.  if it is, the target object is a homeworld.
    int target_id = target->ID();
    const EmpireManager& empires = Empires();
    for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it)
        if (it->second->HomeworldID() == target_id)
            return true;
    return false;
}

///////////////////////////////////////////////////////////
// Capitol                                               //
///////////////////////////////////////////////////////////
Condition::Capitol::Capitol()
{}

std::string Condition::Capitol::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_CAPITOL";
    if (negated)
        description_str += "_NOT";
    return UserString(description_str);
}

std::string Condition::Capitol::Dump() const
{
    return DumpIndent() + "Capitol\n";
}

bool Condition::Capitol::Match(const UniverseObject* source, const UniverseObject* target) const
{
    // check if any empire's capitol's ID is that target object's id.  if it is, the target object is a capitol.
    int target_id = target->ID();
    const EmpireManager& empires = Empires();
    for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it)
        if (it->second->CapitolID() == target_id)
            return true;
    return false;
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
    return str(FlexibleFormat(UserString(description_str)) % value_str);
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
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
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
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
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
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
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
    return str(FlexibleFormat(UserString(description_str)) % m_condition->Description());
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
    return str(FlexibleFormat(UserString(description_str)) % values_str);
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
    return str(FlexibleFormat(UserString(description_str)) % values_str);
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
    return str(FlexibleFormat(UserString(description_str)) % values_str);
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
    return str(FlexibleFormat(UserString(description_str)) % values_str);
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
    return str(FlexibleFormat(UserString(description_str)) % values_str);
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
            if (m_types[i]->Eval(source, target) == system->GetStarType())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// DesignHasHull                                              //
///////////////////////////////////////////////////////////
Condition::DesignHasHull::DesignHasHull(const std::string& name) :
    m_name(name)
{}

std::string Condition::DesignHasHull::Description(bool negated/* = false*/) const
{
    std::string description_str = "DESC_DESIGN_HAS_HULL";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
}

std::string Condition::DesignHasHull::Dump() const
{
    return DumpIndent() + "DesignHasHull name = \"" + m_name + "\"\n";
}

bool Condition::DesignHasHull::Match(const UniverseObject* source, const UniverseObject* target) const
{
    if (const Ship* ship = universe_object_cast<const Ship*>(target))
        if (const ShipDesign* design = ship->Design())
            return (design->Hull() == m_name);
    return false;
}

///////////////////////////////////////////////////////////
// DesignHasPart                                              //
///////////////////////////////////////////////////////////
Condition::DesignHasPart::DesignHasPart(const ValueRef::ValueRefBase<int>* low, const ValueRef::ValueRefBase<int>* high, const std::string& name) :
    m_low(low),
    m_high(high),
    m_name(name)
{
}

Condition::DesignHasPart::~DesignHasPart()
{
    delete m_low;
    delete m_high;
}

std::string Condition::DesignHasPart::Description(bool negated/* = false*/) const
{
    std::string low_str = ValueRef::ConstantExpr(m_low) ? lexical_cast<std::string>(m_low->Eval(0, 0)) : m_low->Description();
    std::string high_str = ValueRef::ConstantExpr(m_high) ? lexical_cast<std::string>(m_high->Eval(0, 0)) : m_high->Description();
    std::string description_str = "DESC_DESIGN_HAS_PART";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
               % low_str
               % high_str
               % m_name);
}

std::string Condition::DesignHasPart::Dump() const{
    return DumpIndent() + "DesignHasPart low = " + m_low->Dump() + "Number high = " + m_high->Dump() + " name = " + m_name;
}

bool Condition::DesignHasPart::Match(const UniverseObject* source, const UniverseObject* target) const
{
    if (const Ship* ship = universe_object_cast<const Ship*>(target)) {
        if (const ShipDesign* design = ship->Design()) {
            const std::vector<std::string>& parts = design->Parts();
            int count = 0;
            for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it)
                if (*it == m_name)
                    ++count;
            int low = m_low->Eval(source, target);      // number matched can depend on some property of target object!
            int high = m_high->Eval(source, target);
            return (low <= count && count < high);
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
        return str(FlexibleFormat(UserString(description_str)) % lexical_cast<std::string>(std::max(0.0, std::min(m_chance->Eval(0, 0), 1.0)) * 100));
    } else {
        std::string description_str = "DESC_CHANCE";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % m_chance->Description());
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
    return str(FlexibleFormat(UserString(description_str))
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
    return str(FlexibleFormat(UserString(description_str))
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
    return str(FlexibleFormat(UserString(description_str)) % UserString(m_name));
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
        return str(FlexibleFormat(UserString(description_str)) % value_str);
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
        return str(FlexibleFormat(UserString(description_str)) % values_str);
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
        if (target->GetVisibility(m_empire_ids[i]->Eval(source, target)) != VIS_NO_VISIBILITY)
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
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it)
        condition_non_targets.insert(it->second);
    m_condition->Eval(source, condition_targets, condition_non_targets);


    //std::cout << "WithinDistance::Eval: objects meeting operand condition: " << m_condition->Dump() << std::endl;
    //for (ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it)
    //    std::cout << "... " << GetTypeName(*it) << " " << (*it)->Name() << std::endl;
    //std::cout << std::endl;


    if (search_domain == NON_TARGETS) {
        // to be transferred to targets, object initially in non_targets
        // needs to be within required distance of at least one
        // condition_target

        // check each non_target
        for (ObjectSet::iterator non_targets_it = non_targets.begin() ; non_targets_it != non_targets.end();) {

            // get current object while incrementing main iterator
            ObjectSet::iterator current_non_target_it = non_targets_it++;
            UniverseObject* non_target_obj = *current_non_target_it;

            // see if current object is within required distance of any condition target
            for (ObjectSet::iterator condition_targets_it = condition_targets.begin(); condition_targets_it != condition_targets.end(); ++condition_targets_it) {
                if (Match(non_target_obj, *condition_targets_it)) {
                    // current object is within required distance of current condition target.
                    // transfer current object to targets set
                    targets.insert(non_target_obj);
                    non_targets.erase(current_non_target_it);
                    break;
                }
            }
        }

    } else {
        // to be transferred to non_targets, object initially in targets needs
        // to be not within required distance of all/any condition targets

        // transfer targets into temp storage
        ObjectSet initial_targets = targets;
        targets.clear();

        // check initial targets
        for (ObjectSet::iterator initial_targets_it = initial_targets.begin() ; initial_targets_it != initial_targets.end();) {

            // get current object while incrementing main iterator
            ObjectSet::iterator current_initial_target_it = initial_targets_it++;
            UniverseObject* initial_target_obj = *current_initial_target_it;

            // see if current object is within required distance of any condition target
            for (ObjectSet::iterator condition_targets_it = condition_targets.begin(); condition_targets_it != condition_targets.end(); ++condition_targets_it) {
                if (Match(initial_target_obj, *condition_targets_it)) {
                    // current object is within required distance of current condition target.
                    // transfer current object to back to targets set
                    targets.insert(initial_target_obj);
                    initial_targets.erase(current_initial_target_it);
                    break;
                }
            }
        }

        // move any initial_targets that weren't in range of any condition
        // target into non_targets
        non_targets.insert(initial_targets.begin(), initial_targets.end());
        initial_targets.clear();
    }
}

std::string Condition::WithinDistance::Description(bool negated/* = false*/) const
{
    std::string value_str = ValueRef::ConstantExpr(m_distance) ? lexical_cast<std::string>(m_distance->Eval(0, 0)) : m_distance->Description();
    std::string description_str = "DESC_WITHIN_DISTANCE";
    if (negated)
        description_str += "_NOT";
    return str(FlexibleFormat(UserString(description_str))
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
    return str(FlexibleFormat(UserString(description_str))
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
            std::pair<std::list<int>, double> path = GetUniverse().ShortestPath(source_system->ID(), target_system->ID());
            if (!path.first.empty()) { // if path.first is empty, no path exists between the systems
                return (static_cast<int>(path.first.size()) - 1) <= jump_limit;
            }
        } else if (source_system) {
            if (const Fleet* target_fleet = FleetFromObject(target)) {
                std::pair<std::list<int>, double> path1 = GetUniverse().ShortestPath(source_system->ID(), target_fleet->PreviousSystemID());
                std::pair<std::list<int>, double> path2 = GetUniverse().ShortestPath(source_system->ID(), target_fleet->NextSystemID());
                if (int jumps = static_cast<int>(std::max(path1.first.size(), path2.first.size())) - 1)
                    return jumps <= jump_limit;
            }
        } else if (target_system) {
           if (const Fleet* source_fleet = FleetFromObject(source)) {
                std::pair<std::list<int>, double> path1 = GetUniverse().ShortestPath(source_fleet->PreviousSystemID(), target_system->ID());
                std::pair<std::list<int>, double> path2 = GetUniverse().ShortestPath(source_fleet->NextSystemID(), target_system->ID());
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
                std::pair<std::list<int>, int> path1 = GetUniverse().LeastJumpsPath(source_fleet_prev_system_id, target_fleet_prev_system_id);
                std::pair<std::list<int>, int> path2 = GetUniverse().LeastJumpsPath(source_fleet_prev_system_id, target_fleet_next_system_id);
                std::pair<std::list<int>, int> path3 = GetUniverse().LeastJumpsPath(source_fleet_next_system_id, target_fleet_prev_system_id);
                std::pair<std::list<int>, int> path4 = GetUniverse().LeastJumpsPath(source_fleet_next_system_id, target_fleet_next_system_id);
                if (int jumps = static_cast<int>(std::max(std::max(path1.second, path2.second),
                                                          std::max(path3.second, path4.second))))
                    return jumps - 1 <= jump_limit;
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////
// ExploredByEmpire                                      //
///////////////////////////////////////////////////////////
Condition::ExploredByEmpire::ExploredByEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids) :
    m_empire_ids(empire_ids)
{}

Condition::ExploredByEmpire::~ExploredByEmpire()
{
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        delete m_empire_ids[i];
    }
}

std::string Condition::ExploredByEmpire::Description(bool negated/* = false*/) const
{
    if (m_empire_ids.size() == 1) {
        std::string value_str = ValueRef::ConstantExpr(m_empire_ids[0]) ? Empires().Lookup(m_empire_ids[0]->Eval(0, 0))->Name() : m_empire_ids[0]->Description();
        std::string description_str = "DESC_EXPLORED_BY_SINGLE_EMPIRE";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % value_str);
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
        std::string description_str = "DESC_EXPLORED_BY_EMPIRES";
        if (negated)
            description_str += "_NOT";
        return str(FlexibleFormat(UserString(description_str)) % values_str);
    }
}

std::string Condition::ExploredByEmpire::Dump() const
{
    std::string retval = DumpIndent() + "ExploredByEmpire empire = ";
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

bool Condition::ExploredByEmpire::Match(const UniverseObject* source, const UniverseObject* target) const
{
    const EmpireManager& empires = Empires();
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        const Empire* empire = empires.Lookup(m_empire_ids[i]->Eval(source, target));
        if (empire->HasExploredSystem(target->ID()))
            return true;
    }
    return false;
}

///////////////////////////////////////////////////////////
// Stationary                                      //
///////////////////////////////////////////////////////////
Condition::Stationary::Stationary()
{}

std::string Condition::Stationary::Description(bool negated/* = false*/) const
{
    return UserString("DESC_STATIONARY");
}

std::string Condition::Stationary::Dump() const
{
    return DumpIndent() + "Stationary\n";
}

bool Condition::Stationary::Match(const UniverseObject* source, const UniverseObject* target) const
{
    // the only objects that can move are fleets and the ships in them.  so,
    // attempt to cast the target object to a fleet or ship, and if it's a ship
    // get the fleet of that ship
    const Fleet* fleet = universe_object_cast<const Fleet*>(target);
    if (!fleet)
        if (const Ship* ship = universe_object_cast<const Ship*>(target))
            fleet = ship->GetFleet();

    if (fleet) {
        // if a fleet is available, it is "moving", or not stationary, if it's
        // next system is a system and isn't the current system.  This will
        // mean fleets that have arrived at a system on the current turn will
        // be stationary, but fleets departing won't be stationary.
        int next_id = fleet->NextSystemID();
        int cur_id = fleet->SystemID();
        if (next_id != UniverseObject::INVALID_OBJECT_ID && next_id != cur_id)
            return false;

    }

    return true;
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
        //std::cout << "And::Eval: input targets:" << std::endl;
        //for (ObjectSet::const_iterator it = targets.begin(); it != targets.end(); ++it)
        //    std::cout << "... " << GetTypeName(*it) << " " << (*it)->Name() << std::endl;
        //std::cout << std::endl;

        //std::cout << "And::Eval: input non_targets:" << std::endl;
        //for (ObjectSet::const_iterator it = non_targets.begin(); it != non_targets.end(); ++it)
        //    std::cout << "... " << GetTypeName(*it) << " " << (*it)->Name() << std::endl;
        //std::cout << std::endl;

    if (search_domain == NON_TARGETS) {
        ObjectSet partly_checked_non_targets;

        // move items in non_targets set that pass first operand condition into
        // partly_checked_non_targets set
        m_operands[0]->Eval(source, partly_checked_non_targets, non_targets, NON_TARGETS);

        //std::cout << "And::Eval: non_target input objects meeting first condition: " << m_operands[0]->Dump() << std::endl;
        //for (ObjectSet::const_iterator it = partly_checked_non_targets.begin(); it != partly_checked_non_targets.end(); ++it)
        //    std::cout << "... " << GetTypeName(*it) << " " << (*it)->Name() << std::endl;
        //std::cout << std::endl;

        // move items that don't pass one of the other conditions back to non_targets
        for (unsigned int i = 1; i < m_operands.size(); ++i) {
            if (partly_checked_non_targets.empty()) break;
            m_operands[i]->Eval(source, partly_checked_non_targets, non_targets, TARGETS);

            //std::cout << "And::Eval: non_target input objects also meeting " << i + 1 <<"th condition: " << m_operands[i]->Dump() << std::endl;
            //for (ObjectSet::const_iterator it = partly_checked_non_targets.begin(); it != partly_checked_non_targets.end(); ++it)
            //    std::cout << "... " << GetTypeName(*it) << " " << (*it)->Name() << std::endl;
            //std::cout << std::endl;
        }

        // merge items that passed all operand conditions into targets
        targets.insert(partly_checked_non_targets.begin(), partly_checked_non_targets.end());
        partly_checked_non_targets.clear();

        // items already in targets set are not checked, and remain in targets set even if
        // they don't match one of the operand conditions

    } else {
        // check all operand conditions on all objects in the targets set, moving those
        // that don't pass a condition to the non-targets set

        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            if (targets.empty()) break;
            m_operands[i]->Eval(source, targets, non_targets, TARGETS);
        }

        // items already in non_targets set are not checked, and remain in non_targets set
        // even if they pass all operand conditions
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
        // check each item in the non-targets set against each of the operand conditions
        // if a non-target item matches an operand condition, move the item to the
        // targets set.

        for (unsigned int i = 0; i < m_operands.size(); ++i) {
            if (non_targets.empty()) break;
            m_operands[i]->Eval(source, targets, non_targets, NON_TARGETS);
        }

        // items already in targets set are not checked and remain in the
        // targets set even if they fail all the operand conditions

    } else {
        ObjectSet partly_checked_targets;

        // move items in targets set the fail the first operand condition into 
        // partly_checked_targets set
        m_operands[0]->Eval(source, targets, partly_checked_targets, TARGETS);

        // move items that pass any of the other conditions back into targets
        for (unsigned int i = 1; i < m_operands.size(); ++i) {
            if (partly_checked_targets.empty()) break;
            m_operands[i]->Eval(source, targets, partly_checked_targets, NON_TARGETS);
        }

        // merge items that failed all operand conditions into non_targets
        non_targets.insert(partly_checked_targets.begin(), partly_checked_targets.end());
        partly_checked_targets.clear();

        // items already in non_targets set are not checked and remain in
        // non_targets set even if they pass one or more of the operand 
        // conditions
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
    if (search_domain == NON_TARGETS) {
        // search non_targets set for items that don't meet the operand
        // condition, and move those to the targets set
        m_operand->Eval(source, non_targets, targets, TARGETS); // swapping order of targets and non_targets set parameters and TARGETS / NON_TARGETS search domain effects NOT on requested search domain
    } else {
        // search targets set for items that meet the operand condition
        // condition, and move those to the non_targets set
        m_operand->Eval(source, non_targets, targets, NON_TARGETS);
    }
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
