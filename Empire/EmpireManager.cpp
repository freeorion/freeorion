#include "EmpireManager.h"

#include "../util/MultiplayerCommon.h"


const std::string EmpireManager::EMPIRE_UPDATE_TAG = "EmpireUpdate";

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
    return it == end() ? 0 : it->second;
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

XMLElement EmpireManager::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/)
{
    XMLElement retval("EmpireManager");
    for (EmpireManager::iterator it = begin(); it != end(); ++it) {
        retval.AppendChild(XMLElement("Empire" + boost::lexical_cast<std::string>(it->second->EmpireID()), it->second->XMLEncode(empire_id)));
    }
    return retval;
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

