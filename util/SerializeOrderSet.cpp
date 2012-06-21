#include "Serialize.h"

#include "../util/OrderSet.h"

#include "../combat/CombatOrder.h"
#include "../combat/OpenSteer/CombatObject.h"

#include "Serialize.ipp"


////////////////////////////////////////////////////////////
// Galaxy Map orders
////////////////////////////////////////////////////////////

// exports for boost serialization of polymorphic Order hierarchy
BOOST_CLASS_EXPORT(RenameOrder)
BOOST_CLASS_EXPORT(NewFleetOrder)
BOOST_CLASS_EXPORT(FleetMoveOrder)
BOOST_CLASS_EXPORT(FleetTransferOrder)
BOOST_CLASS_EXPORT(ColonizeOrder)
BOOST_CLASS_EXPORT(InvadeOrder)
BOOST_CLASS_EXPORT(DeleteFleetOrder)
BOOST_CLASS_EXPORT(ChangeFocusOrder)
BOOST_CLASS_EXPORT(ResearchQueueOrder)
BOOST_CLASS_EXPORT(ProductionQueueOrder)
BOOST_CLASS_EXPORT(ShipDesignOrder)
BOOST_CLASS_EXPORT(ScrapOrder)
BOOST_CLASS_EXPORT(AggressiveOrder)

template <class Archive>
void Order::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_empire)
        & BOOST_SERIALIZATION_NVP(m_executed);
}

template <class Archive>
void RenameOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void NewFleetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet_name)
        & BOOST_SERIALIZATION_NVP(m_system_id)
        & BOOST_SERIALIZATION_NVP(m_new_id)
        & BOOST_SERIALIZATION_NVP(m_ship_ids);
}

template <class Archive>
void FleetMoveOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet)
        & BOOST_SERIALIZATION_NVP(m_start_system)
        & BOOST_SERIALIZATION_NVP(m_dest_system)
        & BOOST_SERIALIZATION_NVP(m_route);
}

template <class Archive>
void FleetTransferOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet_from)
        & BOOST_SERIALIZATION_NVP(m_fleet_to)
        & BOOST_SERIALIZATION_NVP(m_add_ships);
}

template <class Archive>
void ColonizeOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship)
        & BOOST_SERIALIZATION_NVP(m_planet);
}

template <class Archive>
void InvadeOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship)
        & BOOST_SERIALIZATION_NVP(m_planet);
}

template <class Archive>
void DeleteFleetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet);
}

template <class Archive>
void ChangeFocusOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_planet)
        & BOOST_SERIALIZATION_NVP(m_focus);
}

template <class Archive>
void ResearchQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_position)
        & BOOST_SERIALIZATION_NVP(m_remove);
}

template <class Archive>
void ProductionQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_build_type)
        & BOOST_SERIALIZATION_NVP(m_item_name)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_number)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_index)
        & BOOST_SERIALIZATION_NVP(m_new_quantity)
        & BOOST_SERIALIZATION_NVP(m_new_index);
}

template <class Archive>
void ShipDesignOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_delete_design_from_empire)
        & BOOST_SERIALIZATION_NVP(m_create_new_design)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_designed_by_empire_id)
        & BOOST_SERIALIZATION_NVP(m_designed_on_turn)
        & BOOST_SERIALIZATION_NVP(m_hull)
        & BOOST_SERIALIZATION_NVP(m_parts)
        & BOOST_SERIALIZATION_NVP(m_is_monster)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_3D_model)
        & BOOST_SERIALIZATION_NVP(m_name_desc_in_stringtable);
}

template <class Archive>
void ScrapOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template <class Archive>
void AggressiveOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id)
        & BOOST_SERIALIZATION_NVP(m_aggression);
}

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const OrderSet& order_set)
{ oa << BOOST_SERIALIZATION_NVP(order_set); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, OrderSet& order_set)
{ ia >> BOOST_SERIALIZATION_NVP(order_set); }


////////////////////////////////////////////////////////////
// Combat orders
////////////////////////////////////////////////////////////

template <class Archive>
void ShipMission::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_destination)
        & BOOST_SERIALIZATION_NVP(m_target);
}

template void ShipMission::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void ShipMission::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void FighterMission::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_destination)
        & BOOST_SERIALIZATION_NVP(m_target);
}

template void FighterMission::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void FighterMission::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void CombatOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_order_type)
        & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_append);
    switch (m_order_type) {
    case SHIP_ORDER:
        ar & BOOST_SERIALIZATION_NVP(m_ship_mission); break;
    case FIGHTER_ORDER:
        ar & BOOST_SERIALIZATION_NVP(m_fighter_mission); break;
    case SETUP_PLACEMENT_ORDER:
        ar & BOOST_SERIALIZATION_NVP(m_position_and_direction); break;
    }
}

template void CombatOrder::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void CombatOrder::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);
