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

#ifndef _ResourcePool_h_
#include "ResourcePool.h"
#endif

/**
* Class to maintain the state of a single empire.  
* This class keeps track of the following information:
*   - color
*   - name
*   - player name
*   - numeric ID
*   - control state (human or AI)
*   - accumulated research
*   - list of technologies
*   - list of explored systems
*   - list of ship designs
*   - list of sitrep entries
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
    typedef std::set<int>::const_iterator               TechIDItr;
    typedef std::set<int>::const_iterator               SystemIDItr;
    typedef std::map<int, ShipDesign>::const_iterator   ShipDesignItr;
    typedef std::list<SitRepEntry*>::const_iterator     SitRepItr;
    //@}

    /**
     * ControlStatus is used to determine whether an Empire is AI
     * or Human controlled.
     */
    enum ControlStatus {CONTROL_AI =    0,  ///< under AI control
                        CONTROL_HUMAN = 1   ///< under human control
    };

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
    Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id,
           ControlStatus& control); 

    /// Creates an empire from an XMLElement
    /**
     * The empire's fields and lists will be initialized as specified 
     * by the given XLMElement.  This XMLElement should have been created
     * by Empire::XMLEncode()
     */
    Empire(const GG::XMLElement& elem);

    //@}
    /** \name Destructors */ //@{
    ~Empire();
    //@}

    /* **************************************************
   *** ACCESSORS
   *****************************************************/

    /** \name Accessors */ //@{
    /// Returns an Empire's control state
    ControlStatus ControlState() const;

    /// Returns the Empire's name
    const std::string& Name() const;

    /// Returns the Empire's player's name
    const std::string& PlayerName() const;

    /// Returns the Empire's unique numeric ID
    int EmpireID() const;

    /// Returns the Empire's color
    const GG::Clr& Color() const;

    /// Returns the numeric ID of the empire's homeworld
    int HomeworldID() const;

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
     *********************************************************/

    /* ************************************************
       Methods to see if items are in our lists
    **************************************************/

    /// Returns true if the given item is in the appropriate list, false if it is not.  
    bool HasTech(int ID) const;

    /// Returns true if the given item is in the appropriate list, false if it is not.
    bool HasExploredSystem(int ID) const;

    /// Returns the number of entries in the SitRep.
    int NumSitRepEntries() const;


    /* *************************************
       (const) Iterators over our various lists
    ***************************************/

    TechIDItr TechBegin() const;
    TechIDItr TechEnd() const;

    SystemIDItr ExploredBegin() const;
    SystemIDItr ExploredEnd() const;

    ShipDesignItr ShipDesignBegin() const;
    ShipDesignItr ShipDesignEnd() const;

    SitRepItr SitRepBegin() const;
    SitRepItr SitRepEnd() const;

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
     *  For v0.2, this will only be the Empire's name, color, and control state
     *
     *  For future versions, technology, diplomatic status, race, and other
     *  such data will be included as appropriate.
     *
     *   If the viewer has the same empire ID as the host object
     *   then the return value is the same as the no-arg version of XMLEncode()
     */
    GG::XMLElement XMLEncode(const Empire& viewer) const;
    //@}


    /** \name Mutators */ //@{
    /* ************************************************
       Methods to add items to our various lists
    **************************************************/

    /// Inserts the given ID into the Empire's list of Technologies.
    void AddTech(int ID);

    /// Inserts the given ID into the Empire's list of explored systems.
    void AddExploredSystem(int ID);

    /// inserts a copy of the given design into the empire's design list
    int AddShipDesign(const ShipDesign& design);

    /** Inserts the a pointer to given SitRep entry into the empire's sitrep list.
     *  \warning When you call this method, you are transferring ownership
     *  of the entry object to the Empire.
     *  The object pointed to by 'entry' will be deallocated when
     *  the empire's sitrep is cleared.  Be careful you do not have any
     *  references to SitRepEntries lying around when this happens.
     *  You \a must pass in a dynamically allocated sitrep entry
     */
    void AddSitRepEntry( SitRepEntry* entry);

    /* ************************************************
       Methods to remove items from our various lists
    **************************************************/

    /// Removes the given ID from the empire's list
    void RemoveTech(int ID);

    /// Removes the design with the specified ID from the empire's design list
    void RemoveShipDesign(int id);

    /// Clears all sitrep entries;
    void ClearSitRep();


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
    void CheckResearchProgress();
   	
    /// Mutator for empire color
    void SetColor(const GG::Clr& color);

    /// Mutator for control state
    void SetControlState(ControlStatus state);

    /// Mutator for empire name
    void SetName(const std::string& name);

    /// Mutator for empire's player name
    void SetPlayerName(const std::string& player_name);
    //@}

    void UpdateResourcePool();

    const MineralResourcePool&    MineralResPool    () const {return m_mineral_resource_pool;}
    const FoodResourcePool&       FoodResPool       () const {return m_food_resource_pool;}
    const ResearchResourcePool&   ResearchResPool   () const {return m_research_resource_pool;}
    const PopulationResourcePool& PopulationResPool () const {return m_population_resource_pool;}
    const IndustryResourcePool&   IndustryResPool   () const {return m_industry_resource_pool;}
    const TradeResourcePool&      TradeResPool      () const {return m_trade_resource_pool;}

private:
    /// Empire's unique numeric id
    int m_id;

    /// Empire's total accumulated research points
    int m_total_rp;

    /// Empire's name
    std::string m_name;

    /// Empire's Player's name
    std::string m_player_name;

    /// Empire's color
    GG::Clr m_color;

    /// the ID of the empire's homeworld
    int m_homeworld_id;

    /// Empire's control status
    Empire::ControlStatus m_control_state;

    /// the next available ship design ID
    int m_next_design_id;

    /// list of acquired technologies.  These are IDs
    /// referencing Tech objects in the techmanager
    std::set<int> m_techs;   

    /// systems you've explored
    std::set<int> m_explored_systems;

    /// The Empire's ship designs
    std::map<int, ShipDesign> m_ship_designs; 

    /// The Empire's sitrep entries
    std::list<SitRepEntry*> m_sitrep_entries;

    /// The Empire resource pools
    MineralResourcePool     m_mineral_resource_pool;
    FoodResourcePool        m_food_resource_pool;
    ResearchResourcePool    m_research_resource_pool;
    PopulationResourcePool  m_population_resource_pool;
    IndustryResourcePool    m_industry_resource_pool;
    TradeResourcePool       m_trade_resource_pool;
};

#endif




