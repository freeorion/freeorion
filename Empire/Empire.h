// -*- C++ -*-
#ifndef _Empire_h_
#define _Empire_h_

#include <GG/Clr.h>
#include "../util/SitRepEntry.h"
#include "../universe/Tech.h"
#include "../universe/UniverseObject.h"
#include "ResourcePool.h"

#include <deque>
#include <list>
#include <string>

class BuildingType;
class ShipDesign;
class Empire;

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

    /** The ResearchQueue iterator type.  Dereference yields an Element. */
    typedef QueueType::iterator iterator;
    /** The const ResearchQueue iterator type.  Dereference yields an Element. */
    typedef QueueType::const_iterator const_iterator;

    /** \name Structors */ //@{
    ResearchQueue(); ///< Basic ctor.
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
    //@}

    /** \name Mutators */ //@{
    /** Recalculates the RPs spent on and number of turns left for each project in the queue.  Also
        determines the number of projects in prgress, and the total number of RPs spent on the projects
        in the queue.  \note A precondition of this function that \a RPs must be greater than some
        epsilon > 0; see the implementation for the actual value used for epsilon. */
    void Update(Empire* empire, double RPs, const std::map<std::string, double>& research_progress);

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
        ProductionItem(BuildType build_type_, std::string name_);   ///< basic ctor for BuildTypes that use std::string to identify specific items (BuildingTypes)
        ProductionItem(BuildType build_type_, int design_id_);      ///< basic ctor for BuildTypes that use int to indentify the design of the item (ShipDesigns)

        BuildType   build_type;

        // only one of these may be valid, depending on BuildType
        std::string name;
        int         design_id;

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
        Element(BuildType build_type, int design_id, int ordered_, int remaining_, int location_); ///< basic ctor.

        ProductionItem item;
        int            ordered;                 ///< how many of item to produce
        int            remaining;               ///< how many left to produce
        int            location;                ///< the ID of the UniverseObject at which this item is being produced
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

    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ProductionQueueChangedSignalType;    ///< emitted when something is added to, removed from or altered on the queue
    //@}

    /** \name Structors */ //@{
    ProductionQueue(); ///< Basic ctor.
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
    //@}

    /** \name Mutators */ //@{
    /** Recalculates the PPs spent on and number of turns left for each project in the queue.  Also
      * determines the number of projects in progress, and the total number of PPs spent on the projects
      * in the queue.  Does not actually "spend" the PP, but just determines how much should be spent
      * on each item in the queue.  Later call to empire->CheckProductionProgress() will actually 
      * spend PP, remove items from queue and create them in the universe.  \note A precondition of
      * this function that \a PPs must be greater than some epsilon > 0; see the implementation for
      * the actual value used for epsilon. 
      */
    void Update(Empire* empire, double PPs, const std::vector<double>& production_status);

    // STL container-like interface
    void push_back(const Element& element);
    void insert(iterator it, const Element& element);
    void erase(int i);
    iterator erase(iterator it);

    iterator begin();
    iterator end();
    iterator find(int i);
    Element& operator[](int i);

    /** Returns an iterator to the underfunded production project, or end() if none exists. */
    iterator UnderfundedProject(const Empire* empire);

    mutable ProductionQueueChangedSignalType ProductionQueueChangedSignal;
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
*   - accumulated research progress
*   - accumulated production progress
*   - list of technologies
*   - list of explored systems
*   - list of ship designs
*   - list of sitrep entries
*
* In both the client and server, Empires are managed by a subclass of EmpireManager, and can be accessed from
* other modules by using the EmpireManager::Lookup() method to obtain a pointer.
*/ 
class Empire 
{
public:
    /**
     * EmpireManagers must be friends so that they can have
     * access to the constructor and keep it hidden from others
     */
    friend class EmpireManager;

    /** \name Iterator Types */ //@{
    typedef std::set<std::string>::const_iterator   TechItr;
    typedef std::set<std::string>::const_iterator   BuildingTypeItr;
    typedef std::set<int>::const_iterator           SystemIDItr;
    typedef std::set<int>::const_iterator           ShipDesignItr;
    typedef std::list<SitRepEntry*>::const_iterator SitRepItr;
    //@}

    /** \name Constructors */ //@{
    /// Creates an empire with the given properties.
    /**
     * Initializes the empire's fields to the specified values.  All lists
     * (planets, fleets, owned planets, visible fleets, technologies, explored
     * systems, sitrep entries) will be empty after creation
     */
    Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id); 

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

    /// Returns true iff \a name is a tech that has not been researched, and has no unresearched prerequisites.
    bool ResearchableTech(const std::string& name) const;

    /// Returns the queue of techs being or queued to be researched.
    const ResearchQueue& GetResearchQueue() const;

    /** Returns the RPs spent towards tech \a name if it has partial research progress, -1.0 if it is
        unresearched or already available. */
    double ResearchStatus(const std::string& name) const;

    /// Returns the set of all available techs.
    const std::set<std::string>& AvailableTechs() const;

    /// Returns true iff this tech has been completely researched.
    bool TechResearched(const std::string& name) const;

    /// Returns the status (researchable, researched, unresearchable) for this tech for this
    TechStatus GetTechStatus(const std::string& name) const;

    /// Returns the set of all available building types.
    const std::set<std::string>& AvailableBuildingTypes() const;

    /// Returns true if the given building type is known to this empire, false if it is not
    bool BuildingTypeAvailable(const std::string& name) const;

    /// Returns the set of all ship design ids of this empire
    const std::set<int>& ShipDesigns() const;

    /// Returns the set of ship design ids of this empire that the empire can actually build
    std::set<int> AvailableShipDesigns() const;

    /// Returns true iff this ship design can be built by this empire.  If no such ship design exists, returns false
    bool ShipDesignAvailable(int ship_design_id) const;

    /// Returns true iff the given ship design id is in the set of design ids of this empire.  That is, it has been added to this empire.
    bool ShipDesignKept(int ship_design_id) const;

    /// Returns the set of ship part names this empire that the empire can currently build
    std::set<std::string> AvailableShipParts() const;

    /// Returns true iff this ship part can be built by this empire.  If no such ship part exists, returns false
    bool ShipPartAvailable(const std::string& name) const;

    /// Returns the set of ship hull names that that the empire can currently build
    std::set<std::string> AvailableShipHulls() const;

    /// Returns true iff this ship hull can be built by this empire.  If no such ship hull exists, returns false
    bool ShipHullAvailable(const std::string& name) const;

    /// Returns the queue of items being or queued to be produced.
    const ProductionQueue& GetProductionQueue() const;

    /** Returns the PPs spent towards item \a i in the build queue if it has partial progress, -1.0 if there is no such
        index in the production queue. */
    double ProductionStatus(int i) const;

    /** Returns the cost per turn and the number of turns required to produce the indicated item, or (-1.0, -1) if the
        item is unknown, unavailable, or invalid. */
    std::pair<double, int> ProductionCostAndTime(BuildType build_type, std::string name) const;

    /** Returns the cost per turn and the number of turns required to produce the indicated item, or (-1.0, -1) if the
        item is unknown, unavailable, or invalid. */
    std::pair<double, int> ProductionCostAndTime(BuildType build_type, int design_id = UniverseObject::INVALID_OBJECT_ID) const;

    /** Returns the cost per turn and the number of turns required to produce the indicated item, or (-1.0, -1) if the
        item is unknown, unavailable, or invalid. */
    std::pair<double, int> ProductionCostAndTime(const ProductionQueue::ProductionItem& item) const;

    /** Returns true iff this empire can produce the specified item at the specified location. */
    bool BuildableItem(BuildType build_type, std::string name, int location) const;

    /** Returns true iff this empire can produce the specified item at the specified location. */
    bool BuildableItem(BuildType build_type, int design_id, int location) const;

    /** Returns true iff this empire can produce the specified item at the specified location. */
    bool BuildableItem(const ProductionQueue::ProductionItem& item, int location) const;

    /** Returns true if the given item is in the appropriate list, false if it is not. */
    bool HasExploredSystem(int ID) const;

    /** Returns the number of entries in the SitRep. */
    int NumSitRepEntries() const;

    /** clears and sets passed parameters, which are
      * first:  set of system ids where fleets can be supplied by this empire
      * second: starlanes along which fleet supply can flow.  entries are directed, so the same starlane
      *         could appear twice - once for each direction.  the first value is the start and the second
      *         value is the end of the lane traversals that can carry fleet supplies. */
    void GetSupplyableSystemsAndStarlanesUsed(std::set<int>& supplyable_system_ids, 
                                              std::set<std::pair<int, int> >& supply_starlane_traversals) const;

    /** clears and sets passed parameters, which are
      * first:  set sets of system ids that are able to share resources between and within eachother
      * second: starlanes along which resources can be exchanged.  entries are directed, so the same starlane
      *         could appear twice - once for each direction.  the first value is the start and the second
      *         value is the end of the lane traversals that can carry resources between systems. */
    void GetSupplySystemGroupsAndStarlanesUsed(std::set<std::set<int> >& supply_system_groups, std::set<std::pair<int, int> >& supply_starlane_traversals) const;

    /** modifies passed parameter, which is the set of system ids where fleet supply can be propegated from
      * one starlane to the next, or where supply can be delivered if a supply route can reach the system.
      */
    void GetSupplyUnobstructedSystems(std::set<int>& unobstructed_system_ids) const;

    /** modifies passed parameter, which is a map from system id to the range, in starlane jumps that the
      * system can send supplies.
      */
    void GetSystemSupplyRanges(std::map<int, int>& system_supply_ranges) const;

    TechItr TechBegin() const;
    TechItr TechEnd() const;
    BuildingTypeItr AvailableBuildingTypeBegin() const;
    BuildingTypeItr AvailableBuildingTypeEnd() const;
    SystemIDItr ExploredBegin() const;
    SystemIDItr ExploredEnd() const;
    ShipDesignItr ShipDesignBegin() const;
    ShipDesignItr ShipDesignEnd() const;
    SitRepItr SitRepBegin() const;
    SitRepItr SitRepEnd() const;

    /** Returns the number of production points available to the empire (this is the minimum of available industry and available minerals) */
    double ProductionPoints() const;

    /** Returns amount of food empire will distribute this turn.  Assumes Empire::UpdateFoodDistribution() has
      * previously been called to determine this number.  
      * Equivalent to calling GetResearchQueue().TotalRPsSpent() for Research)
      */
    double TotalFoodDistributed() const {return m_food_total_distributed;}

    /** Returns amount of trade empire will spend this turn.  Assumes Empire::UpdateTradeSpending() has
      * previously been called to determine this number.
      * Equivalent to calling GetResearchQueue().TotalRPsSpent() for Research)
      */
    double TotalTradeSpending() const {return m_maintenance_total_cost;}
    //@}

    const ResourcePool& GetMineralResPool() const {return m_mineral_resource_pool;}
    const ResourcePool& GetFoodResPool() const {return m_food_resource_pool;}
    const ResourcePool& GetResearchResPool() const {return m_research_resource_pool;}
    const ResourcePool& GetIndustryResPool() const {return m_industry_resource_pool;}
    const ResourcePool& GetTradeResPool() const {return m_trade_resource_pool;}
    const PopulationPool& GetPopulationPool() const {return m_population_pool;}    



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

    /** Adds the indicated build to the production queue, placing it before position \a pos.  If \a pos < 0 or
        queue.size() <= pos, the build is placed at the end of the queue. */
    void PlaceBuildInQueue(BuildType build_type, int design_id, int number, int location, int pos = -1);

    /** Adds the indicated build to the production queue, placing it before position \a pos.  If \a pos < 0 or
        queue.size() <= pos, the build is placed at the end of the queue. */
    void PlaceBuildInQueue(const ProductionQueue::ProductionItem& item, int number, int location, int pos = -1);

    /// Changes the remaining number to build for queue item \a index to \a quantity
    void SetBuildQuantity(int index, int quantity);

    /// Moves \a tech from the production queue, if it is in the production queue already.
    void MoveBuildWithinQueue(int index, int new_index);

    /// Removes the build at position \a index in the production queue, if such an index exists.
    void RemoveBuildFromQueue(int index);

    /** Processes Builditems on queues of empires other than this empire, at the location with id \a location_id and,
        as appropriate, adds them to the build queue of \a this empire, deletes them, or leaves them on the build 
        queue of their current empire */
    void ConquerBuildsAtLocation(int location_id);

    /// Inserts the given Tech into the Empire's list of available technologies.
    void AddTech(const std::string& name);

    /// Adds a given buildable item (Building, Ship Component, etc.) to the list of available buildable items.
    void UnlockItem(const ItemSpec& item);

    /// Inserts the given BuildingType into the Empire's list of available BuldingTypes.
    void AddBuildingType(const std::string& name);

    /// Inserts the given ID into the Empire's list of explored systems.
    void AddExploredSystem(int ID);

    /// inserts given design id into the empire's set of designs
    void AddShipDesign(int ship_design_id);

    /// inserts given ShipDesign into the Universe, adds the design's id to the Empire's set of ids, and returns the new design's id, which is UniverseObject::INVALID_OBJECT_ID on failure.  If successful, universe takes ownership of passed ShipDesign.
    int AddShipDesign(ShipDesign* ship_design);

    /// generates a random ship name, appending II, III, etc., to it if it has been used before by this empire
    std::string NewShipName();

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

    /// Removes the ShipDesign with the given id from the empire's set
    void RemoveShipDesign(int ship_design_id);

    /// Clears all sitrep entries;
    void ClearSitRep();

    /** Checks for production projects that have been completed, and places them at their respective
      * production sites.  Which projects have been completed is determined by the results of
      * previously-called Update() on the production queue (which determines how much PP each project
      * receives, but does not actually spend them).  This function spends the PP, removes complete
      * items from the queue and creates the results in the universe.  Also updates the empire's 
      * minerals stockpile to account for any excess not used or any shortfall that was made up by
      * taking from the stockpile 
      */
    void CheckProductionProgress();

    /// Checks for tech projects that have been completed, and adds them to the known techs list.
    void CheckResearchProgress();

    /** Eventually : Will check for social projects that have been completed and/or
      * process ongoing social projects... (not sure exactly what form "social projects" will take
      * or how they will work).  Also will update the empire's trade stockpile to account for trade
      * production and expenditures.
      * Currently: Deducts cost of maintenance of buildings from empire's trade stockpile
      */
    void CheckTradeSocialProgress();

    /// Updates food stockpile.  Growth actually occurs in PopGrowthProductionResearchPhase() of objects
    void CheckGrowthFoodProgress();
        
    /// Mutator for empire color
    void SetColor(const GG::Clr& color);

    /// Mutator for empire name
    void SetName(const std::string& name);

    /// Mutator for empire's player name
    void SetPlayerName(const std::string& player_name);
    //@}

    /** Resets production of resources and calculates spending (on each item in queues and 
      * overall) for each resource by calling UpdateResearchQueue, UpdateProductionQueue,
      * UpdateTradeSpending, and UpdateFoodDistribution.  Does not actually "spend" resources,
      * but just determines how much and on what to spend.  Actual spending, removal of items
      * from queue, processing of finished items and population growth happens in various
      * Check(Whatever)Progress functions.
      */
    void UpdateResourcePool();

    /** Calls Update() on empire's research queue, which recalculates the RPs spent on and
      * number of turns left for each tech in the queue.
      */
    void UpdateResearchQueue();

    /** Calls Update() on empire's production queue, which recalculates the PPs spent on and
      * number of turns left for each project in the queue.
      */
    void UpdateProductionQueue();

    /** Eventually: Calls appropriate subsystem Update to calculate trade spent on social projects
      * and maintenance of buildings.  Later call to CheckTradeSocialProgress() will then have the
      * correct allocations of trade.
      * Currently: Sums maintenance costs of all buildings owned by empire, sets m_maintenance_total_cost
      */
    void UpdateTradeSpending();

    /** Allocates available food to PopCenters.  Doesn't actually distribute food; just calculates how
      * how much food each PopCenter gets.  Sets m_food_total_distributed to indicate how much food
      * the empire will distribute (instead of the Production or Research-like system of having a subsystem
      * keep track, since there is no separate food distribution subsystem class).  Does not automatically
      * update population growth estimates, so UpdatePopulationGrowth() may need to be called after calling
      * this function.
      */
    void UpdateFoodDistribution();

    /** Has m_population_pool recalculate all PopCenters' and empire's total expected population growth
      * Assumes UpdateFoodDistribution() has been called to determine food allocations to each planet (which
      * are a factor in the growth prediction calculation).
      */
    void UpdatePopulationGrowth();

    ResourcePool& GetMineralResPool() {return m_mineral_resource_pool;}
    ResourcePool& GetFoodResPool() {return m_food_resource_pool;}
    ResourcePool& GetResearchResPool() {return m_research_resource_pool;}
    ResourcePool& GetIndustryResPool() {return m_industry_resource_pool;}
    ResourcePool& GetTradeResPool() {return m_trade_resource_pool;}
    PopulationPool& GetPopulationPool() {return m_population_pool;}

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
    std::map<std::string, double> m_research_progress;

    /// the queue of items being or waiting to be built
    ProductionQueue m_production_queue;

    /// progress of partially-completed builds; completed items are removed
    std::vector<double> m_production_progress;

    /// list of acquired BuildingType.  These are string names referencing BuildingType objects
    std::set<std::string> m_available_building_types;

    /// systems you've explored
    std::set<int> m_explored_systems;

    /// The Empire's ship designs, indexed by design id
    std::set<int> m_ship_designs;

    /// The Empire's sitrep entries
    std::list<SitRepEntry*> m_sitrep_entries;

    /// The Empire resource & population pools
    ResourcePool m_mineral_resource_pool;
    ResourcePool m_food_resource_pool;
    ResourcePool m_research_resource_pool;
    ResourcePool m_industry_resource_pool;
    ResourcePool m_trade_resource_pool;

    PopulationPool m_population_pool;

    /** set by UpdateFoodDistribution() to indicate how much food the empire will distribute this turn.  Unlike
      * the Research and Production subsystems, food distribution only requires a single function and doesn't
      * have its own queue class.  So, instead of GetFoodQueue().TotalFoodDistributed(), one just calls
      * Empire::TotalFoodDistributed(), which returns this value.
      */
    double m_food_total_distributed;

    /** MAYBE TEMPORARY: Until social projects and/or consequences of unpaid maintenance is implemented.
      * Total maintenance on buildings owned by this empire.  Set by UpdateTradeSpending(), used
      * by CheckTradeSocialProgress() to deduct maintenance cost from trade stockpile
      */
    double m_maintenance_total_cost;

    std::map<std::string, int> m_ship_names_used; // map from name to number of times used

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
        & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(design_id);
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
            & BOOST_SERIALIZATION_NVP(m_research_progress)
            & BOOST_SERIALIZATION_NVP(m_production_queue)
            & BOOST_SERIALIZATION_NVP(m_production_progress)
            & BOOST_SERIALIZATION_NVP(m_available_building_types)
            & BOOST_SERIALIZATION_NVP(m_explored_systems)
            & BOOST_SERIALIZATION_NVP(m_ship_designs)
            & BOOST_SERIALIZATION_NVP(m_sitrep_entries)
            & BOOST_SERIALIZATION_NVP(m_mineral_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_food_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_research_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_industry_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_trade_resource_pool)
            & BOOST_SERIALIZATION_NVP(m_population_pool)
            & BOOST_SERIALIZATION_NVP(m_food_total_distributed)
            & BOOST_SERIALIZATION_NVP(m_maintenance_total_cost)
            & BOOST_SERIALIZATION_NVP(m_ship_names_used);
    }
}

#endif // _Empire_h_




