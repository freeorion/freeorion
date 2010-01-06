#include "EmpireManager.h"

#include "Empire.h"
#include "../universe/Planet.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"


EmpireManager::~EmpireManager()
{
    Clear();
}

const EmpireManager& EmpireManager::operator=(EmpireManager& rhs)
{
    Clear();
    m_empire_map = rhs.m_empire_map;
    rhs.m_empire_map.clear();
    return *this;
}

const Empire* EmpireManager::Lookup(int id) const
{
    const_iterator it = m_empire_map.find(id);
    return it == m_empire_map.end() ? 0 : it->second;
}

EmpireManager::const_iterator EmpireManager::begin() const
{
    return m_empire_map.begin();
}

EmpireManager::const_iterator EmpireManager::end() const
{
    return m_empire_map.end();
}

bool EmpireManager::Eliminated(int id) const
{
    return m_eliminated_empires.find(id) != m_eliminated_empires.end();
}

Empire* EmpireManager::Lookup(int id)
{
    iterator it = m_empire_map.find(id);
    return it == end() ? 0 : it->second;
}

EmpireManager::iterator EmpireManager::begin()
{
    return m_empire_map.begin();
}

EmpireManager::iterator EmpireManager::end()
{
    return m_empire_map.end();
}

void EmpireManager::EliminateEmpire(int id)
{
    if (Empire* emp = Lookup(id)) {
        emp->EliminationCleanup();
        m_eliminated_empires.insert(id);
    } else {
        Logger().errorStream() << "Tried to eliminate nonexistant empire with ID " << id;
    }
}

Empire* EmpireManager::CreateEmpire(int id, const std::string& name, const std::string& player_name, const GG::Clr& color, int planet_ID)
{
    Empire* empire = new Empire(name, player_name, id, color, planet_ID);

    const ObjectMap& objects = GetUniverse().Objects();
    if (const Planet* planet = objects.Object<Planet>(planet_ID)) {
        int sys_id = planet->SystemID();
        if (sys_id != UniverseObject::INVALID_OBJECT_ID)
            empire->AddExploredSystem(sys_id);
    } else {
        Logger().errorStream() << "EmpireManager::CreateEmpire passed invalid planet id (" << planet_ID << ")";
    }

    InsertEmpire(empire);

    return empire;
}

void EmpireManager::InsertEmpire(Empire* empire)
{
    if (empire) {
        assert(!m_empire_map[empire->EmpireID()]);
        m_empire_map[empire->EmpireID()] = empire;
    }
}

void EmpireManager::Clear()
{
    for (EmpireManager::iterator it = begin(); it != end(); ++it) {
        delete it->second;
    }
    m_empire_map.clear();
    m_eliminated_empires.clear();
}
