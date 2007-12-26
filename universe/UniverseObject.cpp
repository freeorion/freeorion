#include "UniverseObject.h"

#include "../util/AppInterface.h"
#include "Meter.h"
#include "../util/MultiplayerCommon.h"
#include "Predicates.h"
#include "System.h"
#include "Special.h"
#include "Universe.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
#include <stdexcept>

#include <stdexcept>


// static(s)
const double UniverseObject::INVALID_POSITION  = -100000.0;
const int    UniverseObject::INVALID_OBJECT_ID = -1;
const int    UniverseObject::MAX_ID            = 2000000000;
// using big negative number to allow for potential negative object ages, which might be useful in the even of time
// travel.
const int UniverseObject::INVALID_OBJECT_AGE = -(1 << 30) - 1;
const int UniverseObject::SINCE_BEFORE_TIME_AGE = (1 << 30) + 1;

UniverseObject::UniverseObject() : 
    StateChangedSignal(Universe::InhibitUniverseObjectSignals()),
    m_id(INVALID_OBJECT_ID),
    m_x(INVALID_POSITION),
    m_y(INVALID_POSITION),
    m_system_id(INVALID_OBJECT_ID),
    m_meters()
{
    m_created_on_turn = CurrentTurn();
}

UniverseObject::UniverseObject(const std::string name, double x, double y, 
                               const std::set<int>& owners/* = std::set<int>()*/) : 
    StateChangedSignal(Universe::InhibitUniverseObjectSignals()),
    m_id(INVALID_OBJECT_ID),
    m_name(name),
    m_x(x),
    m_y(y),
    m_owners(owners),
    m_system_id(INVALID_OBJECT_ID),
    m_meters()
{
    if (m_x < 0.0 || Universe::UniverseWidth() < m_x || m_y < 0.0 || Universe::UniverseWidth() < m_y)
        throw std::invalid_argument("UniverseObject::UniverseObject : Attempted to create an object \"" + m_name + "\" off the map area.");
    m_created_on_turn = CurrentTurn();
}

UniverseObject::~UniverseObject()
{}

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

System* UniverseObject::GetSystem() const
{
    return m_system_id == INVALID_OBJECT_ID ? 0 : GetUniverse().Object<System>(m_system_id);
}

const std::set<std::string>& UniverseObject::Specials() const
{
    return m_specials;
}

const Meter* UniverseObject::GetMeter(MeterType type) const
{
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it != m_meters.end())
        return &(it->second);
    return 0;
}

double UniverseObject::ProjectedCurrentMeter(MeterType type) const
{
        std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
        if (it == m_meters.end())
            throw std::invalid_argument("UniverseObject::ProjectedCurrentMeter was passed a MeterType that this UniverseObject does not have");
        return it->second.Current();    // default to no growth
}

double UniverseObject::MeterPoints(MeterType type) const
{
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::MeterPoints was passed a MeterType that this UniverseObject does not have");
    return it->second.Current();    // default to meter value
}

double UniverseObject::ProjectedMeterPoints(MeterType type) const
{
    // Note that this is the default implementation.  Derived classes might do something different.
    return MeterPoints(type);
}

void UniverseObject::InsertMeter(MeterType meter_type, const Meter& meter)
{
    m_meters[meter_type] = meter;
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

UniverseObject::Visibility UniverseObject::GetVisibility(int empire_id) const
{
    return (Universe::ALL_OBJECTS_VISIBLE || empire_id == ALL_EMPIRES || m_owners.find(empire_id) != m_owners.end()) ? FULL_VISIBILITY : NO_VISIBILITY;
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
    if (m_x + x < 0.0 || Universe::UniverseWidth() < m_x + x || m_y + y < 0.0 || Universe::UniverseWidth() < m_y + y)
        throw std::runtime_error("UniverseObject::Move : Attempted to move object \"" + m_name + "\" off the map area.");
    m_x += x;
    m_y += y;
    StateChangedSignal();
}

void UniverseObject::MoveTo(double x, double y)
{
    if (x < 0.0 || Universe::UniverseWidth() < x || y < 0.0 || Universe::UniverseWidth() < y)
        throw std::invalid_argument("UniverseObject::MoveTo : Attempted to place object \"" + m_name + "\" off the map area.");
    m_x = x;
    m_y = y;
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
    m_owners.insert(id); 
    /* TODO: if adding an owner to an object gives an the added owner an observer in, or ownership of
     * a previoiusly unexplored system, then need to call empire->AddExploredSystem(system_id); */
    StateChangedSignal();
}

void UniverseObject::RemoveOwner(int id)
{
    m_owners.erase(id);
    StateChangedSignal();
}

void UniverseObject::SetSystem(int sys)
{
    m_system_id = sys;
    StateChangedSignal();
}

void UniverseObject::AddSpecial(const std::string& name)
{
    m_specials.insert(name);
}

void UniverseObject::RemoveSpecial(const std::string& name)
{
    m_specials.erase(name);
}

void UniverseObject::ResetMaxMeters()
{
    for (MeterType i = MeterType(0); i != NUM_METER_TYPES; i = MeterType(i + 1)) {
        if (Meter* meter = GetMeter(i)) {
            meter->ResetMax();
        }
    }
}

void UniverseObject::ApplyUniverseTableMaxMeterAdjustments()
{}

void UniverseObject::ClampMeters()
{
    for (MeterType i = MeterType(0); i != NUM_METER_TYPES; i = MeterType(i + 1)) {
        if (Meter* meter = GetMeter(i)) {
            meter->Clamp();
        }
    }
}
