#include "ModeratorAction.h"

#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Enums.h"
#include "Directories.h"
#include "Logger.h"
#include "i18n.h"

#include <boost/lexical_cast.hpp>
#include <boost/serialization/export.hpp>
#include <boost/filesystem/fstream.hpp>

// Some of the moderator actions are very similar to effects, the result of completing a build, and certain orders such as creating a new fleet.
// TODO: eliminate duplication

Moderator::ModeratorAction::ModeratorAction()
{}

/////////////////////////////////////////////////////
// Moderator::DestroyUniverseObject
/////////////////////////////////////////////////////
Moderator::DestroyUniverseObject::DestroyUniverseObject()
{}

Moderator::DestroyUniverseObject::DestroyUniverseObject(int object_id) :
    m_object_id(object_id)
{}

void Moderator::DestroyUniverseObject::Execute() const
{ GetUniverse().RecursiveDestroy(m_object_id); }

std::string Moderator::DestroyUniverseObject::Dump() const {
    std::string retval = "Moderator::DestroyUniverseObject object_id = "
                       + std::to_string(m_object_id);
    return retval;
}

/////////////////////////////////////////////////////
// Moderator::SetOwner
/////////////////////////////////////////////////////
Moderator::SetOwner::SetOwner()
{}

Moderator::SetOwner::SetOwner(int object_id, int new_owner_empire_id) :
    m_object_id(object_id),
    m_new_owner_empire_id(new_owner_empire_id)
{}

void Moderator::SetOwner::Execute() const {
    auto obj = GetUniverseObject(m_object_id);
    if (!obj) {
        ErrorLogger() << "Moderator::SetOwner::Execute couldn't get object with id: " << m_object_id;
        return;
    }
    obj->SetOwner(m_new_owner_empire_id);
}

std::string Moderator::SetOwner::Dump() const {
    std::string retval = "Moderator::SetOwner object_id = "
                       + std::to_string(m_object_id)
                       + " new_owner_empire_id = "
                       + std::to_string(m_new_owner_empire_id);
    return retval;
}

/////////////////////////////////////////////////////
// Moderator::AddStarlane
/////////////////////////////////////////////////////
Moderator::AddStarlane::AddStarlane() :
    m_id_1(INVALID_OBJECT_ID),
    m_id_2(INVALID_OBJECT_ID)
{}

Moderator::AddStarlane::AddStarlane(int system_1_id, int system_2_id) :
    m_id_1(system_1_id),
    m_id_2(system_2_id)
{}

void Moderator::AddStarlane::Execute() const {
    std::shared_ptr<System> sys1 = GetSystem(m_id_1);
    if (!sys1) {
        ErrorLogger() << "Moderator::AddStarlane::Execute couldn't get system with id: " << m_id_1;
        return;
    }
    std::shared_ptr<System> sys2 = GetSystem(m_id_2);
    if (!sys2) {
        ErrorLogger() << "Moderator::AddStarlane::Execute couldn't get system with id: " << m_id_2;
        return;
    }
    sys1->AddStarlane(m_id_2);
    sys2->AddStarlane(m_id_1);
}

std::string Moderator::AddStarlane::Dump() const {
    std::string retval = "Moderator::AddStarlane system_id_1 = "
                       + std::to_string(m_id_1)
                       + " system_id_2 = "
                       + std::to_string(m_id_2);
    return retval;
}

/////////////////////////////////////////////////////
// Moderator::RemoveStarlane
/////////////////////////////////////////////////////
Moderator::RemoveStarlane::RemoveStarlane() :
    m_id_1(INVALID_OBJECT_ID),
    m_id_2(INVALID_OBJECT_ID)
{}

Moderator::RemoveStarlane::RemoveStarlane(int system_1_id, int system_2_id) :
    m_id_1(system_1_id),
    m_id_2(system_2_id)
{}

void Moderator::RemoveStarlane::Execute() const {
    std::shared_ptr<System> sys1 = GetSystem(m_id_1);
    if (!sys1) {
        ErrorLogger() << "Moderator::RemoveStarlane::Execute couldn't get system with id: " << m_id_1;
        return;
    }
    std::shared_ptr<System> sys2 = GetSystem(m_id_2);
    if (!sys2) {
        ErrorLogger() << "Moderator::RemoveStarlane::Execute couldn't get system with id: " << m_id_2;
        return;
    }
    sys1->RemoveStarlane(m_id_2);
    sys2->RemoveStarlane(m_id_1);
}

std::string Moderator::RemoveStarlane::Dump() const {
    std::string retval = "Moderator::RemoveStarlane system_id_1 = "
                       + std::to_string(m_id_1)
                       + " system_id_2 = "
                       + std::to_string(m_id_2);
    return retval;
}

/////////////////////////////////////////////////////
// Moderator::CreateSystem
/////////////////////////////////////////////////////
Moderator::CreateSystem::CreateSystem() :
    m_x(UniverseObject::INVALID_POSITION),
    m_y(UniverseObject::INVALID_POSITION),
    m_star_type(STAR_NONE)
{}

Moderator::CreateSystem::CreateSystem(double x, double y, StarType star_type) :
    m_x(x),
    m_y(y),
    m_star_type(star_type)
{}

namespace {
    std::string GenerateSystemName() {
        static std::vector<std::string> star_names = UserStringList("STAR_NAMES");

        const ObjectMap& objects = Objects();
        std::vector<std::shared_ptr<const System>> systems = objects.FindObjects<System>();

        // pick a name for the system
        for (const std::string& star_name : star_names) {
            // does an existing system have this name?
            bool dupe = false;
            for (auto& system : systems) {
                if (system->Name() == star_name) {
                    dupe = true;
                    break;  // another system has this name. skip to next potential name.
                }
            }
            if (!dupe)
                return star_name; // no systems have this name yet. use it.
        }
        return "";  // fallback to empty name.
    }
}

void Moderator::CreateSystem::Execute() const {
    auto system = GetUniverse().InsertNew<System>(m_star_type, GenerateSystemName(), m_x, m_y);
    if (!system) {
        ErrorLogger() << "CreateSystem::Execute couldn't create system!";
        return;
    }
}

std::string Moderator::CreateSystem::Dump() const {
    std::string retval = "Moderator::CreateSystem x = "
                       + std::to_string(m_x)
                       + " y = "
                       + std::to_string(m_y)
                       + " star_type = "
                       + boost::lexical_cast<std::string>(m_star_type);
    return retval;
}

/////////////////////////////////////////////////////
// Moderator::CreatePlanet
/////////////////////////////////////////////////////
Moderator::CreatePlanet::CreatePlanet() :
    m_system_id(INVALID_OBJECT_ID),
    m_planet_type(PT_SWAMP),
    m_planet_size(SZ_MEDIUM)
{}

Moderator::CreatePlanet::CreatePlanet(int system_id, PlanetType planet_type, PlanetSize planet_size) :
    m_system_id(system_id),
    m_planet_type(planet_type),
    m_planet_size(planet_size)
{}

void Moderator::CreatePlanet::Execute() const {
    std::shared_ptr<System> location = GetSystem(m_system_id);
    if (!location) {
        ErrorLogger() << "CreatePlanet::Execute couldn't get a System object at which to create the planet";
        return;
    }

    //  determine if and which orbits are available
    std::set<int> free_orbits = location->FreeOrbits();
    if (free_orbits.empty()) {
        ErrorLogger() << "CreatePlanet::Execute couldn't find any free orbits in system where planet was to be created";
        return;
    }

    auto planet = GetUniverse().InsertNew<Planet>(m_planet_type, m_planet_size);
    if (!planet) {
        ErrorLogger() << "CreatePlanet::Execute unable to create new Planet object";
        return;
    }

    int orbit = *(free_orbits.begin());
    location->Insert(std::shared_ptr<UniverseObject>(planet), orbit);
}

std::string Moderator::CreatePlanet::Dump() const {
    std::string retval = "Moderator::CreatePlanet system_id = "
                       + std::to_string(m_system_id)
                       + " planet_type = "
                       + boost::lexical_cast<std::string>(m_planet_type)
                       + " planet_size = "
                       + boost::lexical_cast<std::string>(m_planet_size);
    return retval;
}
