#include "ModeratorAction.h"

#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "AppInterface.h"
#include "Logger.h"
#include "i18n.h"

#include <boost/serialization/export.hpp>
#include <boost/filesystem/fstream.hpp>

// Some of the moderator actions are very similar to effects, the result of
// completing a build, and certain orders such as creating a new fleet.
// TODO: maybe eliminate duplication

/////////////////////////////////////////////////////
// Moderator::DestroyUniverseObject
/////////////////////////////////////////////////////
Moderator::DestroyUniverseObject::DestroyUniverseObject(int object_id) :
    m_object_id(object_id)
{}

void Moderator::DestroyUniverseObject::Execute() const {
    const auto& empire_ids = Empires().EmpireIDs();
    GetUniverse().RecursiveDestroy(m_object_id, empire_ids);
    GetUniverse().InitializeSystemGraph(Empires());
}

std::string Moderator::DestroyUniverseObject::Dump() const
{ return "Moderator::DestroyUniverseObject object_id = " + std::to_string(m_object_id); }

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
    auto obj = Objects().get(m_object_id);
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
    auto sys1 = Objects().get<System>(m_id_1);
    if (!sys1) {
        ErrorLogger() << "Moderator::AddStarlane::Execute couldn't get system with id: " << m_id_1;
        return;
    }
    auto sys2 = Objects().get<System>(m_id_2);
    if (!sys2) {
        ErrorLogger() << "Moderator::AddStarlane::Execute couldn't get system with id: " << m_id_2;
        return;
    }
    sys1->AddStarlane(m_id_2);
    sys2->AddStarlane(m_id_1);
    GetUniverse().InitializeSystemGraph(Empires());
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
    auto sys1 = Objects().get<System>(m_id_1);
    if (!sys1) {
        ErrorLogger() << "Moderator::RemoveStarlane::Execute couldn't get system with id: " << m_id_1;
        return;
    }
    auto sys2 = Objects().get<System>(m_id_2);
    if (!sys2) {
        ErrorLogger() << "Moderator::RemoveStarlane::Execute couldn't get system with id: " << m_id_2;
        return;
    }
    sys1->RemoveStarlane(m_id_2);
    sys2->RemoveStarlane(m_id_1);
    GetUniverse().InitializeSystemGraph(Empires());
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
    m_star_type(StarType::STAR_NONE)
{}

Moderator::CreateSystem::CreateSystem(double x, double y, StarType star_type) :
    m_x(x),
    m_y(y),
    m_star_type(star_type)
{}

namespace {
    std::string GenerateSystemName(const ObjectMap& objects) {
        static std::vector<std::string> star_names = UserStringList("STAR_NAMES");

        // pick a name for the system
        for (const std::string& star_name : star_names) {
            // does an existing system have this name?
            bool dupe = false;
            for (auto* system : objects.allRaw<System>()) {
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
    auto* app = IApp::GetApp();
    const auto current_turn = app->CurrentTurn();
    auto& universe = app->GetUniverse();

    auto system = universe.InsertNew<System>(m_star_type, GenerateSystemName(universe.Objects()),
                                             m_x, m_y, current_turn);
    universe.InitializeSystemGraph(app->Empires());
    if (!system) {
        ErrorLogger() << "CreateSystem::Execute couldn't create system!";
        return;
    }
}

std::string Moderator::CreateSystem::Dump() const {
    std::string retval{"Moderator::CreateSystem x = "};
    retval.reserve(128); // guesstimate
    retval.append(std::to_string(m_x))
          .append(" y = ").append(std::to_string(m_y))
          .append(" star_type = ").append(to_string(m_star_type));
    return retval;
}

/////////////////////////////////////////////////////
// Moderator::CreatePlanet
/////////////////////////////////////////////////////
Moderator::CreatePlanet::CreatePlanet() :
    m_system_id(INVALID_OBJECT_ID),
    m_planet_type(PlanetType::PT_SWAMP),
    m_planet_size(PlanetSize::SZ_MEDIUM)
{}

Moderator::CreatePlanet::CreatePlanet(int system_id, PlanetType planet_type, PlanetSize planet_size) :
    m_system_id(system_id),
    m_planet_type(planet_type),
    m_planet_size(planet_size)
{}

void Moderator::CreatePlanet::Execute() const {
    auto* app = IApp::GetApp();
    const auto current_turn = app->CurrentTurn();
    auto& universe = app->GetUniverse();
    auto location = universe.Objects().get<System>(m_system_id);
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

    auto planet = universe.InsertNew<Planet>(m_planet_type, m_planet_size, current_turn);
    if (!planet) {
        ErrorLogger() << "CreatePlanet::Execute unable to create new Planet object";
        return;
    }

    const int orbit = *(free_orbits.begin());
    location->Insert(std::static_pointer_cast<UniverseObject>(std::move(planet)),
                     orbit, current_turn, universe.Objects());
}

std::string Moderator::CreatePlanet::Dump() const {
    std::string retval = "Moderator::CreatePlanet system_id = "
                       + std::to_string(m_system_id)
                       + " planet_type = ";
    retval.append(to_string(m_planet_type)).append(" planet_size = ").append(to_string(m_planet_size));
    return retval;
}
