#ifndef _Empire_h_
#define _Empire_h_


#include <array>
#include <compare>
#include <string>
#include <unordered_set>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include "InfluenceQueue.h"
#include "PopulationPool.h"
#include "ProductionQueue.h"
#include "ResearchQueue.h"
#include "ResourcePool.h"
#include "../universe/EnumsFwd.h"
#include "../universe/Meter.h"
#include "../util/AppInterface.h"
#include "../util/Export.h"
#include "../util/SitRepEntry.h"


struct UnlockableItem;
class ShipDesign;
class ResourcePool;

using EmpireColor = std::array<uint8_t, 4>;


//! Research status of techs, relating to whether they have been or can be
//! researched
FO_ENUM(
    (TechStatus),
    ((INVALID_TECH_STATUS, -1))
    //! Never researchable, or has no researched prerequisites
    ((TS_UNRESEARCHABLE))
    //! Has at least one researched, and at least one unreserached,
    //! prerequisite
    ((TS_HAS_RESEARCHED_PREREQ))
    //! All prerequisites researched
    ((TS_RESEARCHABLE))
    //! Has been researched
    ((TS_COMPLETE))
    ((NUM_TECH_STATUSES))
)


/** Class to maintain the state of a single empire. In both the client and
  * server, Empires are managed by a subclass of EmpireManager, and can be
  * accessed from other modules by using the EmpireManager::Lookup() method to
  * obtain a pointer. */
class FO_COMMON_API Empire final {
public:
    // EmpireManagers must be friends so that they can have access to the constructor and keep it hidden from others
    friend class EmpireManager;

    Empire(std::string name, std::string player_name, int ID, EmpireColor color, bool authenticated);

    [[nodiscard]] const auto&  Name() const noexcept { return m_name; }
    [[nodiscard]] const auto&  PlayerName() const noexcept { return m_player_name; }
    [[nodiscard]] bool         IsAuthenticated() const noexcept { return m_authenticated; }
    [[nodiscard]] int          EmpireID() const noexcept { return m_id; }
    [[nodiscard]] auto         Color() const noexcept { return m_color; }
    [[nodiscard]] int          CapitalID() const noexcept { return m_capital_id; }

    /** Returns an object that is owned by the empire, or null.*/
    [[nodiscard]] std::shared_ptr<const UniverseObject> Source(const ObjectMap& objects) const;

    [[nodiscard]] std::string  Dump() const;

    [[nodiscard]] bool         PolicyAdopted(std::string_view name) const { return m_adopted_policies.count(name); }
    [[nodiscard]] int          TurnPolicyAdopted(std::string_view name) const;
    [[nodiscard]] int          CurrentTurnsPolicyHasBeenAdopted(std::string_view name) const;
    [[nodiscard]] int          CumulativeTurnsPolicyHasBeenAdopted(std::string_view name) const;

    [[nodiscard]] int                           SlotPolicyAdoptedIn(std::string_view name) const;
    [[nodiscard]] std::vector<std::string_view> AdoptedPolicies() const;
    [[nodiscard]] std::vector<std::string_view> InitialAdoptedPolicies() const;
    [[nodiscard]] bool                          PoliciesModified() const noexcept { return m_adopted_policies != m_initial_adopted_policies; }

    /** For each category, returns the slots in which policies have been adopted
      * and what policy is in that slot. */
    [[nodiscard]] std::map<std::string_view, std::map<int, std::string_view>>
    CategoriesSlotsPoliciesAdopted() const;

    /** Returns the policies the empire has adopted and turns on which they were adopted. */
    [[nodiscard]] std::map<std::string_view, int, std::less<>> TurnsPoliciesAdopted() const;
    [[nodiscard]] const auto& PolicyTotalAdoptedDurations() const noexcept { return m_policy_adoption_total_duration; }
    [[nodiscard]] const auto& PolicyCurrentAdoptedDurations() const noexcept { return m_policy_adoption_current_duration; }

    /** Returns the set of policies / slots the empire has avaialble. */
    [[nodiscard]] const auto& AvailablePolicies() const noexcept { return m_available_policies; }
    [[nodiscard]] bool        PolicyAvailable(std::string_view name) const;
    [[nodiscard]] bool        PolicyPrereqsAndExclusionsOK(std::string_view name, int current_turn) const;
    [[nodiscard]] bool        PolicyAffordable(std::string_view name, const ScriptingContext& context) const;
    [[nodiscard]] double      ThisTurnAdoptedPoliciesCost(const ScriptingContext& context) const;
    [[nodiscard]] std::vector<std::pair<std::string_view, int>> TotalPolicySlots() const; // how many total slots does this empire have in each category
    [[nodiscard]] std::vector<std::pair<std::string_view, int>> EmptyPolicySlots() const; // how many empty slots does this empire have in each category

    /** Returns the set of Tech names available to this empire and the turns on
      * which they were researched. */
    [[nodiscard]] const auto& ResearchedTechs() const noexcept { return m_techs; }

    /** Returns the set of BuildingType names availble to this empire. */
    [[nodiscard]] const auto& AvailableBuildingTypes() const noexcept { return m_available_building_types; }

    /** Returns the set of ShipDesign IDs available for this empire to build. */
    [[nodiscard]] std::vector<int>             AvailableShipDesigns(const Universe& universe) const;

    [[nodiscard]] const auto&                  ShipDesigns() const noexcept { return m_known_ship_designs; }
    [[nodiscard]] const auto&                  AvailableShipParts() const noexcept { return m_available_ship_parts; }
    [[nodiscard]] const auto&                  AvailableShipHulls() const noexcept { return m_available_ship_hulls; }

    [[nodiscard]] const std::string&           TopPriorityEnqueuedTech() const;
    [[nodiscard]] const std::string&           MostExpensiveEnqueuedTech(const ScriptingContext& context) const;
    [[nodiscard]] const std::string&           LeastExpensiveEnqueuedTech(const ScriptingContext& context) const;
    [[nodiscard]] const std::string&           MostRPSpentEnqueuedTech() const;
    [[nodiscard]] const std::string&           MostRPCostLeftEnqueuedTech(const ScriptingContext& context) const;

    [[nodiscard]] const std::string&           TopPriorityResearchableTech() const;
    [[nodiscard]] const std::string&           MostExpensiveResearchableTech() const;
    [[nodiscard]] const std::string&           LeastExpensiveResearchableTech(const ScriptingContext& context) const;
    [[nodiscard]] const std::string&           MostRPSpentResearchableTech() const;
    [[nodiscard]] const std::string&           MostRPCostLeftResearchableTech(const ScriptingContext& context) const;

    [[nodiscard]] const Meter*                 GetMeter(std::string_view name) const;
    [[nodiscard]] const auto&                  GetMeters() const noexcept { return m_meters; }

    [[nodiscard]] const ResearchQueue&         GetResearchQueue() const noexcept { return m_research_queue; }
    [[nodiscard]] const ProductionQueue&       GetProductionQueue() const noexcept { return m_production_queue; }
    [[nodiscard]] const InfluenceQueue&        GetInfluenceQueue() const noexcept { return m_influence_queue; }

    [[nodiscard]] bool        ResearchableTech(std::string_view name) const;          ///< Returns true iff \a name is a tech that has not been researched, and has no unresearched prerequisites.
    [[nodiscard]] float       ResearchProgress(const std::string& name, const ScriptingContext& context) const; ///< Returns the RPs spent towards tech \a name if it has partial research progress, or 0.0 if it is already researched.
    [[nodiscard]] bool        TechResearched(const std::string& name) const;          ///< Returns true iff this tech has been completely researched.
    [[nodiscard]] bool        HasResearchedPrereqAndUnresearchedPrereq(std::string_view name) const;    ///< Returns true iff this tech has some but not all prerequisites researched
    [[nodiscard]] TechStatus  GetTechStatus(const std::string& name) const;           ///< Returns the status (researchable, researched, unresearchable) for this tech for this

    [[nodiscard]] bool        BuildingTypeAvailable(const std::string& name) const;   ///< Returns true if the given building type is known to this empire, false if it is not
    [[nodiscard]] bool        ShipDesignAvailable(const ShipDesign& design) const;    ///< Returns true iff this ship design can be built by this empire.
    [[nodiscard]] bool        ShipDesignAvailable(int ship_design_id, const Universe& unvierse) const; ///< Returns true iff this ship design can be built by this empire.  If no such ship design exists, returns false
    [[nodiscard]] bool        ShipDesignKept(int ship_design_id) const;               ///< Returns true iff the given ship design id is in the set of design ids of this empire.  That is, it has been added to this empire.
    [[nodiscard]] bool        ShipPartAvailable(const std::string& name) const;       ///< Returns true iff this ship part can be built by this empire.  If no such ship part exists, returns false
    [[nodiscard]] bool        ShipHullAvailable(const std::string& name) const;       ///< Returns true iff this ship hull can be built by this empire.  If no such ship hull exists, returns false

    [[nodiscard]] float       ProductionStatus(int i, const ScriptingContext& context) const; ///< Returns the PPs spent towards item \a i in the build queue if it has partial progress, -1.0 if there is no such index in the production queue.

    /** Return true iff this empire can produce the specified item at the specified location. */
    [[nodiscard]] bool        ProducibleItem(BuildType build_type, int location,
                                             const ScriptingContext& context) const;
    [[nodiscard]] bool        ProducibleItem(BuildType build_type, const std::string& name, int location,
                                             const ScriptingContext& context) const;
    [[nodiscard]] bool        ProducibleItem(BuildType build_type, int design_id, int location,
                                             const ScriptingContext& context) const;
    [[nodiscard]] bool        ProducibleItem(const ProductionQueue::ProductionItem& item, int location,
                                             const ScriptingContext& context) const;

    /** Return true iff this empire can enqueue the specified item at the specified location. */
    [[nodiscard]] bool        EnqueuableItem(BuildType build_type, const std::string& name, int location,
                                             const ScriptingContext& context) const;
    [[nodiscard]] bool        EnqueuableItem(const ProductionQueue::ProductionItem& item, int location,
                                             const ScriptingContext& context) const;

    [[nodiscard]] bool        HasExploredSystem(int ID) const;                            ///< returns  true if the given item is in the appropriate list, false if it is not.

    [[nodiscard]] bool        Eliminated() const noexcept { return m_eliminated; }        ///< whether this empire has lost the game
    [[nodiscard]] bool        Won() const noexcept { return !m_victories.empty(); }       ///< whether this empire has won the game
    [[nodiscard]] bool        Ready() const noexcept { return m_ready; }                  ///< readiness status of empire

    [[nodiscard]] int         NumSitRepEntries(int turn = INVALID_GAME_TURN) const noexcept; ///< number of entries in the SitRep.

    /** Returns distance in jumps away from each system that this empire can
      * propagate supply. */
    [[nodiscard]] const auto& SystemSupplyRanges() const noexcept { return m_supply_system_ranges; }

    /** Returns set of system ids that are able to propagate supply from one
      * system to the next, or at which supply can be delivered to fleets if
      * supply can reach the system from elsewhere, or in which planets can
      * exchange supply between themselves (even if not leaving the system). */
    [[nodiscard]] const auto& SupplyUnobstructedSystems() const noexcept { return m_supply_unobstructed_systems; }

    /** Returns true if the specified lane travel is preserved against being blockaded (i.e., the empire
     * has in the start system at least one fleet that meets the requirements to preserve the lane (which
     * is determined in Empire::UpdateSupplyUnobstructedSystems(). */
    [[nodiscard]] bool        PreservedLaneTravel(int start_system_id, int dest_system_id) const;

    struct LaneEndpoints {
        int start = INVALID_OBJECT_ID;
        int end = INVALID_OBJECT_ID;
        constexpr auto operator<=>(const LaneEndpoints&) const noexcept = default;
#if (defined(__clang_major__) && (__clang_major__ < 16))
        LaneEndpoints() = default;
        LaneEndpoints(int s, int e) noexcept : start(s), end(e) {};
        LaneEndpoints(LaneEndpoints&&) noexcept = default;
        LaneEndpoints(const LaneEndpoints&) noexcept = default;
        LaneEndpoints& operator=(LaneEndpoints&&) noexcept = default;
        LaneEndpoints& operator=(const LaneEndpoints&) noexcept = default;
#endif
    };
    using LaneSet = boost::container::flat_set<LaneEndpoints>;

    using IntSet = boost::container::flat_set<int>;

    [[nodiscard]] IntSet      ExploredSystems() const;     ///< ids of systems that this empire has explored
    [[nodiscard]] int         TurnSystemExplored(int system_id) const;
    [[nodiscard]] LaneSet     KnownStarlanes(const Universe& universe) const;     ///< map from system id (start) to set of system ids (endpoints) of all starlanes known to this empire
    [[nodiscard]] LaneSet     VisibleStarlanes(const Universe& universe) const;   ///< map from system id (start) to set of system ids (endpoints) of all starlanes visible to this empire this turn
    [[nodiscard]] const auto& SitReps() const noexcept { return m_sitrep_entries; }
    [[nodiscard]] float       ProductionPoints() const;    ///< Returns the empire's current production point output (this is available industry not including stockpile)

    /** Returns ResourcePool for \a resource_type or 0 if no such ResourcePool exists. */
    [[nodiscard]] const ResourcePool& GetResourcePool(ResourceType type) const;
    [[nodiscard]] const auto& GetIndustryPool() const noexcept { return m_industry_pool; }
    [[nodiscard]] const auto& GetResearchPool() const noexcept { return m_research_pool; }
    [[nodiscard]] const auto& GetInfluencePool() const noexcept { return m_influence_pool; }

    [[nodiscard]] float       ResourceStockpile(ResourceType type) const;         ///< returns current stockpiled amount of resource \a type
    [[nodiscard]] float       ResourceOutput(ResourceType type) const;            ///< returns amount of resource \a type being generated by ResourceCenters
    [[nodiscard]] float       ResourceAvailable(ResourceType type) const;         ///< returns amount of resource \a type immediately available.  This = production + stockpile

    [[nodiscard]] const auto& GetPopulationPool() const noexcept { return m_population_pool; }
    [[nodiscard]] float       Population() const;                                 ///< returns total Population of empire

    [[nodiscard]] std::size_t SizeInMemory() const;

    /** If the object with id \a id is a planet owned by this empire, sets that
      * planet to be this empire's capital, and otherwise does nothing. */
    void SetCapitalID(int id, const ObjectMap& objects);

    /** Adopts the specified policy, assuming its conditions are met. Revokes
      * the policy if \a adopt is false; */
    void AdoptPolicy(const std::string& name, const std::string& category,
                     const ScriptingContext& context, int slot = -1);
    void DeAdoptPolicy(const std::string& name);

    /** Reverts adopted policies to the initial state for the current turn.
      * Does not verify if the initial adopted policies were in a valid
      * configuration.*/
    void RevertPolicies();

    /** Checks that all policy adoption conditions are met, removing any that
      * are not allowed. Also copies adopted policies to initial adopted
      * policies. Updates how many turns each policy has (ever) been adopted. */
    void UpdatePolicies(bool update_cumulative_adoption_time, int current_turn);

    /** Returns the meter with the indicated \a name if it exists, or nullptr. */
    [[nodiscard]] Meter* GetMeter(std::string_view name);
    void BackPropagateMeters() noexcept;

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
    void SetTechResearchProgress(const std::string& name, float progress,
                                 const ScriptingContext& context);

    /** Adds the indicated build to the production queue, placing it before
      * position \a pos.  If \a pos < 0 or queue.size() <= pos, the build is
      * placed at the end of the queue. */
    void PlaceProductionOnQueue(const ProductionQueue::ProductionItem& item,
                                boost::uuids::uuid uuid,
                                const ScriptingContext& context,
                                int number, int blocksize, int location, int pos = -1);

    /** Adds a copy of the production item at position \a index below it in
      * the queue, with one less quantity. Sets the quantity of the production
      * item at position \a index to 1, retaining its incomplete progress. */
    void SplitIncompleteProductionItem(int index, boost::uuids::uuid uuid, const ScriptingContext& context);
    /** Adds a copy of the production item at position \a index below it in
      * the queue, with no progress. */
    void DuplicateProductionItem(int index, boost::uuids::uuid uuid, const ScriptingContext& context);

    void SetProductionQuantity(int index, int quantity);     ///< Changes the remaining number to produce for queue item \a index to \a quantity
    void SetProductionQuantityAndBlocksize(int index, int quantity, int blocksize);   ///< Changes the remaining number and blocksize to produce for queue item \a index to \a quantity and \a blocksize
    void SetProductionRallyPoint(int index, int rally_point_id = INVALID_OBJECT_ID);  ///< Sets the rally point for ships produced by this produce, to which they are automatically ordered to move after they are produced.
    void MoveProductionWithinQueue(int index, int new_index);///< Moves queue item at \a index to \a new_index
    void MarkToBeRemoved(int index);                         ///< Marks the item at positon \a index to be removed from the queue
    void MarkNotToBeRemoved(int index);                      ///< Marks the item at position \a index not to be removed from the queue
    void PauseProduction(int index);                         ///< Sets the item at postion \a index paused, if such an index exists
    void ResumeProduction(int index);                        ///< Sets the item at postion \a index unpaused, if such an index exists
    void AllowUseImperialPP(int index, bool allow=true);     ///< Allows or disallows the use of the imperial stockpile for production

    void RemoveProductionFromQueue(int index);               ///< Removes the produce at position \a index in the production queue, if such an index exists.

    void AddNewlyResearchedTechToGrantAtStartOfNextTurn(std::string name); ///< Inserts the given Tech into the Empire's list of innovations. Call ApplyAddedTech to make it effective.
    void ApplyNewTechs(Universe& universe, int current_turn);   ///< Moves all Techs from the Empire's list of innovations into the Empire's list of available technologies.
    void AddPolicy(std::string name, int current_turn);         ///< Inserts the given Policy into the Empire's list of available policies
    void ApplyPolicies(Universe& universe, int current_turn);   ///< Unlocks anything unlocked by adopted policies

    //! Adds a given producible item (Building, Ship Hull, Ship part) to the
    //! list of available items.
    void UnlockItem(const UnlockableItem& item, Universe& universe, int current_turn);

    void AddBuildingType(std::string name, int current_turn);   ///< Inserts the given BuildingType into the Empire's list of available BuldingTypes.
    void AddShipPart(std::string name, int current_turn);       ///< Inserts the given ShipPart into the Empire's list of available ShipPart%s.
    void AddShipHull(std::string name, int current_turn);       ///< Inserts the given ship ShipHull into the Empire's list of available ShipHulls.

    void AddExploredSystem(int ID, int turn, const ObjectMap& objects); ///< Inserts the given ID into the Empire's list of explored systems.

    /** inserts given design id into the empire's set of designs in front of next design */
    void AddShipDesign(int ship_design_id, const Universe& universe, int next_design_id = INVALID_DESIGN_ID);
    int AddShipDesign(ShipDesign ship_design, Universe& universe); ///< inserts given ShipDesign into the Universe, adds the design's id to the Empire's set of ids, and returns the new design's id, which is INVALID_OBJECT_ID on failure.  If successful, universe takes ownership of passed ShipDesign.

    [[nodiscard]] std::string NewShipName(); ///< generates a random ship name, appending II, III, etc., to it if it has been used before by this empire
    void Eliminate(EmpireManager& empires, int current_turn);                         ///< Marks empire as eliminated and cleans up empire after it is eliminated.  Queues are cleared, capital is reset, and other state info not relevant to an eliminated empire is cleared
    void Win(const std::string& reason, const EmpireManager::container_type& empires, ///< Marks this empire as having won for this reason, and sends the appropriate sitreps
             int current_turn);
    void SetReady(bool ready);               ///< Marks this empire with readiness status
    void AutoTurnSetReady();                 ///< Decreases auto-turn counter and set empire ready if not expired or set unready
    void SetAutoTurn(int turns_count);       ///< Set auto-turn counter
    void SetLastTurnReceived(int last_turn_received) noexcept; ///< Set last turn received

    /** Inserts the given SitRep entry into the empire's sitrep list. */
    void AddSitRepEntry(const SitRepEntry& entry);
    void AddSitRepEntry(SitRepEntry&& entry);
    void ClearSitRep();                              ///< Clears all sitrep entries

    void RemoveTech(const std::string& name);        ///< Removes the given Tech from the empire's list
    void RemovePolicy(const std::string& name);      ///< Removes the given Policy from the list available to the empire

    //! Removes a given producible item (Building, Ship Hull, Ship Part) from
    //! the list of available items.
    void LockItem(const UnlockableItem& item);

    void RemoveBuildingType(const std::string& name);///< Removes the given BuildingType from the empire's list
    //! Removes the given ShipPart from the empire's list
    void RemoveShipPart(const std::string& name);

    //! Removes the given ShipHull from the empire's list
    void RemoveShipHull(const std::string& name);

    void RemoveShipDesign(int ship_design_id);       ///< Removes the ShipDesign with the given id from the empire's set

    /** Calculates ranges that systems can send fleet and resource supplies,
      * using the specified st of \a known_objects as the source for supply-
      * producing objects and systems through which it can be propagated. */
    void UpdateSystemSupplyRanges(const std::span<const int> known_objects, const ObjectMap& objects);
    /** Calculates ranges that systems can send fleet and resource supplies. */
    void UpdateSystemSupplyRanges(const Universe& universe);
    /** Calculates systems that can propagate supply (fleet or resource) using
      * the specified set of \a known_systems */
    void UpdateSupplyUnobstructedSystems(const ScriptingContext& context,
                                         const std::span<const int> known_systems,
                                         bool precombat = false);
    /** Calculates systems that can propagate supply using this empire's own /
      * internal list of explored systems. */
    void UpdateSupplyUnobstructedSystems(const ScriptingContext& context, bool precombat = false);
    /** Updates fleet ArrivalStarlane to flag fleets of this empire that are not
      * blockaded post-combat must be done after *all* noneliminated empires
      * have updated their unobstructed systems */
    void UpdateUnobstructedFleets(ObjectMap& objects, const std::unordered_set<int>& known_destroyed_objects) const;
    /** Records, in a list of pending updates, the start_system exit lane to the
      * specified destination as accessible to this empire*/
    void RecordPendingLaneUpdate(int start_system_id, int dest_system_id, const ObjectMap& objects);
    /** Processes all the pending lane access updates.  This is managed as a two
      * step process to avoid order-of-processing issues. */
    void UpdatePreservedLanes();

    /** Checks for production projects that have been completed, and places them
      * at their respective production sites.  Which projects have been
      * completed is determined by the results of previously-called Update() on
      * the production queue (which determines how much PP each project receives
      * but does not actually spend them).  This function spends the PP, removes
      * complete items from the queue and creates the results in the universe. */
    void CheckProductionProgress(
        ScriptingContext& context, const std::vector<std::tuple<std::string_view, int, float, int>>& costs_times);

    /** Checks for tech projects that have been completed, and returns a vector
      * of the techs that should be added to the known techs list. */
    std::vector<std::string> CheckResearchProgress(
        const ScriptingContext& context, const std::vector<std::tuple<std::string_view, double, int>>& costs_times);

    /** Eventually : Will check for social projects that have been completed and
      * / or process ongoing social projects, and update the empire's influence
      * stockpile to account for influence production and expenditures.*/
    void CheckInfluenceProgress();

    void SetColor(EmpireColor color) noexcept { m_color = color; }
    void SetName(std::string name) noexcept { m_name = std::move(name); }
    void SetPlayerName(std::string player_name) { m_player_name = std::move(player_name); }

    void SetResourceStockpile(ResourceType resource_type, float stockpile); ///< Sets current \a stockpile amount of indicated \a resource_type

    /** Determines ResourceCenters that can provide resources for this empire and sets
      * the supply groups used for each ResourcePool as appropriate for each resource.
      * call UpdateResourceSupply before calling this. */
    void InitResourcePools(const ObjectMap& objects, const SupplyManager& supply);

    /** Resets production of resources and calculates allocated resources (on
      * each item in queues and overall) for each resource by calling
      * UpdateResearchQueue, UpdateProductionQueue, UpdateInfluenceSpending.
      * Does not actually "spend" resources, but just determines how much and
      * on what to spend.  Actual consumption of resources, removal of items
      * from queue, processing of finished items and population growth happens
      * in various Check(Whatever)Progress functions. */
    void UpdateResourcePools(const ScriptingContext& context,
                             const std::vector<std::tuple<std::string_view, double, int>>& research_costs,
                             const std::vector<std::pair<int, double>>& annex_costs,
                             const std::vector<std::pair<std::string_view, double>>& policy_costs,
                             const std::vector<std::tuple<std::string_view, int, float, int>>& prod_costs);
    /** Calls Update() on empire's research queue, which recalculates the RPs
      * spent on and number of turns left for each tech in the queue. */
    void UpdateResearchQueue(const ScriptingContext& context,
                             const std::vector<std::tuple<std::string_view, double, int>>& costs_times);
    std::vector<std::tuple<std::string_view, double, int>> TechCostsTimes(const ScriptingContext& context) const;

    /** Calls Update() on empire's production queue, which recalculates the PPs
      * spent on and number of turns left for each project in the queue. */
    void UpdateProductionQueue(const ScriptingContext& context,
                               const std::vector<std::tuple<std::string_view, int, float, int>>& prod_costs);
    std::vector<std::tuple<std::string_view, int, float, int>>
        ProductionCostsTimes(const ScriptingContext& contest) const;

    /** Eventually: Calls appropriate subsystem Update to calculate influence
      * spent on social projects and maintenance of buildings.  Later call to
      * CheckInfluenceProgress() will then have the correct allocations of
      * influence. */
    void UpdateInfluenceSpending(const ScriptingContext& context,
                                 const std::vector<std::pair<int, double>>& annex_costs,
                                 const std::vector<std::pair<std::string_view, double>>& policy_costs);
    std::vector<std::pair<int, double>> PlanetAnnexationCosts(const ScriptingContext& context) const;
    std::vector<std::pair<std::string_view, double>> PolicyAdoptionCosts(const ScriptingContext& context) const;

    void UpdatePopulationGrowth(const ObjectMap& objects);

    /** Resets empire meters. */
    void ResetMeters() noexcept;

    void UpdateOwnedObjectCounters(const Universe& universe);

    /** called after loading a saved game, remove obsolete stuff such as no longer
      * existing policies... */
    void CheckObsoleteGameContent();

    void SetAuthenticated(bool authenticated = true);

    void RecordShipShotDown(const Ship& ship);
    void RecordShipLost(const Ship& ship);
    void RecordShipScrapped(const Ship& ship);
    void RecordBuildingScrapped(const Building& building);
    void RecordPlanetInvaded(const Planet& planet);
    void RecordPlanetDepopulated(const Planet& planet);

    [[nodiscard]] int TotalShipsOwned() const;
    [[nodiscard]] int TotalShipPartsOwned() const;    ///< Total number of parts for all owned ships in this empire
    [[nodiscard]] int TotalBuildingsOwned() const;
    [[nodiscard]] auto& SpeciesShipsOwned() const noexcept { return m_species_ships_owned; }
    [[nodiscard]] auto& ShipDesignsOwned() const noexcept { return m_ship_designs_owned; }
    [[nodiscard]] auto& ShipPartsOwned() const noexcept { return m_ship_parts_owned; }
    [[nodiscard]] auto& ShipPartClassOwned() const noexcept { return m_ship_part_class_owned; }
    [[nodiscard]] auto& SpeciesColoniesOwned() const noexcept { return m_species_colonies_owned; }
    [[nodiscard]] auto OutpostsOwned() const noexcept { return m_outposts_owned; }
    [[nodiscard]] auto& BuildingTypesOwned() const noexcept { return m_building_types_owned; }
    [[nodiscard]] auto& EmpireShipsDestroyed() const noexcept { return m_empire_ships_destroyed; }
    [[nodiscard]] auto& ShipDesignsDestroyed() const noexcept { return m_ship_designs_destroyed; }
    [[nodiscard]] auto& SpeciesShipsDestroyed() const noexcept { return m_species_ships_destroyed; }
    [[nodiscard]] auto& SpeciesPlanetsInvaded() const noexcept { return m_species_planets_invaded; }
    [[nodiscard]] auto& ShipDesignsInProduction() const noexcept { return m_ship_designs_in_production; }
    [[nodiscard]] auto& SpeciesShipsProduced() const noexcept { return m_species_ships_produced; }
    [[nodiscard]] auto& ShipDesignsProduced() const noexcept { return m_ship_designs_produced; }
    [[nodiscard]] auto& SpeciesShipsLost() const noexcept { return m_species_ships_lost; }
    [[nodiscard]] auto& ShipDesignsLost() const noexcept { return m_ship_designs_lost; }
    [[nodiscard]] auto& SpeciesShipsScrapped() const noexcept { return m_species_ships_scrapped; }
    [[nodiscard]] auto& ShipDesignsScrapped() const noexcept { return m_ship_designs_scrapped; }
    [[nodiscard]] auto& SpeciesPlanetsDepoped() const noexcept { return m_species_planets_depoped; }
    [[nodiscard]] auto& SpeciesPlanetsBombed() const noexcept { return m_species_planets_bombed; }
    [[nodiscard]] auto& BuildingTypesProduced() const noexcept { return m_building_types_produced; }
    [[nodiscard]] auto& BuildingTypesScrapped() const noexcept { return m_building_types_scrapped; }
    [[nodiscard]] auto& TurnsSystemsExplored() const noexcept { return m_explored_systems; }
    [[nodiscard]] auto LastTurnReceived() const noexcept { return m_last_turn_received; }

    /** Processes Builditems on queues of empires other than the indicated
      * empires, at the location with id \a location_id and, as appropriate,
      * adds them to the build queue of the indicated empires (if it is an
      * empire), deletes them, or leaves them on the build queue of their
      * current empire */
    static void ConquerProductionQueueItemsAtLocation(int location_id, int empire_id, EmpireManager& empires);

    mutable boost::signals2::signal<void ()> ShipDesignsChangedSignal;
    mutable boost::signals2::signal<void ()> PoliciesChangedSignal;

private:
    void Init();

    int         m_id = ALL_EMPIRES;                ///< Empire's unique numeric id
    int         m_capital_id = INVALID_OBJECT_ID;  ///< the ID of the empire's capital planet
    std::string m_name;                            ///< Empire's name
    std::string m_player_name;                     ///< Empire's Player's name

    EmpireColor m_color = {{128, 255, 255, 255}};

    static constexpr int INVALID_SLOT_INDEX = -1;

    struct PolicyAdoptionInfo {
        PolicyAdoptionInfo() = default;
        PolicyAdoptionInfo(int turn, std::string cat, int slot) noexcept :
            adoption_turn(turn),
            slot_in_category(slot),
            category(std::move(cat))
        {}

        int adoption_turn = INVALID_GAME_TURN;
        int slot_in_category = INVALID_SLOT_INDEX;
        std::string category;

        [[nodiscard]] bool operator==(const PolicyAdoptionInfo&) const noexcept = default;

        friend class boost::serialization::access;
        template <typename Archive>
        void serialize(Archive& ar, const unsigned int version);
    };
    std::map<std::string, PolicyAdoptionInfo, std::less<>> m_adopted_policies;                 ///< map from policy name to turn, category, and slot in/on which it was adopted
    std::map<std::string, PolicyAdoptionInfo, std::less<>> m_initial_adopted_policies;         ///< adopted policies at start of turn
    std::map<std::string, int>                             m_policy_adoption_total_duration;   ///< how many turns each policy has been adopted over the course of the game by this empire
    std::map<std::string, int>                             m_policy_adoption_current_duration; ///< how many turns each currently-adopted policy has been adopted since it was last adopted. somewhat redundant with adoption_turn in AdoptionInfo, but seems necessary to avoid off-by-one issues between client and server
    std::set<std::string, std::less<>>                     m_available_policies;               ///< names of unlocked policies

public:
    // prepare policy info to serialize for various recipient empires
    void PrepPolicyInfoForSerialization(const ScriptingContext& context);

private:
    std::map<int, decltype(m_adopted_policies)>                 m_adopted_policies_to_serialize_for_empires;
    std::map<int, decltype(m_initial_adopted_policies)>         m_initial_adopted_policies_to_serialize_for_empires;
    std::map<int, decltype(m_policy_adoption_total_duration)>   m_policy_adoption_total_duration_to_serialize_for_empires;
    std::map<int, decltype(m_policy_adoption_current_duration)> m_policy_adoption_current_duration_to_serialize_for_empires;
    std::map<int, decltype(m_available_policies)>               m_available_policies_to_serialize_for_empires;

    const decltype(m_adopted_policies)& GetAdoptedPoliciesToSerialize(int encoding_empire) const;
    const decltype(m_initial_adopted_policies)& GetInitialPoliciesToSerialize(int encoding_empire) const;
    const decltype(m_policy_adoption_total_duration)& GetAdoptionTotalDurationsToSerialize(int encoding_empire) const;
    const decltype(m_policy_adoption_current_duration)& GetAdoptionCurrentDurationsToSerialize(int encoding_empire) const;
    const decltype(m_available_policies)& GetAvailablePoliciesToSerialize(int encoding_empire) const;


    using StringFlatSet = boost::container::flat_set<std::string, std::less<>>;
    using StringIntMap = boost::container::flat_map<std::string, int, std::less<>>;
    using MeterMap = boost::container::flat_map<std::string, Meter, std::less<>>;

    StringFlatSet                   m_victories;                ///< The ways that the empire has won, if any
    StringFlatSet                   m_newly_researched_techs;   ///< names of researched but not yet effective technologies, and turns on which they were acquired.
    StringIntMap                    m_techs;                    ///< names of researched technologies, and turns on which they were acquired.
    MeterMap                        m_meters;                   ///< empire meters

    ResearchQueue                   m_research_queue;           ///< the queue of techs being or waiting to be researched
    std::map<std::string, float>    m_research_progress;        ///< fractional progress (0 to 1) of partially-researched techs; fully researched techs are removed

    ProductionQueue                 m_production_queue;         ///< the queue of items being or waiting to be built
    InfluenceQueue                  m_influence_queue;

    StringFlatSet                   m_available_building_types; ///< acquired BuildingTypes
    StringFlatSet                   m_available_ship_parts;     ///< acquired ShipParts
    StringFlatSet                   m_available_ship_hulls;     ///< acquired ShipHulls

    std::map<int, int>              m_explored_systems;         ///< systems explored by this empire and the turn on which they were explored
    std::set<int>                   m_known_ship_designs;       ///< ids of ship designs in the universe that this empire knows about

    std::vector<SitRepEntry>        m_sitrep_entries;           ///< The Empire's sitrep entries

    ResourcePool                    m_research_pool{ResourceType::RE_RESEARCH};
    ResourcePool                    m_industry_pool{ResourceType::RE_INDUSTRY};
    ResourcePool                    m_influence_pool{ResourceType::RE_INFLUENCE};
    PopulationPool                  m_population_pool;

    std::map<std::string, int>      m_ship_names_used;          ///< map from name to number of times used

    std::map<std::string, int>      m_species_ships_owned;      ///< how many ships of each species does this empire currently own?
    std::map<int, int>              m_ship_designs_owned;       ///< how many ships of each design does this empire currently own?

    //! How many ShipPart%s are currently owned, indexed by ShipPart
    std::map<std::string, int>      m_ship_parts_owned;

    std::map<ShipPartClass, int>    m_ship_part_class_owned;    ///< how many ship parts are currently owned, indexed by ShipPartClass
    std::map<std::string, int>      m_species_colonies_owned;   ///< how many colonies of each species does this empire currently own?
    std::map<std::string, int>      m_building_types_owned;     ///< how many buildings does this empire currently own?

    std::map<int, int>              m_ship_designs_in_production;   ///< how many ships of each design has this empire in active production in its production queue

    std::unordered_set<int>         m_ships_destroyed;
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
    std::map<int, std::set<int>>    m_preserved_system_exit_lanes;  ///< for each system known to this empire, the set of exit lanes preserved for fleet travel even if otherwise blockaded
    std::map<int, std::set<int>>    m_pending_system_exit_lanes;    ///< pending updates to m_preserved_system_exit_lanes
    int                             m_auto_turn_count = 0;          ///< auto-turn counter value
    int                             m_last_turn_received = INVALID_GAME_TURN; ///< last turn empire completedly received game state

public:
    // prepare tech, queue, and availability info for serialization for various empires
    void PrepQueueAvailabilityInfoForSerialization(const ScriptingContext& context);

private:
    std::map<int, decltype(m_techs)>                    m_techs_to_serialize_for_empires;
    std::map<int, decltype(m_research_queue)>           m_research_queue_to_serialize_for_empires;
    std::map<int, decltype(m_research_progress)>        m_research_progress_to_serialize_for_empires;
    std::map<int, decltype(m_production_queue)>         m_production_queue_to_serialize_for_empires;
    std::map<int, decltype(m_influence_queue)>          m_influence_queue_to_serialize_for_empires;
    std::map<int, decltype(m_available_building_types)> m_available_building_types_to_serialize_for_empires;
    std::map<int, decltype(m_available_ship_parts)>     m_available_ship_parts_to_serialize_for_empires;
    std::map<int, decltype(m_available_ship_hulls)>     m_available_ship_hulls_to_serialize_for_empires;

    const decltype(Empire::m_techs)& GetTechsToSerialize(int encoding_empire);
    const decltype(Empire::m_research_queue)& GetResearchQueueToSerialize(int encoding_empire);
    const decltype(Empire::m_research_progress)& GetResearchProgressToSerialize(int encoding_empire);
    const decltype(Empire::m_production_queue)& GetProductionQueueToSerialize(int encoding_empire);
    const decltype(Empire::m_influence_queue)& GetInfluenceQueueToSerialize(int encoding_empire);
    const decltype(Empire::m_available_building_types)& GetAvailableBuildingsToSerialize(int encoding_empire);
    const decltype(Empire::m_available_ship_parts)& GetAvailablePartsToSerialize(int encoding_empire);
    const decltype(Empire::m_available_ship_hulls)& GetAvailableHullsToSerialize(int encoding_empire);


    /** The source id is the id of any object owned by the empire.  It is
        mutable so that Source() can be const and still cache its result. */
    mutable int m_source_id = INVALID_OBJECT_ID;

    int         m_outposts_owned = 0;       ///< how many uncolonized outposts does this empire currently own?

    bool        m_ready = false;            ///< readiness status of empire
    bool        m_authenticated = false;    ///< Empire's Player's authentication flag. Set if only player with empire's player's name should play this empire.
    bool        m_eliminated = false;       ///< Whether the empire has lost

    friend class boost::serialization::access;
    Empire() { Init(); }
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif
