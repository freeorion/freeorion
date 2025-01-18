#include "UniverseObject.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include "Enums.h"
#include "Pathfinder.h"
#include "ScriptingContext.h"
#include "Condition.h"
#include "ValueRef.h"
#include "Special.h"
#include "System.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/i18n.h"

namespace ValueRef {
    std::string_view MeterToName(const MeterType meter);
}

UniverseObject::UniverseObject(UniverseObjectType type, std::string name,
                               double x, double y, int owner_id, int creation_turn) :
    m_name(std::move(name)),
    m_owner_empire_id(owner_id),
    m_created_on_turn(creation_turn),
    m_x(x),
    m_y(y),
    m_type(type)
{}

UniverseObject::UniverseObject(UniverseObjectType type, std::string name, int owner_id, int creation_turn) :
    m_name(std::move(name)),
    m_owner_empire_id(owner_id),
    m_created_on_turn(creation_turn),
    m_type(type)
{}

assignable_blocking_combiner::assignable_blocking_combiner(const Universe& universe) :
    blocking([&universe]() -> bool { return universe.UniverseObjectSignalsInhibited(); })
{}

void UniverseObject::SetSignalCombiner(const Universe& universe)
{ StateChangedSignal.set_combiner(CombinerType{universe}); }

void UniverseObject::Copy(const UniverseObject& copied_object,
                          Visibility vis, const std::set<std::string>& visible_specials,
                          const Universe&)
{
    if (&copied_object == this)
        return;

    static constexpr Meter DEFAULT_METER;

    auto censored_meters = copied_object.CensoredMeters(vis);
    for (const auto type : copied_object.m_meters | range_keys) {
        // if there is an update to meter from censored meters, update this object's copy
        auto censored_it = censored_meters.find(type);
        const bool have_censored_meter = censored_it != censored_meters.end();
        const Meter& copied_object_meter = have_censored_meter ? censored_it->second : DEFAULT_METER;

        // get existing meter in this object, or insert a copy
        auto [this_meter_it, inserted_new] = m_meters.try_emplace(type, copied_object_meter);
        if (!have_censored_meter || inserted_new)
            continue;

        // don't overwrite previously-known meter value history with sentinel values used for insufficiently visible objects
        if (copied_object_meter == Meter{Meter::LARGE_VALUE, Meter::LARGE_VALUE})
            continue;

        // some new info available, so can overwrite only meter info
        Meter& this_meter = this_meter_it->second;
        this_meter = copied_object_meter;
    }


    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_type =                  copied_object.m_type;
        this->m_id =                    copied_object.m_id;
        this->m_system_id =             copied_object.m_system_id;
        this->m_x =                     copied_object.m_x;
        this->m_y =                     copied_object.m_y;

        this->m_specials.clear();
        this->m_specials.reserve(copied_object.m_specials.size());
        for (const auto& [entry_special_name, entry_special] : copied_object.m_specials) {
            if (visible_specials.contains(entry_special_name))
                this->m_specials[entry_special_name] = entry_special;
        }

        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            this->m_owner_empire_id =   copied_object.m_owner_empire_id;
            this->m_created_on_turn =   copied_object.m_created_on_turn;

            if (vis >= Visibility::VIS_FULL_VISIBILITY)
                this->m_name =          copied_object.m_name;
        }
    }
}

int UniverseObject::AgeInTurns(int current_turn) const noexcept {
    if (m_created_on_turn == BEFORE_FIRST_TURN)
        return SINCE_BEFORE_TIME_AGE;
    if ((m_created_on_turn == INVALID_GAME_TURN) || (current_turn == INVALID_GAME_TURN))
        return INVALID_OBJECT_AGE;
    return current_turn - m_created_on_turn;
}

bool UniverseObject::HasSpecial(std::string_view name) const {
    return std::any_of(m_specials.begin(), m_specials.end(),
                       [name](const auto& s) { return name == s.first; });
}

int UniverseObject::SpecialAddedOnTurn(std::string_view name) const {
    auto it = std::find_if(m_specials.begin(), m_specials.end(),
                           [name](const auto& s) { return name == s.first; });
    return (it == m_specials.end()) ? INVALID_GAME_TURN : it->second.first;
}

float UniverseObject::SpecialCapacity(std::string_view name) const {
    auto it = std::find_if(m_specials.begin(), m_specials.end(),
                           [name](const auto& s) { return name == s.first; });
    return (it == m_specials.end()) ? 0.0f : it->second.second;
}

std::string UniverseObject::Dump(uint8_t ntabs) const {
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const auto& universe = context.ContextUniverse();
    const auto& objects = context.ContextObjects();
    auto system = objects.get<System>(this->SystemID());

    std::string retval;
    retval.reserve(2048); // guesstimate
    retval.append(to_string(m_type)).append(" ")
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
        int near_id = universe.GetPathfinder().NearestSystemTo(this->X(), this->Y(), objects);
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
        auto empire = context.GetEmpire(m_owner_empire_id);
        retval.append(" owner: ").append(empire ? empire->Name() : "(Unknown Empire)");
    }
    retval.append(" created on turn: ").append(std::to_string(m_created_on_turn))
          .append(" specials: ");
    for (auto& [special_name, turn_amount] : m_specials)
        retval.append("(").append(special_name).append(", ")
              .append(std::to_string(turn_amount.first)).append(", ")
              .append(std::to_string(turn_amount.second)).append(") ");
    retval.append("  Meters: ");
    for (auto& [meter_type, meter] : m_meters)
        retval.append(ValueRef::MeterToName(meter_type)).append(": ").append(meter.Dump().data()).append("  ");
    return retval;
}

UniverseObject::IDSet UniverseObject::VisibleContainedObjectIDs(int empire_id, const EmpireObjectVisMap& vis) const {
    auto object_id_visible = [empire_id, &vis](int object_id) -> bool {
        auto empire_it = vis.find(empire_id);
        if (empire_it == vis.end())
            return false;
        auto obj_it = empire_it->second.find(object_id);
        return obj_it != empire_it->second.end() &&
            obj_it->second >= Visibility::VIS_BASIC_VISIBILITY;
    };

    IDSet retval;
    retval.reserve(ContainedObjectIDs().size());
    for (int object_id : ContainedObjectIDs()) {
        if (object_id_visible(object_id))
            retval.insert(object_id);
    }
    return retval;
}

const Meter* UniverseObject::GetMeter(MeterType type) const noexcept {
    if constexpr (noexcept(m_meters.find(type))) {
        const auto it = m_meters.find(type);
        if (it != m_meters.end())
            return &(it->second);
    } else {
        const auto end_it = m_meters.end();
        for (auto it = m_meters.begin(); it != end_it; ++it)
            if (it->first == type)
                return &it->second;
    }
    return nullptr;
}

Visibility UniverseObject::GetVisibility(int empire_id, const EmpireIDtoObjectIDtoVisMap& v) const {
    auto empire_it = v.find(empire_id);
    if (empire_it == v.end())
        return Visibility::VIS_NO_VISIBILITY;
    auto obj_it = empire_it->second.find(m_id);
    return (obj_it == empire_it->second.end()) ? Visibility::VIS_NO_VISIBILITY : obj_it->second;
}

Visibility UniverseObject::GetVisibility(int empire_id, const Universe& u) const
{ return GetVisibility(empire_id, u.GetEmpireObjectVisibility()); }

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

void UniverseObject::MoveTo(const UniverseObject* object) {
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

Meter* UniverseObject::GetMeter(MeterType type) noexcept {
    if constexpr (noexcept(m_meters.find(type))) {
        const auto it = m_meters.find(type);
        if (it != m_meters.end())
            return &(it->second);
    } else {
        const auto end_it = m_meters.end();
        for (auto it = m_meters.begin(); it != end_it; ++it)
            if (it->first == type)
                return &it->second;
    }
    return nullptr;
}

void UniverseObject::BackPropagateMeters() noexcept {
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
     * to call empire->AddExploredSystem(system_id, context.current_turn, context.ContextObjects()); */
}

void UniverseObject::SetSystem(int sys) {
    //DebugLogger() << "UniverseObject::SetSystem(int sys)";
    if (sys != m_system_id) {
        m_system_id = sys;
        StateChangedSignal();
    }
}

void UniverseObject::AddSpecial(std::string name, float capacity, int turn)
{ m_specials[std::move(name)] = std::pair{turn, capacity}; }

void UniverseObject::SetSpecialCapacity(std::string name, float capacity, int turn) {
    auto it = m_specials.find(name);
    if (it != m_specials.end())
        it->second.second = capacity;
    else
        m_specials.emplace(std::piecewise_construct,
                           std::forward_as_tuple(std::move(name)),
                           std::forward_as_tuple(turn, capacity));
}

std::size_t UniverseObject::SizeInMemory() const {
    std::size_t retval = 0;
    retval += sizeof(UniverseObject);
    retval += sizeof(MeterMap::value_type)*m_meters.capacity();
    retval += sizeof(SpecialMap::value_type)*m_specials.capacity();
    for (const auto& name : m_specials | range_keys)
        retval += sizeof(std::decay_t<decltype(name)>::value_type)*name.capacity();
    return retval;
}

void UniverseObject::RemoveSpecial(const std::string& name)
{ m_specials.erase(name); }

UniverseObject::MeterMap UniverseObject::CensoredMeters(Visibility vis) const {
    if (vis >= Visibility::VIS_PARTIAL_VISIBILITY)
        return m_meters;
    else if (vis == Visibility::VIS_BASIC_VISIBILITY && m_meters.contains(MeterType::METER_STEALTH))
        return MeterMap{{MeterType::METER_STEALTH, Meter{Meter::LARGE_VALUE, Meter::LARGE_VALUE}}};
    return {};
}

void UniverseObject::ResetTargetMaxUnpairedMeters() {
    if constexpr (noexcept(m_meters.find(MeterType::METER_STEALTH))) {
        const auto it = m_meters.find(MeterType::METER_STEALTH);
        if (it != m_meters.end())
            it->second.ResetCurrent();
    } else {
        const auto end_it = m_meters.end();
        for (auto it = m_meters.begin(); it != end_it; ++it)
            if (it->first == MeterType::METER_STEALTH)
                return it->second.ResetCurrent();
    }
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
