#include "Condition.h"

#include "../util/AppInterface.h"
#include "Building.h"
#include "Fleet.h"
#include "../util/Parse.h"
#include "../util/Random.h"
#include "System.h"

namespace {
    Condition::ConditionBase* NewAll(const GG::XMLElement& elem)                    {return new Condition::All(elem);}
    Condition::ConditionBase* NewEmpireAffiliation(const GG::XMLElement& elem)      {return new Condition::EmpireAffiliation(elem);}
    Condition::ConditionBase* NewSelf(const GG::XMLElement& elem)                   {return new Condition::Self(elem);}
    Condition::ConditionBase* NewType(const GG::XMLElement& elem)                   {return new Condition::Type(elem);}
    Condition::ConditionBase* NewBuilding(const GG::XMLElement& elem)               {return new Condition::Building(elem);}
    Condition::ConditionBase* NewHasSpecial(const GG::XMLElement& elem)             {return new Condition::HasSpecial(elem);}
    Condition::ConditionBase* NewContains(const GG::XMLElement& elem)               {return new Condition::Contains(elem);}
    Condition::ConditionBase* NewPlanetSize(const GG::XMLElement& elem)             {return new Condition::PlanetSize(elem);}
    Condition::ConditionBase* NewPlanetType(const GG::XMLElement& elem)             {return new Condition::PlanetType(elem);}
    Condition::ConditionBase* NewPlanetEnvironment(const GG::XMLElement& elem)      {return new Condition::PlanetEnvironment(elem);}
    Condition::ConditionBase* NewFocusType(const GG::XMLElement& elem)              {return new Condition::FocusType(elem);}
    Condition::ConditionBase* NewStarType(const GG::XMLElement& elem)               {return new Condition::StarType(elem);}
    Condition::ConditionBase* NewChance(const GG::XMLElement& elem)                 {return new Condition::Chance(elem);}
    Condition::ConditionBase* NewMeterValue(const GG::XMLElement& elem)             {return new Condition::MeterValue(elem);}
    //Condition::ConditionBase* NewStockpileValue(const GG::XMLElement& elem)         {return new Condition::StockpileValue(elem);}
    Condition::ConditionBase* NewVisibleToEmpire(const GG::XMLElement& elem)        {return new Condition::VisibleToEmpire(elem);}
    Condition::ConditionBase* NewWithinDistance(const GG::XMLElement& elem)         {return new Condition::WithinDistance(elem);}
    Condition::ConditionBase* NewWithinStarlaneJumps(const GG::XMLElement& elem)    {return new Condition::WithinStarlaneJumps(elem);}
    Condition::ConditionBase* NewEffectTarget(const GG::XMLElement& elem)           {return new Condition::EffectTarget(elem);}
    Condition::ConditionBase* NewAnd(const GG::XMLElement& elem)                    {return new Condition::And(elem);}
    Condition::ConditionBase* NewOr(const GG::XMLElement& elem)                     {return new Condition::Or(elem);}
    Condition::ConditionBase* NewNot(const GG::XMLElement& elem)                    {return new Condition::Not(elem);}

    bool temp_header_bool = RecordHeaderFile(ConditionRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

GG::XMLObjectFactory<Condition::ConditionBase> Condition::ConditionFactory()
{
    static GG::XMLObjectFactory<Condition::ConditionBase> factory;
    static bool init = false;
    if (!init) {
        factory.AddGenerator("Condition::All", &NewAll);
        factory.AddGenerator("Condition::EmpireAffiliation", &NewEmpireAffiliation);
        factory.AddGenerator("Condition::Self", &NewSelf);
        factory.AddGenerator("Condition::Type", &NewType);
        factory.AddGenerator("Condition::Building", &NewBuilding);
        factory.AddGenerator("Condition::HasSpecial", &NewHasSpecial);
        factory.AddGenerator("Condition::Contains", &NewContains);
        factory.AddGenerator("Condition::PlanetSize", &NewPlanetSize);
        factory.AddGenerator("Condition::PlanetType", &NewPlanetType);
        factory.AddGenerator("Condition::PlanetEnvironment", &NewPlanetEnvironment);
        factory.AddGenerator("Condition::FocusType", &NewFocusType);
        factory.AddGenerator("Condition::StarType", &NewStarType);
        factory.AddGenerator("Condition::Chance", &NewChance);
        factory.AddGenerator("Condition::MeterValue", &NewMeterValue);
        //factory.AddGenerator("Condition::StockpileValue", &NewStockpileValue);
        factory.AddGenerator("Condition::VisibleToEmpire", &NewVisibleToEmpire);
        factory.AddGenerator("Condition::WithinDistance", &NewWithinDistance);
        factory.AddGenerator("Condition::WithinStarlaneJumps", &NewWithinStarlaneJumps);
        factory.AddGenerator("Condition::EffectTarget", &NewEffectTarget);
        factory.AddGenerator("Condition::And", &NewAnd);
        factory.AddGenerator("Condition::Or", &NewOr);
        factory.AddGenerator("Condition::Not", &NewNot);
        init = true;
    }
    return factory;
}

///////////////////////////////////////////////////////////
// ConditionBase                                         //
///////////////////////////////////////////////////////////
Condition::ConditionBase::ConditionBase()
{
}

Condition::ConditionBase::~ConditionBase()
{
}

void Condition::ConditionBase::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain/* = NON_TARGETS*/) const
{
    ObjectSet& from_set = search_domain == TARGETS ? targets : non_targets;
    ObjectSet& to_set = search_domain == TARGETS ? non_targets : targets;
    ObjectSet::iterator it = from_set.begin();
    ObjectSet::iterator end_it = from_set.end();
    for ( ; it != end_it; ) {
        if (search_domain == TARGETS ? !Match(source, *it) : Match(source, *it)) {
            to_set.insert(*it);
            ObjectSet::iterator temp = it++;
            from_set.erase(temp);
        } else {
            ++it;
        }
    }
}

bool Condition::ConditionBase::Match(const UniverseObject* source, const UniverseObject* target) const
{
    return false;
}

///////////////////////////////////////////////////////////
// All                                                   //
///////////////////////////////////////////////////////////
Condition::All::All()
{
}

Condition::All::All(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::All")
        throw std::runtime_error("Condition::All : Attempted to create a All condition from an XML element with a tag other than \"Condition::All\".");
}

void Condition::All::Eval(const UniverseObject* source, ObjectSet& targets, ObjectSet& non_targets, SearchDomain search_domain/* = NON_TARGETS*/) const
{
    if (search_domain == NON_TARGETS) {
        targets.insert(non_targets.begin(), non_targets.end());
        non_targets.clear();
    }
}

///////////////////////////////////////////////////////////
// EmpireAffiliation                                     //
///////////////////////////////////////////////////////////
Condition::EmpireAffiliation::EmpireAffiliation(const ValueRef::ValueRefBase<int>* empire_id, EmpireAffiliationType affiliation, bool exclusive) :
    m_empire_id(empire_id),
    m_affiliation(affiliation),
    m_exclusive(exclusive)
{
}

Condition::EmpireAffiliation::EmpireAffiliation(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::EmpireAffiliation")
        throw std::runtime_error("Condition::EmpireAffiliation : Attempted to create a EmpireAffiliation condition from an XML element with a tag other than \"Condition::EmpireAffiliation\".");

    m_empire_id = ParseArithmeticExpression<int>(elem.Child("empire_id").Text());
    m_affiliation = boost::lexical_cast<EmpireAffiliationType>(elem.Child("affiliation").Text());
    m_exclusive = boost::lexical_cast<bool>(elem.Child("exclusive").Text());
}

Condition::EmpireAffiliation::~EmpireAffiliation()
{
    delete m_empire_id;
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
{
}

Condition::Self::Self(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::Self")
        throw std::runtime_error("Condition::Self : Attempted to create a Self condition from an XML element with a tag other than \"Condition::Self\".");
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
{
}

Condition::Type::Type(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::Type")
        throw std::runtime_error("Condition::Type : Attempted to create aType condition from an XML element with a tag other than \"Condition::Type\".");

    m_type = ParseArithmeticExpression<UniverseObjectType>(elem.Text());
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
        return dynamic_cast<const ProdCenter*>(target);
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
{
}

Condition::Building::Building(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::Building")
        throw std::runtime_error("Condition::Building : Attempted to create a Building condition from an XML element with a tag other than \"Condition::Building\".");

    m_name = elem.Text();
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
{
}

Condition::HasSpecial::HasSpecial(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::HasSpecial")
        throw std::runtime_error("Condition::HasSpecial : Attempted to create a HasSpecial condition from an XML element with a tag other than \"Condition::HasSpecial\".");

    m_name = elem.Text();
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
{
}

Condition::Contains::Contains(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::Contains")
        throw std::runtime_error("Condition::Contains : Attempted to create a Contains condition from an XML element with a tag other than \"Condition::Contains\".");

    m_condition = ConditionFactory().GenerateObject(elem.Child(0));
}

bool Condition::Contains::Match(const UniverseObject* source, const UniverseObject* target) const
{
    // get the list of all UniverseObjects that satisfy m_condition
    ObjectSet condition_targets;
    ObjectSet condition_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        condition_non_targets.insert(it->second);
    }
    m_condition->Eval(source, condition_targets, condition_non_targets);

    if (condition_targets.empty())
        return false;

    if (const System* system = universe_object_cast<const System*>(target)) {
        bool found = false;
        for (ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it) {
            if ((*it)->SystemID() == system->ID()) {
                found = true;
                break;
            }
        }
        return found;
    } else if (const Planet* planet = universe_object_cast<const Planet*>(target)) {
        bool found = false;
        for (ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it) {
            if (planet->ContainsBuilding((*it)->ID())) {
                found = true;
                break;
            }
        }
        return found;
    } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(target)) {
        bool found = false;
        for (ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it) {
            if (fleet->ContainsShip((*it)->ID())) {
                found = true;
                break;
            }
        }
        return found;
    } else {
        return false;
    }
}

///////////////////////////////////////////////////////////
// PlanetType                                            //
///////////////////////////////////////////////////////////
Condition::PlanetType::PlanetType(const std::vector<const ValueRef::ValueRefBase< ::PlanetType>*>& types) :
    m_types(types)
{
}

Condition::PlanetType::PlanetType(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::PlanetType")
        throw std::runtime_error("Condition::PlanetType : Attempted to create a PlanetType condition from an XML element with a tag other than \"Condition::PlanetType\".");

    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        m_types.push_back(ParseArithmeticExpression< ::PlanetType>(it->Text()));
    }
}

Condition::PlanetType::~PlanetType()
{
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        delete m_types[i];
    }
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
{
}

Condition::PlanetSize::PlanetSize(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::PlanetSize")
        throw std::runtime_error("Condition::PlanetSize : Attempted to create a PlanetSize condition from an XML element with a tag other than \"Condition::PlanetSize\".");

    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        m_sizes.push_back(ParseArithmeticExpression< ::PlanetSize>(it->Text()));
    }
}

Condition::PlanetSize::~PlanetSize()
{
    for (unsigned int i = 0; i < m_sizes.size(); ++i) {
        delete m_sizes[i];
    }
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
{
}

Condition::PlanetEnvironment::PlanetEnvironment(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::PlanetEnvironment")
        throw std::runtime_error("Condition::PlanetEnvironment : Attempted to create a PlanetEnvironment condition from an XML element with a tag other than \"Condition::PlanetEnvironment\".");

    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        m_environments.push_back(ParseArithmeticExpression< ::PlanetEnvironment>(it->Text()));
    }
}

Condition::PlanetEnvironment::~PlanetEnvironment()
{
    for (unsigned int i = 0; i < m_environments.size(); ++i) {
        delete m_environments[i];
    }
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
{
}

Condition::FocusType::FocusType(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::FocusType")
        throw std::runtime_error("Condition::FocusType : Attempted to create a FocusType condition from an XML element with a tag other than \"Condition::FocusType\".");

    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        m_foci.push_back(ParseArithmeticExpression< ::FocusType>(it->Text()));
    }
    m_primary = boost::lexical_cast< ::FocusType>(elem.Child("primary").Text());
}

Condition::FocusType::~FocusType()
{
    for (unsigned int i = 0; i < m_foci.size(); ++i) {
        delete m_foci[i];
    }
}

bool Condition::FocusType::Match(const UniverseObject* source, const UniverseObject* target) const
{
    if (const ProdCenter* prod_center = dynamic_cast<const ProdCenter*>(target)) {
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
{
}

Condition::StarType::StarType(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::StarType")
        throw std::runtime_error("Condition::StarType : Attempted to create a StarType condition from an XML element with a tag other than \"Condition::StarType\".");

    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        m_types.push_back(ParseArithmeticExpression< ::StarType>(it->Text()));
    }
}

Condition::StarType::~StarType()
{
    for (unsigned int i = 0; i < m_types.size(); ++i) {
        delete m_types[i];
    }
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
{
}

Condition::Chance::Chance(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::Chance")
        throw std::runtime_error("Condition::Chance : Attempted to create aChance condition from an XML element with a tag other than \"Condition::Chance\".");

    m_chance = ParseArithmeticExpression<double>(elem.Child("chance").Text());
}

Condition::Chance::~Chance()
{
    delete m_chance;
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
{
}

Condition::MeterValue::MeterValue(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::MeterValue")
        throw std::runtime_error("Condition::MeterValue : Attempted to create a MeterValue condition from an XML element with a tag other than \"Condition::MeterValue\".");

    m_meter = boost::lexical_cast<MeterType>(elem.Child("meter").Text());
    m_low = ParseArithmeticExpression<double>(elem.Child("low").Text());
    m_high = ParseArithmeticExpression<double>(elem.Child("high").Text());
    m_max_meter = boost::lexical_cast<bool>(elem.Child("max_meter").Text());
}

Condition::MeterValue::~MeterValue()
{
    delete m_low;
    delete m_high;
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
// StockpileValue                                        //
///////////////////////////////////////////////////////////
/*Condition::StockpileValue::StockpileValue(StockpileType stockpile, const ValueRef::ValueRefBase<double>* low, const ValueRef::ValueRefBase<double>* high) :
    m_stockpile(stockpile),
    m_low(low),
    m_high(high)
{
}

Condition::StockpileValue::StockpileValue(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::StockpileValue")
        throw std::runtime_error("Condition::StockpileValue : Attempted to create a StockpileValue condition from an XML element with a tag other than \"Condition::StockpileValue\".");

    m_stockpile = boost::lexical_cast<StockpileType>(elem.Child("stockpile").Text());
    m_low = ParseArithmeticExpression<double>(elem.Child("low").Text());
    m_high = ParseArithmeticExpression<double>(elem.Child("high").Text());
}

Condition::StockpileValue::~StockpileValue()
{
    delete m_low;
    delete m_high;
}

bool Condition::StockpileValue::Match(const UniverseObject* source, const UniverseObject* target) const
{
    //TODO
    return false;
}*/

///////////////////////////////////////////////////////////
// VisibleToEmpire                                       //
///////////////////////////////////////////////////////////
Condition::VisibleToEmpire::VisibleToEmpire(const std::vector<const ValueRef::ValueRefBase<int>*>& empire_ids) :
    m_empire_ids(empire_ids)
{
}

Condition::VisibleToEmpire::VisibleToEmpire(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::TargetVisibleToEmpire")
        throw std::runtime_error("Condition::TargetVisibleToEmpire : Attempted to create a TargetVisibleToEmpire condition from an XML element with a tag other than \"Condition::TargetVisibleToEmpire\".");

    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        m_empire_ids.push_back(ParseArithmeticExpression<int>(it->Text()));
    }
}

Condition::VisibleToEmpire::~VisibleToEmpire()
{
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        delete m_empire_ids[i];
    }
}

bool Condition::VisibleToEmpire::Match(const UniverseObject* source, const UniverseObject* target) const
{
    bool retval = false;
    for (unsigned int i = 0; i < m_empire_ids.size(); ++i) {
        if (target->GetVisibility(m_empire_ids[i]->Eval(source, target)) != UniverseObject::NO_VISIBILITY)
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
{
}

Condition::WithinDistance::WithinDistance(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::WithinDistance")
        throw std::runtime_error("Condition::WithinDistance : Attempted to create a WithinDistance condition from an XML element with a tag other than \"Condition::WithinDistance\".");

    m_distance = ParseArithmeticExpression<double>(elem.Child("distance").Text());
    m_condition = ConditionFactory().GenerateObject(elem.Child("condition").Text());
}

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
            if (search_domain == TARGETS ? !Match(*it, *it2) : Match(*it, *it2)) {
                to_set.insert(*it2);
                ObjectSet::iterator temp = it2++;
                from_set.erase(temp);
            } else {
                ++it2;
            }
        }
    }
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
{
}

Condition::WithinStarlaneJumps::WithinStarlaneJumps(const GG::XMLElement& elem)
{
    if (elem.Tag() != "ConditionWithinStarlaneJumps")
        throw std::runtime_error("Condition::WithinStarlaneJumps : Attempted to create a WithinStarlaneJumps condition from an XML element with a tag other than \"Condition::WithinStarlaneJumps\".");

    m_jumps = ParseArithmeticExpression<int>(elem.Child("jumps").Text());
    m_condition = ConditionFactory().GenerateObject(elem.Child("condition").Text());
}

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
            if (search_domain == TARGETS ? !Match(*it, *it2) : Match(*it, *it2)) {
                to_set.insert(*it2);
                ObjectSet::iterator temp = it2++;
                from_set.erase(temp);
            } else {
                ++it2;
            }
        }
    }
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
            if (const Fleet* target_fleet = universe_object_cast<const Fleet*>(target)) {
                std::pair<std::list<System*>, double> path1 = GetUniverse().ShortestPath(source_system->ID(), target_fleet->PreviousSystemID());
                std::pair<std::list<System*>, double> path2 = GetUniverse().ShortestPath(source_system->ID(), target_fleet->NextSystemID());
                if (int jumps = static_cast<int>(std::max(path1.first.size(), path2.first.size())) - 1)
                    return jumps <= jump_limit;
            }
        } else if (target_system) {
            if (const Fleet* source_fleet = universe_object_cast<const Fleet*>(source)) {
                std::pair<std::list<System*>, double> path1 = GetUniverse().ShortestPath(source_fleet->PreviousSystemID(), target_system->ID());
                std::pair<std::list<System*>, double> path2 = GetUniverse().ShortestPath(source_fleet->NextSystemID(), target_system->ID());
                if (int jumps = static_cast<int>(std::max(path1.first.size(), path2.first.size())))
                    return jumps - 1 <= jump_limit;
            }
        } else {
            const Fleet* target_fleet = universe_object_cast<const Fleet*>(target);
            const Fleet* source_fleet = universe_object_cast<const Fleet*>(source);
            if (source_fleet && target_fleet) {
                int source_fleet_prev_system_id = source_fleet->PreviousSystemID();
                int source_fleet_next_system_id = source_fleet->NextSystemID();
                int target_fleet_prev_system_id = target_fleet->PreviousSystemID();
                int target_fleet_next_system_id = target_fleet->NextSystemID();
                std::pair<std::list<System*>, double> path1 = GetUniverse().ShortestPath(source_fleet_prev_system_id, target_fleet_prev_system_id);
                std::pair<std::list<System*>, double> path2 = GetUniverse().ShortestPath(source_fleet_prev_system_id, target_fleet_next_system_id);
                std::pair<std::list<System*>, double> path3 = GetUniverse().ShortestPath(source_fleet_next_system_id, target_fleet_prev_system_id);
                std::pair<std::list<System*>, double> path4 = GetUniverse().ShortestPath(source_fleet_next_system_id, target_fleet_next_system_id);
                if (int jumps = static_cast<int>(std::max(std::max(path1.first.size(), path2.first.size()),
                                                          std::max(path1.first.size(), path2.first.size()))))
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
{
}

Condition::EffectTarget::EffectTarget(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::EffectTarget")
        throw std::runtime_error("Condition::EffectTarget : Attempted to create a EffectTarget condition from an XML element with a tag other than \"Condition::EffectTarget\".");

    // TODO
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

Condition::And::And(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::And")
        throw std::runtime_error("Condition::And : Attempted to create a And condition from an XML element with a tag other than \"Condition::And\".");

    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        m_operands.push_back(ConditionFactory().GenerateObject(*it));
    }

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
    // get the list of all UniverseObjects that satisfy the m_operands AND'ed together
    ObjectSet operand_targets;
    ObjectSet operand_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        operand_non_targets.insert(it->second);
    }
    m_operands[0]->Eval(source, operand_targets, operand_non_targets);

    for (unsigned int i = 1; i < m_operands.size(); ++i) {
        m_operands[i]->Eval(source, operand_targets, operand_non_targets, TARGETS);
        if (targets.empty())
            break;
    }

    ObjectSet::iterator operand_it = search_domain == TARGETS ? operand_non_targets.begin() : operand_targets.begin();
    ObjectSet::iterator operand_end_it = search_domain == TARGETS ? operand_non_targets.end() : operand_targets.end();
    ObjectSet& from = search_domain == TARGETS ? targets : non_targets;
    ObjectSet& to = search_domain == TARGETS ? non_targets : targets;
    for (; operand_it != operand_end_it; ++operand_it) {
        if (search_domain == TARGETS) {
            from.erase(*operand_it);
        } else {
            to.insert(*operand_it);
        }
    }
}

///////////////////////////////////////////////////////////
// Or                                                    //
///////////////////////////////////////////////////////////
Condition::Or::Or(const std::vector<const ConditionBase*>& operands) :
    m_operands(operands)
{
    assert(!m_operands.empty());
}

Condition::Or::Or(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::Or")
        throw std::runtime_error("Condition::Or : Attempted to create a Or condition from an XML element with a tag other than \"Condition::Or\".");

    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        m_operands.push_back(ConditionFactory().GenerateObject(*it));
    }

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
    // get the list of all UniverseObjects that satisfy the m_operands OR'ed together
    ObjectSet operand_targets;
    ObjectSet operand_non_targets;
    const Universe& universe = GetUniverse();
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it) {
        operand_non_targets.insert(it->second);
    }
    m_operands[0]->Eval(source, operand_targets, operand_non_targets);

    for (unsigned int i = 1; i < m_operands.size(); ++i) {
        m_operands[i]->Eval(source, operand_targets, operand_non_targets, NON_TARGETS);
        if (operand_non_targets.empty())
            break;
    }

    ObjectSet::iterator operand_it = search_domain == TARGETS ? operand_non_targets.begin() : operand_targets.begin();
    ObjectSet::iterator operand_end_it = search_domain == TARGETS ? operand_non_targets.end() : operand_targets.end();
    ObjectSet& from = search_domain == TARGETS ? targets : non_targets;
    ObjectSet& to = search_domain == TARGETS ? non_targets : targets;
    for (; operand_it != operand_end_it; ++operand_it) {
        if (search_domain == TARGETS) {
            from.erase(*operand_it);
        } else {
            to.insert(*operand_it);
        }
    }
}

///////////////////////////////////////////////////////////
// Not                                                   //
///////////////////////////////////////////////////////////
Condition::Not::Not(const ConditionBase* operand) :
    m_operand(operand)
{
    assert(m_operand);
}

Condition::Not::Not(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Condition::Not")
        throw std::runtime_error("Condition::Not : Attempted to create a Not condition from an XML element with a tag other than \"Condition::Not\".");

    if (elem.NumChildren() != 1)
        throw std::runtime_error("Condition::Not : Attempted to create a Not condition with more than one or no operand conditions.");

    m_operand = ConditionFactory().GenerateObject(elem.Child(0));

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
