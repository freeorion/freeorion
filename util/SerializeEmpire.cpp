#include "Serialize.h"

#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "Serialize.ipp"


template <class Archive>
void ResearchQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    std::string tech_name;
    if (Archive::is_saving::value) {
        assert(tech);
        tech_name = tech->Name();
    }
    ar  & BOOST_SERIALIZATION_NVP(tech_name)
        & BOOST_SERIALIZATION_NVP(allocated_rp)
        & BOOST_SERIALIZATION_NVP(turns_left);
    if (Archive::is_loading::value) {
        assert(tech_name != "");
        tech = GetTech(tech_name);
    }
}

template void ResearchQueue::Element::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void ResearchQueue::Element::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void ResearchQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_total_RPs_spent);
}

template void ResearchQueue::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void ResearchQueue::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void ProductionQueue::ProductionItem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(build_type)
        & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(design_id);
}

template void ProductionQueue::ProductionItem::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void ProductionQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(item)
        & BOOST_SERIALIZATION_NVP(ordered)
        & BOOST_SERIALIZATION_NVP(remaining)
        & BOOST_SERIALIZATION_NVP(location)
        & BOOST_SERIALIZATION_NVP(allocated_pp)
        & BOOST_SERIALIZATION_NVP(turns_left_to_next_item)
        & BOOST_SERIALIZATION_NVP(turns_left_to_completion);
}

template void ProductionQueue::Element::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void ProductionQueue::Element::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void ProductionQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_system_group_allocated_pp);
}

template void ProductionQueue::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void ProductionQueue::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void Empire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_color);

    if (Universe::ALL_OBJECTS_VISIBLE ||
        Universe::s_encoding_empire == ALL_EMPIRES ||
        m_id == Universe::s_encoding_empire)
    {
        ar  & BOOST_SERIALIZATION_NVP(m_capitol_id)
            & BOOST_SERIALIZATION_NVP(m_techs)
            & BOOST_SERIALIZATION_NVP(m_meters)
            & BOOST_SERIALIZATION_NVP(m_research_queue)
            & BOOST_SERIALIZATION_NVP(m_research_progress)
            & BOOST_SERIALIZATION_NVP(m_production_queue)
            & BOOST_SERIALIZATION_NVP(m_production_progress)
            & BOOST_SERIALIZATION_NVP(m_available_building_types)
            & BOOST_SERIALIZATION_NVP(m_available_part_types)
            & BOOST_SERIALIZATION_NVP(m_available_hull_types)
            & BOOST_SERIALIZATION_NVP(m_explored_systems)

            & BOOST_SERIALIZATION_NVP(m_fleet_supplyable_system_ids)
            & BOOST_SERIALIZATION_NVP(m_fleet_supply_starlane_traversals)
            & BOOST_SERIALIZATION_NVP(m_fleet_supply_system_ranges)
            & BOOST_SERIALIZATION_NVP(m_resource_supply_groups)
            & BOOST_SERIALIZATION_NVP(m_resource_supply_starlane_traversals)
            & BOOST_SERIALIZATION_NVP(m_resource_supply_obstructed_starlane_traversals)
            & BOOST_SERIALIZATION_NVP(m_resource_supply_system_ranges)
            & BOOST_SERIALIZATION_NVP(m_supply_unobstructed_systems)

            & BOOST_SERIALIZATION_NVP(m_ship_designs)
            & BOOST_SERIALIZATION_NVP(m_sitrep_entries)
            & BOOST_SERIALIZATION_NVP(m_resource_pools)
            & BOOST_SERIALIZATION_NVP(m_population_pool)
            & BOOST_SERIALIZATION_NVP(m_maintenance_total_cost)
            & BOOST_SERIALIZATION_NVP(m_ship_names_used);
    }
}

template void Empire::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void Empire::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void EmpireManager::serialize(Archive& ar, const unsigned int version)
{
    if (Archive::is_loading::value) {
        Clear();    // clean up any existing dynamically allocated contents before replacing containers with deserialized data
    }

    ar  & BOOST_SERIALIZATION_NVP(m_empire_map)
        & BOOST_SERIALIZATION_NVP(m_eliminated_empires);
}

template void EmpireManager::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void EmpireManager::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

#if 0
void Serialize(FREEORION_OARCHIVE_TYPE& oa, const Empire& empire)
{ oa << BOOST_SERIALIZATION_NVP(empire); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const EmpireManager& empire_manager)
{ oa << BOOST_SERIALIZATION_NVP(empire_manager); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, Empire& empire)
{ ia >> BOOST_SERIALIZATION_NVP(empire); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, EmpireManager& empire_manager)
{ ia >> BOOST_SERIALIZATION_NVP(empire_manager); }
#endif
