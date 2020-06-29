#include "Building.h"

#include "BuildingType.h"
#include "Enums.h"
#include "UniverseObjectVisitor.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"


Building::Building(int empire_id, std::string const& building_type, int produced_by_empire_id) :
    m_building_type(building_type),
    m_produced_by_empire_id(produced_by_empire_id)
{
    UniverseObject::SetOwner(empire_id);
    const BuildingType* type = GetBuildingType(m_building_type);
    if (type)
        Rename(UserString(type->Name()));
    else
        Rename(UserString("ENC_BUILDING"));

    UniverseObject::Init();
}

Building::Building() = default;

Building::~Building() = default;

auto Building::Clone(int empire_id) const -> Building*
{
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return nullptr;

    Building* retval = new Building();
    retval->Copy(shared_from_this(), empire_id);
    return retval;
}

void Building::Copy(std::shared_ptr<UniverseObject const> copied_object, int empire_id)
{
    if (copied_object.get() == this)
        return;
    auto copied_building = std::dynamic_pointer_cast<const Building>(copied_object);
    if (!copied_building) {
        ErrorLogger() << "Building::Copy passed an object that wasn't a Building";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_planet_id =                 copied_building->m_planet_id;

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            this->m_name =                      copied_building->m_name;

            this->m_building_type =             copied_building->m_building_type;
            this->m_produced_by_empire_id = copied_building->m_produced_by_empire_id;

            if (vis >= VIS_FULL_VISIBILITY) {
                this->m_ordered_scrapped =      copied_building->m_ordered_scrapped;
            }
        }
    }
}

auto Building::HostileToEmpire(int empire_id) const -> bool
{
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
           Empires().GetDiplomaticStatus(Owner(), empire_id) == DIPLO_WAR;
}

auto Building::Tags() const -> std::set<std::string>
{
    if (const BuildingType* type = ::GetBuildingType(m_building_type))
        return type->Tags();
    return {};
}

auto Building::HasTag(std::string const& name) const -> bool
{
    const BuildingType* type = GetBuildingType(m_building_type);
    return type && type->Tags().count(name);
}

auto Building::ObjectType() const -> UniverseObjectType
{ return OBJ_BUILDING; }

auto Building::ContainedBy(int object_id) const -> bool
{
    return object_id != INVALID_OBJECT_ID
        && (    object_id == m_planet_id
            ||  object_id == this->SystemID());
}

auto Building::Dump(unsigned short ntabs) const -> std::string
{
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " building type: " << m_building_type
       << " produced by empire id: " << m_produced_by_empire_id;
    return os.str();
}

auto Building::Accept(UniverseObjectVisitor const& visitor) const -> std::shared_ptr<UniverseObject>
{ return visitor.Visit(std::const_pointer_cast<Building>(std::static_pointer_cast<const Building>(shared_from_this()))); }

void Building::SetPlanetID(int planet_id)
{
    if (planet_id != m_planet_id) {
        m_planet_id = planet_id;
        StateChangedSignal();
    }
}

void Building::ResetTargetMaxUnpairedMeters()
{ UniverseObject::ResetTargetMaxUnpairedMeters(); }

void Building::Reset()
{
    UniverseObject::SetOwner(ALL_EMPIRES);
    m_ordered_scrapped = false;
}

void Building::SetOrderedScrapped(bool b)
{
    bool initial_status = m_ordered_scrapped;
    if (b == initial_status) return;
    m_ordered_scrapped = b;
    StateChangedSignal();
}
