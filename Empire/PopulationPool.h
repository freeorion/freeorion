#ifndef _PopulationPool_h_
#define _PopulationPool_h_

#include "../util/Export.h"

#include <boost/signals2/signal.hpp>
#include <boost/serialization/nvp.hpp>
#include "../universe/ConstantsFwd.h"
#include "../util/ranges.h"
#include <vector>

class ObjectMap;

/** The PopulationPool class keeps track of an empire's total population and its growth. */
class FO_COMMON_API PopulationPool {
public:
    const auto& PopCenterIDs() const noexcept { return m_pop_center_ids; }
    float Population() const noexcept { return m_population; }

    /** emitted after updating population and growth numbers */
    mutable boost::signals2::signal<void ()> ChangedSignal;

    void SetPopCenters(std::vector<UniverseObjectID> pop_center_ids);
    void Update(const ObjectMap& objects); ///< recalculates total population and growth

private:
    std::vector<UniverseObjectID> m_pop_center_ids;    ///< UniverseObject ids of PopCenters that contribute to the pool
    float                         m_population = 0.0f; ///< total population of all PopCenters in pool

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

template <typename Archive>
void PopulationPool::serialize(Archive& ar, const unsigned int version)
{
    std::vector<int> ids;
    if constexpr (Archive::is_saving::value)
        ids = m_pop_center_ids | range_transform([](const auto& id) noexcept { return Value(id); }) | range_to_vec;
    ar  & boost::serialization::make_nvp("m_pop_center_ids", ids);
    if constexpr (Archive::is_loading::value)
        m_pop_center_ids = ids | range_transform([](const auto& id) noexcept { return UniverseObjectID{id}; }) | range_to_vec;
}


#endif
