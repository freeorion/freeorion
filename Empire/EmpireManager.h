// -*- C++ -*-

#ifndef _EmpireManager_h_
#define _EmpireManager_h_

#include <list>
#include <map>
#include <string>

#ifndef _Empire_h_
#include "Empire.h"
#endif

#ifndef _GGClr_h_
#include "GGClr.h"
#endif

#ifndef _XMLDoc_h_
#include "XMLDoc.h"
#endif


/**
* Base class for Empire managers.  The Empire Manager
* maintains all of the Empire Objects that will exist in FreeOrion
*
*   There are client and server versions.  
*  - The server version is able to generate XML updates for clients
*  - The client version is able to process those updates
*
*/
class EmpireManager
{
public:
    /** \name Iterator types */ //@{
    /// Iterator over Empires
    typedef std::map<int, Empire*>::iterator iterator; 

    /// Const Iterator over Empires
    typedef std::map<int, Empire*>::const_iterator const_iterator;
    //@}

    /** \name Constructors */ //@{
    /// Default Constructor
    EmpireManager();
    //@}

    /** \name Destructors */ //@{
    /// Virtual Destructor - Deallocates all Empires
    /** Deallocates all empires that exist in the manager. */
    virtual ~EmpireManager();
    //@}

    /** \name Const Iterators */ //@{
    /**
    * Returns a const iterator pointing at the first empire
    * in the manager.  
    */
    const_iterator begin() const;
    /**
    * Returns a const iterator which is past the end of
    * the list of empires.  
    */
    const_iterator end() const;
    //@}

    /** \name Empire Lookup By ID */ //@{
    
    /// Provides read-only access to empires stored in the manager.
    /**
    * Lookup will look up an empire by its EmpireID
    * and return a pointer to that empire, if it exists in the manager, 
    * or NULL if it does not.  
    */
    const Empire* Lookup(int ID) const;
    
    /// Provides full access to empires stored in the manager.
    /**
    * Lookup will look up an empire by its EmpireID
    * and return a pointer to that empire, if it exists in the manager, 
    * or NULL if it does not
    */
    Empire* Lookup(int ID);
    //@}

    /** \name Non-Const Iterators */ //@{
    /**
    * Returns an iterator pointing at the first empire
    * in the manager.  
    */
    iterator begin();
     /**
    * Returns an iterator which is past the end of
    * the list of empires.  
    */
    iterator end();
    //@}

    /// Changes the properties of an empire, if it exists.
    /**
    * UpdateEmpireStatus changes the id, name, or color
    * of the empire whose ID equals empireID.  Returns true if successful
    * false if not.  If the empire manager does not have an empire object 
    * for the specified empire this method will return false.
    *
    */
    virtual bool UpdateEmpireStatus(int empireID, 
                            std::string &name, 
                            GG::Clr color);

    /** \name Constants */ //@{
    /// Tag for empire update XMLElements
    static const std::string EMPIRE_UPDATE_TAG;

    /**
    * Adds the given empire to the manager's map.  Does not modify the
    * given empire object, or any others.
    * 
    */
    void InsertEmpire(Empire* empire);

    /**
    * Removes the given empire from the manager's map.  Does 
    * not modify the given empire object, or any others.
    */
    void RemoveEmpire(Empire* empire);

    /**
    * Removes all empires from the manager's map and deallocates those empires.
    *
    */
    void RemoveAllEmpires();

private:
    // map of IDs to empires for fast lookup
    std::map<int, Empire*> m_empire_map;    

};









#endif
