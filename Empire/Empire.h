// -*- C++ -*-
#ifndef _Empire_h_
#define _Empire_h_

#ifndef _GGClr_h_
#include "GGClr.h"
#endif

#ifndef _ResourcePool_h_
#include "ResourcePool.h"
#endif

#ifndef _SitRepEntry_h_
#include "../util/SitRepEntry.h"
#endif

#ifndef _Tech_h_
#include "../universe/Tech.h"
#endif

#ifndef _XMLDoc_h_
#include "XMLDoc.h"
#endif

#include <boost/tuple/tuple.hpp>

#include <list>
#include <string>

class BuildingType;
class ShipDesign;

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
    typedef std::set<std::string>::const_iterator             TechItr;
    typedef std::set<std::string>::const_iterator             BuildingTypeItr;
    typedef std::set<int>::const_iterator                     SystemIDItr;
    typedef std::map<std::string, ShipDesign>::const_iterator ShipDesignItr;
    typedef std::list<SitRepEntry*>::const_iterator           SitRepItr;
    //@}

    struct ResearchQueue
    {
        /** The type of a single element in the research queue.  Such an element includes the tech itself, the
            spending being done on this tech, and the number of turns until this tech is completed. */
        typedef boost::tuple<const Tech*, double, int> QueueElement;

        typedef std::deque<QueueElement> QueueType;

        /** The ResearchQueue iterator type.  Dereference yields a QueueElement. */
        typedef QueueType::iterator iterator;
        /** The const ResearchQueue iterator type.  Dereference yields a QueueElement. */
        typedef QueueType::const_iterator const_iterator;

        /** \name Structors */ //@{
        ResearchQueue(); ///< Basic ctor.
        ResearchQueue(const GG::XMLElement& elem); ///< Constructs a ResearchQueue from an XMLElement.
        //@}

        /** \name Accessors */ //@{
        bool InQueue(const Tech* tech) const; ///< Returns true iff \a tech is in this queue.
        int ProjectsInProgress() const;       ///< Returns the number of research projects currently (perhaps partially) funded.
        double TotalRPsSpent() const;         ///< Returns the number of RPs spent on the projects in this queue.

        // STL container-like interface
        bool empty() const;
        unsigned int size() const;
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator find(const Tech* tech) const;

        /** Returns an iterator to the underfunded research project, or end() if none exists. */
        const_iterator UnderfundedProject() const;

        GG::XMLElement XMLEncode() const; ///< Encodes this queue as an XMLElement.
        //@}

        /** \name Mutators */ //@{
        /** Recalculates the RPs spent on and number of turns left for each project in the queue.  Also
            determines the number of projects in prgress, and the total number of RPs spent on the projects
            in the queue.  \note A precondition of this function that \a RPs must be greater than some
            epsilon > 0; see the implementation for the actual value used for epsilon. */
        void Update(double RPs, const std::map<std::string, double>& research_status);

        // STL container-like interface
        void push_back(const Tech* tech);
        void insert(iterator it, const Tech* tech);
        void erase(iterator it);

        iterator begin();
        iterator end();
        iterator find(const Tech* tech);

        /** Returns an iterator to the underfunded research project, or end() if none exists. */
        iterator UnderfundedProject();
        //@}

    private:
        QueueType m_queue;
        int       m_projects_in_progress;
        double    m_total_RPs_spent;
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
    Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id); 

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

    /// Returns the numeric ID of the empire's capitol
    int CapitolID() const;

    /// Returns the Empire's accumulated RPs
    /** 
     * Gets the empire's accumulated research points. 
     * This number keeps accumulating even after research breakthroughs 
     * have been achieved.
     */
    int TotalRP() const;

    /// Returns the ship design with the requested name, or 0 if none exists.
    const ShipDesign* GetShipDesign(const std::string& name) const;

    /* ******************************************************
     *  The Empire object maintains containers of the following 
     *  objects (all referenced by their object IDs)
     *    - Tech advances
     *    - Explored Systems
     *********************************************************/

    /* ************************************************
       Methods to see if items are in our lists
    **************************************************/

    /// Returns true iff \a name is an unavailable tech, and it has not unavailable prerequisites.
    bool ResearchableTech(const std::string& name) const;

    /// Returns the queue of techs being or queued to be researched.
    const ResearchQueue& GetResearchQueue() const;

    /** Returns the RPs spent towards tech \a name if it has partial research progress, -1.0 if it is
        unresearched or already available. */
    double ResearchStatus(const std::string& name) const;

    /// Returns the set of all available techs.
    const std::set<std::string>& AvailableTechs() const;

    /// progress of partially-researched techs; fully researched techs are cannot be found in this container
    bool TechAvailable(const std::string& name) const;

    /// Returns true if the given building type is known to this empire, false if it is not.
    bool BuildingTypeAvailable(const std::string& name) const;

    /** Returns the BuildingType called \a name.  This will be equivalent to the BuildingType
        returned by the global GetBuildingType() function if this empire has not discovered
        any Techs that modify it, or the modified version otherwise. */
    const BuildingType* GetBuildingType(const std::string& name) const;

    /// Returns true if the given item is in the appropriate list, false if it is not.
    bool HasExploredSystem(int ID) const;

    /// Returns the number of entries in the SitRep.
    int NumSitRepEntries() const;


    /* *************************************
       (const) Iterators over our various lists
    ***************************************/

    TechItr TechBegin() const;
    TechItr TechEnd() const;

    BuildingTypeItr BuildingTypeBegin() const;
    BuildingTypeItr BuildingTypeEnd() const;

    SystemIDItr ExploredBegin() const;
    SystemIDItr ExploredEnd() const;

    ShipDesignItr ShipDesignBegin() const;
    ShipDesignItr ShipDesignEnd() const;

    SitRepItr SitRepBegin() const;
    SitRepItr SitRepEnd() const;

    const MineralResourcePool&    MineralResPool    () const {return m_mineral_resource_pool;}
    const FoodResourcePool&       FoodResPool       () const {return m_food_resource_pool;}
    const ResearchResourcePool&   ResearchResPool   () const {return m_research_resource_pool;}
    const PopulationResourcePool& PopulationResPool () const {return m_population_resource_pool;}
    const IndustryResourcePool&   IndustryResPool   () const {return m_industry_resource_pool;}
    const TradeResourcePool&      TradeResPool      () const {return m_trade_resource_pool;}

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
     *   If the viewer has the same empire ID as the host object
     *   then the return value is the same as the no-arg version of XMLEncode()
     */
    GG::XMLElement XMLEncode(const Empire& viewer) const;
    //@}


    /** \name Mutators */ //@{
    /* ************************************************
       Methods to add items to our various lists
    **************************************************/

    /** Adds \a tech to the research queue, placing it before position \a pos.  If \a tech is already in the queue,
        it is moved to \a pos, then removed from its former position.  If \a pos < 0 or queue.size() <= pos, \a tech
        is placed at the end of the queue. If \a tech is already available, no action is taken. */
    void PlaceTechInQueue(const Tech* tech, int pos = -1);

    /// Removes \a tech from the research queue, if it is in the research queue already.
    void RemoveTechFromQueue(const Tech* tech);

    /// Inserts the given Tech into the Empire's list of available technologies.
    void AddTech(const std::string& name);

    /// Adds a given buildable item (Building, Ship Component, etc.) to the list of available buildable items.
    void UnlockItem(const Tech::ItemSpec& item);

    /// Inserts the given BuildingType into the Empire's list of available BuldingTypes.
    void AddBuildingType(const std::string& name);

    /** Adds EffectsGroups \a effects to the empire-specific version of BuildingType \a name. */
    void RefineBuildingType(const std::string& name, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects);

    /** Removes all refinements to Buildings, ShipComponents, etc., that are caused by Tech effects. */
    void ClearRefinements();

    /// Inserts the given ID into the Empire's list of explored systems.
    void AddExploredSystem(int ID);

    /// inserts a copy of the given design into the empire's design list
    void AddShipDesign(const ShipDesign& design);

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

    /// Removes the given Tech from the empire's list
    void RemoveTech(const std::string& name);

    /// Removes a given buildable item (Building, ShipComponent, etc.) from the list of available buildable items.
    void LockItem(const Tech::ItemSpec& item);

    /// Removes the given BuildingType from the empire's list
    void RemoveBuildingType(const std::string& name);

    /// Clears all sitrep entries;
    void ClearSitRep();


    /// Adds reseach points to the accumulated total. 
    /// Checks for new tech advances.
    /** 
     * This method checks for new technological
     * advances that have been achieved, and add them to the technology list.
     */
    void CheckResearchProgress();
   	
    /// Mutator for empire color
    void SetColor(const GG::Clr& color);

    /// Mutator for empire name
    void SetName(const std::string& name);

    /// Mutator for empire's player name
    void SetPlayerName(const std::string& player_name);
    //@}

    void UpdateResourcePool();
    void UpdateResearchQueue();

    MineralResourcePool&    MineralResPool    () {return m_mineral_resource_pool;}
    FoodResourcePool&       FoodResPool       () {return m_food_resource_pool;}
    ResearchResourcePool&   ResearchResPool   () {return m_research_resource_pool;}
    PopulationResourcePool& PopulationResPool () {return m_population_resource_pool;}
    IndustryResourcePool&   IndustryResPool   () {return m_industry_resource_pool;}
    TradeResourcePool&      TradeResPool      () {return m_trade_resource_pool;}

private:
    /// Empire's unique numeric id
    int m_id;

    /// Empire's name
    std::string m_name;

    /// Empire's Player's name
    std::string m_player_name;

    /// Empire's color
    GG::Clr m_color;

    /// the ID of the empire's homeworld
    int m_homeworld_id;

    /// list of acquired technologies.  These are string names referencing Tech objects
    std::set<std::string> m_techs;

    /// the queue of techs being or waiting to be researched
    ResearchQueue m_research_queue;

    /// progress of partially-researched techs; fully researched techs are removed
    std::map<std::string, double> m_research_status;

    /// list of acquired BuildingType.  These are string names referencing BuildingType objects
    std::set<std::string> m_building_types;

    std::map<std::string, BuildingType*> m_modified_building_types;

    /// systems you've explored
    std::set<int> m_explored_systems;

    /// The Empire's ship designs
    std::map<std::string, ShipDesign> m_ship_designs;

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

inline std::pair<std::string, std::string> EmpireRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _Empire_h_




