#ifndef _PopulationPool_h_
#define _PopulationPool_h_

#include "../util/Export.h"

#include <boost/signals2/signal.hpp>
#include <boost/serialization/nvp.hpp>

#include <vector>

/** The PopulationPool class keeps track of an empire's total population and its growth. */
class FO_COMMON_API PopulationPool {
public:
    /** \name Structors */ //@{
    PopulationPool();
    //@}

    /** \name Accessors */ //@{
    const std::vector<int>& PopCenterIDs() const { return m_pop_center_ids; }   ///< returns the PopCenter vector
    float Population() const;   ///< returns current total population
    //@}

    /** \name Mutators */ //@{
    /** emitted after updating population and growth numbers */
    mutable boost::signals2::signal<void ()> ChangedSignal;

    void    SetPopCenters(const std::vector<int>& pop_center_ids);
    void    Update();                           ///< recalculates total population and growth
    //@}

private:
    std::vector<int>    m_pop_center_ids;       ///< UniverseObject ids of PopCenters that contribute to the pool
    float               m_population = 0.0f;    ///< total population of all PopCenters in pool

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

template <class Archive>
void PopulationPool::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_pop_center_ids);
}

#endif //_PopulationPool_h_
