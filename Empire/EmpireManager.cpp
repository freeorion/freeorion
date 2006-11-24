#include "EmpireManager.h"

#include "Empire.h"
#include "../universe/Planet.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"


EmpireManager::~EmpireManager()
{
    RemoveAllEmpires();
}

const EmpireManager& EmpireManager::operator=(EmpireManager& rhs)
{
    RemoveAllEmpires();
    m_empire_map = rhs.m_empire_map;
    rhs.m_empire_map.clear();
    return *this;
}

const Empire* EmpireManager::Lookup(int ID) const
{
    const_iterator it = m_empire_map.find(ID);
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

Empire* EmpireManager::Lookup(int ID)
{
    iterator it = m_empire_map.find(ID);
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

Empire* EmpireManager::CreateEmpire(int id, const std::string& name, const std::string& player_name, const GG::Clr& color, int planet_ID)
{
    Empire* empire = new Empire(name, player_name, id, color, planet_ID);
    Universe& universe = GetUniverse();
    Planet* planet = universe.Object<Planet>(planet_ID);
    empire->AddExploredSystem(planet->SystemID());
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

void EmpireManager::RemoveEmpire(Empire* empire)
{
    if (empire)
        m_empire_map.erase(empire->EmpireID());
}

void EmpireManager::RemoveAllEmpires()
{
    for (EmpireManager::iterator it = begin(); it != end(); ++it) {
        delete it->second;
    }
    m_empire_map.clear();
}

bool EmpireManager::EliminateEmpire(int id)
{
    Empire* emp = Lookup(id);
    RemoveEmpire(emp);
    delete emp;
    return emp;
}
