#include "UniverseObject.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include "Enums.h"
#include "Pathfinder.h"
#include "ScriptingContext.h"
#include "Special.h"
#include "System.h"
#include "UniverseObjectVisitor.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/i18n.h"

namespace ValueRef {
    const std::string& MeterToName(MeterType meter);
}

UniverseObject::UniverseObject() :
    StateChangedSignal(blocking_combiner<boost::signals2::optional_last_value<void>>(
        GetUniverse().UniverseObjectSignalsInhibited())),
    m_created_on_turn(CurrentTurn())
{}

UniverseObject::UniverseObject(std::string name, double x, double y) :
    StateChangedSignal(blocking_combiner<boost::signals2::optional_last_value<void>>(
        GetUniverse().UniverseObjectSignalsInhibited())),
    m_name(std::move(name)),
    m_x(x),
    m_y(y),
    m_created_on_turn(CurrentTurn())
{}

void UniverseObject::Copy(std::shared_ptr<const UniverseObject> copied_object,
                          Visibility vis, const std::set<std::string>& visible_specials,
                          const Universe&)
{
    if (copied_object.get() == this)
        return;
    if (!copied_object) {
        ErrorLogger() << "UniverseObject::Copy passed a null object";
        return;
    }

    auto censored_meters = copied_object->CensoredMeters(vis);
    for (auto& [type, copied_meter] : copied_object->m_meters) {
        (void)copied_meter;

        // get existing meter in this object, or create a default one
        auto m_meter_it = m_meters.find(type);
        bool meter_already_known = (m_meter_it != m_meters.end());
        if (!meter_already_known)
            m_meters[type]; // default initialize to (0, 0).  Alternative: = Meter(Meter::INVALID_VALUE, Meter::INVALID_VALUE);
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

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_id =                    copied_object->m_id;
        this->m_system_id =             copied_object->m_system_id;
        this->m_x =                     copied_object->m_x;
        this->m_y =                     copied_object->m_y;

        this->m_specials.clear();
        for (const auto& entry_special : copied_object->m_specials) {
            if (visible_specials.count(entry_special.first))
                this->m_specials[entry_special.first] = entry_special.second;
        }

        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            this->m_owner_empire_id =   copied_object->m_owner_empire_id;
            this->m_created_on_turn =   copied_object->m_created_on_turn;

            if (vis >= Visibility::VIS_FULL_VISIBILITY)
                this->m_name =          copied_object->m_name;
        }
    }
}

void UniverseObject::Init()
{ AddMeter(MeterType::METER_STEALTH); }

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

std::set<std::string> UniverseObject::Tags(const ScriptingContext&) const
{ return {}; }

bool UniverseObject::HasTag(const std::string& name, const ScriptingContext&) const
{ return false; }

UniverseObjectType UniverseObject::ObjectType() const
{ return UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE; }

std::string UniverseObject::Dump(unsigned short ntabs) const {
    const auto& objects{Objects()};
    auto system = objects.get<System>(this->SystemID());


    std::string retval;
    retval.reserve(2048); // guesstimate
    retval.append(to_string(this->ObjectType())).append(" ")
          .append(std::to_string(this->ID())).append(": ").append(this->Name());

    if (system) {
        auto& sys_name = system->Name();
        if (sys_name.empty())
            retval.append("  at: (System ").append(std::to_string(system->ID())).append(")");
        else
            retval.append("  at: ").append(sys_name);
    } else {
        retval.append("  at: (").append(std::to_string(this->X())).append(", ")
              .append(std::to_string(this->Y())).append(")");
        int near_id = GetUniverse().GetPathfinder()->NearestSystemTo(this->X(), this->Y(), objects);
        auto near_system = objects.get<System>(near_id);
        if (near_system) {
            auto& sys_name = near_system->Name();
            if (sys_name.empty())
                retval.append(" nearest (System ").append(std::to_string(near_system->ID())).append(")");
            else
                retval.append(" nearest ").append(near_system->Name());
        }
    }
    if (Unowned()) {
        retval.append(" owner: (Unowned) ");
    } else {
        auto& empire_name = Empires().GetEmpireName(m_owner_empire_id);
        if (!empire_name.empty())
            retval.append(" owner: ").append(empire_name);
        else
            retval.append(" owner: (Unknown Empire)");
    }
    retval.append(" created on turn: ").append(std::to_string(m_created_on_turn))
          .append(" specials: ");
    for (auto& [special_name, turn_amount] : m_specials)
        retval.append("(").append(special_name).append(", ")
              .append(std::to_string(turn_amount.first)).append(", ")
              .append(std::to_string(turn_amount.second)).append(") ");
    retval.append("  Meters: ");
    for (auto& [meter_type, meter] : m_meters)
        retval.append(ValueRef::MeterToName(meter_type)).append(": ").append(meter.Dump()).append("  ");
    return retval;
}

namespace {
    std::set<int> EMPTY_SET;
}

const std::set<int>& UniverseObject::ContainedObjectIDs() const
{ return EMPTY_SET; }

std::set<int> UniverseObject::VisibleContainedObjectIDs(int empire_id) const {
    std::set<int> retval;
    const Universe& universe = GetUniverse(); // TODO: pass in
    for (int object_id : ContainedObjectIDs()) {
        if (universe.GetObjectVisibilityByEmpire(object_id, empire_id) >= Visibility::VIS_BASIC_VISIBILITY)
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

void UniverseObject::AddMeter(MeterType meter_type) {
    if (MeterType::INVALID_METER_TYPE == meter_type)
        ErrorLogger() << "UniverseObject::AddMeter asked to add invalid meter type!";
    else
        m_meters[meter_type];
}

bool UniverseObject::Unowned() const
{ return Owner() == ALL_EMPIRES; }

bool UniverseObject::OwnedBy(int empire) const
{ return empire != ALL_EMPIRES && empire == Owner(); }

bool UniverseObject::HostileToEmpire(int, const EmpireManager&) const
{ return false; }

Visibility UniverseObject::GetVisibility(int empire_id, const EmpireIDtoObjectIDtoVisMap& v) const {
    auto empire_it = v.find(empire_id);
    if (empire_it == v.end())
        return Visibility::VIS_NO_VISIBILITY;
    auto obj_it = empire_it->second.find(m_id);
    if (obj_it == empire_it->second.end())
        return Visibility::VIS_NO_VISIBILITY;
    return obj_it->second;
}

Visibility UniverseObject::GetVisibility(int empire_id, const Universe& u) const
{ return GetVisibility(empire_id, u.GetEmpireObjectVisibility()); }

const std::string& UniverseObject::PublicName(int, const Universe&) const
{ return m_name; }

std::shared_ptr<UniverseObject> UniverseObject::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<UniverseObject>(shared_from_this())); }

void UniverseObject::SetID(int id) {
    m_id = id;
    StateChangedSignal();
}

void UniverseObject::Rename(std::string name) {
    m_name = std::move(name);
    StateChangedSignal();
}

void UniverseObject::Move(double x, double y)
{ MoveTo(m_x + x, m_y + y); }

void UniverseObject::MoveTo(const std::shared_ptr<const UniverseObject>& object) {
    if (!object) {
        ErrorLogger() << "UniverseObject::MoveTo : attempted to move to a null object.";
        return;
    }
    MoveTo(object->X(), object->Y());
}

void UniverseObject::MoveTo(const std::shared_ptr<UniverseObject>& object) {
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
    for (auto& m : m_meters)
        m.second.BackPropagate();
}

void UniverseObject::SetOwner(int id) {
    if (m_owner_empire_id != id) {
        m_owner_empire_id = id;
        StateChangedSignal();
    }
    /* TODO: if changing object ownership gives an the new owner an
     * observer in, or ownership of a previoiusly unexplored system, then need
     * to call empire->AddExploredSystem(system_id, CurrentTurn(), context.ContextObjects()); */
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

UniverseObject::MeterMap UniverseObject::CensoredMeters(Visibility vis) const {
    MeterMap retval;
    if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
        retval = m_meters;
    } else if (vis == Visibility::VIS_BASIC_VISIBILITY && m_meters.count(MeterType::METER_STEALTH))
        retval.emplace(MeterType::METER_STEALTH, Meter{Meter::LARGE_VALUE, Meter::LARGE_VALUE});
    return retval;
}

void UniverseObject::ResetTargetMaxUnpairedMeters() {
    auto it = m_meters.find(MeterType::METER_STEALTH);
    if (it != m_meters.end())
        it->second.ResetCurrent();
}

void UniverseObject::ResetPairedActiveMeters() {
    // iterate over paired active meters (those that have an associated max or
    // target meter.  if another paired meter type is added to Enums.h, it
    // should be added here as well.
    for (auto& m : m_meters) {
        if (m.first > MeterType::METER_TROOPS)
            break;
        if (m.first >= MeterType::METER_POPULATION)
            m.second.SetCurrent(m.second.Initial());
    }
}

void UniverseObject::ClampMeters() {
    auto it = m_meters.find(MeterType::METER_STEALTH);
    if (it != m_meters.end())
        it->second.ClampCurrentToRange();
}
