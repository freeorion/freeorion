#include "EmpireManager.h"

#include "../util/MultiplayerCommon.h"


namespace {
    bool temp_header_bool = RecordHeaderFile(EmpireManagerRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


const std::string EmpireManager::EMPIRE_UPDATE_TAG = "EmpireUpdate";

/**  Constructors */ 
EmpireManager::EmpireManager()
{
    // nothing to do yet
}
   
   
/** Destructors */
EmpireManager::~EmpireManager()
{
    // kill 'em all!
    EmpireManager::iterator itr = begin();
    while(itr != end())
    {
        delete (*itr).second;
        itr++;
    }
}
   
/**  Const Iterators */ 

EmpireManager::const_iterator EmpireManager::begin() const
{
    return m_empire_map.begin();
}

EmpireManager::const_iterator EmpireManager::end() const
{
    return m_empire_map.end();
}


/**  Empire Lookup By ID */ 

/**
* Lookup will look up an empire by its EmpireID
* and return a pointer to that empire, if it exists in teh manager
*  or NULL if it does not.  (This one is the const version)
*/
const Empire* EmpireManager::Lookup(int ID) const
{
    const_iterator itr = m_empire_map.find(ID);

    if(itr == end())
    {
        return NULL;
    }
    
    return (*itr).second;
    
}
/**
* Lookup will look up an empire by its EmpireID
* and return a pointer to that empire, if it exists in teh manager
*  or NULL if it does not
*/
Empire* EmpireManager::Lookup(int ID)
{
    iterator itr = m_empire_map.find(ID);

    if(itr == end())
    {
        return NULL;
    }
    
    return (*itr).second;
}
  
  
  
/**  Non-Const Iterators */ 

EmpireManager::iterator EmpireManager::begin()
{
    return m_empire_map.begin();
}

EmpireManager::iterator EmpireManager::end()
{
    return m_empire_map.end();
}


bool EmpireManager::UpdateEmpireStatus(int empireID, 
                        std::string &name, 
                        GG::Clr color)
{
    Empire* emp = Lookup(empireID);
    
    if(emp != NULL)
    {
        emp->SetName(name);
        emp->SetColor(color);
        
        return true;
    }
    else
    {
        return false;
    }
   
}
  

  
/**
* InsertEmpire adds the given empire to the manager's map
* 
*/
void EmpireManager::InsertEmpire(Empire* empire)
{
    if(empire != NULL)
    {
        std::pair<int, Empire*> newpair(empire->EmpireID(), empire);
        m_empire_map.insert(newpair);
    }
}

/**
* RemoveEmpire removes the given empire from the manager's map
*/
void EmpireManager::RemoveEmpire(Empire* empire)
{
    if(empire != NULL)
    {
        m_empire_map.erase(empire->EmpireID());
    }
}

void EmpireManager::RemoveAllEmpires()
{
    for(EmpireManager::iterator itr = begin(); itr != end(); itr++)
        delete itr->second;
    m_empire_map.clear();
}

