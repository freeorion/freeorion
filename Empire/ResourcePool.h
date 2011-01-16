// -*- C++ -*-
#ifndef _ResourcePool_h_
#define _ResourcePool_h_

#include "../universe/Enums.h"

#include <boost/signal.hpp>
#include <boost/serialization/nvp.hpp>

#include <vector>
#include <set>

class ResourceCenter;
class PopCenter;
class UniverseObject;
class Empire;

/** The ResourcePool class keeps track of an empire's stockpile and production
  * of a particular resource (food, minerals, trade, research or industry). */
class ResourcePool
{
public:
    /** \name Structors */ //@{
    ResourcePool(ResourceType type);
    //@}

    /** \name Accessors */ //@{
    const std::vector<int>&         ResourceCenterIDs() const;              ///< returns UniverseObject IDs ResourceCenters in the pool
    int                             StockpileSystemID() const;              ///< returns ID of stockpile system

    double                          Stockpile() const;                      ///< returns current stockpiled amount of resource
    double                          Production() const;                     ///< returns amount of resource being produced by all ResourceCenter
    double                          GroupProduction(int system_id) const;   ///< returns amount of resource being produced by resource sharing group that contains the system with id \a system_id

    double                          TotalAvailable() const;                 ///< returns amount of resource immediately available = production + stockpile from all ResourceCenters, ignoring limitations of connections between centers
    std::map<std::set<int>, double> Available() const;                      ///< returns the sets of groups of systems that can share resources, and the amount of this pool's resource that each group has available
    double                          GroupAvailable(int system_id) const;    ///< returns amount of resource available in resource sharing group that contains the system with id \a system_id

    std::string                     Dump() const;
    //@}

    /** \name Mutators */ //@{
    mutable boost::signal<void ()> ChangedSignal;                       ///< emitted after updating production, or called externally to indicate that stockpile and change need to be refreshed

    void        SetResourceCenters(const std::vector<int>& resource_center_ids);
    void        SetSystemSupplyGroups(const std::set<std::set<int> >& supply_system_groups);    ///< specifies which sets systems can share resources.  any two sets should have no common systems.

   /** specifies which system has access to the resource stockpile.  Systems in a supply group with the system
       that can access the stockpile can store resources in and extract resources from the stockpile.  stockpiled
       resources are saved from turn to turn. */
    void        SetStockpileSystem(int stockpile_system_id);

    void        SetStockpile(double d);                 ///< sets current sockpiled amount of resource

    void        Update();                               ///< recalculates total resource production
    //@}

private:
    ResourcePool(); ///< default ctor needed for serialization

    std::vector<int>                    m_resource_center_ids;                      ///< UniverseObject id's of ResourceCenters in this pool
    std::set<std::set<int> >            m_supply_system_groups;                     ///< sets of systems that can share resource
    std::map<std::set<int>, double>     m_supply_system_groups_resource_production; ///< cached map from connected group of systems that can share resources, to how much resource is produced by ResourceCenters in the group.  regenerated during update from other state information.
    int                                 m_stockpile_system_id;                      ///< object id where stockpile for this pool is located
    double                              m_stockpile;                                ///< current stockpiled amount of resource
    ResourceType                        m_type;                                     ///< what kind of resource does this pool hold?

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The PopulationPool class keeps track of an empire's total population and its growth. */
class PopulationPool
{
public:
    /** \name Structors */ //@{
    PopulationPool();
    //@}

    /** \name Accessors */ //@{
    const std::vector<int>& PopCenterIDs() const {return m_pop_center_ids;}  ///< returns the PopCenter vector

    double                  Population() const;     ///< returns current total population
    double                  Growth() const;         ///< returns predicted growth for next turn    
    //@}

    /** \name Mutators */ //@{
    mutable boost::signal<void ()> ChangedSignal;   ///< emitted after updating population and growth numbers

    void    SetPopCenters(const std::vector<int>& pop_center_ids);

    void    Update();                               ///< recalculates total population and growth
    //@}

private:
    std::vector<int>    m_pop_center_ids;   ///< UniverseObject ids of PopCenters that contribute to the pool
    double              m_population;       ///< total population of all PopCenters in pool
    double              m_growth;           ///< total predicted population growth for next turn for all PopCenters in pool

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations

template <class Archive>
void ResourcePool::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_resource_center_ids)
        & BOOST_SERIALIZATION_NVP(m_stockpile)
        & BOOST_SERIALIZATION_NVP(m_stockpile_system_id)
        & BOOST_SERIALIZATION_NVP(m_supply_system_groups);
}

template <class Archive>
void PopulationPool::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_pop_center_ids);
}

#endif // _ResourcePool_h_
