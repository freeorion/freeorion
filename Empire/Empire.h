#ifndef _Empire_h_
#define _Empire_h_

#include "ResourcePool.h"
#include "../util/Export.h"
#include "../universe/Meter.h"
#include "../universe/UniverseObject.h"

#include <GG/Clr.h>

#include <deque>
#include <string>

struct ItemSpec;
class ShipDesign;
class SitRepEntry;
FO_COMMON_API extern const int INVALID_DESIGN_ID;
FO_COMMON_API extern const int INVALID_GAME_TURN;
FO_COMMON_API extern const int INVALID_OBJECT_ID;
FO_COMMON_API extern const int ALL_EMPIRES;

struct FO_COMMON_API ResearchQueue {
    /** The type of a single element in the research queue. */
    struct Element {
        Element() :
            empire_id(ALL_EMPIRES),
            allocated_rp(0.0f),
            turns_left(0),
            paused(false)
        {}
        Element(const std::string& name_, int empire_id_, float spending_, int turns_left_, bool paused_ = false) :
            name(name_),
            empire_id(empire_id_),
            allocated_rp(spending_),
            turns_left(turns_left_),
            paused(paused_)
        {}
        std::string Dump() const;
        std::string name;
        int         empire_id;
        float       allocated_rp;
        int         turns_left;
        bool        paused;
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
    ResearchQueue(int empire_id) :
        m_projects_in_progress(0),
        m_total_RPs_spent(0.0),
        m_empire_id(empire_id)
    {}
    //@}

    /** \name Accessors */ //@{
    bool                        InQueue(const std::string& tech_name) const;///< Returns true iff \a tech is in this queue.
    bool                        Paused(const std::string& tech_name) const; ///< Returns true iff \a tech is in this queue and paused.
    bool                        Paused(int idx) const;                      ///< Returns true iff there are at least \a idx - 1 items in the queue and item with index \a idx is paused
    int                         ProjectsInProgress() const;                 ///< Returns the number of research projects currently (perhaps partially) funded.
    float                       TotalRPsSpent() const;                      ///< Returns the number of RPs currently spent on the projects in this queue.
    int                         EmpireID() const { return m_empire_id; }
    std::vector<std::string>    AllEnqueuedProjects() const;
    std::string                 Dump() const;

    // STL container-like interface
    bool            empty() const;
    unsigned int    size() const;
    const_iterator  begin() const;
    const_iterator  end() const;
    const_iterator  find(const std::string& tech_name) const;
    const Element&  operator[](int i) const;
    //@}

    /** \name Mutators */ //@{
    /** Recalculates the RPs spent on and number of turns left for each project
      * in the queue.  Also determines the number of projects in prgress, and
      * the total number of RPs spent on the projects in the queue.  \note A
      * precondition of this function that \a RPs must be greater than some
      * epsilon > 0; see the implementation for the actual value used for
      * epsilon. */
    void            Update(float RPs, const std::map<std::string, float>& research_progress);

    // STL container-like interface
    void            push_back(const std::string& tech_name, bool paused = false);
    void            insert(iterator it, const std::string& tech_name, bool paused = false);
    void            erase(iterator it);

    iterator        begin();
    iterator        end();
    iterator        find(const std::string& tech_name);

    void            clear();

    mutable boost::signals2::signal<void ()> ResearchQueueChangedSignal;
    //@}

private:
    QueueType   m_queue;
    int         m_projects_in_progress;
    float       m_total_RPs_spent;
    int         m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

struct FO_COMMON_API ProductionQueue {
    /** The type that specifies a single production item (BuildType and name string). */
    struct FO_COMMON_API ProductionItem {
        ProductionItem();

        ProductionItem(BuildType build_type_, std::string name_);   ///< basic ctor for BuildTypes that use std::string to identify specific items (BuildingTypes)
        ProductionItem(BuildType build_type_, int design_id_);      ///< basic ctor for BuildTypes that use int to indentify the design of the item (ShipDesigns)

        bool CostIsProductionLocationInvariant() const;             ///< indicates whether production location can change the cost of this item. This is useful for cachcing cost results per-location or once for all locations.

        bool operator<(const ProductionItem& rhs) const;

        bool EnqueueConditionPassedAt(int location_id) const;

        std::map<std::string, std::map<int, float>> CompletionSpecialConsumption(int location_id) const;// for each special name, what object ids have those special capacities reduced by what amount
        std::map<MeterType, std::map<int, float>>   CompletionMeterConsumption(int location_id) const;  // for each meter type, what object ids have those meters reduced by what amount

        std::string Dump() const;

        BuildType   build_type;
        // only one of these may be valid, depending on BuildType
        std::string name;
        int         design_id = INVALID_DESIGN_ID;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    /** The type of a single element in the production queue. */
    struct FO_COMMON_API Element {
        Element();

        Element(ProductionItem item_, int empire_id_,
                int ordered_, int remaining_, int blocksize_,
                int location_, bool paused_ = false,
                bool allowed_imperial_stockpile_use_ = false);

        Element(BuildType build_type, std::string name, int empire_id_,
                int ordered_, int remaining_, int blocksize_,
                int location_, bool paused_ = false,
                bool allowed_imperial_stockpile_use_ = false);

        Element(BuildType build_type, int design_id, int empire_id_,
                int ordered_, int remaining_, int blocksize_,
                int location_, bool paused_ = false,
                bool allowed_imperial_stockpile_use_ = false);

        ProductionItem  item;
        int             empire_id = ALL_EMPIRES;
        int             ordered = 1;                ///< how many of item (blocks) to produce
        int             blocksize = 1;              ///< size of block to produce (default=1)
        int             remaining;                  ///< how many left to produce
        int             location;                   ///< the ID of the UniverseObject at which this item is being produced
        float           allocated_pp = 0.0f;        ///< PP allocated to this ProductionQueue Element by Empire production update
        float           progress = 0.0f;            ///< fraction of this item that is complete.
        float           progress_memory = 0.0f;     ///< updated by server turn processing; aides in allowing blocksize changes to be undone in same turn w/o progress loss
        int             blocksize_memory = 1;       ///< used along with progress_memory
        int             turns_left_to_next_item = -1;
        int             turns_left_to_completion = -1;
        int             rally_point_id = INVALID_OBJECT_ID;
        bool            paused = false;
        bool            allowed_imperial_stockpile_use = false;

        std::string Dump() const;

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
    ProductionQueue(int empire_id);
    //@}

    /** \name Accessors */ //@{
    int     ProjectsInProgress() const;         ///< Returns the number of production projects currently (perhaps partially) funded.
    float   TotalPPsSpent() const;              ///< Returns the number of PPs currently spent on the projects in this queue.
    int     EmpireID() const { return m_empire_id; }

    /** Returns map from sets of object ids that can share resources to amount
      * of PP available in those groups of objects ; does not include stockpile */
    std::map<std::set<int>, float> AvailablePP(const std::shared_ptr<ResourcePool>& industry_pool) const;

    /** Returns map from sets of object ids that can share resources to amount
      * of PP allocated to production queue elements that have build locations
      * in systems in the group. */
    const std::map<std::set<int>, float>& AllocatedPP() const;

    /** Returns map from sets of object ids that can share resources to amount
     * of stockpile PP allocated to production queue elements that have build locations
     * in systems in the group. */
    const std::map<std::set<int>, float>& AllocatedStockpilePP() const;

    /** Returns sum of stockpile meters of empire-owned objects. */
    float StockpileCapacity() const;

    /** Returns the value expected for the Imperial Stockpile for the next turn, based on the current
     * ProductionQueue allocations. */
    float ExpectedNewStockpileAmount() const {return m_expected_new_stockpile_amount; }

    /** Returns sets of object ids that have more available than allocated PP */
    std::set<std::set<int>> ObjectsWithWastedPP(const std::shared_ptr<ResourcePool>& industry_pool) const;


    // STL container-like interface
    bool            empty() const;
    unsigned int    size() const;
    const_iterator  begin() const;
    const_iterator  end() const;
    const_iterator  find(int i) const;
    const Element&  operator[](int i) const;

    /** \name Mutators */ //@{
    /** Recalculates the PPs spent on and number of turns left for each project in the queue.  Also
      * determines the number of projects in progress, and the industry consumed by projects
      * in each resource-sharing group of systems.  Does not actually "spend" the PP; a later call to
      * empire->CheckProductionProgress() will actually spend PP, remove items from queue and create them
      * in the universe. */
    void Update();

    // STL container-like interface
    void        push_back(const Element& element);
    void        insert(iterator it, const Element& element);
    void        erase(int i);
    iterator    erase(iterator it);

    iterator    begin();
    iterator    end();
    iterator    find(int i);
    Element&    operator[](int i);

    void        clear();

    mutable boost::signals2::signal<void ()> ProductionQueueChangedSignal;
    //@}

private:
    QueueType                       m_queue;
    int                             m_projects_in_progress;
    std::map<std::set<int>, float>  m_object_group_allocated_pp;
    std::map<std::set<int>, float>  m_object_group_allocated_stockpile_pp;
    float                           m_expected_new_stockpile_amount;
    int                             m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** Class to maintain the state of a single empire. In both the client and
  * server, Empires are managed by a subclass of EmpireManager, and can be
  * accessed from other modules by using the EmpireManager::Lookup() method to
  * obtain a pointer. */
class FO_COMMON_API Empire {
public:
    // EmpireManagers must be friends so that they can have access to the constructor and keep it hidden from others
    friend class EmpireManager;

    /** \name Iterator Types */ //@{
    typedef std::set<int>::const_iterator               SystemIDItr;
    typedef std::vector<SitRepEntry>::const_iterator    SitRepItr;
    //@}

    /** \name Structors */ //@{
    Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color);  ///< basic constructor
    ~Empire();
    //@}

    /** \name Accessors */ //@{
    const std::string&  Name() const;                               ///< Returns the Empire's name
    const std::string&  PlayerName() const;                         ///< Returns the Empire's player's name
    int                 EmpireID() const;                           ///< Returns the Empire's unique numeric ID
    const GG::Clr&      Color() const;                              ///< Returns the Empire's color
    int                 CapitalID() const;                          ///< Returns the numeric ID of the empire's capital
    int                 StockpileID(ResourceType res) const;        ///< Returns the numeric ID of the empire's stockpile location for the resource of type \a res

    /** Return an object id that is owned by the empire or INVALID_OBJECT_ID. */
    int                 SourceID() const;
    /** Return an object that is owned by the empire or null.*/
    std::shared_ptr<const UniverseObject> Source() const;

    std::string Dump() const;

    /** Returns the set of Tech names available to this empire. */
    const std::map<std::string, int>& ResearchedTechs() const;

    /** Returns the set of BuildingType names availble to this empire. */
    const std::set<std::string>& AvailableBuildingTypes() const;

    /** Returns the set of ShipDesign IDs available for this empire to build. */
    std::set<int> AvailableShipDesigns() const;

    const std::set<int>&            ShipDesigns() const;                ///< Returns the set of all ship design ids of this empire
    const std::set<std::string>&    AvailableShipParts() const;         ///< Returns the set of ship part names this empire that the empire can currently build
    const std::set<std::string>&    AvailableShipHulls() const;         ///< Returns the set of ship hull names that that the empire can currently build

    const std::string&              TopPriorityEnqueuedTech() const;
    const std::string&              MostExpensiveEnqueuedTech() const;
    const std::string&              LeastExpensiveEnqueuedTech() const;
    const std::string&              MostRPSpentEnqueuedTech() const;
    const std::string&              MostRPCostLeftEnqueuedTech() const;

    const std::string&              TopPriorityResearchableTech() const;
    const std::string&              MostExpensiveResearchableTech() const;
    const std::string&              LeastExpensiveResearchableTech() const;
    const std::string&              MostRPSpentResearchableTech() const;
    const std::string&              MostRPCostLeftResearchableTech() const;

    const Meter*                                    GetMeter(const std::string& name) const;
    std::map<std::string, Meter>::const_iterator    meter_begin() const { return m_meters.begin(); }
    std::map<std::string, Meter>::const_iterator    meter_end() const   { return m_meters.end(); }

    const ResearchQueue&    GetResearchQueue() const;                   ///< Returns the queue of techs being or queued to be researched.
    const ProductionQueue&  GetProductionQueue() const;                 ///< Returns the queue of items being or queued to be produced.

    bool        ResearchableTech(const std::string& name) const;        ///< Returns true iff \a name is a tech that has not been researched, and has no unresearched prerequisites.
    float       ResearchProgress(const std::string& name) const;        ///< Returns the RPs spent towards tech \a name if it has partial research progress, or 0.0 if it is already researched.
    bool        TechResearched(const std::string& name) const;          ///< Returns true iff this tech has been completely researched.
    bool        HasResearchedPrereqAndUnresearchedPrereq(const std::string& name) const;    ///< Returns true iff this tech has some but not all prerequisites researched
    TechStatus  GetTechStatus(const std::string& name) const;           ///< Returns the status (researchable, researched, unresearchable) for this tech for this

    bool        BuildingTypeAvailable(const std::string& name) const;   ///< Returns true if the given building type is known to this empire, false if it is not
    bool        ShipDesignAvailable(const ShipDesign& design) const;    ///< Returns true iff this ship design can be built by this empire.
    bool        ShipDesignAvailable(int ship_design_id) const;          ///< Returns true iff this ship design can be built by this empire.  If no such ship design exists, returns false
    bool        ShipDesignKept(int ship_design_id) const;               ///< Returns true iff the given ship design id is in the set of design ids of this empire.  That is, it has been added to this empire.
    bool        ShipPartAvailable(const std::string& name) const;       ///< Returns true iff this ship part can be built by this empire.  If no such ship part exists, returns false
    bool        ShipHullAvailable(const std::string& name) const;       ///< Returns true iff this ship hull can be built by this empire.  If no such ship hull exists, returns false

    float       ProductionStatus(int i) const;                          ///< Returns the PPs spent towards item \a i in the build queue if it has partial progress, -1.0 if there is no such index in the production queue.

    /** Returns the total cost per item (blocksize 1) and the minimum number of
      * turns required to produce the indicated item, or (-1.0, -1) if the item
      * is unknown, unavailable, or invalid. */
    std::pair<float, int>   ProductionCostAndTime(const ProductionQueue::Element& element) const;
    std::pair<float, int>   ProductionCostAndTime(const ProductionQueue::ProductionItem& item, int location_id) const;

    bool                    ProducibleItem(BuildType build_type, const std::string& name, int location) const;  ///< Returns true iff this empire can produce the specified item at the specified location.
    bool                    ProducibleItem(BuildType build_type, int design_id, int location) const;            ///< Returns true iff this empire can produce the specified item at the specified location.
    bool                    ProducibleItem(const ProductionQueue::ProductionItem& item, int location) const;    ///< Returns true iff this empire can produce the specified item at the specified location.

    bool                    EnqueuableItem(BuildType build_type, const std::string& name, int location) const;  ///< Returns true iff this empire can enqueue the specified item at the specified location.
    bool                    EnqueuableItem(const ProductionQueue::ProductionItem& item, int location) const;    ///< Returns true iff this empire can enqueue the specified item at the specified location.

    bool                    HasExploredSystem(int ID) const;                            ///< returns  true if the given item is in the appropriate list, false if it is not.

    bool                    Eliminated() const;                                         ///< whether this empire has lost the game
    bool                    Won() const;                                                ///< whether this empire has won the game

    int                     NumSitRepEntries(int turn = INVALID_GAME_TURN) const;       ///< number of entries in the SitRep.

    /** Returns distance in jumps away from each system that this empire can
      * propagate supply. */
    const std::map<int, float>&             SystemSupplyRanges() const;

    /** Returns set of system ids that are able to propagate supply from one
      * system to the next, or at which supply can be delivered to fleets if
      * supply can reach the system from elsewhere, or in which planets can
      * exchange supply between themselves (even if not leaving the system). */
    const std::set<int>&                    SupplyUnobstructedSystems() const;

    /** Returns true if the start system is known to this empire and travel to
        the dest system is not restricted by blockade.  */
    const bool                              UnrestrictedLaneTravel(int start_system_id, int dest_system_id) const;

    const std::set<int>&                    ExploredSystems() const;    ///< returns set of ids of systems that this empire has explored
    const std::map<int, std::set<int>>      KnownStarlanes() const;     ///< returns map from system id (start) to set of system ids (endpoints) of all starlanes known to this empire
    const std::map<int, std::set<int>>      VisibleStarlanes() const;   ///< returns map from system id (start) to set of system ids (endpoints) of all starlanes visible to this empire this turn

    SitRepItr               SitRepBegin() const;                ///< starting iterator for sitrep entries for this empire
    SitRepItr               SitRepEnd() const;                  ///< end iterator for sitreps

    float                   ProductionPoints() const;           ///< Returns the empire's current production point output (this is available industry not including stockpile)

    /** Returns ResourcePool for \a resource_type or 0 if no such ResourcePool
        exists. */
    const std::shared_ptr<ResourcePool> GetResourcePool(ResourceType resource_type) const;

    float                   ResourceStockpile(ResourceType type) const;         ///< returns current stockpiled amount of resource \a type
    float                   ResourceOutput(ResourceType type) const;            ///< returns amount of resource \a type being generated by ResourceCenters
    float                   ResourceAvailable(ResourceType type) const;         ///< returns amount of resource \a type immediately available.  This = production + stockpile

    const PopulationPool&   GetPopulationPool() const;                          ///< Returns PopulationPool
    float                   Population() const;                                 ///< returns total Population of empire
    //@}

    /** \name Mutators */ //@{
    /** If the object with id \a id is a planet owned by this empire, sets that
      * planet to be this empire's capital, and otherwise does nothing. */
    void SetCapitalID(int id);

    /** Returns the meter with the indicated \a name if it exists, or nullptr. */
    Meter* GetMeter(const std::string& name);
    void BackPropagateMeters();

    /** Adds \a tech to the research queue, placing it before position \a pos.
      * If \a tech is already in the queue, it is moved to \a pos, then removed
      * from its former position.  If \a pos < 0 or queue.size() <= pos, \a tech
      * is placed at the end of the queue. If \a tech is already available, no
      * action is taken. */
    void PlaceTechInQueue(const std::string& name, int pos = -1);
    /** Removes tech with \a name from the research queue, if it is in the
      * research queue already. */
    void RemoveTechFromQueue(const std::string& name);

    void PauseResearch(const std::string& name);
    void ResumeResearch(const std::string& name);

    /** Sets research progress of tech with \a name to \a progress. */
    void SetTechResearchProgress(const std::string& name, float progress);
    /** Adds the indicated build to the production queue, placing it before
      * position \a pos.  If \a pos < 0 or queue.size() <= pos, the build is
      * placed at the end of the queue. */
    void PlaceProductionOnQueue(BuildType build_type, const std::string& name,
                                int number, int blocksize, int location, int pos = -1);
    /** Adds the indicated build to the production queue, placing it before
      * position \a pos.  If \a pos < 0 or queue.size() <= pos, the build is
      * placed at the end of the queue. */
    void PlaceProductionOnQueue(BuildType build_type, int design_id, int number,
                                int blocksize, int location, int pos = -1);
    /** Adds the indicated build to the production queue, placing it before
      * position \a pos.  If \a pos < 0 or queue.size() <= pos, the build is
      * placed at the end of the queue. */
    void PlaceProductionOnQueue(const ProductionQueue::ProductionItem& item, int number,
                                int blocksize, int location, int pos = -1);
    void SetProductionQuantity(int index, int quantity);     ///< Changes the remaining number to produce for queue item \a index to \a quantity
    void SetProductionQuantityAndBlocksize(int index, int quantity, int blocksize);   ///< Changes the remaining number and blocksize to produce for queue item \a index to \a quantity and \a blocksize 
    void SplitIncompleteProductionItem(int index);           ///< Adds a copy of the production item at position \a index below it in the queue, with one less quantity. Sets the quantity of the production item at position \a index to 1, retaining its incomplete progress.
    void DuplicateProductionItem(int index);                 ///< Adds a copy of the production item at position \a index below it in the queue, with no progress.
    void SetProductionRallyPoint(int index, int rally_point_id = INVALID_OBJECT_ID);  ///< Sets the rally point for ships produced by this produce, to which they are automatically ordered to move after they are produced.
    void MoveProductionWithinQueue(int index, int new_index);///< Moves \a tech from the production queue, if it is in the production queue already.
    void RemoveProductionFromQueue(int index);               ///< Removes the produce at position \a index in the production queue, if such an index exists.
    void PauseProduction(int index);
    void ResumeProduction(int index);
    void AllowUseImperialPP(int index, bool allow=true);  ///< Allows or disallows the use of the imperial stockpile for production

    void AddTech(const std::string& name);           ///< Inserts the given Tech into the Empire's list of available technologies.
    void UnlockItem(const ItemSpec& item);           ///< Adds a given producible item (Building, Ship Hull, Ship part) to the list of available items.
    void AddBuildingType(const std::string& name);   ///< Inserts the given BuildingType into the Empire's list of available BuldingTypes.
    void AddPartType(const std::string& name);       ///< Inserts the given ship PartType into the Empire's list of available BuldingTypes.
    void AddHullType(const std::string& name);       ///< Inserts the given ship HullType into the Empire's list of available BuldingTypes.
    void AddExploredSystem(int ID);                  ///< Inserts the given ID into the Empire's list of explored systems.

    /** inserts given design id into the empire's set of designs in front of next design */
    void AddShipDesign(int ship_design_id, int next_design_id = INVALID_DESIGN_ID);
    int AddShipDesign(ShipDesign* ship_design);     ///< inserts given ShipDesign into the Universe, adds the design's id to the Empire's set of ids, and returns the new design's id, which is INVALID_OBJECT_ID on failure.  If successful, universe takes ownership of passed ShipDesign.

    std::string NewShipName();                              ///< generates a random ship name, appending II, III, etc., to it if it has been used before by this empire
    void Eliminate();                                ///< Marks empire as eliminated and cleans up empire after it is eliminated.  Queues are cleared, capital is reset, and other state info not relevant to an eliminated empire is cleared
    void Win(const std::string& reason);             ///< Marks this empire as having won for this reason, and sends the appropriate sitreps

    /** Inserts the a pointer to given SitRep entry into the empire's sitrep list.
     *  \warning When you call this method, you are transferring ownership
     *  of the entry object to the Empire.
     *  The object pointed to by 'entry' will be deallocated when
     *  the empire's sitrep is cleared.  Be careful you do not have any
     *  references to SitRepEntries lying around when this happens.
     *  You \a must pass in a dynamically allocated sitrep entry */
    void AddSitRepEntry(const SitRepEntry& entry);
    void ClearSitRep();                              ///< Clears all sitrep entries

    void RemoveTech(const std::string& name);        ///< Removes the given Tech from the empire's list
    void LockItem(const ItemSpec& item);             ///< Removes a given producible item (Building, Ship Hull, Ship Part) from the list of available items.
    void RemoveBuildingType(const std::string& name);///< Removes the given BuildingType from the empire's list
    void RemovePartType(const std::string& name);    ///< Removes the given PartType from the empire's list
    void RemoveHullType(const std::string& name);    ///< Removes the given HullType from the empire's list
    void RemoveShipDesign(int ship_design_id);       ///< Removes the ShipDesign with the given id from the empire's set

    /** Calculates ranges that systems can send fleet and resource supplies,
      * using the specified st of \a known_objects as the source for supply-
      * producing objects and systems through which it can be propagated. */
    void UpdateSystemSupplyRanges(const std::set<int>& known_objects);
    /** Calculates ranges that systems can send fleet and resource supplies. */
    void UpdateSystemSupplyRanges();
    /** Calculates systems that can propagate supply (fleet or resource) using
      * the specified set of \a known_systems */
    void UpdateSupplyUnobstructedSystems(const std::set<int>& known_systems);
    /** Calculates systems that can propagate supply using this empire's own /
      * internal list of explored systems. */
    void UpdateSupplyUnobstructedSystems();
    /** Updates fleet ArrivalStarlane to flag fleets of this empire that are not blockaded post-combat 
     *  must be done after *all* noneliminated empires have updated their unobstructed systems* */
    void UpdateUnobstructedFleets();
    /** Records, in a list of pending updates, the start_system exit lane to the specified destination as accessible to this empire*/
    void RecordPendingLaneUpdate(int start_system_id, int dest_system_id);
    /** Processes all the pending lane access updates.  This is managed as a two step process to avoid order-of-processing issues. */
    void UpdateAvailableLanes();

    /** Checks for production projects that have been completed, and places them
      * at their respective production sites.  Which projects have been
      * completed is determined by the results of previously-called Update() on
      * the production queue (which determines how much PP each project receives
      * but does not actually spend them).  This function spends the PP, removes
      * complete items from the queue and creates the results in the universe. */
    void CheckProductionProgress();
    /** Checks for tech projects that have been completed, and adds them to the
      * known techs list. */
    void CheckResearchProgress();
    /** Eventually : Will check for social projects that have been completed and
      * / or process ongoing social projects... (not sure exactly what form
      * "social projects" will take or how they will work).  Also will update
      * the empire's trade stockpile to account for trade production and
      * expenditures. Currently: Deducts cost of maintenance of buildings from
      * empire's trade stockpile */
    void CheckTradeSocialProgress();

    void SetColor(const GG::Clr& color);                 ///< Mutator for empire color
    void SetName(const std::string& name);               ///< Mutator for empire name
    void SetPlayerName(const std::string& player_name);  ///< Mutator for empire's player name

    void SetResourceStockpile(ResourceType resource_type, float stockpile); ///< Sets current \a stockpile amount of indicated \a resource_type

    /** Determines ResourceCenters that can provide resources for this empire and sets
      * the supply groups used for each ResourcePool as appropriate for each resource.
      * call UpdateResourceSupply before calling this. */
    void InitResourcePools();

    /** Resets production of resources and calculates allocated resources (on
      * each item in queues and overall) for each resource by calling
      * UpdateResearchQueue, UpdateProductionQueue, UpdateTradeSpending.  Does
      * not actually "spend" resources, but just determines how much and on what
      * to spend.  Actual consumption of resources, removal of items from queue,
      * processing of finished items and population growth happens in various
      * Check(Whatever)Progress functions. */
    void UpdateResourcePools();
    /** Calls Update() on empire's research queue, which recalculates the RPs
      * spent on and number of turns left for each tech in the queue. */
    void UpdateResearchQueue();
    /** Calls Update() on empire's production queue, which recalculates the PPs
      * spent on and number of turns left for each project in the queue. */
    void UpdateProductionQueue();
    /** Eventually: Calls appropriate subsystem Update to calculate trade spent
      * on social projects and maintenance of buildings.  Later call to
      * CheckTradeSocialProgress() will then have the correct allocations of
      * trade. Currently: Sums maintenance costs of all buildings owned by
      * empire, sets m_maintenance_total_cost */
    void UpdateTradeSpending();
    /** Has m_population_pool recalculate all PopCenters' and empire's total
      * expected population growth */
    void UpdatePopulationGrowth();

    /** Resets empire meters. */
    void ResetMeters();

    void UpdateOwnedObjectCounters();

    int TotalShipsOwned() const;
    int TotalShipPartsOwned() const;    ///< Total number of parts for all owned ships in this empire
    int TotalBuildingsOwned() const;

    std::map<std::string, int>&     SpeciesShipsOwned()     { return m_species_ships_owned; }
    std::map<int, int>&             ShipDesignsOwned()      { return m_ship_designs_owned; }
    std::map<std::string, int>&     ShipPartTypesOwned()    { return m_ship_part_types_owned; }
    std::map<ShipPartClass, int>&   ShipPartClassOwned()    { return m_ship_part_class_owned; }
    std::map<std::string, int>&     SpeciesColoniesOwned()  { return m_species_colonies_owned; }
    int&                            OutpostsOwned()         { return m_outposts_owned; }
    std::map<std::string, int>&     BuildingTypesOwned()    { return m_building_types_owned; }

    std::map<int, int>&         EmpireShipsDestroyed()  { return m_empire_ships_destroyed; }
    std::map<int, int>&         ShipDesignsDestroyed()  { return m_ship_designs_destroyed; }
    std::map<std::string, int>& SpeciesShipsDestroyed() { return m_species_ships_destroyed; }

    std::map<std::string, int>& SpeciesPlanetsInvaded() { return m_species_planets_invaded; }

    std::map<std::string, int>& SpeciesShipsProduced()  { return m_species_ships_produced; }
    std::map<int, int>&         ShipDesignsProduced()   { return m_ship_designs_produced; }

    std::map<std::string, int>& SpeciesShipsLost()      { return m_species_ships_lost; }
    std::map<int, int>&         ShipDesignsLost()       { return m_ship_designs_lost; }

    std::map<std::string, int>& SpeciesShipsScrapped()  { return m_species_ships_scrapped; }
    std::map<int, int>&         ShipDesignsScrapped()   { return m_ship_designs_scrapped; }

    std::map<std::string, int>& SpeciesPlanetsDepoped() { return m_species_planets_depoped; }
    std::map<std::string, int>& SpeciesPlanetsBombed()  { return m_species_planets_bombed; }

    std::map<std::string, int>& BuildingTypesProduced() { return m_building_types_produced; }
    std::map<std::string, int>& BuildingTypesScrapped() { return m_building_types_scrapped; }
    //@}

    /** Processes Builditems on queues of empires other than the indicated
      * empires, at the location with id \a location_id and, as appropriate,
      * adds them to the build queue of the indicated empires (if it is an
      * empire), deletes them, or leaves them on the build queue of their
      * current empire */
    static void ConquerProductionQueueItemsAtLocation(int location_id, int empire_id);

    mutable boost::signals2::signal<void ()>  ShipDesignsChangedSignal;

private:
    void        Init();

    int                             m_id;                       ///< Empire's unique numeric id
    std::string                     m_name;                     ///< Empire's name
    std::string                     m_player_name;              ///< Empire's Player's name
    GG::Clr                         m_color;                    ///< Empire's color
    int                             m_capital_id = INVALID_OBJECT_ID;  ///< the ID of the empire's capital planet

    /** The source id is the id of any object owned by the empire.  It is
        mutable so that Source() can be const and still cache its result. */
    mutable int                     m_source_id = INVALID_OBJECT_ID;

    bool                            m_eliminated;               ///< Whether the empire has lost
    std::set<std::string>           m_victories;                ///< The ways that the empire has won, if any

    std::map<std::string, int>      m_techs;                    ///< names of researched technologies, and turns on which they were acquired.
    std::map<std::string, Meter>    m_meters;                   ///< empire meters, including ratings scales used by species to judge empires

    ResearchQueue                   m_research_queue;           ///< the queue of techs being or waiting to be researched
    std::map<std::string, float>    m_research_progress;        ///< progress of partially-researched techs; fully researched techs are removed

    ProductionQueue                 m_production_queue;         ///< the queue of items being or waiting to be built

    std::set<std::string>           m_available_building_types; ///< list of acquired BuildingType.  These are string names referencing BuildingType objects
    std::set<std::string>           m_available_part_types;     ///< list of acquired ship PartType.  These are string names referencing PartType objects
    std::set<std::string>           m_available_hull_types;     ///< list of acquired ship HullType.  These are string names referencing HullType objects
    std::set<int>                   m_explored_systems;         ///< systems you've explored
    std::set<int>                   m_ship_designs;             ///< The Empire's ship designs sorted by id

    std::vector<SitRepEntry>        m_sitrep_entries;           ///< The Empire's sitrep entries

    std::map<ResourceType, std::shared_ptr<ResourcePool>> m_resource_pools;

    PopulationPool                                              m_population_pool;

    std::map<std::string, int>      m_ship_names_used;          ///< map from name to number of times used

    std::map<std::string, int>      m_species_ships_owned;      ///< how many ships of each species does this empire currently own?
    std::map<int, int>              m_ship_designs_owned;       ///< how many ships of each design does this empire currently own?
    std::map<std::string, int>      m_ship_part_types_owned;    ///< how many ship parts are currently owned, indexed by PartType
    std::map<ShipPartClass, int>    m_ship_part_class_owned;    ///< how many ship parts are currently owned, indexed by ShipPartClass
    std::map<std::string, int>      m_species_colonies_owned;   ///< how many colonies of each species does this empire currently own?
    int                             m_outposts_owned;           ///< how many uncolonized outposts does this empire currently own?
    std::map<std::string, int>      m_building_types_owned;     ///< how many buildings does this empire currently own?

    std::map<int, int>              m_empire_ships_destroyed;   ///< how many ships of each empire has this empire destroyed?
    std::map<int, int>              m_ship_designs_destroyed;   ///< how many ships of each design has this empire destroyed?
    std::map<std::string, int>      m_species_ships_destroyed;  ///< how many ships crewed by each species has this empire destroyed?
    std::map<std::string, int>      m_species_planets_invaded;  ///< how many planets populated by each species has this empire captured?

    std::map<std::string, int>      m_species_ships_produced;   ///< how many ships crewed by each species has this empire produced?
    std::map<int, int>              m_ship_designs_produced;    ///< how many ships of each design has this empire produced?
    std::map<std::string, int>      m_species_ships_lost;       ///< how mahy ships crewed by each species has this empire lost in combat?
    std::map<int, int>              m_ship_designs_lost;        ///< how many ships of each design has this empire lost in combat?
    std::map<std::string, int>      m_species_ships_scrapped;   ///< how many ships crewed by each species has this empire scrapped?
    std::map<int, int>              m_ship_designs_scrapped;    ///< how many ships of each design has this empire scrapped?

    std::map<std::string, int>      m_species_planets_depoped;  ///< how many planets populated by each species have depopulated while owned by this empire?
    std::map<std::string, int>      m_species_planets_bombed;   ///< how many planets populated by each species has this empire bombarded?

    std::map<std::string, int>      m_building_types_produced;  ///< how many buildings of each type has this empire produced?
    std::map<std::string, int>      m_building_types_scrapped;  ///< how many buildings of each type has this empire scrapped?

    // cached calculation results, returned by reference
    std::map<int, float>            m_supply_system_ranges;         ///< number of starlane jumps away from each system (by id) supply can be conveyed.  This is the number due to a system's contents conveying supply and is computed and set by UpdateSystemSupplyRanges
    std::set<int>                   m_supply_unobstructed_systems;  ///< ids of system that don't block supply from flowing
    std::map<int, std::set<int>>    m_available_system_exit_lanes;  ///< for each system known to this empire, the set of available/non-blockaded exit lanes for fleet travel
    std::map<int, std::set<int>>    m_pending_system_exit_lanes;    ///< pending updates to m_available_system_exit_lanes

    friend class boost::serialization::access;
    Empire();
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // _Empire_h_
