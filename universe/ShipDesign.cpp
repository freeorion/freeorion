#include "ShipDesign.h"

#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../parse/Parse.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "Condition.h"
#include "Effect.h"
#include "Planet.h"
#include "Ship.h"
#include "Predicates.h"
#include "Species.h"
#include "Universe.h"
#include "ValueRef.h"
#include "Enums.h"

#include <cfloat>
#include <boost/filesystem/fstream.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

extern FO_COMMON_API const int INVALID_DESIGN_ID = -1;

using boost::io::str;

namespace {
    const bool CHEAP_AND_FAST_SHIP_PRODUCTION = false;    // makes all ships cost 1 PP and take 1 turn to build
    const std::string EMPTY_STRING;

    std::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, float increase) {
        typedef std::shared_ptr<Effect::EffectsGroup> EffectsGroupPtr;
        typedef std::vector<Effect::EffectBase*> Effects;
        Condition::Source* scope = new Condition::Source;
        Condition::Source* activation = new Condition::Source;
        ValueRef::ValueRefBase<double>* vr =
            new ValueRef::Operation<double>(
                ValueRef::PLUS,
                new ValueRef::Variable<double>(ValueRef::EFFECT_TARGET_VALUE_REFERENCE, std::vector<std::string>()),
                new ValueRef::Constant<double>(increase)
            );
        return EffectsGroupPtr(
            new Effect::EffectsGroup(
                scope, activation, Effects(1, new Effect::SetMeter(meter_type, vr))));
    }

    std::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, const std::string& part_name, float increase, bool allow_stacking = true) {
        typedef std::shared_ptr<Effect::EffectsGroup> EffectsGroupPtr;
        typedef std::vector<Effect::EffectBase*> Effects;
        Condition::Source* scope = new Condition::Source;
        Condition::Source* activation = new Condition::Source;

        ValueRef::ValueRefBase<double>* value_vr =
            new ValueRef::Operation<double>(
                ValueRef::PLUS,
                new ValueRef::Variable<double>(ValueRef::EFFECT_TARGET_VALUE_REFERENCE, std::vector<std::string>()),
                new ValueRef::Constant<double>(increase)
            );

        ValueRef::ValueRefBase<std::string>* part_name_vr =
            new ValueRef::Constant<std::string>(part_name);

        std::string stacking_group = (allow_stacking ? "" :
            (part_name + "_" + boost::lexical_cast<std::string>(meter_type) + "_PartMeter"));

        return EffectsGroupPtr(
            new Effect::EffectsGroup(
                scope, activation,
                Effects(1, new Effect::SetShipPartMeter(meter_type, part_name_vr, value_vr)),
                part_name, stacking_group));
    }

    bool DesignsTheSame(const ShipDesign& one, const ShipDesign& two) {
        return (
            one.Name()              == two.Name() &&
            one.Description()       == two.Description() &&
            one.DesignedOnTurn()    == two.DesignedOnTurn() &&
            one.Hull()              == two.Hull() &&
            one.Parts()             == two.Parts() &&
            one.Icon()              == two.Icon() &&
            one.Model()             == two.Model()
        );
        // not checking that IDs are the same, since the purpose of this is to
        // check if a design that might be added to the universe (which doesn't
        // have an ID yet) is the same as one that has already been added
        // (which does have an ID)
    }

    const unsigned int CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int

    // unsigned types (eg. bool, unsigned int, unsigned long int, unsigned char)
    template <class T, typename std::enable_if<std::is_unsigned<T>::value, T>::type* = nullptr>
    void CheckSumCombine(unsigned int& sum, const T& t) {
        std::cout << "CheckSumCombine(unsigned T): " << typeid(t).name() << std::endl << std::endl;
        sum += static_cast<unsigned int>(t);
        sum %= CHECKSUM_MODULUS;
    }

    // signed types (eg. int, char) but not float or double which are covered by specialized functions
    template <class T, typename std::enable_if<std::is_signed<T>::value, T>::type* = nullptr>
    void CheckSumCombine(unsigned int& sum, const T& t) {
        //std::cout << "CheckSumCombine(signed T): " << typeid(t).name() << std::endl << std::endl;
        sum += static_cast<unsigned int>(std::abs(t));
        sum %= CHECKSUM_MODULUS;
    }

    // enums
    template <class T>
    void CheckSumCombine(unsigned int& sum, T t, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr) {
        std::cout << "CheckSumCombine(enum): " << typeid(t).name() << std::endl << std::endl;
        CheckSumCombine(sum, static_cast<int>(t) + 10);
    }

    // pointer types
    template <class T>
    void CheckSumCombine(unsigned int& sum, T p,
                         decltype(*std::declval<T>())* val = nullptr)
    {
        std::cout << "CheckSumCombine(T*): " << typeid(p).name() << std::endl << std::endl;
        if (p)
            CheckSumCombine(sum, *p);
    }

    // fallback do nothing for unsupported types (without GetCheckSum functions)
    void CheckSumCombine(...)
    { std::cout << "CheckSumCombine(...)" << std::endl << std::endl; }

    // applies to pairs (including map value types)
    template <class C, class D>
    void CheckSumCombine(unsigned int& sum, const std::pair<C, D>& p)
    {
        std::cout << "CheckSumCombine(pair): " << typeid(p).name() << std::endl << std::endl;
        CheckSumCombine(sum, p.first);
        CheckSumCombine(sum, p.second);
    }

    // should apply to vectors, maps, and strings
    template <class C>
    void CheckSumCombine(unsigned int& sum, const C& c,
                         const typename C::const_iterator* it = nullptr,
                         const typename C::value_type* val = nullptr)
    {
        std::cout << "CheckSumCombine(C container): " << typeid(c).name() << std::endl << std::endl;
        for (const typename C::value_type& t : c)
            CheckSumCombine(sum, t);
        sum += c.size();
        sum %= CHECKSUM_MODULUS;
    }

    // applies to classes that have GetCheckSum methods
    template <class C>
    void CheckSumCombine(unsigned int& sum, C& c,
                         decltype(((C*)nullptr)->GetCheckSum())* val = nullptr)
    {
        std::cout << "CheckSumCombine(C with GetCheckSum): " << typeid(c).name() << std::endl << std::endl;
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }

    // doubles
    void CheckSumCombine(unsigned int& sum, const double& t) {
        std::cout << "CheckSumCombine(double): " << typeid(t).name() << std::endl << std::endl;
        assert(DBL_MAX_10_EXP < 400);
        if (t == 0.0)
            return;
        // biggest and smallest possible double should be ~10^(+/-308)
        // taking log gives a number in the range +/- 309
        // adding 400 gives numbers in the range ~0 to 800
        // multiplying by 10'000 gives numbers in the range ~0 to 8'000'000
        sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 400.0) * 10000.0);
        sum %= CHECKSUM_MODULUS;
    }
    // floats
    void CheckSumCombine(unsigned int& sum, const float& t) {
        std::cout << "CheckSumCombine(float): " << typeid(t).name() << std::endl << std::endl;
        assert(FLT_MAX_10_EXP < 40);
        if (t == 0.0f)
            return;
        // biggest and smallest possible float should be ~10^(+/-38)
        // taking log gives a number in the range +/- 39
        // adding 400 ives numbers in the range ~0 to 80
        // multiplying by 100'000 gives numbers in the range ~0 to 8'000'000
        sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 40.0f) * 100000.0f);
        sum %= CHECKSUM_MODULUS;
    }

    void CheckSumCombine(unsigned int& sum, const HullType::Slot& slot) {
        std::cout << "CheckSumCombine(Slot): " << typeid(slot).name() << std::endl << std::endl;
        CheckSumCombine(sum, slot.x);
        CheckSumCombine(sum, slot.y);
        CheckSumCombine(sum, slot.type);
    }
}

////////////////////////////////////////////////
// Free Functions                             //
////////////////////////////////////////////////
const PartTypeManager& GetPartTypeManager()
{ return PartTypeManager::GetPartTypeManager(); }

const PartType* GetPartType(const std::string& name)
{ return GetPartTypeManager().GetPartType(name); }

const HullTypeManager& GetHullTypeManager()
{ return HullTypeManager::GetHullTypeManager(); }

const HullType* GetHullType(const std::string& name)
{ return GetHullTypeManager().GetHullType(name); }

const ShipDesign* GetShipDesign(int ship_design_id)
{ return GetUniverse().GetShipDesign(ship_design_id); }


/////////////////////////////////////
// PartTypeManager                 //
/////////////////////////////////////
// static
PartTypeManager* PartTypeManager::s_instance = nullptr;

PartTypeManager::PartTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one PartTypeManager.");

    try {
        parse::ship_parts(m_parts);
    } catch (const std::exception& e) {
        ErrorLogger() << "Failed parsing ship parts: error: " << e.what();
        throw;
    }

    TraceLogger() << "Part Types:";
    for (const auto& entry : m_parts) {
        const PartType* p = entry.second;
        TraceLogger() << " ... " << p->Name() << " class: " << p->Class();
    }

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

PartTypeManager::~PartTypeManager() {
    for (std::map<std::string, PartType*>::value_type& entry : m_parts) {
        delete entry.second;
    }
}

const PartType* PartTypeManager::GetPartType(const std::string& name) const {
    std::map<std::string, PartType*>::const_iterator it = m_parts.find(name);
    return it != m_parts.end() ? it->second : nullptr;
}

const PartTypeManager& PartTypeManager::GetPartTypeManager() {
    static PartTypeManager manager;
    return manager;
}

PartTypeManager::iterator PartTypeManager::begin() const
{ return m_parts.begin(); }

PartTypeManager::iterator PartTypeManager::end() const
{ return m_parts.end(); }

unsigned int PartTypeManager::GetCheckSum() const {
    unsigned int retval{0};
    for (auto const& name_part_pair : m_parts)
        CheckSumCombine(retval, name_part_pair);
    CheckSumCombine(retval, m_parts.size());

    return retval;
}


////////////////////////////////////////////////
// PartType
////////////////////////////////////////////////

PartType::PartType() :
    m_name("invalid part type"),
    m_description("indescribable"),
    m_class(INVALID_SHIP_PART_CLASS),
    m_capacity(0.0f),
    m_secondary_stat(1.0f),
    m_production_cost(0),
    m_production_time(0),
    m_producible(false),
    m_mountable_slot_types(),
    m_tags(),
    m_production_meter_consumption(),
    m_production_special_consumption(),
    m_location(0),
    m_exclusions(),
    m_effects(),
    m_icon(),
    m_add_standard_capacity_effect(false)
{}

PartType::PartType(ShipPartClass part_class, double capacity, double stat2,
                   const CommonParams& common_params, const MoreCommonParams& more_common_params,
                   std::vector<ShipSlotType> mountable_slot_types,
                   const std::string& icon, bool add_standard_capacity_effect) :
    m_name(more_common_params.name),
    m_description(more_common_params.description),
    m_class(part_class),
    m_capacity(capacity),
    m_secondary_stat(stat2),
    m_production_cost(common_params.production_cost),
    m_production_time(common_params.production_time),
    m_producible(common_params.producible),
    m_mountable_slot_types(mountable_slot_types),
    m_tags(),
    m_production_meter_consumption(common_params.production_meter_consumption),
    m_production_special_consumption(common_params.production_special_consumption),
    m_location(common_params.location),
    m_exclusions(more_common_params.exclusions),
    m_effects(),
    m_icon(icon),
    m_add_standard_capacity_effect(add_standard_capacity_effect)
{
    //std::cout << "part type: " << m_name << " producible: " << m_producible << std::endl;
    Init(common_params.effects);
    for (const std::string& tag : common_params.tags)
        m_tags.insert(boost::to_upper_copy<std::string>(tag));
}

void PartType::Init(const std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects) {
    if ((m_capacity != 0 || m_secondary_stat != 0) && m_add_standard_capacity_effect) {
        switch (m_class) {
        case PC_COLONY:
        case PC_TROOPS:
            m_effects.push_back(IncreaseMeter(METER_CAPACITY,       m_name, m_capacity, false));
            break;
        case PC_FIGHTER_HANGAR: {   // capacity indicates how many fighters are stored in this type of part (combined for all copies of the part)
            m_effects.push_back(IncreaseMeter(METER_MAX_CAPACITY,       m_name, m_capacity, true));         // stacking capacities allowed for this part, so each part contributes to the total capacity
            m_effects.push_back(IncreaseMeter(METER_MAX_SECONDARY_STAT, m_name, m_secondary_stat, false));  // stacking damage not allowed, as damage per shot should be the same regardless of number of shots
            break;
        }
        case PC_FIGHTER_BAY:        // capacity indicates how many fighters each instance of the part can launch per combat bout...
        case PC_DIRECT_WEAPON: {    // capacity indicates weapon damage per shot
            m_effects.push_back(IncreaseMeter(METER_MAX_CAPACITY,       m_name, m_capacity, false));
            m_effects.push_back(IncreaseMeter(METER_MAX_SECONDARY_STAT, m_name, m_secondary_stat, false));
            break;
        }
        case PC_SHIELD:
            m_effects.push_back(IncreaseMeter(METER_MAX_SHIELD,     m_capacity));
            break;
        case PC_DETECTION:
            m_effects.push_back(IncreaseMeter(METER_DETECTION,      m_capacity));
            break;
        case PC_STEALTH:
            m_effects.push_back(IncreaseMeter(METER_STEALTH,        m_capacity));
            break;
        case PC_FUEL:
            m_effects.push_back(IncreaseMeter(METER_MAX_FUEL,       m_capacity));
            break;
        case PC_ARMOUR:
            m_effects.push_back(IncreaseMeter(METER_MAX_STRUCTURE,  m_capacity));
            break;
        case PC_SPEED:
            m_effects.push_back(IncreaseMeter(METER_SPEED,          m_capacity));
            break;
        case PC_RESEARCH:
            m_effects.push_back(IncreaseMeter(METER_TARGET_RESEARCH,m_capacity));
            break;
        case PC_INDUSTRY:
            m_effects.push_back(IncreaseMeter(METER_TARGET_INDUSTRY,m_capacity));
            break;
        case PC_TRADE:
            m_effects.push_back(IncreaseMeter(METER_TARGET_TRADE,   m_capacity));
            break;
        default:
            break;
        }
    }

    for (std::shared_ptr<Effect::EffectsGroup> effect : effects) {
        effect->SetTopLevelContent(m_name);
        m_effects.push_back(effect);
    }
}

PartType::~PartType()
{ delete m_location; }

float PartType::Capacity() const
{ return m_capacity; }

float PartType::SecondaryStat() const
{ return m_secondary_stat; }

std::string PartType::CapacityDescription() const {
    std::string desc_string;
    float main_stat = Capacity();
    float sdry_stat = SecondaryStat();

    switch (m_class) {
    case PC_FUEL:
    case PC_TROOPS:
    case PC_COLONY:
    case PC_FIGHTER_BAY:
        desc_string += str(FlexibleFormat(UserString("PART_DESC_CAPACITY")) % main_stat);
        break;
    case PC_DIRECT_WEAPON:
        desc_string += str(FlexibleFormat(UserString("PART_DESC_DIRECT_FIRE_STATS")) % main_stat % sdry_stat);
        break;
    case PC_FIGHTER_HANGAR:
        desc_string += str(FlexibleFormat(UserString("PART_DESC_HANGAR_STATS")) % main_stat % sdry_stat);
        break;
    case PC_SHIELD:
        desc_string = str(FlexibleFormat(UserString("PART_DESC_SHIELD_STRENGTH")) % main_stat);
        break;
    case PC_DETECTION:
        desc_string = str(FlexibleFormat(UserString("PART_DESC_DETECTION")) % main_stat);
        break;
    default:
        desc_string = str(FlexibleFormat(UserString("PART_DESC_STRENGTH")) % main_stat);
        break;
    }
    return desc_string;
}

bool PartType::CanMountInSlotType(ShipSlotType slot_type) const {
    if (INVALID_SHIP_SLOT_TYPE == slot_type)
        return false;
    for (ShipSlotType mountable_slot_type : m_mountable_slot_types)
        if (mountable_slot_type == slot_type)
            return true;
    return false;
}

bool PartType::ProductionCostTimeLocationInvariant() const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION)
        return true;
    if (m_production_cost && !m_production_cost->TargetInvariant())
        return false;
    if (m_production_time && !m_production_time->TargetInvariant())
        return false;
    return true;
}

float PartType::ProductionCost(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION || !m_production_cost) {
        return 1.0f;
    } else {
        if (m_production_cost->ConstantExpr())
            return static_cast<float>(m_production_cost->Eval());

        const auto arbitrary_large_number = 999999.9f;

        std::shared_ptr<UniverseObject> location = GetUniverseObject(location_id);
        if (!location)
            return arbitrary_large_number;

        std::shared_ptr<const UniverseObject> source = Empires().GetSource(empire_id);
        if (!source && !m_production_cost->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source, location);

        return static_cast<float>(m_production_cost->Eval(context));
    }
}

int PartType::ProductionTime(int empire_id, int location_id) const {
    const auto arbitrary_large_number = 9999;

    if (CHEAP_AND_FAST_SHIP_PRODUCTION || !m_production_time) {
        return 1;
    } else {
        if (m_production_time->ConstantExpr())
            return m_production_time->Eval();

        std::shared_ptr<UniverseObject> location = GetUniverseObject(location_id);
        if (!location)
            return arbitrary_large_number;

        std::shared_ptr<const UniverseObject> source = Empires().GetSource(empire_id);
        if (!source && !m_production_time->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source, location);

        return m_production_time->Eval(context);
    }
}

unsigned int PartType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSumCombine(retval, m_name);
    CheckSumCombine(retval, m_description);
    CheckSumCombine(retval, m_class);
    CheckSumCombine(retval, m_capacity);
    CheckSumCombine(retval, m_secondary_stat);
    CheckSumCombine(retval, m_production_cost);
    CheckSumCombine(retval, m_production_time);
    CheckSumCombine(retval, m_producible);
    CheckSumCombine(retval, m_mountable_slot_types);
    CheckSumCombine(retval, m_tags);
    CheckSumCombine(retval, m_production_meter_consumption);
    CheckSumCombine(retval, m_production_special_consumption);
    CheckSumCombine(retval, m_location);
    CheckSumCombine(retval, m_exclusions);
    CheckSumCombine(retval, m_effects);
    CheckSumCombine(retval, m_icon);
    CheckSumCombine(retval, m_add_standard_capacity_effect);

    return retval;
}


////////////////////////////////////////////////
// HullType
////////////////////////////////////////////////
HullType::Slot::Slot() :
    type(INVALID_SHIP_SLOT_TYPE), x(0.5), y(0.5)
{}

void HullType::Init(const std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects) {
    if (m_fuel != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_FUEL,       m_fuel));
    if (m_stealth != 0)
        m_effects.push_back(IncreaseMeter(METER_STEALTH,        m_stealth));
    if (m_structure != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_STRUCTURE,  m_structure));
    if (m_speed != 0)
        m_effects.push_back(IncreaseMeter(METER_SPEED,          m_speed));

    for (std::shared_ptr<Effect::EffectsGroup> effect : effects) {
        effect->SetTopLevelContent(m_name);
        m_effects.push_back(effect);
    }
}

HullType::~HullType()
{ delete m_location; }

unsigned int HullType::NumSlots(ShipSlotType slot_type) const {
    unsigned int count = 0;
    for (const Slot& slot : m_slots)
        if (slot.type == slot_type)
            ++count;
    return count;
}


// HullType:: and PartType::ProductionCost and ProductionTime are almost identical.
// Chances are, the same is true of buildings and techs as well.
// TODO: Eliminate duplication
bool HullType::ProductionCostTimeLocationInvariant() const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION)
        return true;
    if (m_production_cost && !m_production_cost->LocalCandidateInvariant())
        return false;
    if (m_production_time && !m_production_time->LocalCandidateInvariant())
        return false;
    return true;
}

float HullType::ProductionCost(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION || !m_production_cost) {
        return 1.0f;
    } else {
        if (m_production_cost->ConstantExpr())
            return static_cast<float>(m_production_cost->Eval());

        const auto arbitrary_large_number = 999999.9f;

        std::shared_ptr<UniverseObject> location = GetUniverseObject(location_id);
        if (!location)
            return arbitrary_large_number;

        std::shared_ptr<const UniverseObject> source = Empires().GetSource(empire_id);
        if (!source && !m_production_cost->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source, location);

        return static_cast<float>(m_production_cost->Eval(context));
    }
}

int HullType::ProductionTime(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION || !m_production_time) {
        return 1;
    } else {
        if (m_production_time->ConstantExpr())
            return m_production_time->Eval();

        const auto arbitrary_large_number = 999999;

        std::shared_ptr<UniverseObject> location = GetUniverseObject(location_id);
        if (!location)
            return arbitrary_large_number;

        std::shared_ptr<const UniverseObject> source = Empires().GetSource(empire_id);
        if (!source && !m_production_time->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source, location);

        return m_production_time->Eval(context);
    }
}

unsigned int HullType::GetCheckSum() const {
    unsigned int retval{0};

    //// test
    //std::map<unsigned long int, std::shared_ptr<Blah>> int_pBlah_map{
    //    {103U, std::make_shared<Blah>()}, {0, nullptr}};
    //CheckSumCombine(retval, int_pBlah_map);
    //std::map<MeterType, std::string> metertype_string_map{{METER_INDUSTRY, "STRING!"}};
    //CheckSumCombine(retval, metertype_string_map);
    //return retval;
    //// end test

    CheckSumCombine(retval, m_name);
    CheckSumCombine(retval, m_description);
    CheckSumCombine(retval, m_speed);
    CheckSumCombine(retval, m_fuel);
    CheckSumCombine(retval, m_stealth);
    CheckSumCombine(retval, m_structure);
    CheckSumCombine(retval, m_production_cost);
    CheckSumCombine(retval, m_production_time);
    CheckSumCombine(retval, m_producible);
    CheckSumCombine(retval, m_slots);
    CheckSumCombine(retval, m_tags);
    CheckSumCombine(retval, m_production_meter_consumption);
    CheckSumCombine(retval, m_production_special_consumption);
    CheckSumCombine(retval, m_location);
    CheckSumCombine(retval, m_exclusions);
    CheckSumCombine(retval, m_effects);
    CheckSumCombine(retval, m_graphic);
    CheckSumCombine(retval, m_icon);

    return retval;
}


/////////////////////////////////////
// HullTypeManager                 //
/////////////////////////////////////
// static
HullTypeManager* HullTypeManager::s_instance = nullptr;

HullTypeManager::HullTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one HullTypeManager.");

    try {
        parse::ship_hulls(m_hulls);
    } catch (const std::exception& e) {
        ErrorLogger() << "Failed parsing ship hulls: error: " << e.what();
        throw;
    }

    TraceLogger() << "Hull Types:";
    for (const auto& entry : m_hulls) {
        const HullType* h = entry.second;
        TraceLogger() << " ... " << h->Name();
    }

    // Only update the global pointer on sucessful construction.
    s_instance = this;

    DebugLogger() << "HullTypeManager checksum: " << GetCheckSum();
}

HullTypeManager::~HullTypeManager() {
    for (std::map<std::string, HullType*>::value_type& entry : m_hulls) {
        delete entry.second;
    }
}

const HullType* HullTypeManager::GetHullType(const std::string& name) const {
    std::map<std::string, HullType*>::const_iterator it = m_hulls.find(name);
    return it != m_hulls.end() ? it->second : nullptr;
}

const HullTypeManager& HullTypeManager::GetHullTypeManager() {
    static HullTypeManager manager;
    return manager;
}

HullTypeManager::iterator HullTypeManager::begin() const
{ return m_hulls.begin(); }

HullTypeManager::iterator HullTypeManager::end() const
{ return m_hulls.end(); }

unsigned int HullTypeManager::GetCheckSum() const {
    unsigned int retval{0};
    for (auto const& name_hull_pair : m_hulls)
        CheckSumCombine(retval, name_hull_pair);
    CheckSumCombine(retval, m_hulls.size());

    return retval;
}

////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() :
    m_name(),
    m_description(),
    m_uuid(boost::uuids::nil_generator()()),
    m_designed_on_turn(UniverseObject::INVALID_OBJECT_AGE),
    m_designed_by_empire(ALL_EMPIRES),
    m_hull(),
    m_parts(),
    m_is_monster(false),
    m_icon(),
    m_3D_model(),
    m_name_desc_in_stringtable(false)
{}

ShipDesign::ShipDesign(const std::string& name, const std::string& description,
                       int designed_on_turn, int designed_by_empire, const std::string& hull,
                       const std::vector<std::string>& parts,
                       const std::string& icon, const std::string& model,
                       bool name_desc_in_stringtable, bool monster,
                       const boost::uuids::uuid& uuid /*= boost::uuids::nil_uuid()*/) :
    m_name(name),
    m_description(description),
    m_uuid(uuid),
    m_designed_on_turn(designed_on_turn),
    m_designed_by_empire(designed_by_empire),
    m_hull(hull),
    m_parts(parts),
    m_is_monster(monster),
    m_icon(icon),
    m_3D_model(model),
    m_name_desc_in_stringtable(name_desc_in_stringtable)
{
    // expand parts list to have empty values if fewer parts are given than hull has slots
    if (const HullType* hull_type = GetHullType(m_hull)) {
        if (m_parts.size() < hull_type->NumSlots())
            m_parts.resize(hull_type->NumSlots(), "");
    }

    if (!ValidDesign(m_hull, m_parts)) {
        ErrorLogger() << "constructing an invalid ShipDesign!";
        ErrorLogger() << Dump();
    }
    BuildStatCaches();
}

ShipDesign::ShipDesign(const std::string& name, const std::string& description,
                       const std::string& hull,
                       const std::vector<std::string>& parts,
                       const std::string& icon, const std::string& model,
                       bool name_desc_in_stringtable, bool monster,
                       const boost::uuids::uuid& uuid /*= boost::uuids::nil_uuid()*/) :
    ShipDesign(name, description, 0, ALL_EMPIRES, hull, parts, icon, model, name_desc_in_stringtable, monster, uuid)
{}

const std::string& ShipDesign::Name(bool stringtable_lookup /* = true */) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_name);
    else
        return m_name;
}

void ShipDesign::SetName(const std::string& name) {
    if (m_name != "") {
        m_name = name;
    }
}

void ShipDesign::SetUUID(const boost::uuids::uuid& uuid)
{ m_uuid = uuid; }

const std::string& ShipDesign::Description(bool stringtable_lookup /* = true */) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_description);
    else
        return m_description;
}

void ShipDesign::SetDescription(const std::string& description) {
    if (m_description != "") {
        m_description = description;
    }
}

bool ShipDesign::ProductionCostTimeLocationInvariant() const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION)
        return true;
    // as seen in ShipDesign::ProductionCost, the location is passed as the
    // local candidate in the ScriptingContext

    // check hull and all parts
    if (const HullType* hull = GetHullType(m_hull))
        if (!hull->ProductionCostTimeLocationInvariant())
            return false;
    for (const std::string& part_name : m_parts)
        if (const PartType* part = GetPartType(part_name))
            if (!part->ProductionCostTimeLocationInvariant())
                return false;

    // if hull and all parts are invariant, so is whole design
    return true;
}

float ShipDesign::ProductionCost(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION) {
        return 1.0f;
    } else {
        float cost_accumulator = 0.0f;
        if (const HullType* hull = GetHullType(m_hull))
            cost_accumulator += hull->ProductionCost(empire_id, location_id);
        for (const std::string& part_name : m_parts)
            if (const PartType* part = GetPartType(part_name))
                cost_accumulator += part->ProductionCost(empire_id, location_id);
        return std::max(0.0f, cost_accumulator);
    }
}

float ShipDesign::PerTurnCost(int empire_id, int location_id) const
{ return ProductionCost(empire_id, location_id) / std::max(1, ProductionTime(empire_id, location_id)); }

int ShipDesign::ProductionTime(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION) {
        return 1;
    } else {
        int time_accumulator = 1;
        if (const HullType* hull = GetHullType(m_hull))
            time_accumulator = std::max(time_accumulator, hull->ProductionTime(empire_id, location_id));
        for (const std::string& part_name : m_parts)
            if (const PartType* part = GetPartType(part_name))
                time_accumulator = std::max(time_accumulator, part->ProductionTime(empire_id, location_id));
        return std::max(1, time_accumulator);
    }
}

bool ShipDesign::CanColonize() const {
    for (const std::string& part_name : m_parts) {
        if (part_name.empty())
            continue;
        if (const PartType* part = GetPartType(part_name))
            if (part->Class() == PC_COLONY)
                return true;
    }
    return false;
}

float ShipDesign::Defense() const {
    // accumulate defense from defensive parts in design.
    float total_defense = 0.0f;
    const PartTypeManager& part_manager = GetPartTypeManager();
    for (const std::string& part_name : Parts()) {
        const PartType* part = part_manager.GetPartType(part_name);
        if (part && (part->Class() == PC_SHIELD || part->Class() == PC_ARMOUR))
            total_defense += part->Capacity();
    }
    return total_defense;
}

float ShipDesign::Attack() const {
    // total damage against a target with the no shield.
    return AdjustedAttack(0.0f);
}

float ShipDesign::AdjustedAttack(float shield) const {
    // total damage against a target with the given shield (damage reduction)
    // assuming full load of fighters that are not destroyed during the battle
    int available_fighters = 0;
    int fighter_launch_capacity = 0;
    float fighter_damage = 0.0f;
    float direct_attack = 0.0f;

    for (const std::string& part_name : m_parts) {
        const PartType* part = GetPartType(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();

        // direct weapon and fighter-related parts all handled differently...
        if (part_class == PC_DIRECT_WEAPON) {
            float part_attack = part->Capacity();
            if (part_attack > shield)
                direct_attack += (part_attack - shield)*part->SecondaryStat();  // here, secondary stat is number of shots per round
        } else if (part_class == PC_FIGHTER_HANGAR) {
            available_fighters = part->Capacity();                              // stacked meter
        } else if (part_class == PC_FIGHTER_BAY) {
            fighter_launch_capacity += part->Capacity();
            fighter_damage = part->SecondaryStat();                             // here, secondary stat is fighter damage per shot
        }
    }

    int fighter_shots = std::min(available_fighters, fighter_launch_capacity);  // how many fighters launched in bout 1
    available_fighters -= fighter_shots;
    int launched_fighters = fighter_shots;
    int num_bouts = GetUniverse().GetNumCombatRounds();
    int remaining_bouts = num_bouts - 2;  // no attack for first round, second round already added
    while (remaining_bouts > 0) {
        int fighters_launched_this_bout = std::min(available_fighters, fighter_launch_capacity);
        available_fighters -= fighters_launched_this_bout;
        launched_fighters += fighters_launched_this_bout;
        fighter_shots += launched_fighters;
        --remaining_bouts;
    }

    // how much damage does a fighter shot do?
    fighter_damage = std::max(0.0f, fighter_damage);

    return direct_attack + fighter_shots*fighter_damage/num_bouts;   // divide by bouts because fighter calculation is for a full combat, but direct firefor one attack
}

std::vector<std::string> ShipDesign::Parts(ShipSlotType slot_type) const {
    std::vector<std::string> retval;

    const HullType* hull = GetHull();
    if (!hull) {
        ErrorLogger() << "Design hull not found: " << m_hull;
        return retval;
    }
    const std::vector<HullType::Slot>& slots = hull->Slots();

    if (m_parts.empty())
        return retval;

    // add to output vector each part that is in a slot of the indicated ShipSlotType
    for (unsigned int i = 0; i < m_parts.size(); ++i)
        if (slots[i].type == slot_type)
            retval.push_back(m_parts[i]);

    return retval;
}

std::vector<std::string> ShipDesign::Weapons() const {
    std::vector<std::string> retval;
    retval.reserve(m_parts.size());
    for (const std::string& part_name : m_parts) {
        const PartType* part = GetPartType(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();
        if (part_class == PC_DIRECT_WEAPON || part_class == PC_FIGHTER_BAY)
        { retval.push_back(part_name); }
    }
    return retval;
}

bool ShipDesign::ProductionLocation(int empire_id, int location_id) const {
    Empire* empire = GetEmpire(empire_id);
    if (!empire) {
        DebugLogger() << "ShipDesign::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    // must own the production location...
    std::shared_ptr<const UniverseObject> location = GetUniverseObject(location_id);
    if (!location->OwnedBy(empire_id))
        return false;

    std::shared_ptr<const Planet> planet = std::dynamic_pointer_cast<const Planet>(location);
    std::shared_ptr<const Ship> ship;
    if (!planet)
        ship = std::dynamic_pointer_cast<const Ship>(location);
    if (!planet && !ship)
        return false;

    // ships can only be produced by species that are not planetbound
    const std::string& species_name = planet ? planet->SpeciesName() : (ship ? ship->SpeciesName() : EMPTY_STRING);
    if (species_name.empty())
        return false;
    const Species* species = GetSpecies(species_name);
    if (!species)
        return false;

    if (!species->CanProduceShips())
        return false;
    // also, species that can't colonize can't produce colony ships
    if (this->CanColonize() && !species->CanColonize())
        return false;

    // apply hull location conditions to potential location
    const HullType* hull = GetHull();
    if (!hull) {
        ErrorLogger() << "ShipDesign::ProductionLocation  ShipDesign couldn't get its own hull with name " << m_hull;
        return false;
    }
    // evaluate using location as the source, as it should be an object owned by this empire.
    ScriptingContext location_as_source_context(location);
    if (!hull->Location()->Eval(location_as_source_context, location))
        return false;

    // apply external and internal parts' location conditions to potential location
    for (const std::string& part_name : m_parts) {
        if (part_name.empty())
            continue;       // empty slots don't limit build location

        const PartType* part = GetPartType(part_name);
        if (!part) {
            ErrorLogger() << "ShipDesign::ProductionLocation  ShipDesign couldn't get part with name " << part_name;
            return false;
        }
        if (!part->Location()->Eval(location_as_source_context, location))
            return false;
    }
    // location matched all hull and part conditions, so is a valid build location
    return true;
}

void ShipDesign::SetID(int id)
{ m_id = id; }

bool ShipDesign::ValidDesign(const std::string& hull, const std::vector<std::string>& parts) {
    // ensure hull type exists and has at least enough slots for passed parts
    const HullType* hull_type = GetHullTypeManager().GetHullType(hull);
    if (!hull_type) {
        DebugLogger() << "ShipDesign::ValidDesign: hull not found: " << hull;
        return false;
    }

    unsigned int size = parts.size();
    if (size > hull_type->NumSlots()) {
        DebugLogger() << "ShipDesign::ValidDesign: given " << size << " parts for hull with " << hull_type->NumSlots() << " slots";
        return false;
    }

    const std::vector<HullType::Slot>& slots = hull_type->Slots();

    // check hull exclusions against all parts...
    const std::set<std::string>& hull_exclusions = hull_type->Exclusions();
    for (const std::string& part_name : parts) {
        if (part_name.empty())
            continue;
        if (hull_exclusions.find(part_name) != hull_exclusions.end())
            return false;
    }

    // check part exclusions against other parts and hull
    std::set<std::string> already_seen_component_names;
    already_seen_component_names.insert(hull);
    for (const std::string& part_name : parts) {
        const PartType* part_type = GetPartType(part_name);
        if (!part_type)
            continue;
        for (const std::string& excluded : part_type->Exclusions()) {
            if (already_seen_component_names.find(excluded) != already_seen_component_names.end())
                return false;
        }
        already_seen_component_names.insert(part_name);
    }


    // ensure all passed parts can be mounted in slots of type they were passed for
    for (unsigned int i = 0; i < size; ++i) {
        const std::string& part_name = parts[i];
        if (part_name.empty())
            continue;   // if part slot is empty, ignore - doesn't invalidate design

        const PartType* part = GetPartType(part_name);
        if (!part) {
            DebugLogger() << "ShipDesign::ValidDesign: part not found: " << part_name;
            return false;
        }

        // verify part can mount in indicated slot
        ShipSlotType slot_type = slots[i].type;
        if (!(part->CanMountInSlotType(slot_type))) {
            DebugLogger() << "ShipDesign::ValidDesign: part " << part_name << " can't be mounted in " << boost::lexical_cast<std::string>(slot_type) << " slot";
            return false;
        }
    }

    return true;
}

void ShipDesign::BuildStatCaches() {
    const HullType* hull = GetHullType(m_hull);
    if (!hull) {
        ErrorLogger() << "ShipDesign::BuildStatCaches couldn't get hull with name " << m_hull;
        return;
    }

    m_producible =      hull->Producible();
    m_detection =       hull->Detection();
    m_colony_capacity = hull->ColonyCapacity();
    m_troop_capacity =  hull->TroopCapacity();
    m_stealth =         hull->Stealth();
    m_fuel =            hull->Fuel();
    m_shields =         hull->Shields();
    m_structure =       hull->Structure();
    m_speed =           hull->Speed();

    for (const std::string& part_name : m_parts) {
        if (part_name.empty())
            continue;

        const PartType* part = GetPartType(part_name);
        if (!part) {
            ErrorLogger() << "ShipDesign::BuildStatCaches couldn't get part with name " << part_name;
            continue;
        }

        if (!part->Producible())
            m_producible = false;

        ShipPartClass part_class = part->Class();

        switch (part_class) {
        case PC_DIRECT_WEAPON:
            m_is_armed = true;
            break;
        case PC_FIGHTER_BAY:
        case PC_FIGHTER_HANGAR:
            m_has_fighters = true;
            break;
        case PC_COLONY:
            m_colony_capacity += part->Capacity();
            break;
        case PC_TROOPS:
            m_troop_capacity += part->Capacity();
            break;
        case PC_STEALTH:
            m_stealth += part->Capacity();
            break;
        case PC_SPEED:
            m_speed += part->Capacity();
            break;
        case PC_SHIELD:
            m_shields += part->Capacity();
            break;
        case PC_FUEL:
            m_fuel += part->Capacity();
            break;
        case PC_ARMOUR:
            m_structure += part->Capacity();
            break;
        case PC_DETECTION:
            m_detection += part->Capacity();
            break;
        case PC_BOMBARD:
            m_can_bombard = true;
            break;
        case PC_RESEARCH:
            m_research_generation += part->Capacity();
            break;
        case PC_INDUSTRY:
            m_industry_generation += part->Capacity();
            break;
        case PC_TRADE:
            m_trade_generation += part->Capacity();
            break;
        case PC_PRODUCTION_LOCATION:
            m_is_production_location = true;
            break;
        default:
            break;
        }

        m_num_part_types[part_name]++;
        if (part_class > INVALID_SHIP_PART_CLASS && part_class < NUM_SHIP_PART_CLASSES)
            m_num_part_classes[part_class]++;
    }
}

std::string ShipDesign::Dump() const {
    std::string retval = DumpIndent() + "ShipDesign\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "uuid = \"" + boost::uuids::to_string(m_uuid) + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";

    if (!m_name_desc_in_stringtable)
        retval += DumpIndent() + "NoStringtableLookup\n";
    retval += DumpIndent() + "hull = \"" + m_hull + "\"\n";
    retval += DumpIndent() + "parts = ";
    if (m_parts.empty()) {
        retval += "[]\n";
    } else if (m_parts.size() == 1) {
        retval += "\"" + *m_parts.begin() + "\"\n";
    } else {
        retval += "[\n";
        ++g_indent;
        for (const std::string& part_name : m_parts) {
            retval += DumpIndent() + "\"" + part_name + "\"\n";
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    if (!m_icon.empty())
        retval += DumpIndent() + "icon = \"" + m_icon + "\"\n";
    retval += DumpIndent() + "model = \"" + m_3D_model + "\"\n";
    --g_indent;
    return retval; 
}

unsigned int ShipDesign::GetCheckSum() const {
    unsigned int retval{0};
    CheckSumCombine(retval, m_id);
    CheckSumCombine(retval, m_name);
    CheckSumCombine(retval, m_description);
    CheckSumCombine(retval, m_designed_on_turn);
    CheckSumCombine(retval, m_designed_by_empire);
    CheckSumCombine(retval, m_hull);
    CheckSumCombine(retval, m_parts);
    CheckSumCombine(retval, m_is_monster);
    CheckSumCombine(retval, m_icon);
    CheckSumCombine(retval, m_3D_model);
    CheckSumCombine(retval, m_name_desc_in_stringtable);

    return retval;
}

bool operator ==(const ShipDesign& first, const ShipDesign& second) {
    if (first.Hull() != second.Hull())
        return false;

    std::map<std::string, int> first_parts;
    std::map<std::string, int> second_parts;

    for (const std::string& part_name : first.Parts())
    { ++first_parts[part_name]; }

    for (const std::string& part_name : second.Parts())
    { ++second_parts[part_name]; }

    return first_parts == second_parts;
}


/////////////////////////////////////
// PredefinedShipDesignManager     //
/////////////////////////////////////
// static(s)
PredefinedShipDesignManager* PredefinedShipDesignManager::s_instance = nullptr;

PredefinedShipDesignManager::PredefinedShipDesignManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one PredefinedShipDesignManager.");

    DebugLogger() << "Initializing PredefinedShipDesignManager";

    std::vector<std::unique_ptr<ShipDesign>> designs;
    std::vector<boost::uuids::uuid> ordering;
    parse::ship_designs(designs, ordering);

    for (auto& design : designs)
        if (m_ship_designs.count(design->Name()))
            ErrorLogger() << design->Name() << " ship design does not have a unique name.";
        else
            m_ship_designs[design->Name()] = std::move(design);

    designs.clear();
    ordering.clear();
    parse::monster_designs(designs, ordering);

    for (auto& design : designs)
        if (m_monster_designs.count(design->Name()))
            ErrorLogger() << design->Name() << " ship design does not have a unique name.";
        else
            m_monster_designs[design->Name()] = std::move(design);


    TraceLogger() << "Predefined Ship Designs:";
    for (const auto& entry : m_ship_designs)
        TraceLogger() << " ... " << entry.second->Name();

    TraceLogger() << "Monster Ship Designs:";
    for (const auto& entry : m_monster_designs)
        TraceLogger() << " ... " << entry.second->Name();

    // Only update the global pointer on sucessful construction.
    s_instance = this;

    DebugLogger() << "PredefinedShipDesignManager checksum: " << GetCheckSum();
}

namespace {
    void AddDesignToUniverse(std::map<std::string, int>& design_generic_ids,
                             const std::unique_ptr<ShipDesign>& design, bool monster)
    {
        if (!design)
            return;

        Universe& universe = GetUniverse();
        /* check if there already exists this same design in the universe. */
        for (Universe::ship_design_iterator it = universe.beginShipDesigns();
             it != universe.endShipDesigns(); ++it)
        {
            const ShipDesign* existing_design = it->second;
            if (!existing_design) {
                ErrorLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse found an invalid design in the Universe";
                continue;
            }

            if (DesignsTheSame(*existing_design, *design)) {
                DebugLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse found there already is an exact duplicate of a design to be added, so is not re-adding it";
                design_generic_ids[design->Name(false)] = existing_design->ID();
                return; // design already added; don't need to do so again
            }
        }


        // generate id for new design
        int new_design_id = GetNewDesignID();
        if (new_design_id == INVALID_DESIGN_ID) {
            ErrorLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse Unable to get new design id";
            return;
        }


        // duplicate design to add to Universe
        ShipDesign* copy = new ShipDesign(design->Name(false), design->Description(false),
                                          design->DesignedOnTurn(), design->DesignedByEmpire(),
                                          design->Hull(), design->Parts(), design->Icon(),
                                          design->Model(), design->LookupInStringtable(),
                                          monster, design->UUID());
        if (!copy) {
            ErrorLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse() couldn't duplicate the design with name " << design->Name();
            return;
        }

        bool success = universe.InsertShipDesignID(copy, new_design_id);
        if (!success) {
            ErrorLogger() << "Empire::AddShipDesign Unable to add new design to universe";
            delete copy;
            return;
        }

        design_generic_ids[design->Name(false)] = new_design_id;
    };
}

const std::map<std::string, int>& PredefinedShipDesignManager::AddShipDesignsToUniverse() const {
    m_design_generic_ids.clear();   // std::map<std::string, int>

    for (const auto& entry : m_ship_designs)
        AddDesignToUniverse(m_design_generic_ids, entry.second, false);

    for (const auto& entry : m_monster_designs)
        AddDesignToUniverse(m_design_generic_ids, entry.second, true);

    return m_design_generic_ids;
}

PredefinedShipDesignManager& PredefinedShipDesignManager::GetPredefinedShipDesignManager() {
    static PredefinedShipDesignManager manager;
    return manager;
}


std::vector<const ShipDesign*> PredefinedShipDesignManager::GetOrderedShipDesigns() const {
    std::vector<const ShipDesign*> retval;
    for (const auto& name_and_ptr : m_ship_designs)
        retval.push_back(name_and_ptr.second.get());
    return retval;
}

std::vector<const ShipDesign*> PredefinedShipDesignManager::GetOrderedMonsterDesigns() const {
    std::vector<const ShipDesign*> retval;
    for (const auto& name_and_ptr : m_monster_designs)
        retval.push_back(name_and_ptr.second.get());
    return retval;
}

int PredefinedShipDesignManager::GetDesignID(const std::string& name) const {
    std::map<std::string, int>::const_iterator it = m_design_generic_ids.find(name);
    if (it == m_design_generic_ids.end())
        return INVALID_DESIGN_ID;
    return it->second;
}

unsigned int PredefinedShipDesignManager::GetCheckSum() const {
    unsigned int retval{0};
    for (auto const& name_design_pair : m_ship_designs)
        CheckSumCombine(retval, name_design_pair);
    CheckSumCombine(retval, m_ship_designs.size());

    return retval;
}


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
const PredefinedShipDesignManager& GetPredefinedShipDesignManager()
{ return PredefinedShipDesignManager::GetPredefinedShipDesignManager(); }

const ShipDesign* GetPredefinedShipDesign(const std::string& name)
{ return GetUniverse().GetGenericShipDesign(name); }
