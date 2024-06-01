#ifndef _ProductionQueue_h_
#define _ProductionQueue_h_

#include "../util/Export.h"
#include "../universe/ConstantsFwd.h"
#include "../universe/Enums.h"
#include "../universe/ScriptingContext.h"

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <boost/serialization/access.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>

class ResourcePool;

//! The general type of production being done at a ProdCenter.  Within each
//! valid type, a specific kind of item is being built, e.g. under BT_BUILDING
//! a kind of building called "SuperFarm" might be built.
FO_ENUM(
    (BuildType),
    ((INVALID_BUILD_TYPE, -1))
    //! No building is taking place
    ((BT_NOT_BUILDING))
    //! A Building object is being produced
    ((BT_BUILDING))
    //! A Ship object is being produced
    ((BT_SHIP))
    //! A project may generate effects while on the queue, may or may not ever
    //! complete, and does not result in a ship or building being produced
    ((BT_PROJECT))
    ((BT_STOCKPILE))
    ((NUM_BUILD_TYPES))
)

struct FO_COMMON_API ProductionQueue {
    /** The type that specifies a single production item (BuildType and name string). */
    struct FO_COMMON_API ProductionItem {
    private:
        static constexpr bool pi_move_construct_noexcept = noexcept(std::string{std::declval<std::string&&>()});

    public:
        ProductionItem() = default;

        explicit ProductionItem(BuildType build_type_) :
            build_type(build_type_),
            name(build_type_ == BuildType::BT_STOCKPILE ? /*UserStringNop*/("PROJECT_BT_STOCKPILE") : "")
        {} ///< for BuildTypes with one type of item (e.g. stockpile transfer item)

        ProductionItem(BuildType build_type_, std::string name_) noexcept(pi_move_construct_noexcept) :
            build_type(build_type_),
            name(std::move(name_))
        {} ///< for BuildTypes that use std::string to identify specific items (BuildingTypes)

        ProductionItem(BuildType build_type_, int design_id_, const Universe& universe); ///< for BuildTypes that use int to indentify the design of the item (ShipDesigns)

        [[nodiscard]] bool CostIsProductionLocationInvariant(const Universe& universe) const; ///< indicates whether production location can change the cost of this item. This is useful for cachcing cost results per-location or once for all locations.

        /** Returns the total cost per item (blocksize 1) and the minimum number of
          * turns required to produce the indicated item, or (-1.0, -1) if the item
          * is unknown, unavailable, or invalid. */
        [[nodiscard]] std::pair<float, int> ProductionCostAndTime(int empire_id, int location_id,
                                                                  const ScriptingContext& context) const;

        // non-defaulted operator< to handle different build_type differently
        [[nodiscard]] bool operator<(const ProductionItem& rhs) const noexcept {
            if (build_type < rhs.build_type)
                return true;
            else if (build_type > rhs.build_type)
                return false;
            else if (build_type == BuildType::BT_BUILDING)
                return name < rhs.name;
            else if (build_type == BuildType::BT_SHIP)
                return design_id < rhs.design_id;
            return false;
        }

        [[nodiscard]] bool operator==(const ProductionItem&) const noexcept  = default;

        [[nodiscard]] bool EnqueueConditionPassedAt(int location_id, const ScriptingContext& context) const;

        [[nodiscard]] std::map<std::string, std::map<int, float>> CompletionSpecialConsumption(
            int location_id, const ScriptingContext& context) const; // for each special name, what object ids have those special capacities reduced by what amount for full completion of the production item
        [[nodiscard]] std::map<MeterType, std::map<int, float>>   CompletionMeterConsumption(
            int location_id, const ScriptingContext& context) const;  // for each meter type, what object ids have those meters reduced by what amount for full completion of the production item

        [[nodiscard]] std::string Dump() const;

        BuildType   build_type = BuildType::INVALID_BUILD_TYPE;
        // only one of these may be valid, depending on BuildType
        std::string name;
        int         design_id = INVALID_DESIGN_ID;

    private:
        friend class boost::serialization::access;
        template <typename Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    /** The type of a single element in the production queue. */
    struct FO_COMMON_API Element {
        Element() = default;

        Element(ProductionItem item_, int empire_id_, boost::uuids::uuid uuid_, int ordered_,
                int remaining_, int blocksize_, int location_) :
            item(std::move(item_)),
            empire_id(empire_id_),
            ordered(ordered_),
            blocksize(blocksize_),
            remaining(remaining_),
            location(location_),
            blocksize_memory(blocksize_),
            allowed_imperial_stockpile_use(item.build_type != BuildType::BT_STOCKPILE),
            uuid(uuid_)
        {}

        /** Returns the total cost per item (blocksize 1) and the minimum number of
          * turns required to produce the indicated item, or (-1.0, -1) if the item
          * is unknown, unavailable, or invalid. */
        [[nodiscard]] auto ProductionCostAndTime(const ScriptingContext& context) const
        { return item.ProductionCostAndTime(empire_id, location, context); }


        ProductionItem      item;
        int                 empire_id = ALL_EMPIRES;
        int                 ordered = 0;                ///< how many of item (blocks) to produce
        int                 blocksize = 1;              ///< size of block to produce (default=1)
        int                 remaining = 0;              ///< how many left to produce
        int                 location = INVALID_OBJECT_ID;///< the ID of the UniverseObject at which this item is being produced
        float               allocated_pp = 0.0f;        ///< PP allocated to this ProductionQueue Element by Empire production update
        float               progress = 0.0f;            ///< fraction of this item that is complete.
        float               progress_memory = 0.0f;     ///< updated by server turn processing; aides in allowing blocksize changes to be undone in same turn w/o progress loss
        int                 blocksize_memory = 1;       ///< used along with progress_memory
        int                 turns_left_to_next_item = -1;
        int                 turns_left_to_completion = -1;
        int                 rally_point_id = INVALID_OBJECT_ID;
        bool                paused = false;
        bool                to_be_removed = false;
        bool                allowed_imperial_stockpile_use = false;
        boost::uuids::uuid  uuid = boost::uuids::nil_uuid();

        [[nodiscard]] std::string Dump() const;

    private:
        friend class boost::serialization::access;
        template <typename Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    typedef std::deque<Element> QueueType;

    /** The ProductionQueue iterator type.  Dereference yields a Element. */
    typedef QueueType::iterator iterator;
    /** The const ProductionQueue iterator type.  Dereference yields a Element. */
    typedef QueueType::const_iterator const_iterator;

    ProductionQueue() = default;
    explicit ProductionQueue(int empire_id);

    [[nodiscard]] int     ProjectsInProgress() const noexcept { return m_projects_in_progress; } ///< number of production projects currently (perhaps partially) funded.
    [[nodiscard]] float   TotalPPsSpent() const; ///< number of PPs currently spent on the projects in this queue.
    [[nodiscard]] int     EmpireID() const noexcept { return m_empire_id; }

    /** Returns map from sets of object ids that can share resources to amount
      * of PP allocated to production queue elements that have build locations
      * in systems in the group. */
    [[nodiscard]] auto& AllocatedPP() const noexcept { return m_object_group_allocated_pp; }

    /** Returns map from sets of object ids that can share resources to amount
     * of stockpile PP allocated to production queue elements that have build locations
     * in systems in the group. */
    [[nodiscard]] auto& AllocatedStockpilePP() const noexcept { return m_object_group_allocated_stockpile_pp; }

    /** Returns sum of stockpile meters of empire-owned objects. */
    [[nodiscard]] float StockpileCapacity(const ObjectMap& objects) const;

    /** Returns the value expected for the Imperial Stockpile for the next turn, based on the current
    * ProductionQueue allocations. */
    [[nodiscard]] float ExpectedNewStockpileAmount() const noexcept { return m_expected_new_stockpile_amount; }

    /** Returns the PP amount expected to be transferred via stockpiling projects to the Imperial Stockpile
    * for the next turn, based on the current ProductionQueue allocations. */
    [[nodiscard]] float ExpectedProjectTransferToStockpile() const noexcept { return m_expected_project_transfer_to_stockpile; }

    /** Returns sets of object ids that have more available than allocated PP */
    [[nodiscard]] std::vector<std::vector<int>> ObjectsWithWastedPP(const ResourcePool& industry_pool) const;

    // STL container-like interface
    [[nodiscard]] bool           empty() const noexcept { return m_queue.empty(); }
    [[nodiscard]] auto           size() const noexcept { return m_queue.size(); }
    [[nodiscard]] auto           begin() const noexcept { return m_queue.begin(); }
    [[nodiscard]] auto           end() const noexcept { return m_queue.end(); }
    [[nodiscard]] const_iterator find(int i) const;
    [[nodiscard]] const Element& operator[](int i) const;

    [[nodiscard]] const_iterator find(boost::uuids::uuid uuid) const;
    [[nodiscard]] int            IndexOfUUID(boost::uuids::uuid uuid) const;

    /** Recalculates the PPs spent on and number of turns left for each project
      * in the queue.  Also determines the number of projects in progress, and
      * the industry consumed by projects in each resource-sharing group of
      * systems.  Does not actually "spend" the PP; a later call to
      * empire->CheckProductionProgress(...) will actually spend PP, remove
      * items from queue and create them in the universe. */
    void Update(const ScriptingContext& context,
                const std::vector<std::tuple<std::string_view, int, float, int>>& prod_costs);

    // STL container-like interface
    void     push_back(Element element);
    void     insert(iterator it, Element element);
    void     erase(int i);
    iterator erase(iterator it);

    [[nodiscard]] auto     begin() noexcept { return m_queue.begin(); }
    [[nodiscard]] auto     end() noexcept { return m_queue.end(); }
    [[nodiscard]] iterator find(int i);
    Element&               operator[](int i);

    void clear();

    mutable boost::signals2::signal<void ()> ProductionQueueChangedSignal;

private:
    using int_flat_set = boost::container::flat_set<int>;
    QueueType                       m_queue;
    int                             m_projects_in_progress = 0;
    std::map<int_flat_set, float>   m_object_group_allocated_pp;
    std::map<int_flat_set, float>   m_object_group_allocated_stockpile_pp;
    float                           m_expected_new_stockpile_amount = 0.0f;
    float                           m_expected_project_transfer_to_stockpile = 0.0f;
    int                             m_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif
