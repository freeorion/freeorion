
#ifndef _FREEORION_EMPIREMANAGER_H_
#define _FREEORION_EMPIREMANAGER_H_

#include <list>
#include <map>
#include <string>

#ifndef _FREEORION_EMPIRE_H_
#include "Empire.h"
#endif

#ifndef _GGClr_h_
#include "GG\GGClr.h"
#endif

#ifndef _XMLDoc_h_
#include "GG\XML\XMLDoc.h"
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
    typedef std::map<int, Empire*>::iterator EmpireItr; 
    
    /// Const Iterator over Empires
    typedef std::map<int, Empire*>::const_iterator ConstEmpireItr;
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
    * ConstEmpireBegin returns an const iterator pointing at the first empire
    * in the manager.  
    */
    ConstEmpireItr EmpireBegin() const;
    /**
    * ConstEmpireEnd returns an iterator which is past the end of
    * the list of empires.  
    */
    ConstEmpireItr EmpireEnd() const;
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
    * EmpireBegin returns an iterator pointing at the first empire
    * in the manager.  
    */
    EmpireItr EmpireBegin();
     /**
    * EmpireEnd returns an iterator which is past the end of
    * the list of empires.  
    */
    EmpireItr EmpireEnd();
    //@}

    /// Changes the properties of an empire, if it exists.
    /**
    * UpdateEmpireStatus changes the name, color, or control status
    * of the empire whose ID equals empireID.  Returns true if successful
    * false if not.  If the empire manager does not have an empire object 
    * for the specified empire it will create one, otherwise it will update 
    * it's data.  
    */
    virtual bool UpdateEmpireStatus(int empireID, 
                            std::string &name, 
                            GG::Clr color, 
                            Empire::ControlStatus control);

  
protected:
  
    /// Used by derived classes to add an empire to the set of empires.
    /**
    * Adds the given empire to the manager's map.  Does not modify the
    * given empire object, or any others.
    * 
    */
    void AddEmpireToManager(Empire* empire);
    
    /// Used by derived classes to remove an empire from the set of empires.
    /**
    * Removes the given empire from the manager's map.  Does 
    * not modify the given empire object, or any others.
    */
    void RemoveEmpireFromManager(Empire* empire);
    
    
private:
    
    // map of IDs to empires for fast lookup
    std::map<int, Empire*> m_empire_map;    
    
                
};









#endif
