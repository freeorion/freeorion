#include "UniverseObject.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "Meter.h"
#include "System.h"
#include "Special.h"
#include "Universe.h"
#include "Predicates.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
#include <stdexcept>


// static(s)
const double    UniverseObject::INVALID_POSITION  = -100000.0;
const int       UniverseObject::INVALID_OBJECT_ID = -1;
const int       UniverseObject::MAX_ID            = 2000000000;
const int       UniverseObject::INVALID_OBJECT_AGE = -(1 << 30) - 1;  // using big negative number to allow for potential negative object ages, which might be useful in the event of time travel.
const int       UniverseObject::SINCE_BEFORE_TIME_AGE = (1 << 30) + 1;

UniverseObject::UniverseObject() :
    StateChangedSignal(Universe::UniverseObjectSignalsInhibited()),
    m_id(INVALID_OBJECT_ID),
    m_name(""),
    m_x(INVALID_POSITION),
    m_y(INVALID_POSITION),
    m_owners(),
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
    m_owners(owners),
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
    m_owners(rhs.m_owners),
    m_system_id(rhs.m_system_id),
    m_specials(rhs.m_specials),
    m_meters(rhs.m_meters),
    m_created_on_turn(rhs.m_created_on_turn)
{}

UniverseObject::~UniverseObject()
{}

void UniverseObject::Copy(const UniverseObject* copied_object, Visibility vis)
{
    if (copied_object == this)
        return;
    if (!copied_object) {
        Logger().errorStream() << "UniverseObject::Copy passed a null object";
        return;
    }

    std::map<MeterType, Meter> meters = copied_object->CensoredMeters(vis);
    this->m_meters = meters;

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_id =                    copied_object->m_id;
        this->m_x =                     copied_object->m_x;
        this->m_y =                     copied_object->m_y;
        this->m_system_id =             copied_object->m_system_id;

        if (vis >= VIS_PARTIAL_VISIBILITY) {

            this->m_name =              copied_object->m_name;  // may be overwritten by derived class' Copy
            this->m_owners =            copied_object->m_owners;
            this->m_specials =          copied_object->m_specials;
            this->m_created_on_turn =   copied_object->m_created_on_turn;

            //if (vis >= VIS_FULL_VISIBILITY) {
            //}
        }
    }
}

void UniverseObject::Init()
{
    AddMeter(METER_STEALTH);
}

int UniverseObject::ID() const
{
    return m_id;
}

const std::string& UniverseObject::Name() const
{
    return m_name;
}

double UniverseObject::X() const
{
    return m_x;
}

double UniverseObject::Y() const
{
    return m_y;
}

int UniverseObject::CreationTurn() const
{
    return m_created_on_turn;
}

int UniverseObject::AgeInTurns() const
{
    if (m_created_on_turn == BEFORE_FIRST_TURN)
        return SINCE_BEFORE_TIME_AGE;
    if ((m_created_on_turn == INVALID_GAME_TURN) || (CurrentTurn() == INVALID_GAME_TURN))
        return INVALID_OBJECT_AGE;
    return CurrentTurn() - m_created_on_turn;
}

const std::set<int>& UniverseObject::Owners() const
{
    return m_owners;
}

int UniverseObject::SystemID() const
{
    return m_system_id;
}

const std::set<std::string>& UniverseObject::Specials() const
{
    return m_specials;
}

const std::string& UniverseObject::TypeName() const
{
    return UserString("UNIVERSEOBJECT");
}

std::vector<int> UniverseObject::FindObjectIDs() const
{
    return std::vector<int>();
}

bool UniverseObject::Contains(int object_id) const
{
    return false;
}

bool UniverseObject::ContainedBy(int object_id) const
{
    const UniverseObject* object = GetMainObjectMap().Object(object_id);
    if (object)
        return object->Contains(m_id);
    else
        return false;
}

const Meter* UniverseObject::GetMeter(MeterType type) const
{
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it != m_meters.end())
        return &(it->second);
    return 0;
}

double UniverseObject::CurrentMeterValue(MeterType type) const
{
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::CurrentMeterValue was passed a MeterType that this UniverseObject does not have");

    return it->second.Current();
}

double UniverseObject::InitialMeterValue(MeterType type) const
{
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::InitialMeterValue was passed a MeterType that this UniverseObject does not have");

    return it->second.Initial();
}

double UniverseObject::PreviousMeterValue(MeterType type) const
{
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::PreviousMeterValue was passed a MeterType that this UniverseObject does not have");

    return it->second.Previous();
}

double UniverseObject::NextTurnCurrentMeterValue(MeterType type) const
{
    return UniverseObject::CurrentMeterValue(type);
}

void UniverseObject::AddMeter(MeterType meter_type)
{
    if (INVALID_METER_TYPE == meter_type)
        Logger().errorStream() << "UniverseObject::AddMeter asked to add invalid meter type!";
    else
        m_meters[meter_type];
}

bool UniverseObject::Unowned() const 
{
    return m_owners.empty();
}

bool UniverseObject::OwnedBy(int empire) const 
{
    return m_owners.find(empire) != m_owners.end();
}

bool UniverseObject::WhollyOwnedBy(int empire) const 
{
    return m_owners.size() == 1 && m_owners.find(empire) != m_owners.end();
}

Visibility UniverseObject::GetVisibility(int empire_id) const
{
    return GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);
}

const std::string& UniverseObject::PublicName(int empire_id) const
{
    return m_name;
}

UniverseObject* UniverseObject::Accept(const UniverseObjectVisitor& visitor) const
{
    return visitor.Visit(const_cast<UniverseObject* const>(this));
}

void UniverseObject::SetID(int id)
{
    m_id = id;
    StateChangedSignal();
}

void UniverseObject::Rename(const std::string& name)
{
    m_name = name;
    StateChangedSignal();
}

void UniverseObject::Move(double x, double y)
{
    MoveTo(m_x + x, m_y + y);
}

void UniverseObject::MoveTo(int object_id)
{
    MoveTo(GetMainObjectMap().Object(object_id));
}

void UniverseObject::MoveTo(UniverseObject* object)
{
    if (!object)
        throw std::invalid_argument("UniverseObject::MoveTo passed an invalid object or object id");

    MoveTo(object->X(), object->Y());
}

void UniverseObject::MoveTo(double x, double y)
{
    //Logger().debugStream() << "UniverseObject::MoveTo(double x, double y)";
    if (x < 0.0 || Universe::UniverseWidth() < x || y < 0.0 || Universe::UniverseWidth() < y)
        Logger().debugStream() << "UniverseObject::MoveTo : Placing object \"" + m_name + "\" off the map area.";

    m_x = x;
    m_y = y;

    if (System* system = GetObject<System>(this->SystemID()))
        system->Remove(this);
    StateChangedSignal();
}

Meter* UniverseObject::GetMeter(MeterType type)
{
    std::map<MeterType, Meter>::iterator it = m_meters.find(type);
    if (it != m_meters.end())
        return &(it->second);
    return 0;
}

void UniverseObject::AddOwner(int id)
{
    if (id != ALL_EMPIRES && m_owners.find(id) == m_owners.end()) {
        m_owners.insert(id);
        StateChangedSignal();
    }
    /* TODO: if adding an owner to an object gives an the added owner an
     * observer in, or ownership of a previoiusly unexplored system, then need
     * to call empire->AddExploredSystem(system_id); */
}

void UniverseObject::RemoveOwner(int id)
{
    if (m_owners.find(id) != m_owners.end()) {
        m_owners.erase(id);
        StateChangedSignal();
    }
}

void UniverseObject::ClearOwners()
{
    const std::set<int> initial_owners = m_owners;
    for (std::set<int>::const_iterator it = initial_owners.begin(); it != initial_owners.end(); ++it)
        RemoveOwner(*it);
}

void UniverseObject::SetSystem(int sys)
{
    //Logger().debugStream() << "UniverseObject::SetSystem(int sys)";
    if (sys != m_system_id) {
        m_system_id = sys;
        StateChangedSignal();
    }
}

void UniverseObject::AddSpecial(const std::string& name)
{
    m_specials.insert(name);
}

void UniverseObject::RemoveSpecial(const std::string& name)
{
    m_specials.erase(name);
}

std::map<MeterType, Meter> UniverseObject::CensoredMeters(Visibility vis) const
{
    std::map<MeterType, Meter> retval;
    if (vis >= VIS_PARTIAL_VISIBILITY) {
        retval = m_meters;
    } else {
        for (std::map<MeterType, Meter>::const_iterator it = m_meters.begin(); it != m_meters.end(); ++it)
            retval[it->first] = Meter();
    }
    return retval;
}

void UniverseObject::ResetTargetMaxUnpairedMeters(MeterType meter_type/* = INVALID_METER_TYPE*/)
{
    GetMeter(METER_STEALTH)->ResetCurrent();
}

void UniverseObject::ClampMeters()
{
    GetMeter(METER_STEALTH)->ClampCurrentToRange();
}

