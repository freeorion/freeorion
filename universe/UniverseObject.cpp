#include "UniverseObject.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../Empire/EmpireManager.h"
#include "Meter.h"
#include "System.h"
#include "Special.h"
#include "Pathfinder.h"
#include "Universe.h"
#include "Predicates.h"
#include "Enums.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>

// static(s)
const int INVALID_OBJECT_ID      = -1;
const int TEMPORARY_OBJECT_ID    = -2;

const double    UniverseObject::INVALID_POSITION  = -100000.0;
const int       UniverseObject::INVALID_OBJECT_AGE = -(1 << 30) - 1;  // using big negative number to allow for potential negative object ages, which might be useful in the event of time travel.
const int       UniverseObject::SINCE_BEFORE_TIME_AGE = (1 << 30) + 1;

UniverseObject::UniverseObject() :
    StateChangedSignal(blocking_combiner<boost::signals2::optional_last_value<void>>(
        GetUniverse().UniverseObjectSignalsInhibited())),
    m_x(INVALID_POSITION),
    m_y(INVALID_POSITION),
    m_created_on_turn(CurrentTurn() )
{}

UniverseObject::UniverseObject(const std::string name, double x, double y) :
    StateChangedSignal(blocking_combiner<boost::signals2::optional_last_value<void>>(
        GetUniverse().UniverseObjectSignalsInhibited())),
    m_name(name),
    m_x(x),
    m_y(y),
    m_created_on_turn(CurrentTurn() )
{}

UniverseObject::~UniverseObject()
{}

void UniverseObject::Copy(std::shared_ptr<const UniverseObject> copied_object,
                          Visibility vis, const std::set<std::string>& visible_specials)
{
    if (copied_object.get() == this)
        return;
    if (!copied_object) {
        ErrorLogger() << "UniverseObject::Copy passed a null object";
        return;
    }

    auto censored_meters = copied_object->CensoredMeters(vis);
    for (const auto& entry : copied_object->m_meters) {
        MeterType type = entry.first;

        // get existing meter in this object, or create a default one
        auto m_meter_it = m_meters.find(type);
        bool meter_already_known = (m_meter_it != m_meters.end());
        if (!meter_already_known)
            m_meters[type]; // default initialize to (0, 0).  Alternative: = Meter(Meter::INVALID_VALUE, Meter::INVALID_VALUE);*/
        Meter& this_meter = m_meters[type];

        // if there is an update to meter from censored meters, update this object's copy
        auto censored_it = censored_meters.find(type);
        if (censored_it != censored_meters.end()) {
            const Meter& copied_object_meter = censored_it->second;

            if (!meter_already_known) {
                // have no previous info, so just use whatever is given
                this_meter = copied_object_meter;

            } else {
                // don't want to override legit meter history with sentinel values used for insufficiently visible objects
                if (copied_object_meter.Initial() != Meter::LARGE_VALUE ||
                    copied_object_meter.Current() != Meter::LARGE_VALUE)
                {
                    // some new info available, so can overwrite only meter info
                    this_meter = copied_object_meter;
                }
            }
        }
    }

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_id =                    copied_object->m_id;
        this->m_system_id =             copied_object->m_system_id;
        this->m_x =                     copied_object->m_x;
        this->m_y =                     copied_object->m_y;

        this->m_specials.clear();
        for (const auto& entry_special : copied_object->m_specials) {
            if (visible_specials.count(entry_special.first))
                this->m_specials[entry_special.first] = entry_special.second;
        }

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            this->m_owner_empire_id =   copied_object->m_owner_empire_id;
            this->m_created_on_turn =   copied_object->m_created_on_turn;

            if (vis >= VIS_FULL_VISIBILITY) {
                this->m_name =          copied_object->m_name;
            }
        }
    }
}

void UniverseObject::Init()
{ AddMeter(METER_STEALTH); }

int UniverseObject::ID() const
{ return m_id; }

const std::string& UniverseObject::Name() const
{ return m_name; }

double UniverseObject::X() const
{ return m_x; }

double UniverseObject::Y() const
{ return m_y; }

int UniverseObject::CreationTurn() const
{ return m_created_on_turn; }

int UniverseObject::AgeInTurns() const {
    if (m_created_on_turn == BEFORE_FIRST_TURN)
        return SINCE_BEFORE_TIME_AGE;
    if ((m_created_on_turn == INVALID_GAME_TURN) || (CurrentTurn() == INVALID_GAME_TURN))
        return INVALID_OBJECT_AGE;
    return CurrentTurn() - m_created_on_turn;
}

int UniverseObject::Owner() const
{ return m_owner_empire_id; }

int UniverseObject::SystemID() const
{ return m_system_id; }

const std::map<std::string, std::pair<int, float>>& UniverseObject::Specials() const
{ return m_specials; }

bool UniverseObject::HasSpecial(const std::string& name) const
{ return m_specials.count(name); }

int UniverseObject::SpecialAddedOnTurn(const std::string& name) const {
    auto it = m_specials.find(name);
    if (it == m_specials.end())
        return INVALID_GAME_TURN;
    return it->second.first;
}

float UniverseObject::SpecialCapacity(const std::string& name) const {
    auto it = m_specials.find(name);
    if (it == m_specials.end())
        return 0.0f;
    return it->second.second;
}

std::set<std::string> UniverseObject::Tags() const
{ return std::set<std::string>(); }

bool UniverseObject::HasTag(const std::string& name) const
{ return false; }

UniverseObjectType UniverseObject::ObjectType() const
{ return INVALID_UNIVERSE_OBJECT_TYPE; }

std::string UniverseObject::Dump(unsigned short ntabs) const {
    auto system = GetSystem(this->SystemID());

    std::stringstream os;

    os << boost::lexical_cast<std::string>(this->ObjectType()) << " "
       << this->ID() << ": "
       << this->Name();
    if (system) {
        const std::string& sys_name = system->Name();
        if (sys_name.empty())
            os << "  at: (System " << system->ID() << ")";
        else
            os << "  at: " << sys_name;
    } else {
        os << "  at: (" << this->X() << ", " << this->Y() << ")";
        int near_id = GetPathfinder()->NearestSystemTo(this->X(), this->Y());
        auto near_system = GetSystem(near_id);
        if (near_system) {
            const std::string& sys_name = near_system->Name();
            if (sys_name.empty())
                os << " nearest (System " << near_system->ID() << ")";
            else
                os << " nearest " << near_system->Name();
        }
    }
    if (Unowned()) {
        os << " owner: (Unowned) ";
    } else {
        const std::string& empire_name = Empires().GetEmpireName(m_owner_empire_id);
        if (!empire_name.empty())
            os << " owner: " << empire_name;
        else
            os << " owner: (Unknown Empire)";
    }
    os << " created on turn: " << m_created_on_turn
       << " specials: ";
    for (const auto& entry : m_specials)
        os << "(" << entry.first << ", " << entry.second.first << ", " << entry.second.second << ") ";
    os << "  Meters: ";
    for (const auto& entry : m_meters)
        os << entry.first
           << ": " << entry.second.Dump() << "  ";
    return os.str();
}

namespace {
    std::set<int> EMPTY_SET;
}

const std::set<int>& UniverseObject::ContainedObjectIDs() const
{ return EMPTY_SET; }

std::set<int> UniverseObject::VisibleContainedObjectIDs(int empire_id) const {
    std::set<int> retval;
    const Universe& universe = GetUniverse();
    for (int object_id : ContainedObjectIDs()) {
        if (universe.GetObjectVisibilityByEmpire(object_id, empire_id) >= VIS_BASIC_VISIBILITY)
            retval.insert(object_id);
    }
    return retval;
}

int UniverseObject::ContainerObjectID() const
{ return INVALID_OBJECT_ID; }

bool UniverseObject::Contains(int object_id) const
{ return false; }

bool UniverseObject::ContainedBy(int object_id) const
{ return false; }

const Meter* UniverseObject::GetMeter(MeterType type) const {
    auto it = m_meters.find(type);
    if (it != m_meters.end())
        return &(it->second);
    return nullptr;
}

float UniverseObject::CurrentMeterValue(MeterType type) const {
    auto it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::CurrentMeterValue was passed a MeterType that this UniverseObject does not have: " + boost::lexical_cast<std::string>(type));

    return it->second.Current();
}

float UniverseObject::InitialMeterValue(MeterType type) const {
    auto it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::InitialMeterValue was passed a MeterType that this UniverseObject does not have: " + boost::lexical_cast<std::string>(type));

    return it->second.Initial();
}

void UniverseObject::AddMeter(MeterType meter_type) {
    if (INVALID_METER_TYPE == meter_type)
        ErrorLogger() << "UniverseObject::AddMeter asked to add invalid meter type!";
    else
        m_meters[meter_type];
}

bool UniverseObject::Unowned() const
{ return Owner() == ALL_EMPIRES; }

bool UniverseObject::OwnedBy(int empire) const
{ return empire != ALL_EMPIRES && empire == Owner(); }

bool UniverseObject::HostileToEmpire(int empire_id) const
{ return false; }

Visibility UniverseObject::GetVisibility(int empire_id) const
{ return GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id); }

const std::string& UniverseObject::PublicName(int empire_id) const
{ return m_name; }

std::shared_ptr<UniverseObject> UniverseObject::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<UniverseObject>(shared_from_this())); }

void UniverseObject::SetID(int id) {
    m_id = id;
    StateChangedSignal();
}

void UniverseObject::Rename(const std::string& name) {
    m_name = name;
    StateChangedSignal();
}

void UniverseObject::Move(double x, double y)
{ MoveTo(m_x + x, m_y + y); }

void UniverseObject::MoveTo(int object_id)
{ MoveTo(GetUniverseObject(object_id)); }

void UniverseObject::MoveTo(std::shared_ptr<UniverseObject> object) {
    if (!object) {
        ErrorLogger() << "UniverseObject::MoveTo : attempted to move to a null object.";
        return;
    }
    MoveTo(object->X(), object->Y());
}

void UniverseObject::MoveTo(double x, double y) {
    if (m_x == x && m_y == y)
        return;

    m_x = x;
    m_y = y;

    StateChangedSignal();
}

Meter* UniverseObject::GetMeter(MeterType type) {
    auto it = m_meters.find(type);
    if (it != m_meters.end())
        return &(it->second);
    return nullptr;
}

void UniverseObject::BackPropagateMeters() {
    for (MeterType i = MeterType(0); i != NUM_METER_TYPES; i = MeterType(i + 1))
        if (Meter* meter = this->GetMeter(i))
            meter->BackPropagate();
}

void UniverseObject::SetOwner(int id) {
    if (m_owner_empire_id != id) {
        m_owner_empire_id = id;
        StateChangedSignal();
    }
    /* TODO: if changing object ownership gives an the new owner an
     * observer in, or ownership of a previoiusly unexplored system, then need
     * to call empire->AddExploredSystem(system_id); */
}

void UniverseObject::SetSystem(int sys) {
    //DebugLogger() << "UniverseObject::SetSystem(int sys)";
    if (sys != m_system_id) {
        m_system_id = sys;
        StateChangedSignal();
    }
}

void UniverseObject::AddSpecial(const std::string& name, float capacity)
{ m_specials[name] = std::make_pair(CurrentTurn(), capacity); }

void UniverseObject::SetSpecialCapacity(const std::string& name, float capacity) {
    if (m_specials.count(name))
        m_specials[name].second = capacity;
    else
        AddSpecial(name, capacity);
}

void UniverseObject::RemoveSpecial(const std::string& name)
{ m_specials.erase(name); }

std::map<MeterType, Meter> UniverseObject::CensoredMeters(Visibility vis) const {
    std::map<MeterType, Meter> retval;
    if (vis >= VIS_PARTIAL_VISIBILITY) {
        retval = m_meters;
    } else if (vis == VIS_BASIC_VISIBILITY && m_meters.count(METER_STEALTH))
        retval[METER_STEALTH] = Meter(Meter::LARGE_VALUE, Meter::LARGE_VALUE);
    return retval;
}

void UniverseObject::ResetTargetMaxUnpairedMeters() {
    if (Meter* meter = GetMeter(METER_STEALTH))
        meter->ResetCurrent();
}

void UniverseObject::ResetPairedActiveMeters() {
    // iterate over paired active meters (those that have an associated max or
    // target meter.  if another paired meter type is added to Enums.h, it
    // should be added here as well.
    for (MeterType meter_type = MeterType(METER_POPULATION);
         meter_type <= MeterType(METER_TROOPS);
         meter_type = MeterType(meter_type + 1))
    {
        if (Meter* meter = GetMeter(meter_type))
            meter->SetCurrent(meter->Initial());
    }
}

void UniverseObject::ClampMeters()
{ GetMeter(METER_STEALTH)->ClampCurrentToRange(); }
