
#ifndef _TechManager_h_
#define _TechManager_h_

#include <map>

#ifndef _TechLevel_h_
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
*   Only the TechManager should create TechLevel objects.
*/
class TechManager
{
public:
    /** \name Iterator Types */ //@{
    typedef std::map<int, TechLevel*>::iterator iterator;
    typedef std::map<int, TechLevel*>::const_iterator const_iterator;

   //@}
    
    /** \name Accessors */ //@{
    /// Provides access to a single instance of the TechManager class.
    static TechManager& instance();
   
    /// Returns a const iterator to the first TechLevel
    /**
       begin and end allow 
       iteration over the techlevels in this manager
    */
    const_iterator begin() const;
    /// Returns a const iterator past the last TechLevel
    /**
       begin and end allow 
       iteration over the techlevels in this manager
    */
    const_iterator end() const;
    
    
   
     /// Returns a pointer to the TechLevel with the given ID.
     /**
       method to get at a TechLevel object given its ID.
       Returns a pointer to the tech with the given ID, or NULL
       if no such tech exists in the manager.  This is the const version.
    */
    const TechLevel* Lookup(int ID) const;
    /// Returns a pointer to the TechLevel with the given ID.
    /**
       method to get at a TechLevel object given its ID.
       Returns a pointer to the tech with the given ID, or NULL
       if no such tech exists in the manager
    */
    TechLevel* Lookup(int ID);
  
    /**
       begin and end allow 
       iteration over the techlevels in this manager
    */
    iterator begin();
    /**
       begin and end allow 
       iteration over the techlevels in this manager
    */
    iterator end();

    //@}


    /** \name Mutators */ //@{
    
    /// For version 0.1, creates hardcoded techlevels.
    /**
       reads a file containing technology items and populates itself
       with the technologies in the file.  For V0.1, this will simply
       create the hardcoded technology levels described in TechLevel.h
    */
    bool LoadTechTree(const std::string& TechFileName);

    /// Empties the manager and deallocates all managed objects.
    /**
       Removes all technologies in the TechManager and deallocates them
       Any pointers, iterators, or references to TechLevel objects in 
       this manager will be invalidated
    */
    void ClearAll();
    
    //@}

protected:
    
    /** \name Constructors */ //@{
    /// Constructs an empty manager.
    /**
    * Constructor.  Constructs an empty manager
    */
    TechManager();
    //@}
    

private:

    std::map<int, TechLevel*> m_tech_map;
    int m_next_id;

};


#endif
