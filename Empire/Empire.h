// -*- C++ -*-
#ifndef _Empire_h_
#define _Empire_h_

#include <list>
#include <string>

#ifndef _GGClr_h_
#include "GGClr.h"
#endif

#ifndef _XMLDoc_h_
#include "XMLDoc.h"
#endif

#ifndef _SitRepEntry_h_
#include "../util/SitRepEntry.h"
#endif

#ifndef _TechManager_h_
#include "TechManager.h"
#endif

// include ship.h so we can see the shipdesign object
#ifndef _Ship_h_
#include "../universe/Ship.h"
#endif

/**
* Class to maintain the state of a single empire.  
* This class keeps track of the following information:
*   - color
*   - name
*   - numeric ID
*   - control state (human or AI)
*   - accumulated research
*   - list of sitrep entries
*   - list of owned planets
*   - list of owned fleets
*   - list of technologies
*   - list of visible fleets
*   - list of explored systems
*   - list of ship designs
*
* Only ServerEmpire should create these objects.  
* In both the client and server, Empires are managed by a subclass of EmpireManager, 
* and can be accessed from other modules by using the EmpireManager::Lookup() 
* method to obtain a pointer.
*/ 
class Empire     
{
public:
    /** 
     * EmpireManagers must be friends so that they can have
     * access to the constructor and keep it hidden from others
     */
    friend class EmpireManager;
    /** 
     * EmpireManagers must be friends so that they can have
     * access to the constructor and keep it hidden from others
     */
    friend class ServerEmpireManager;
    /** 
     * EmpireManagers must be friends so that they can have
     * access to the constructor and keep it hidden from others
     */
    friend class ClientEmpireManager;
    

    /** \name Iterator Types */ //@{
    // these typedefs allow me to change the container I use
    // without breaking anybody else's code
    typedef std::set<int>::iterator PlanetIDItr;
    typedef std::set<int>::iterator FleetIDItr;
    typedef std::set<int>::iterator SystemIDItr;
    typedef std::set<int>::iterator TechIDItr;
  
    typedef std::set<int>::const_iterator    ConstPlanetIDItr;
    typedef std::set<int>::const_iterator    ConstFleetIDItr;
    typedef std::set<int>::const_iterator    ConstSystemIDItr;
    typedef std::set<int>::const_iterator    ConstTechIDItr;
    
    typedef std::list<SitRepEntry*>::iterator SitRepItr;
    typedef std::list<SitRepEntry*>::const_iterator    ConstSitRepItr;
    
    typedef std::map<int, ShipDesign>::iterator ShipDesignItr;
    typedef std::map<int, ShipDesign>::const_iterator ConstShipDesignItr;
    //@}
    
    /**
     * ControlStatus is used to determine whether an Empire is AI
     * or Human controlled.
     */
    enum ControlStatus
        {
            CONTROL_AI=0,     /// under AI control
            CONTROL_HUMAN=1   /// under human control
        }  ;

    /* **************************************************
    *** ACCESSORS
    *****************************************************/

    /** \name Accessors */ //@{
    /// Returns an Empire's control state
    ControlStatus ControlState() const;
	
    /// Returns the Empire's name
    const std::string& Name() const;
    
    /// Returns the Empire's unique numeric ID
    int EmpireID() const;
    
    /// Returns the Empire's color
    const GG::Clr& Color() const;
    
    /// Returns the Empire's accumulated RPs
    /** 
     * Gets the empire's accumulated research points. 
     * This number keeps accumulating even after research breakthroughs 
     * have been achieved.
     */
    int TotalRP() const;
    
    /// Searches for a ship design and copies over the input design and returns success/failure
    bool CopyShipDesign(int design_id, ShipDesign& design_target);

    /* ******************************************************
     *  The Empire object maintains containers of the following 
     *  objects (all referenced by their object IDs)
     *    - Tech advances
     *    - Explored Systems
     *    - Owned Planets
     *    - Owned Fleets
     *    - Visible Fleets
     *    - SitRepEntries
     *********************************************************/
    
    /* ************************************************
       Methods to see if items are in our lists
    **************************************************/

    
    /// Returns true if the given item is in the appropriate list, false if it is not.  
    bool HasTech(int ID) const;
    
    /// Returns true if the given item is in the appropriate list, false if it is not.
    bool HasPlanet(int ID) const;
    
    /// Returns true if the given item is in the appropriate list, false if it is not.
    bool HasExploredSystem(int ID) const;
    
    /// Returns true if the given item is in the appropriate list, false if it is not.
    bool HasFleet(int ID) const;
    
    /// Returns true if the given item is in the appropriate list, false if it is not.
    bool HasVisibleFleet(int ID) const;
    
    
    /* *************************************
       (const) Iterators over our various lists
    ***************************************/

    ConstTechIDItr TechBegin() const;
    ConstTechIDItr TechEnd() const;
    
    ConstPlanetIDItr PlanetBegin() const;
    ConstPlanetIDItr PlanetEnd() const;
    
    ConstFleetIDItr FleetBegin() const;
    ConstFleetIDItr FleetEnd() const;
                                     
    ConstSystemIDItr ExploredBegin() const;
    ConstSystemIDItr ExploredEnd() const;
    
    ConstFleetIDItr VisibleFleetBegin() const;
    ConstFleetIDItr VisibleFleetEnd() const;
    
    ConstSitRepItr SitRepBegin() const;
    ConstSitRepItr SitRepEnd() const;
    
    std::list<SitRepEntry*> *GetSitRepEntries( ) { return &m_sitrep_entries; };
    

    ConstShipDesignItr ShipDesignBegin() const;
    ConstShipDesignItr ShipDesignEnd() const;
      
    /* *************************************
       (non-const) Iterators over our various lists
    ***************************************/
 
    TechIDItr TechBegin();
    TechIDItr TechEnd();
    
    PlanetIDItr PlanetBegin();
    PlanetIDItr PlanetEnd();
    
    FleetIDItr FleetBegin();
    FleetIDItr FleetEnd();
    
    SystemIDItr ExploredBegin();
    SystemIDItr ExploredEnd();
    
    FleetIDItr VisibleFleetBegin();
    FleetIDItr VisibleFleetEnd();
    
    SitRepItr SitRepBegin();
    SitRepItr SitRepEnd();
    
    ShipDesignItr ShipDesignBegin() ;
    ShipDesignItr ShipDesignEnd() ;

    /// Encodes an empire into an XMLElement
    /**
     * This method encodes an empire into an XMLElement, which can then
     * be transmitted over the network and used by a client to replicate
     * the empire object.  
     *
     * This method is used by the Server to generate turn updates.
     *
     * All data on the empire is encoded.
     *
     *
     */
    GG::XMLElement XMLEncode() const;


    /// Encodes an empire into an XMLElement, from the perspective of another
    /**
     * This method encodes the empire into an XMLElement, but includes only
     * the information that the specified empire will have access to.
     * 
     *  For version 0.1, this will only be the Empire's name, color, and control state
     *
     *  For future versions, technology, diplomatic status, race, and other
     *  such data will be included as appropriate.
     *
     *   If the viewer has the same empire ID as the host object
     *   then the return value is the same as the no-arg version of XMLEncode()
     */
    GG::XMLElement XMLEncode(const Empire& viewer) const;


    //@}

    /* ****************************************************
   *** MUTATORS
   *******************************************************/

    /** \name Mutators */ //@{

    /* ************************************************
       Methods to add items to our various lists
    **************************************************/

    /// Inserts the given ID into the Empire's list of Technologies.
    void AddTech(int ID);

    /// Inserts the given ID into the Empire's list of owned planets.
    void AddPlanet(int ID);

    /// Inserts the given ID into the Empire's list of explored systems.
    void AddExploredSystem(int ID);

    /// Inserts the given ID into the Empire's list of owned fleets.
    void AddFleet(int ID);

    /// Inserts the given ID into the Empire's list of visible fleets.
    void AddVisibleFleet(int ID);

    /// Inserts the a pointer to given sitrep entry into the empire's sitrep list
    /**
     *  WARNING: When you call this method, you are transferring ownership
     *  of the entry object to the Empire.
     *  The object pointed to by 'entry' will be deallocated when
     *  the empire's sitrep is cleared.  Be careful you do not have any
     *  references to SitRepEntries lieing around when this happens.
     *  You MUST pass in a dynamically allocated sitrep entry
     */
    void AddSitRepEntry( SitRepEntry* entry);


    /// inserts a copy of the given design into the empire's design list
    int AddShipDesign(const ShipDesign& design);
    
    /* ************************************************
       Methods to remove items from our various lists
    **************************************************/

    /// Removes the given ID from the empire's list
    void RemoveTech(int ID);
    
    /// Removes the given ID from the empire's list
    void RemoveOwnedPlanet(int ID);
    
    /// Removes the given ID from the empire's list
    void RemoveExploredSystem(int ID);
    
    /// Removes the given ID from the empire's list
    void RemoveFleet(int ID);
    
    /// Removes the given ID from the empire's list
    void RemoveVisibleFleet(int ID);
    
    /// Removes all entries from the Empire's SitRep
    /** 
     * Any SitRep entry objects currently referenced by the Empire's SitRep collection
     * will be deallocated, and the pointers discarded.
     */
    void ClearSitRep();

    /** 
     * returns the size of sitreps container
     */
    int NumSitReps() { return m_sitrep_entries.size( ); }
    
    /// Removes the design with the specified ID from the empire's design list
    void RemoveShipDesign(int id);
    
    
    /// Applies changes to the Empire object
    /**
     * The given element (elem) is the result of an XMLDiff between the calling
     * object and another empire object.  The appropriate changes are made
     * to ensure that this Empire object is identical to the one used to produce
     * the diff.
     *
     * This method should be used for processing turn updates in the client
     * to ensure that an empire's state on the client side is the same as its 
     * state server side.
     */
    void XMLMerge(const GG::XMLElement& elem);
    
    /// Adds reseach points to the accumulated total. 
    /** 
     * Increments the empire's accumulated research points 
     * by the specified amount.  Returns total accumulated research points
     * after the addition. 
     */
    int AddRP(int moreRPs);

    /// Checks for new tech advances.
    /** 
     * This method checks for new technological
     * advances that have been achieved, and add them to the technology list.
     */
    void CheckResearchProgress( );
       	
    /// Mutator for empire color
    void Color(const GG::Clr& color);
    
   	/// Mutator for control state
    void ControlState(ControlStatus state);
    
   	/// Mutator for empire name
    void Name(const std::string& name);
    
    //@}
   
protected:

    /* *****************************************************
    ** CONSTRUCTORS
    ********************************************************/

    /** \name Constructors */ //@{
    /// Creates an empire with the given properties.
    /**
     * Initializes the empire's fields to the specified values.  All lists
     * (planets, fleets, owned planets, visible fleets, technologies, explored
     * systems, sitrep entries) will be empty after creation
     */
    Empire(const std::string& name, int ID, const GG::Clr& color, 
           ControlStatus& control); 

    /// Creates an empire from an XMLElement
    /**
     * The empire's fields and lists will be initialized as specified 
     * by the given XLMElement.  This XMLElement should have been created
     * by Empire::XMLEncode()
     */
    Empire(const GG::XMLElement& elem);
    
    //@}
    
private:
    /// Helper method to encode a list of integers into an XMLElement
    /** 
     *  The contents of the list are encoded using the encoding method described
     *  in the GG documentation.
     *  Each container element is an XML subelement called itemN where N is the 
     *  index of the container element.  Each subelement has a "value" attribute
     *  equal to the value of the container element.
     */
    static void EncodeIntList(GG::XMLElement& container, const std::set<int>& lst);

    /// Empire's unique numeric id
    int m_id;
    
    /// Empire's total accumulated research points
    int m_total_rp;
    
    /// Empire's name
    std::string m_name;
    
    /// Empire's color
    GG::Clr m_color;
    
    /// Empire's control status
	Empire::ControlStatus m_control_state;

    /// the next available ship design ID
    int m_next_design_id;
    
    /// The Empire's sitrep entries
    std::list<SitRepEntry*> m_sitrep_entries;


    // all of the following are lists of ObjectIDs which 
    // reference objects in the Universe module

    /// list of acquired technologies.  These are IDs
    /// referencing TechLevel objects in the techmanager
    std::set<int> m_techs;   
    
    /// planets you own
    std::set<int> m_planets;       
    
    /// fleets you own
    std::set<int> m_fleets;
    
    /// systems you've explored        
    std::set<int> m_explored_systems; 
    
    /// fleets you can see but dont own    
    std::set<int> m_visible_fleets;   
    
    std::map<int, ShipDesign> m_ship_designs; 
    
};

#endif




