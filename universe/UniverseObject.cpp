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
    m_x(INVALID_POSITION),
    m_y(INVALID_POSITION),
    m_system_id(INVALID_OBJECT_ID),
    m_meters()
{
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
    m_meters()
{
    if (m_x < 0.0 || Universe::UniverseWidth() < m_x || m_y < 0.0 || Universe::UniverseWidth() < m_y)
        throw std::invalid_argument("UniverseObject::UniverseObject : Attempted to create an object \"" + m_name + "\" off the map area.");
    m_created_on_turn = CurrentTurn();
}

UniverseObject::~UniverseObject()
{}

void UniverseObject::Init()
{
    InsertMeter(METER_STEALTH, Meter());
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

System* UniverseObject::GetSystem() const
{
    return SystemID() == INVALID_OBJECT_ID ? 0 : GetUniverse().Object<System>(SystemID());
}

const std::set<std::string>& UniverseObject::Specials() const
{
    return m_specials;
}

std::vector<UniverseObject*> UniverseObject::FindObjects() const
{
    return std::vector<UniverseObject*>();
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
    const UniverseObject* object = GetUniverse().Object(object_id);
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

double UniverseObject::ProjectedCurrentMeter(MeterType type) const
{
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::ProjectedCurrentMeter was passed a MeterType that this UniverseObject does not have");

    if (type == METER_STEALTH)
        return it->second.Max();
    else
        return it->second.Current();    // default to no growth
}

double UniverseObject::MeterPoints(MeterType type) const
{
    std::map<MeterType, Meter>::const_iterator it = m_meters.find(type);
    if (it == m_meters.end())
        throw std::invalid_argument("UniverseObject::MeterPoints was passed a MeterType that this UniverseObject does not have");
    return it->second.InitialCurrent();
}

double UniverseObject::ProjectedMeterPoints(MeterType type) const
{
    return ProjectedCurrentMeter(type);
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
    MoveTo(GetUniverse().Object(object_id));
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

    if (System* system = GetSystem())
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
    if (id != ALL_EMPIRES) {
        m_owners.insert(id);
        StateChangedSignal();
    }
    /* TODO: if adding an owner to an object gives an the added owner an
     * observer in, or ownership of a previoiusly unexplored system, then need
     * to call empire->AddExploredSystem(system_id); */

}

void UniverseObject::RemoveOwner(int id)
{
    m_owners.erase(id);
    StateChangedSignal();
}

void UniverseObject::ClearOwners()
{
    for (std::set<int>::iterator it = m_owners.begin(); it != m_owners.end(); ++it)
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

void UniverseObject::ResetMaxMeters(MeterType meter_type)
{
    if (meter_type == INVALID_METER_TYPE) {
        for (std::map<MeterType, Meter>::iterator it = m_meters.begin(); it != m_meters.end(); ++it)
            it->second.ResetMax();
    } else {
        if (Meter* meter = GetMeter(meter_type))
            meter->ResetMax();
        else
            Logger().errorStream() << "UniverseObject::ResetMaxMeters called with MeterType this object does not have";
    }
}

void UniverseObject::ApplyUniverseTableMaxMeterAdjustments(MeterType meter_type)
{}

void UniverseObject::ClampMeters()
{
    for (std::map<MeterType, Meter>::iterator it = m_meters.begin(); it != m_meters.end(); ++it)
        it->second.Clamp();
}

void UniverseObject::PopGrowthProductionResearchPhase()
{
    Meter* meter = GetMeter(METER_STEALTH);
    assert(meter);
    meter->SetCurrent(meter->Max());
}

void UniverseObject::MovementPhase()
{}
