#include "TechManager.h"

TechManager& TechManager::instance()
{
    static TechManager it;
    return it;
}


TechManager::TechManager() : 
    m_next_id(1)
{

}


TechManager::const_iterator TechManager::begin() const
{
    return m_tech_map.begin();
}

TechManager::const_iterator TechManager::end() const
{
    return m_tech_map.end();
}

/** Technology Lookup By ID */ 
 /**
   method to get at a Tech object given its ID.
   Returns a pointer to the tech with the given ID, or NULL
   if no such tech exists in the manager.  This is the const version.
*/
const Tech* TechManager::Lookup(int ID) const
{
    const_iterator itr = m_tech_map.find(ID);
    
    if(itr == end())
    {
        return NULL;
    }
    else
    {
        return (*itr).second;
    }
}
/**
   method to get at a Tech object given its ID.
   Returns a pointer to the tech with the given ID, or NULL
   if no such tech exists in the manager
*/
Tech* TechManager::Lookup(int ID)
{
    iterator itr = m_tech_map.find(ID);
    
    if(itr == end())
    {
        return NULL;
    }
    else
    {
        return (*itr).second;
    }
}



TechManager::iterator TechManager::begin() 
{
    return m_tech_map.begin();
}

TechManager::iterator TechManager::end() 
{
    return m_tech_map.end();
}



/**
   reads a file containing technology items and populates itself
   with the technologies in the file.  For V0.2, this will simply
   create the hardcoded technology levels described in Tech.h
*/
bool TechManager::LoadTechTree(const std::string& TechFileName)
{
    // make hardcoded techlevels as per v0.2 requirements document
    Tech* mark2 = new Tech(((int)Tech::TECH_MARK2), std::string("Mark II"), 1750);
    Tech* dbase = new Tech(((int)Tech::TECH_BASE),  std::string("Defense Base"), 1000);
    Tech* mark3 = new Tech(((int)Tech::TECH_MARK3), std::string("Mark III"), 3500);
    Tech* mark4 = new Tech(((int)Tech::TECH_MARK4), std::string("Mark IV"), 7000);
    
    std::pair<int, Tech*> the_pair;
    
    the_pair.first = (int)Tech::TECH_MARK2;
    the_pair.second = mark2;
    m_tech_map.insert(the_pair);
    
    the_pair.first = (int)Tech::TECH_BASE;
    the_pair.second = dbase;
    m_tech_map.insert(the_pair);
    
    the_pair.first = (int)Tech::TECH_MARK3;
    the_pair.second = mark3;
    m_tech_map.insert(the_pair);
    
    the_pair.first = (int)Tech::TECH_MARK4;
    the_pair.second = mark4;
    m_tech_map.insert(the_pair);
    
    return true;
}

/**
   Removes all technologies in the TechManager and deallocates them
   Any pointers, iterators, or references to Tech objects in 
   this manager will be invalidated
*/
void TechManager::ClearAll()
{
    m_tech_map.clear();
}

