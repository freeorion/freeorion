#ifndef _PopulationPool_h_
#define _PopulationPool_h_

#include "../util/Export.h"

#include <boost/signals2/signal.hpp>
#include <boost/serialization/nvp.hpp>

#include <vector>
class ObjectMap;

/** The PopulationPool class keeps track of an empire's total population and its growth. */
class FO_COMMON_API PopulationPool {
public:
    const auto& PopCenterIDs() const noexcept { return m_pop_center_ids; }
    float Population() const noexcept { return m_population; }

    /** emitted after updating population and growth numbers */
    mutable boost::signals2::signal<void ()> ChangedSignal;

    void SetPopCenters(std::vector<int> pop_center_ids);
    void Update(const ObjectMap& objects); ///< recalculates total population and growth

private:
    std::vector<int> m_pop_center_ids;    ///< UniverseObject ids of PopCenters that contribute to the pool
    float            m_population = 0.0f; ///< total population of all PopCenters in pool

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

template <typename Archive>
void PopulationPool::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_pop_center_ids);
}


#endif
