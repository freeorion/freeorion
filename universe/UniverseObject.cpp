#include "UniverseObject.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "Meter.h"
#include "System.h"
#include "Special.h"
#include "Universe.h"
#include "Predicates.h"

#include <stdexcept>


// static(s)
const double    UniverseObject::INVALID_POSITION  = -100000.0;
const int       UniverseObject::INVALID_OBJECT_AGE = -(1 << 30) - 1;  // using big negative number to allow for potential negative object ages, which might be useful in the event of time travel.
const int       UniverseObject::SINCE_BEFORE_TIME_AGE = (1 << 30) + 1;

UniverseObject::UniverseObject() :
    StateChangedSignal(Universe::UniverseObjectSignalsInhibited()),
    m_id(INVALID_OBJECT_ID),
    m_name(""),
    m_x(INVALID_POSITION),
    m_y(INVALID_POSITION),
    m_owner_empire_id(ALL_EMPIRES),
    m_system_id(INVALID_OBJECT_ID),
    m_meters(),
    m_created_on_turn(-1)
{
    //Logger().debugStream() << "UniverseObject::UniverseObject()";
    m_created_on_turn = CurrentTurn();
}

UniverseObject::UniverseObject(const std::string name, double x, double y,
                               const std::set<int>& owners/* = std::set<int>()*/) :
    StateChangedSignal(Universe::UniverseObjectSignalsInhibited()),
    m_id(INVALID_OBJECT_ID),
    m_name(name),
    m_x(x),
    m_y(y),
    m_owner_empire_id(ALL_EMPIRES),
    m_system_id(INVALID_OBJECT_ID),
    m_meters(),
    m_created_on_turn(-1)
{
    //Logger().debugStream() << "UniverseObject::UniverseObject(" << name << ", " << x << ", " << y << ")";
    if (m_x < 0.0 || Universe::UniverseWidth() < m_x || m_y < 0.0 || Universe::UniverseWidth() < m_y)
        throw std::invalid_argument("UniverseObject::UniverseObject : Attempted to create an object \"" + m_name + "\" off the map area.");
    m_created_on_turn = CurrentTurn();
}

UniverseObject::UniverseObject(const UniverseObject& rhs) :
    StateChangedSignal(Universe::UniverseObjectSignalsInhibited()),
    m_id(rhs.m_id),
    m_name(rhs.m_name),
    m_x(rhs.m_x),
    m_y(rhs.m_y),
    m_owner_empire_id(rhs.m_owner_empire_id),
    m_system_id(rhs.m_system_id),
    m_specials(rhs.m_specials),
    m_meters(rhs.m_meters),
    m_created_on_turn(rhs.m_created_on_turn)
{}

UniverseObject::~UniverseObject()
{}

void UniverseObject::Copy(const UniverseObject* copied_object, Visibility vis,
                          const std::set<std::string>& visible_specials)
{
    if (copied_object == this)
        return;
    if (!copied_object) {
        Logger().errorStream() << "UniverseObject::Copy passed a null object";
        return;
    }

    std::map<MeterType, Meter> censored_meters = copied_object->CensoredMeters(vis);
    for (std::map<MeterType, Meter>::const_iterator it = copied_object->m_meters.begin();
         it != copied_object->m_meters.end(); ++it)
    {
        MeterType type = it->first;

        // get existing meter in this object, or create a default one
        Meter& this_meter = this->m_meters[type];

        // if there is an update to meter from censored meters, update this object's copy
        std::map<MeterType, Meter>::const_iterator censored_it = censored_meters.find(type);
        if (censored_it != censored_meters.end())
            this_meter = censored_it->second;
    }

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_id =                    copied_object->m_id;
        this->m_x =                     copied_object->m_x;
        this->m_y =                     copied_object->m_y;
        this->m_system_id =             copied_object->m_system_id;

        this->m_specials.clear();
        for (std::map<std::string, int>::const_iterator copied_special_it = copied_object->m_specials.begin();
             copied_special_it != copied_object->m_specials.end(); ++copied_special_it)
        {
            Logger().debugStream() << "UniverseObject::Copy " << copied_object->Name() << " has special " << copied_special_it->first;
            if (visible_specials.find(copied_special_it->first) != visible_specials.end()) {
                this->m_specials[copied_special_it->first] = copied_special_it->second;
                Logger().debugStream() << " ... which is copied.";
            }
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

const std::map<std::string, int>& UniverseObject::Specials() const
{ return m_specials; }

bool UniverseObject::HasSpecial(const std::string& name) const
{ return m_specials.find(name) != m_specials.end(); }

int UniverseObject::SpecialAddedOnTurn(const std::string& name) const {
    std::map<std::string, int>::const_iterator it = m_specials.find(name);
    if (it == m_specials.end())
        return INVALID_GAME_TURN;
    return it->second;
}

std::vector<std::string> UniverseObject::Tags() const {
    static std::vector<std::string> EMPTY_STRING_VEC;
    return EMPTY_STRING_VEC;
}

bool UniverseObject::HasTag(const std::string& name) const
{ return false; }

const std::string& UniverseObject::TypeName() const
{ return UserString("UNIVERSEOBJECT"); }

std::string UniverseObject::Dump() const {
    const System* system = GetSystem(this->SystemID());

    std::stringstream os;

    os << TypeName() << " "
       << this->ID() << ": "
       << this->Name()
       << (system ? ("  at: " + system->Name()) : "")
       << " owner: " << m_owner_empire_id
       << " created on turn: " << m_created_on_turn
       << " specials: ";
    for (std::map<std::string, int>::const_iterator it = m_specials.begin(); it != m_specials.end(); ++it)
        os << "(" << it->first << ", " << it->second << ") ";
    os << "  Meters: ";
    for (std::map<MeterType, Meter>::const_iterator it = m_meters.begin(); it != m_meters.end(); ++it)
        os << UserString(GG::GetEnumMap<MeterType>().FromEnum(it->first))
           << ": " << it->second.Current() << "  ";
    return os.str();
}

std::vector<int> UniverseObject::FindObjectIDs() const
{ return std::vector<int>(); }

bool UniverseObject::Contains(int object_id) const
{ return false; }

bool UniverseObject::ContainedBy(int object_id) const {
    const UniverseObject* object = GetUniverseObject(object_id);
    if (object)
        return object->Contains(m_id);
    else
        return false;
}

const Meter* UniverseObject::GetMeter(MeterType type) const {
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it != m_meters.end())
        return &(it->second);
    return 0;
}

double UniverseObject::CurrentMeterValue(MeterType type) const {
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::CurrentMeterValue was passed a MeterType that this UniverseObject does not have");

    return it->second.Current();
}

double UniverseObject::InitialMeterValue(MeterType type) const {
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::InitialMeterValue was passed a MeterType that this UniverseObject does not have");

    return it->second.Initial();
}

double UniverseObject::NextTurnCurrentMeterValue(MeterType type) const
{ return UniverseObject::CurrentMeterValue(type); }

void UniverseObject::AddMeter(MeterType meter_type) {
    if (INVALID_METER_TYPE == meter_type)
        Logger().errorStream() << "UniverseObject::AddMeter asked to add invalid meter type!";
    else
        m_meters[meter_type];
}

bool UniverseObject::Unowned() const
{ return m_owner_empire_id == ALL_EMPIRES; }

bool UniverseObject::OwnedBy(int empire) const 
{ return empire != ALL_EMPIRES && empire == m_owner_empire_id; }

Visibility UniverseObject::GetVisibility(int empire_id) const
{ return GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id); }

const std::string& UniverseObject::PublicName(int empire_id) const
{ return m_name; }

UniverseObject* UniverseObject::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(const_cast<UniverseObject* const>(this)); }

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

void UniverseObject::MoveTo(UniverseObject* object) {
    if (!object)
        throw std::invalid_argument("UniverseObject::MoveTo passed an invalid object or object id");

    if (object->SystemID() == this->SystemID()) {
        // don't call MoveTo(double, double) as that would remove from old (current system)
        m_x = object->X();
        m_y = object->Y();
        StateChangedSignal();
    } else {
        // move to location in space, removing from old system
        MoveTo(object->X(), object->Y());
    }
}

void UniverseObject::MoveTo(double x, double y) {
    //Logger().debugStream() << "UniverseObject::MoveTo(double x, double y)";
    if (x < 0.0 || Universe::UniverseWidth() < x || y < 0.0 || Universe::UniverseWidth() < y)
        Logger().debugStream() << "UniverseObject::MoveTo : Placing object \"" + m_name + "\" off the map area.";

    m_x = x;
    m_y = y;

    // remove object from its old system (unless object is a system, as that would attempt to remove it from itself)
    if (this->ID() != this->SystemID())
        if (System* system = GetSystem(this->SystemID()))
            system->Remove(this->ID());

    StateChangedSignal();
}

Meter* UniverseObject::GetMeter(MeterType type) {
    std::map<MeterType, Meter>::iterator it = m_meters.find(type);
    if (it != m_meters.end())
        return &(it->second);
    return 0;
}

void UniverseObject::BackPropegateMeters() {
    for (MeterType i = MeterType(0); i != NUM_METER_TYPES; i = MeterType(i + 1))
        if (Meter* meter = this->GetMeter(i))
            meter->BackPropegate();
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
    //Logger().debugStream() << "UniverseObject::SetSystem(int sys)";
    if (sys != m_system_id) {
        m_system_id = sys;
        StateChangedSignal();
    }
}

void UniverseObject::AddSpecial(const std::string& name)
{ m_specials[name] = CurrentTurn(); }

void UniverseObject::RemoveSpecial(const std::string& name)
{ m_specials.erase(name); }

std::map<MeterType, Meter> UniverseObject::CensoredMeters(Visibility vis) const {
    std::map<MeterType, Meter> retval;
    if (vis >= VIS_PARTIAL_VISIBILITY)
        retval = m_meters;
    return retval;
}

std::map<std::string, int> UniverseObject::CensoredSpecials(Visibility vis) const {
    std::map<std::string, int> retval;
    if (vis >= VIS_PARTIAL_VISIBILITY)
        retval = m_specials;
    return retval;
}

void UniverseObject::ResetTargetMaxUnpairedMeters()
{ GetMeter(METER_STEALTH)->ResetCurrent(); }

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

