#ifndef _ProductionQueue_h_
#define _ProductionQueue_h_

#include "../util/Export.h"
#include "../universe/Enums.h"

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <boost/serialization/access.hpp>
#include <boost/signals2/signal.hpp>

class ResourcePool;
FO_COMMON_API extern const int INVALID_DESIGN_ID;
FO_COMMON_API extern const int INVALID_OBJECT_ID;
FO_COMMON_API extern const int ALL_EMPIRES;


struct FO_COMMON_API ProductionQueue {
    /** The type that specifies a single production item (BuildType and name string). */
    struct FO_COMMON_API ProductionItem {
        ProductionItem();

        ProductionItem(BuildType build_type_);   ///< basic ctor for BuildTypes only have one type of item (e.g. stockpile transfer item)
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
                bool allowed_imperial_stockpile_use_ = true);

        Element(BuildType build_type, std::string name, int empire_id_,
                int ordered_, int remaining_, int blocksize_,
                int location_, bool paused_ = false,
                bool allowed_imperial_stockpile_use_ = true);

        Element(BuildType build_type, int design_id, int empire_id_,
                int ordered_, int remaining_, int blocksize_,
                int location_, bool paused_ = false,
                bool allowed_imperial_stockpile_use_ = true);

        ProductionItem  item;
        int             empire_id = ALL_EMPIRES;
        int             ordered = 0;                ///< how many of item (blocks) to produce
        int             blocksize = 1;              ///< size of block to produce (default=1)
        int             remaining = 0;              ///< how many left to produce
        int             location = INVALID_OBJECT_ID;///< the ID of the UniverseObject at which this item is being produced
        float           allocated_pp = 0.0f;        ///< PP allocated to this ProductionQueue Element by Empire production update
        float           progress = 0.0f;            ///< fraction of this item that is complete.
        float           progress_memory = 0.0f;     ///< updated by server turn processing; aides in allowing blocksize changes to be undone in same turn w/o progress loss
        int             blocksize_memory = 1;       ///< used along with progress_memory
        int             turns_left_to_next_item = -1;
        int             turns_left_to_completion = -1;
        int             rally_point_id = INVALID_OBJECT_ID;
        bool            paused = false;
        bool            allowed_imperial_stockpile_use = true;

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
    float ExpectedNewStockpileAmount() const { return m_expected_new_stockpile_amount; }

    /** Returns the PP amount expected to be transferred via stockpiling projects to the Imperial Stockpile
    * for the next turn, based on the current ProductionQueue allocations. */
    float ExpectedProjectTransferToStockpile() const { return m_expected_project_transfer_to_stockpile; }

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
    int                             m_projects_in_progress = 0;
    std::map<std::set<int>, float>  m_object_group_allocated_pp;
    std::map<std::set<int>, float>  m_object_group_allocated_stockpile_pp;
    float                           m_expected_new_stockpile_amount = 0.0f;
    float                           m_expected_project_transfer_to_stockpile = 0.0f;
    int                             m_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif //  _ProductionQueue_h_
