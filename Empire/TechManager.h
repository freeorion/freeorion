
#ifndef _FREEORION_TECHMANAGER_H_
#define _FREEORION_TECHMANAGER_H_

#include <map>

#ifndef _FREEORION_TECHLEVEL_H_
#include "TechLevel.h"
#endif

/**
*   Manager for technology advancements.  This maintains
*   a set of all the possible technological advancements
*   that are possible in FreeOrion.
*
*   This class will later be extended to track various kinds of things
*   such as building types, weapon types, etc, in addition to technology
*   levels. 
*
*   It supports methods for reading technology advancements from files
*
*   Only the TechManager should create TechLevel objects
*/
class TechManager
{
public:
    /** \name Iterator Types */ //@{
    typedef std::map<int, TechLevel*>::iterator TechItr;
    typedef std::map<int, TechLevel*>::const_iterator ConstTechItr;
    //@}
    
    /** \name Constructors */ //@{
    /**
    * Constructor.  Constructs an empty manager
    */
    TechManager();
    //@}
    
    /** \name Const Iterators */ //@{
    /**
       TechBegin and TechEnd allow 
       iteration over the techlevels in this manager
    */
    ConstTechItr TechBegin() const;
    /**
       ConstTechBegin and ConstTechEnd allow 
       iteration over the techlevels in this manager
    */
    ConstTechItr TechEnd() const;
    //@}
    
    /** \name Technology Lookup By ID */ //@{
     /**
       method to get at a TechLevel object given its ID.
       Returns a pointer to the tech with the given ID, or NULL
       if no such tech exists in the manager.  This is the const version.
    */
    const TechLevel* Lookup(int ID) const;
    /**
       method to get at a TechLevel object given its ID.
       Returns a pointer to the tech with the given ID, or NULL
       if no such tech exists in the manager
    */
    TechLevel* Lookup(int ID);
   //@}


   /** \name Non-Const Iterators */ //@{
    /**
       TechBegin and TechEnd allow 
       iteration over the techlevels in this manager
    */
    TechItr TechBegin();
    /**
       TechBegin and TechEnd allow 
       iteration over the techlevels in this manager
    */
    TechItr TechEnd();
    //@}


    /**
       reads a file containing technology items and populates itself
       with the technologies in the file.  For V0.1, this will simply
       create the hardcoded technology levels described in TechLevel.h
    */
    bool LoadTechTree(const std::string& TechFileName);

    /**
       Removes all technologies in the TechManager and deallocates them
       Any pointers, iterators, or references to TechLevel objects in 
       this manager will be invalidated
    */
    void ClearAll();

private:
    std::map<int, TechLevel*> m_tech_map;
    int m_next_id;

};


#endif
