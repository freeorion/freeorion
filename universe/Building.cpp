#include "Building.h"

#include "BuildingType.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"


Building::Building(int empire_id, std::string building_type, int produced_by_empire_id, int creation_turn) :
    UniverseObject{UniverseObjectType::OBJ_BUILDING, "", empire_id, creation_turn},
    m_building_type(std::move(building_type)),
    m_produced_by_empire_id(produced_by_empire_id)
{
    const BuildingType* type = GetBuildingType(m_building_type);
    Rename(type ? UserString(type->Name()) : UserString("ENC_BUILDING"));
}

std::shared_ptr<UniverseObject> Building::Clone(const Universe& universe, int empire_id) const {
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    auto retval = std::make_shared<Building>();
    retval->Copy(*this, universe, empire_id);
    return retval;
}

void Building::Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id) {
    if (&copied_object == this)
        return;
    if (copied_object.ObjectType() != UniverseObjectType::OBJ_BUILDING) {
        ErrorLogger() << "Building::Copy passed an object that wasn't a Building";
        return;
    }

    Copy(static_cast<const Building&>(copied_object), universe, empire_id);
}

void Building::Copy(const Building& copied_building, const Universe& universe, int empire_id) {
    if (&copied_building == this)
        return;

    const int copied_object_id = copied_building.ID();
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    const auto visible_specials = universe.GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_building, vis, visible_specials, universe);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_planet_id =                 copied_building.m_planet_id;

        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            this->m_name =                  copied_building.m_name;

            this->m_building_type =         copied_building.m_building_type;
            this->m_produced_by_empire_id = copied_building.m_produced_by_empire_id;

            if (vis >= Visibility::VIS_FULL_VISIBILITY)
                this->m_ordered_scrapped =  copied_building.m_ordered_scrapped;
        }
    }
}

bool Building::HostileToEmpire(int empire_id, const EmpireManager& empires) const {
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
        empires.GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

UniverseObject::TagVecs Building::Tags() const {
    const BuildingType* type = ::GetBuildingType(m_building_type);
    return type ? TagVecs{type->Tags()} : TagVecs{};
}

bool Building::HasTag(std::string_view name) const {
    const BuildingType* type = GetBuildingType(m_building_type);
    return type && type->HasTag(name);
}

bool Building::ContainedBy(int object_id) const noexcept {
    return object_id != INVALID_OBJECT_ID
        && (    object_id == m_planet_id
            ||  object_id == this->SystemID());
}

std::size_t Building::SizeInMemory() const {
    std::size_t retval = UniverseObject::SizeInMemory();
    retval += sizeof(Building) - sizeof(UniverseObject);

    retval += sizeof(decltype(m_building_type)::value_type)*m_building_type.capacity();

    return retval;
}

std::string Building::Dump(uint8_t ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " building type: " << m_building_type
       << " produced by empire id: " << m_produced_by_empire_id;
    return os.str();
}

void Building::SetPlanetID(int planet_id) {
    if (planet_id != m_planet_id) {
        m_planet_id = planet_id;
        StateChangedSignal();
    }
}

void Building::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    //// give buildings base stealth slightly above 0, so that they can't be seen from a distance without high detection ability
    //if (Meter* stealth = GetMeter(MeterType::METER_STEALTH))
    //    stealth->AddToCurrent(0.01f);
}

void Building::Reset() {
    UniverseObject::SetOwner(ALL_EMPIRES);
    m_ordered_scrapped = false;
}

void Building::SetOrderedScrapped(bool b) {
    bool initial_status = m_ordered_scrapped;
    if (b == initial_status) return;
    m_ordered_scrapped = b;
    StateChangedSignal();
}

