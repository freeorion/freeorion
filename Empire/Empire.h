// -*- C++ -*-
#ifndef _Empire_h_
#define _Empire_h_

#ifndef _GG_Clr_h_
#include <GG/Clr.h>
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
#include "../util/XMLDoc.h"
#endif

#include <deque>
#include <list>
#include <string>

class BuildingType;
class ShipDesign;

struct ResearchQueue
{
    /** The type of a single element in the research queue. */
    struct Element
    {
        Element(); ///< default ctor.
        Element(const Tech* tech_, double spending_, int turns_left_); ///< basic ctor.

        const Tech* tech;
        double      spending;
        int         turns_left;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    typedef std::deque<Element> QueueType;

    /** The ResearchQueue iterator type.  Dereference yields a Element. */
    typedef QueueType::iterator iterator;
    /** The const ResearchQueue iterator type.  Dereference yields a Element. */
    typedef QueueType::const_iterator const_iterator;

    /** \name Structors */ //@{
    ResearchQueue(); ///< Basic ctor.
    ResearchQueue(const XMLElement& elem); ///< Constructs a ResearchQueue from an XMLElement.
    //@}

    /** \name Accessors */ //@{
    bool InQueue(const Tech* tech) const; ///< Returns true iff \a tech is in this queue.
    int ProjectsInProgress() const;       ///< Returns the number of research projects currently (perhaps partially) funded.
    double TotalRPsSpent() const;         ///< Returns the number of RPs currently spent on the projects in this queue.

    // STL container-like interface
    bool empty() const;
    unsigned int size() const;
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator find(const Tech* tech) const;

    /** Returns an iterator to the underfunded research project, or end() if none exists. */
    const_iterator UnderfundedProject() const;

    XMLElement XMLEncode() const; ///< Encodes this queue as an XMLElement.
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

struct ProductionQueue
{
    /** The type that specifies a single production item (BuildType and name string). */
    struct ProductionItem
    {
        ProductionItem(); ///< default ctor.
        ProductionItem(BuildType build_type_, std::string name_); ///< basic ctor.
        ProductionItem(const XMLElement& elem); ///< XML ctor.

        XMLElement XMLEncode() const; ///< Encodes this item as an XMLElement.

        BuildType   build_type;
        std::string name;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    /** The type of a single element in the production queue. */
    struct Element
    {
        Element(); ///< default ctor.
        Element(ProductionItem item_, int ordered_, int remaining_, int location_); ///< basic ctor.
        Element(BuildType build_type, std::string name, int ordered_, int remaining_, int location_); ///< basic ctor.
        Element(const XMLElement& elem); ///< XML ctor.

        XMLElement XMLEncode() const; ///< Encodes this element as an XMLElement.

        ProductionItem item;
        int            ordered;
        int            remaining;
        int            location;                 ///< the ID of the UniverseObject at which this item is being produced
        double         spending;
        int            turns_left_to_next_item;
        int            turns_left_to_completion;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    typedef std::deque<Element> QueueType;

    /** The ProductionQueue iterator type.  Dereference yields a Element. */
    typedef QueueType::iterator iterator;
    /** The const ProductionQueue iterator type.  Dereference yields a Element. */
    typedef QueueType::const_iterator const_iterator;

    /** \name Structors */ //@{
    ProductionQueue(); ///< Basic ctor.
    ProductionQueue(const XMLElement& elem); ///< Constructs a ProductionQueue from an XMLElement.
    //@}

    /** \name Accessors */ //@{
    int ProjectsInProgress() const;       ///< Returns the number of production projects currently (perhaps partially) funded.
    double TotalPPsSpent() const;         ///< Returns the number of PPs currently spent on the projects in this queue.

    // STL container-like interface
    bool empty() const;
    unsigned int size() const;
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator find(int i) const;
    const Element& operator[](int i) const;

    /** Returns an iterator to the underfunded production project, or end() if none exists. */
    const_iterator UnderfundedProject(const Empire* empire) const;

    XMLElement XMLEncode() const; ///< Encodes this queue as an XMLElement.
    //@}

    /** \name Mutators */ //@{
    /** Recalculates the PPs spent on and number of turns left for each project in the queue.  Also
        determines the number of projects in progress, and the total number of PPs spent on the projects
        in the queue.  \note A precondition of this function that \a PPs must be greater than some
        epsilon > 0; see the implementation for the actual value used for epsilon. */
    void Update(Empire* empire, double PPs, const std::vector<double>& production_status);

    // STL container-like interface
    void push_back(const Element& element);
    void insert(iterator it, const Element& element);
    void erase(int i);
    void erase(iterator it);

    iterator begin();
    iterator end();
    iterator find(int i);
    Element& operator[](int i);

    /** Returns an iterator to the underfunded production project, or end() if none exists. */
    iterator UnderfundedProject(const Empire* empire);
    //@}

private:
    QueueType m_queue;
    int       m_projects_in_progress;
    double    m_total_PPs_spent;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

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
    Empire(const XMLElement& elem);

    //@}
    /** \name Destructors */ //@{
    ~Empire();
    //@}

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

    /// Returns the ship design with the requested name, or 0 if none exists.
    const ShipDesign* GetShipDesign(const std::string& name) const;

    /* ******************************************************
     *  The Empire object maintains containers of the following 
     *  objects (all referenced by their object IDs)
     *    - Tech advances
     *    - Explored Systems
     *********************************************************/

    /// Returns true iff \a name is an unavailable tech, and it has no unavailable prerequisites.
    bool ResearchableTech(const std::string& name) const;

    /// Returns the queue of techs being or queued to be researched.
    const ResearchQueue& GetResearchQueue() const;

    /** Returns the RPs spent towards tech \a name if it has partial research progress, -1.0 if it is
        unresearched or already available. */
    double ResearchStatus(const std::string& name) const;

    /// Returns the set of all available techs.
    const std::set<std::string>& AvailableTechs() const;

    /// Returns true iff this tech has been completely resarched.
    bool TechAvailable(const std::string& name) const;

    /// Returns the set of all available building types.
    const std::set<std::string>& AvailableBuildingTypes() const;

    /// Returns true if the given building type is known to this empire, false if it is not.
    bool BuildingTypeAvailable(const std::string& name) const;

    /// Returns the queue of items being or queued to be produced.
    const ProductionQueue& GetProductionQueue() const;

    /** Returns the PPs spent towards item \a i in the build queue if it has partial progress, -1.0 if there is no such
        index in the production queue. */
    double ProductionStatus(int i) const;

    /** Returns the cost per turn and the number of turns required to produce the indicated item, or (-1.0, -1) if the
        item is unknown, unavailable, or invalid. */
    std::pair<double, int> ProductionCostAndTime(BuildType build_type, std::string name) const;

    /** Returns true iff this empire can produce the specified item at the specified location. */
    bool BuildableItem(BuildType build_type, std::string name, int location) const;

    /// Returns true if the given item is in the appropriate list, false if it is not.
    bool HasExploredSystem(int ID) const;

    /// Returns the number of entries in the SitRep.
    int NumSitRepEntries() const;

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

    /// Returns the number of production points available to the empire (this is the minimum of current industry and mineral outputs).
    double ProductionPoints() const;

    /** Encodes the dat of this empire as visible to the empire with id \a empire_id (or all data if \a empire_id ==
        ALL_EMPIRES) */
    XMLElement XMLEncode(int empire_id = ALL_EMPIRES) const;
    //@}


    /** \name Mutators */ //@{
    /** Adds \a tech to the research queue, placing it before position \a pos.  If \a tech is already in the queue,
        it is moved to \a pos, then removed from its former position.  If \a pos < 0 or queue.size() <= pos, \a tech
        is placed at the end of the queue. If \a tech is already available, no action is taken. */
    void PlaceTechInQueue(const Tech* tech, int pos = -1);

    /// Removes \a tech from the research queue, if it is in the research queue already.
    void RemoveTechFromQueue(const Tech* tech);

    /** Adds the indicated build to the production queue, placing it before position \a pos.  If \a pos < 0 or
        queue.size() <= pos, the build is placed at the end of the queue. */
    void PlaceBuildInQueue(BuildType build_type, const std::string& name, int number, int location, int pos = -1);

    /// Changes the remaining number to build for queue item \a index to \a quantity
    void SetBuildQuantity(int index, int quantity);

    /// Moves \a tech from the production queue, if it is in the production queue already.
    void MoveBuildWithinQueue(int index, int new_index);

    /// Removes the build at position \a index in the production queue, if such an index exists.
    void RemoveBuildFromQueue(int index);

    /// Inserts the given Tech into the Empire's list of available technologies.
    void AddTech(const std::string& name);

    /// Adds a given buildable item (Building, Ship Component, etc.) to the list of available buildable items.
    void UnlockItem(const ItemSpec& item);

    /// Inserts the given BuildingType into the Empire's list of available BuldingTypes.
    void AddBuildingType(const std::string& name);

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
    void AddSitRepEntry(SitRepEntry* entry);

    /// Removes the given Tech from the empire's list
    void RemoveTech(const std::string& name);

    /// Removes a given buildable item (Building, ShipComponent, etc.) from the list of available buildable items.
    void LockItem(const ItemSpec& item);

    /// Removes the given BuildingType from the empire's list
    void RemoveBuildingType(const std::string& name);

    /// Clears all sitrep entries;
    void ClearSitRep();

    /// Checks for tech projects that have been completed, and adds them to the known techs list.
    void CheckResearchProgress();

    /// Checks for production projects that have been completed, and places them at their respective production sites.
    void CheckProductionProgress();
        
    /// Mutator for empire color
    void SetColor(const GG::Clr& color);

    /// Mutator for empire name
    void SetName(const std::string& name);

    /// Mutator for empire's player name
    void SetPlayerName(const std::string& player_name);
    //@}

    void UpdateResourcePool();
    void UpdateResearchQueue();
    void UpdateProductionQueue();

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

    /// the queue of items being or waiting to be built
    ProductionQueue m_production_queue;

    /// progress of partially-completed builds; completed items are removed
    std::vector<double> m_production_status;

    /// list of acquired BuildingType.  These are string names referencing BuildingType objects
    std::set<std::string> m_building_types;

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

    friend class boost::serialization::access;
    Empire();
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// template implementations
template <class Archive>
void ResearchQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    std::string tech_name;
    if (Archive::is_saving::value) {
        assert(tech);
        tech_name = tech->Name();
    }
    ar  & BOOST_SERIALIZATION_NVP(tech_name)
        & BOOST_SERIALIZATION_NVP(spending)
        & BOOST_SERIALIZATION_NVP(turns_left);
    if (Archive::is_loading::value) {
        assert(tech_name != "");
        tech = GetTech(tech_name);
    }
}

template <class Archive>
void ResearchQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_total_RPs_spent);
}

template <class Archive>
void ProductionQueue::ProductionItem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(build_type)
        & BOOST_SERIALIZATION_NVP(name);
}

template <class Archive>
void ProductionQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(item)
        & BOOST_SERIALIZATION_NVP(ordered)
        & BOOST_SERIALIZATION_NVP(remaining)
        & BOOST_SERIALIZATION_NVP(location)
        & BOOST_SERIALIZATION_NVP(spending)
        & BOOST_SERIALIZATION_NVP(turns_left_to_next_item)
        & BOOST_SERIALIZATION_NVP(turns_left_to_completion);
}

template <class Archive>
void ProductionQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_total_PPs_spent);
}

template <class Archive>
void Empire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_color);
    if (Universe::ALL_OBJECTS_VISIBLE ||
        Universe::s_encoding_empire == ALL_EMPIRES || m_id == Universe::s_encoding_empire) {
        ar  & BOOST_SERIALIZATION_NVP(m_homeworld_id)
            & BOOST_SERIALIZATION_NVP(m_techs)
            & BOOST_SERIALIZATION_NVP(m_research_queue)
            & BOOST_SERIALIZATION_NVP(m_research_status)
            & BOOST_SERIALIZATION_NVP(m_production_queue)
            & BOOST_SERIALIZATION_NVP(m_production_status)
            & BOOST_SERIALIZATION_NVP(m_building_types)
            & BOOST_SERIALIZATION_NVP(m_explored_systems)
            & BOOST_SERIALIZATION_NVP(m_ship_designs)
            & BOOST_SERIALIZATION_NVP(m_sitrep_entries)
            & BOOST_SERIALIZATION_NVP(m_mineral_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_food_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_research_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_population_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_industry_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_trade_resource_pool);
    }
}

#endif // _Empire_h_




