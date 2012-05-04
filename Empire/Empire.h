// -*- C++ -*-
#ifndef _Empire_h_
#define _Empire_h_

#include <GG/Clr.h>
#include "../util/SitRepEntry.h"
#include "../universe/Tech.h"
#include "../universe/UniverseObject.h"
#include "ResourcePool.h"
#include "../universe/Meter.h"

#include <deque>
#include <list>
#include <string>

class BuildingType;
class ShipDesign;
class Empire;
class Meter;

class Alignment {
public:
    Alignment(const std::string& name, const std::string& description, const std::string& graphic) :
        m_name(name),
        m_description(description),
        m_graphic(graphic)
    {}
    Alignment() :
        m_name(),
        m_description(),
        m_graphic()
    {}
    const std::string&  Name() const;
    const std::string&  Description() const;
    const std::string&  Graphic() const;
private:
    std::string m_name;
    std::string m_description;
    std::string m_graphic;
};

struct ResearchQueue {
    /** The type of a single element in the research queue. */
    struct Element {
        Element() :
            name(),
            allocated_rp(0.0),
            turns_left(0)
        {}
        Element(const std::string& name_, double spending_, int turns_left_) :
            name(name_),
            allocated_rp(spending_),
            turns_left(turns_left_)
        {}
        std::string name;
        double      allocated_rp;
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

    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ResearchQueueChangedSignalType;    ///< emitted when something is added to, removed from or altered on the queue
    //@}

    /** \name Structors */ //@{
    ResearchQueue() :
        m_projects_in_progress(0),
        m_total_RPs_spent(0.0)
    {}
    //@}

    /** \name Accessors */ //@{
    bool            InQueue(const std::string& tech_name) const;    ///< Returns true iff \a tech is in this queue.
    int             ProjectsInProgress() const;         ///< Returns the number of research projects currently (perhaps partially) funded.
    double          TotalRPsSpent() const;              ///< Returns the number of RPs currently spent on the projects in this queue.

    // STL container-like interface
    bool            empty() const;
    unsigned int    size() const;
    const_iterator  begin() const;
    const_iterator  end() const;
    const_iterator  find(const std::string& tech_name) const;
    const Element&  operator[](int i) const;

    /** Returns an iterator to the underfunded research project, or end() if
      * none exists. */
    const_iterator  UnderfundedProject() const;
    //@}

    /** \name Mutators */ //@{
    /** Recalculates the RPs spent on and number of turns left for each project
      * in the queue.  Also determines the number of projects in prgress, and
      * the total number of RPs spent on the projects in the queue.  \note A
      * precondition of this function that \a RPs must be greater than some
      * epsilon > 0; see the implementation for the actual value used for
      * epsilon. */
    void            Update(Empire* empire, double RPs, const std::map<std::string, double>& research_progress);

    // STL container-like interface
    void            push_back(const std::string& tech_name);
    void            insert(iterator it, const std::string& tech_name);
    void            erase(iterator it);

    iterator        begin();
    iterator        end();
    iterator        find(const std::string& tech_name);

    void            clear();

    /** Returns an iterator to the underfunded research project, or end() if
      * none exists. */
    iterator        UnderfundedProject();

    mutable ResearchQueueChangedSignalType ResearchQueueChangedSignal;
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

        ProductionItem  item;
        int             ordered;                    ///< how many of item to produce
        int             remaining;                  ///< how many left to produce
        int             location;                   ///< the ID of the UniverseObject at which this item is being produced
        double          allocated_pp;               ///< amount of PP allocated to this ProductionQueue Element by Empire production update
        int             turns_left_to_next_item;
        int             turns_left_to_completion;

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
    int                             ProjectsInProgress() const;         ///< Returns the number of production projects currently (perhaps partially) funded.
    double                          TotalPPsSpent() const;              ///< Returns the number of PPs currently spent on the projects in this queue.

    /** Returns map from sets of system ids that can share resources to amount
      * of PP available in those groups of systems */
    std::map<std::set<int>, double> AvailablePP(const std::map<ResourceType, boost::shared_ptr<ResourcePool> >& resource_pools) const;

    /** Returns map from sets of system ids that can share resources to amount
      * of PP allocated to production queue elements that have build locations
      * in systems in the group. */
    std::map<std::set<int>, double> AllocatedPP() const;


    // STL container-like interface
    bool                            empty() const;
    unsigned int                    size() const;
    const_iterator                  begin() const;
    const_iterator                  end() const;
    const_iterator                  find(int i) const;
    const Element&                  operator[](int i) const;

    /** Returns an iterator to the underfunded production project, or end() if none exists. */
    const_iterator                  UnderfundedProject(const Empire* empire) const;
    //@}

    /** \name Mutators */ //@{
    /** Recalculates the PPs spent on and number of turns left for each project in the queue.  Also
      * determines the number of projects in progress, and the minerals and industry consumed by projects
      * in each resource-sharing group of systems.  Does not actually "spend" the PP; a later call to
      * empire->CheckProductionProgress() will actually spend PP, remove items from queue and create them
      * in the universe. */
    void                            Update(Empire* empire, const std::map<ResourceType,
                                           boost::shared_ptr<ResourcePool> >& resource_pools,
                                           const std::vector<double>& production_status);

    // STL container-like interface
    void                            push_back(const Element& element);
    void                            insert(iterator it, const Element& element);
    void                            erase(int i);
    iterator                        erase(iterator it);

    iterator                        begin();
    iterator                        end();
    iterator                        find(int i);
    Element&                        operator[](int i);

    void                            clear();

    /** Returns an iterator to the underfunded production project, or end() if none exists. */
    iterator                        UnderfundedProject(const Empire* empire);

    mutable ProductionQueueChangedSignalType ProductionQueueChangedSignal;
    //@}

private:
    QueueType                       m_queue;
    int                             m_projects_in_progress;
    std::map<std::set<int>, double> m_object_group_allocated_pp;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** Class to maintain the state of a single empire. In both the client and
  * server, Empires are managed by a subclass of EmpireManager, and can be
  * accessed from other modules by using the EmpireManager::Lookup() method to
  * obtain a pointer. */
class Empire {
public:
    // EmpireManagers must be friends so that they can have access to the constructor and keep it hidden from others
    friend class EmpireManager;

    /** \name Iterator Types */ //@{
    typedef std::set<std::string>::const_iterator   TechItr;
    typedef std::set<std::string>::const_iterator   BuildingTypeItr;
    typedef std::set<int>::const_iterator           SystemIDItr;
    typedef std::set<int>::const_iterator           ShipDesignItr;
    typedef std::list<SitRepEntry*>::const_iterator SitRepItr;
    //@}

    /** \name Structors */ //@{
    Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color);  ///< basic constructor
    ~Empire();
    //@}

    /** \name Accessors */ //@{
    const std::string&      Name() const;                                           ///< Returns the Empire's name
    const std::string&      PlayerName() const;                                     ///< Returns the Empire's player's name

    int                     EmpireID() const;                                       ///< Returns the Empire's unique numeric ID

    const GG::Clr&          Color() const;                                          ///< Returns the Empire's color

    int                     CapitalID() const;                                      ///< Returns the numeric ID of the empire's capital
    int                     StockpileID(ResourceType res = INVALID_RESOURCE_TYPE) const;    ///< Returns the numeric ID of the empire's stockpile location for the resource of type \a res

    std::string             Dump() const;

    const std::set<std::string>&    AvailableTechs() const;                         ///< Returns the set of all available techs.
    const std::set<std::string>&    AvailableBuildingTypes() const;                 ///< Returns the set of all available building types.
    std::set<int>                   AvailableShipDesigns() const;                   ///< Returns the set of ship design ids of this empire that the empire can actually build
    const std::set<int>&            ShipDesigns() const;                            ///< Returns the set of all ship design ids of this empire
    const std::set<std::string>&    AvailableShipParts() const;                     ///< Returns the set of ship part names this empire that the empire can currently build
    const std::set<std::string>&    AvailableShipHulls() const;                     ///< Returns the set of ship hull names that that the empire can currently build

    const Meter*            GetMeter(const std::string& name) const;                ///< Returns the alignment meter with the indicated name, if any, or 0 if no such alignment meter exists
    std::map<std::string, Meter>::const_iterator
                            meter_begin() const { return m_meters.begin(); }
    std::map<std::string, Meter>::const_iterator
                            meter_end() const   { return m_meters.end(); }

    bool                    ResearchableTech(const std::string& name) const;        ///< Returns true iff \a name is a tech that has not been researched, and has no unresearched prerequisites.
    const                   ResearchQueue& GetResearchQueue() const;                ///< Returns the queue of techs being or queued to be researched.
    double                  ResearchProgress(const std::string& name) const;          ///< Returns the RPs spent towards tech \a name if it has partial research progress, or 0.0 if it is already researched.
    bool                    TechResearched(const std::string& name) const;          ///< Returns true iff this tech has been completely researched.
    TechStatus              GetTechStatus(const std::string& name) const;           ///< Returns the status (researchable, researched, unresearchable) for this tech for this

    bool                    BuildingTypeAvailable(const std::string& name) const;   ///< Returns true if the given building type is known to this empire, false if it is not
    bool                    ShipDesignAvailable(int ship_design_id) const;          ///< Returns true iff this ship design can be built by this empire.  If no such ship design exists, returns false
    bool                    ShipDesignKept(int ship_design_id) const;               ///< Returns true iff the given ship design id is in the set of design ids of this empire.  That is, it has been added to this empire.
    bool                    ShipPartAvailable(const std::string& name) const;       ///< Returns true iff this ship part can be built by this empire.  If no such ship part exists, returns false
    bool                    ShipHullAvailable(const std::string& name) const;       ///< Returns true iff this ship hull can be built by this empire.  If no such ship hull exists, returns false

    const                   ProductionQueue& GetProductionQueue() const;            ///< Returns the queue of items being or queued to be produced.
    double                  ProductionStatus(int i) const;                          ///< Returns the PPs spent towards item \a i in the build queue if it has partial progress, -1.0 if there is no such index in the production queue.

    /** Returns the maximum cost per turn and the minimum number of turns
      * required to produce the indicated item, or (-1.0, -1) if the item is
      * unknown, unavailable, or invalid. */
    std::pair<double, int>  ProductionCostAndTime(BuildType build_type, std::string name) const;
    /** Returns the maximum cost per turn and the minimum number of turns
      * required to produce the indicated item, or (-1.0, -1) if the item is
      * unknown, unavailable, or invalid. */
    std::pair<double, int>  ProductionCostAndTime(BuildType build_type, int design_id = INVALID_OBJECT_ID) const;
    /** Returns the maximum cost per turn and the minimum number of turns
      * required to produce the indicated item, or (-1.0, -1) if the item is
      * unknown, unavailable, or invalid. */
    std::pair<double, int>  ProductionCostAndTime(const ProductionQueue::ProductionItem& item) const;

    bool                    BuildableItem(BuildType build_type, const std::string& name, int location) const;  ///< Returns true iff this empire can produce the specified item at the specified location.
    bool                    BuildableItem(BuildType build_type, int design_id, int location) const;            ///< Returns true iff this empire can produce the specified item at the specified location.
    bool                    BuildableItem(const ProductionQueue::ProductionItem& item, int location) const;    ///< Returns true iff this empire can produce the specified item at the specified location.

    bool                    HasExploredSystem(int ID) const;                                ///< returns  true if the given item is in the appropriate list, false if it is not.

    int                     NumSitRepEntries() const;                                       ///< number of entries in the SitRep.

    const std::set<int>&                    FleetSupplyableSystemIDs() const;               ///< returns set of system ids where fleets can be supplied by this empire (as determined by object supply meters and rules of supply propagation and blockade)
    const std::set<std::pair<int, int> >&   FleetSupplyStarlaneTraversals() const;          ///< returns set of directed starlane traversals along which supply can flow.  results are pairs of system ids of start and end system of traversal
    const std::map<int, int>&               FleetSupplyRanges() const;                      ///< returns map from system id to number of starlane jumps away the system can deliver fleet supply

    const std::set<std::set<int> >&         ResourceSupplyGroups() const;                   ///< returns set of sets of systems that can share industry and minerals (systems in separate groups are blockaded or otherwise separated)
    const std::set<std::pair<int, int> >&   ResourceSupplyStarlaneTraversals() const;       ///< returns set of directed starlane traversals along which system resource exchange (industry, minerals) can flow.  results are pairs of system ids of start and end of traversal
    const std::set<std::pair<int, int> >&   ResourceSupplyOstructedStarlaneTraversals() const;      ///< returns set of directed starlane traversals along which system resources could flow for this empire, but which can't due to some obstruction in the destination system
    const std::map<int, int>&               ResourceSupplyRanges() const;                   ///< returns map from system id to number of starlane jumps away the system can exchange resources

    bool                                    FleetOrResourceSupplyableAtSystem(int system_id) const; ///< returns true if system with id \a system_id is fleet supplyable or in one of the resource supply groups of this empire.

    const std::set<int>&                    SupplyUnobstructedSystems() const;              ///< returns set of system ids that are able to propagate supply from one system to the next, or at which supply can be delivered to fleets if supply can reach the system from elsewhere

    /** modifies passed parameter, which is a map from system id to the range,
      * in starlane jumps that the system can send supplies. */
    void                    GetSystemSupplyRanges(std::map<int, int>& system_supply_ranges) const;

    const std::set<int>&    ExploredSystems() const;            ///< returns set of ids of systems that this empire has explored
    const std::map<int, std::set<int> >
                            KnownStarlanes() const;             ///< returns map from system id (start) to set of system ids (endpoints) of all starlanes known to this empire
    const std::map<int, std::set<int> >
                            VisibleStarlanes() const;           ///< returns map from system id (start) to set of system ids (endpoints) of all starlanes visible to this empire this turn

    TechItr                 TechBegin() const;                  ///< starting iterator for techs this empire has researched
    TechItr                 TechEnd() const;                    ///< end iterator for techs
    BuildingTypeItr         AvailableBuildingTypeBegin() const; ///< starting iterator for building types this empire can produce
    BuildingTypeItr         AvailableBuildingTypeEnd() const;   ///< end iterator for building types
    ShipDesignItr           ShipDesignBegin() const;            ///< starting iterator for ship designs this empire has on file.  individual designs may or may not be producible for this empire
    ShipDesignItr           ShipDesignEnd() const;              ///< end iterator for ship designs
    SitRepItr               SitRepBegin() const;                ///< starting iterator for sitrep entries for this empire
    SitRepItr               SitRepEnd() const;                  ///< end iterator for sitreps

    double                  ProductionPoints() const;           ///< Returns the number of production points available to the empire (this is the minimum of available industry and available minerals)

    /** Returns amount of trade empire will spend this turn.  Assumes
      * Empire::UpdateTradeSpending() has previously been called to determine
      * this number. */
    double                  TotalTradeSpending() const {return m_maintenance_total_cost;}

    const ResourcePool*     GetResourcePool(ResourceType resource_type) const;  ///< Returns ResourcePool for \a resource_type or 0 if no such ResourcePool exists
    double                  ResourceStockpile(ResourceType type) const;         ///< returns current stockpiled amount of resource \a type
    double                  ResourceMaxStockpile(ResourceType type) const;      ///< returns maximum allowed stockpile of resource \a type
    double                  ResourceProduction(ResourceType type) const;        ///< returns amount of resource \a type being produced by ResourceCenters
    double                  ResourceAvailable(ResourceType type) const;         ///< returns amount of resource \a type immediately available.  This = production + stockpile

    const PopulationPool&   GetPopulationPool() const;                          ///< Returns PopulationPool
    double                  Population() const;                                 ///< returns total Population of empire
    //@}

    /** \name Mutators */ //@{
    /** If the object with id \a id is a planet owned by this empire, sets that
      * planet to be this empire's capital, and otherwise does nothing. */
    void                    SetCapitalID(int id);

    /** Returns the alignment meter with the indicated \a name, if any, or 0 if
      * no such alignment meter exists. */
    Meter*                  GetMeter(const std::string& name);

    void                    BackPropegateMeters();

    /** Adds \a tech to the research queue, placing it before position \a pos.
      * If \a tech is already in the queue, it is moved to \a pos, then removed
      * from its former position.  If \a pos < 0 or queue.size() <= pos, \a tech
      * is placed at the end of the queue. If \a tech is already available, no
      * action is taken. */
    void                    PlaceTechInQueue(const std::string& name, int pos = -1);

    /** Removes tech with \a name from the research queue, if it is in the
      * research queue already. */
    void                    RemoveTechFromQueue(const std::string& name);

    /** Sets research progress of tech with \a name to \a progress. */
    void                    SetTechResearchProgress(const std::string& name, double progress);

    /** Adds the indicated build to the production queue, placing it before
      * position \a pos.  If \a pos < 0 or queue.size() <= pos, the build is
      * placed at the end of the queue. */
    void                    PlaceBuildInQueue(BuildType build_type, const std::string& name, int number, int location, int pos = -1);

    /** Adds the indicated build to the production queue, placing it before
      * position \a pos.  If \a pos < 0 or queue.size() <= pos, the build is
      * placed at the end of the queue. */
    void                    PlaceBuildInQueue(BuildType build_type, int design_id, int number, int location, int pos = -1);

    /** Adds the indicated build to the production queue, placing it before
      * position \a pos.  If \a pos < 0 or queue.size() <= pos, the build is
      * placed at the end of the queue. */
    void                    PlaceBuildInQueue(const ProductionQueue::ProductionItem& item, int number, int location, int pos = -1);

    void                    SetBuildQuantity(int index, int quantity);      ///< Changes the remaining number to build for queue item \a index to \a quantity
    void                    MoveBuildWithinQueue(int index, int new_index); ///< Moves \a tech from the production queue, if it is in the production queue already.
    void                    RemoveBuildFromQueue(int index);                ///< Removes the build at position \a index in the production queue, if such an index exists.

    /** Processes Builditems on queues of empires other than this empire, at
      * the location with id \a location_id and, as appropriate, adds them to
      * the build queue of \a this empire, deletes them, or leaves them on the
      * build queue of their current empire */
    void                    ConquerBuildsAtLocation(int location_id);

    void                    AddTech(const std::string& name);               ///< Inserts the given Tech into the Empire's list of available technologies.
    void                    UnlockItem(const ItemSpec& item);               ///< Adds a given buildable item (Building, Ship Component, etc.) to the list of available buildable items.
    void                    AddBuildingType(const std::string& name);       ///< Inserts the given BuildingType into the Empire's list of available BuldingTypes.
    void                    AddPartType(const std::string& name);           ///< Inserts the given ship PartType into the Empire's list of available BuldingTypes.
    void                    AddHullType(const std::string& name);           ///< Inserts the given ship HullType into the Empire's list of available BuldingTypes.
    void                    AddExploredSystem(int ID);                      ///< Inserts the given ID into the Empire's list of explored systems.
    void                    AddShipDesign(int ship_design_id);              ///< inserts given design id into the empire's set of designs
    int                     AddShipDesign(ShipDesign* ship_design);         ///< inserts given ShipDesign into the Universe, adds the design's id to the Empire's set of ids, and returns the new design's id, which is INVALID_OBJECT_ID on failure.  If successful, universe takes ownership of passed ShipDesign.

    std::string             NewShipName();                                  ///< generates a random ship name, appending II, III, etc., to it if it has been used before by this empire

    void                    EliminationCleanup();                           ///< Cleans up empire after it is eliminated.  Queues are cleared, capital is reset, and other state info not relevant to an eliminated empire is cleared

    /** Inserts the a pointer to given SitRep entry into the empire's sitrep list.
     *  \warning When you call this method, you are transferring ownership
     *  of the entry object to the Empire.
     *  The object pointed to by 'entry' will be deallocated when
     *  the empire's sitrep is cleared.  Be careful you do not have any
     *  references to SitRepEntries lying around when this happens.
     *  You \a must pass in a dynamically allocated sitrep entry */
    void                    AddSitRepEntry(SitRepEntry* entry);

    void                    ClearSitRep();                                  ///< Clears all sitrep entries

    void                    RemoveTech(const std::string& name);            ///< Removes the given Tech from the empire's list
    void                    LockItem(const ItemSpec& item);                 ///< Removes a given buildable item (Building, ShipComponent, etc.) from the list of available buildable items.
    void                    RemoveBuildingType(const std::string& name);    ///< Removes the given BuildingType from the empire's list
    void                    RemovePartType(const std::string& name);        ///< Removes the given PartType from the empire's list
    void                    RemoveHullType(const std::string& name);        ///< Removes the given HullType from the empire's list
    void                    RemoveShipDesign(int ship_design_id);           ///< Removes the ShipDesign with the given id from the empire's set

    void                    UpdateSystemSupplyRanges(const std::set<int>& known_objects);           ///< Calculates ranges that systems can send fleet and resource supplies, using the specified st of \a known_objects as the source for supply-producing objects and systems through which it can be propegated
    void                    UpdateSystemSupplyRanges();                     ///< Calculates ranges that systems can send fleet and resource supplies.
    void                    UpdateSupplyUnobstructedSystems(const std::set<int>& known_systems);    ///< Calculates systems that can propegate supply (fleet or resource) using the specified set of \a known_systems
    void                    UpdateSupplyUnobstructedSystems();              ///< Calculates systems that can propegate supply using this empire's own / internal list of explored systems
    void                    UpdateFleetSupply(const std::map<int, std::set<int> >& starlanes);      ///< Calculates systems at which fleets of this empire can be supplied and starlane traversals used to do so, using the indicated \a starlanes but subject to obstruction of supply by various factors.  Call UpdateSystemSupplyRanges and UpdateSupplyUnobstructedSystems before calling this.
    void                    UpdateFleetSupply();                            ///< Calculates systems at which fleets of this empire can be supplied and starlane traversals used to do so using this empire's set of known starlanes.  Call UpdateSystemSupplyRanges and UpdateSupplyUnobstructedSystems before calling this.
    void                    UpdateResourceSupply(const std::map<int, std::set<int> >& starlanes);   ///< Calculates groups of systems of this empire which can exchange resources and the starlane traversals used to do so, using the indicated \a starlanes but subject to obstruction of supply propegation by various factors.  Call UpdateSystemSupplyRanges and UpdateSupplyUnobstructedSystems before calling this.
    void                    UpdateResourceSupply();                         ///< Calculates groups of systems of this empire which can exchange resources and starlane traversals used to do so using this empire's set of known starlanes.  Call UpdateSystemSupplyRanges and UpdateSupplyUnobstructedSystems before calling this.

    /** Checks for production projects that have been completed, and places them at their respective
      * production sites.  Which projects have been completed is determined by the results of
      * previously-called Update() on the production queue (which determines how much PP each project
      * receives, but does not actually spend them).  This function spends the PP, removes complete
      * items from the queue and creates the results in the universe.  Also updates the empire's
      * minerals stockpile to account for any excess not used or any shortfall that was made up by
      * taking from the stockpile. */
    void                    CheckProductionProgress();

    /** Checks for tech projects that have been completed, and adds them to the known techs list. */
    void                    CheckResearchProgress();

    /** Eventually : Will check for social projects that have been completed and/or
      * process ongoing social projects... (not sure exactly what form "social projects" will take
      * or how they will work).  Also will update the empire's trade stockpile to account for trade
      * production and expenditures.
      * Currently: Deducts cost of maintenance of buildings from empire's trade stockpile */
    void                    CheckTradeSocialProgress();


    void                    SetColor(const GG::Clr& color);                 ///< Mutator for empire color
    void                    SetName(const std::string& name);               ///< Mutator for empire name
    void                    SetPlayerName(const std::string& player_name);  ///< Mutator for empire's player name

    void                    SetResourceStockpile(ResourceType resource_type, double stockpile); ///< Sets current \a stockpile amount of indicated \a resource_type
    void                    SetResourceMaxStockpile(ResourceType resource_type, double max);    ///< Sets \a max amount of stockpile of indicated \a resource_typ

    /** Determines ResourceCenters that can provide resources for this empire and sets
      * the supply groups used for each ResourcePool as appropriate for each resource.
      * call UpdateResourceSupply before calling this. */
    void                    InitResourcePools();

    /** Resets production of resources and calculates allocated resources (on each item in
      * queues and overall) for each resource by calling UpdateResearchQueue, UpdateProductionQueue,
      * UpdateTradeSpending.  Does not actually "spend" resources,
      * but just determines how much and on what to spend.  Actual consumption of resources, removal
      * of items from queue, processing of finished items and population growth happens in various
      * Check(Whatever)Progress functions. */
    void                    UpdateResourcePools();

    /** Calls Update() on empire's research queue, which recalculates the RPs spent on and
      * number of turns left for each tech in the queue. */
    void                    UpdateResearchQueue();

    /** Calls Update() on empire's production queue, which recalculates the PPs spent on and
      * number of turns left for each project in the queue. */
    void                    UpdateProductionQueue();

    /** Eventually: Calls appropriate subsystem Update to calculate trade spent on social projects
      * and maintenance of buildings.  Later call to CheckTradeSocialProgress() will then have the
      * correct allocations of trade.
      * Currently: Sums maintenance costs of all buildings owned by empire, sets m_maintenance_total_cost */
    void                    UpdateTradeSpending();

    /** Has m_population_pool recalculate all PopCenters' and empire's total
      * expected population growth */
    void                    UpdatePopulationGrowth();
    //@}

    /** Resets empire meters. */
    void                    ResetMeters();

    mutable boost::signal<void ()>  ShipDesignsChangedSignal;

private:
    void                    Init();

    int                             m_id;                       ///< Empire's unique numeric id
    std::string                     m_name;                     ///< Empire's name
    std::string                     m_player_name;              ///< Empire's Player's name
    GG::Clr                         m_color;                    ///< Empire's color
    int                             m_capital_id;               ///< the ID of the empire's capital planet

    std::set<std::string>           m_techs;                    ///< list of acquired technologies.  These are string names referencing Tech objects

    std::map<std::string, Meter>    m_meters;                   ///< empire meters, including ratings scales used by species to judge empires

    ResearchQueue                   m_research_queue;           ///< the queue of techs being or waiting to be researched
    std::map<std::string, double>   m_research_progress;        ///< progress of partially-researched techs; fully researched techs are removed

    ProductionQueue                 m_production_queue;         ///< the queue of items being or waiting to be built
    std::vector<double>             m_production_progress;      ///< progress of partially-completed builds; completed items are removed

    std::set<std::string>           m_available_building_types; ///< list of acquired BuildingType.  These are string names referencing BuildingType objects
    std::set<std::string>           m_available_part_types;     ///< list of acquired ship PartType.  These are string names referencing PartType objects
    std::set<std::string>           m_available_hull_types;     ///< list of acquired ship HullType.  These are string names referencing HullType objects
    std::set<int>                   m_explored_systems;         ///< systems you've explored
    std::set<int>                   m_ship_designs;             ///< The Empire's ship designs, indexed by design id

    std::list<SitRepEntry*>         m_sitrep_entries;           ///< The Empire's sitrep entries

    std::map<ResourceType, boost::shared_ptr<ResourcePool> >    m_resource_pools;
    PopulationPool                                              m_population_pool;

    /** MAYBE TEMPORARY: Until social projects and/or consequences of unpaid maintenance is implemented.
      * Total maintenance on buildings owned by this empire.  Set by UpdateTradeSpending(), used
      * by CheckTradeSocialProgress() to deduct maintenance cost from trade stockpile */
    double                          m_maintenance_total_cost;

    std::map<std::string, int>      m_ship_names_used;                                  ///< map from name to number of times used

    // cached calculation results, returned by reference
    std::set<int>                   m_fleet_supplyable_system_ids;                      ///< ids of systems where fleets can remain for a turn to be resupplied.  computed and set by UpdateFleetSupply
    std::set<std::pair<int, int> >  m_fleet_supply_starlane_traversals;                 ///< ordered pairs of system ids between which a starlane runs that can be used to convey supply to fleets.
    std::map<int, int>              m_fleet_supply_system_ranges;                       ///< number of starlane jumps away from each system (by id) fleet supply can be conveyed.  This is the number due to a system's contents conveying supply and is computed and set by UpdateSystemSupplyRanges
    std::set<std::set<int> >        m_resource_supply_groups;                           ///< sets of system ids that are connected by resource supply lines and are able to share resources between systems or between objects in systems
    std::set<std::pair<int, int> >  m_resource_supply_starlane_traversals;              ///< ordered pairs of system ids between which a starlane runs that can be used to convey resources between systems
    std::set<std::pair<int, int> >  m_resource_supply_obstructed_starlane_traversals;   ///< ordered pairs of system ids between which a starlane could be used to convey resources between system, but is not because something is obstructing the resource flow.  That is, the resource flow isn't limited by range, but by something blocking its flow.
    std::map<int, int>              m_resource_supply_system_ranges;                    ///< number of starlane jumps away from each system (by id) that resources can be conveyed.
    std::set<int>                   m_supply_unobstructed_systems;                      ///< ids of system that don't block supply (resource or fleet) from flowing


    friend class boost::serialization::access;
    Empire();
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // _Empire_h_
