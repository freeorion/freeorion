// -*- C++ -*-
#ifndef _TechManager_h_
#define _TechManager_h_

#include <map>

#ifndef _Tech_h_
#include "TechLevel.h"
#endif

/**
*   Singleton object to manage technology advancements.  This maintains
*   a set of all the possible technological advancements
*   that are possible in FreeOrion.
*
*   This class will later be extended to track various kinds of things
*   such as building types, weapon types, etc, in addition to technology
*   levels. 
*
*   It will support methods for:
*       - constructing a technology database
*       - iterating over technologies and looking them up by ID
*    
*   Only the TechManager should create Tech objects.
*/
class TechManager
{
public:
    /** \name Iterator Types */ //@{
    typedef std::map<int, Tech*>::iterator iterator;
    typedef std::map<int, Tech*>::const_iterator const_iterator;
    //@}
    
    /** \name Accessors */ //@{
    /** Provides access to a single instance of the TechManager class. */ 
    static TechManager& instance();

    /** Returns a const iterator to the first Tech */
    const_iterator begin() const;
    
    /** Returns a const iterator past the last Tech */
    const_iterator end() const;

    /** Returns a pointer to the tech with the given ID, or NULL
        if no such tech exists in the manager.*/
    const Tech* Lookup(int ID) const;

    /** Returns a pointer to the tech with the given ID, or NULL
        if no such tech exists in the manager */
    Tech* Lookup(int ID);
  
    /** Returns an iterator to the first Tech */
    iterator begin();

    /** Returns an iterator past the last Tech */
    iterator end();
    //@}


    /** \name Mutators */ //@{
    // For version 0.1, creates hardcoded techlevels.
    /** reads a file containing technology items and populates itself
        with the technologies in the file.  For V0.1, this will simply
        create the hardcoded technology levels described in Tech.h */
    bool LoadTechTree(const std::string& TechFileName);

    /** Removes all technologies in the TechManager and deallocates them.
        Any pointers, iterators, or references to Tech objects in 
        this manager will be invalidated */
    void ClearAll();
    //@}

protected:
    /** \name Constructors */ //@{
    /** Constructs an empty manager. */
    TechManager();
    //@}

private:
    std::map<int, Tech*> m_tech_map;
    int m_next_id;
};

#endif
