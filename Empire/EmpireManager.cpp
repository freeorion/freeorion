
#ifndef _FREEORION_EMPIREMANAGER_H_
#include "EmpireManager.h"
#endif

/**  Constructors */ 
EmpireManager::EmpireManager()
{
    // nothing to do yet
}
   
   
/** Destructors */
EmpireManager::~EmpireManager()
{
    // kill 'em all!
    EmpireItr itr = EmpireBegin();
    while(itr != EmpireEnd())
    {
        delete (*itr).second;
        itr++;
    }
}
   
/**  Const Iterators */ 

/**
* EmpireBegin returns an const iterator pointing at the first empire
* in the manager.  
*/
EmpireManager::ConstEmpireItr EmpireManager::EmpireBegin() const
{
    return m_empire_map.begin();
}
/**
* EmpireEnd returns an iterator which is past the end of
* the list of empires.  
*/
EmpireManager::ConstEmpireItr EmpireManager::EmpireEnd() const
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
    ConstEmpireItr itr = m_empire_map.find(ID);

    if(itr == EmpireEnd())
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
    EmpireItr itr = m_empire_map.find(ID);

    if(itr == EmpireEnd())
    {
        return NULL;
    }
    
    return (*itr).second;
}
  
  
  
/**  Non-Const Iterators */ 

/**
* EmpireBegin returns an iterator pointing at the first empire
* in the manager.  
*/
EmpireManager::EmpireItr EmpireManager::EmpireBegin()
{
    return m_empire_map.begin();
}
 /**
* EmpireEnd returns an iterator which is past the end of
* the list of empires.  
*/
EmpireManager::EmpireItr EmpireManager::EmpireEnd()
{
    return m_empire_map.end();
}


/**
* UpdateEmpireStatus changes the name, color, or control status
* of the empire whose ID equals empireID.  Returns true if successful
* false if not.  If the empire manager does not have an empire object 
* for the specified empire it will create one, otherwise it will update 
* it's data.  
*/
bool EmpireManager::UpdateEmpireStatus(int empireID, 
                        std::string &name, 
                        GG::Clr color, 
                        Empire::ControlStatus control)
{
    Empire* emp = Lookup(empireID);
    
    if(emp != NULL)
    {
        emp->Name(name);
        emp->Color(color);
        emp->ControlState(control);
    }
    else
    {
        emp = new Empire(name, empireID, color, control);
        AddEmpireToManager(emp);
    } 
    
    return true;
}
  

  
/**
* AddEmpire adds the given empire to the manager's map
* 
*/
void EmpireManager::AddEmpireToManager(Empire* empire)
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
void EmpireManager::RemoveEmpireFromManager(Empire* empire)
{
    if(empire != NULL)
    {
        m_empire_map.erase(empire->EmpireID());
    }
}

