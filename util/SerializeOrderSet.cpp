#include "Serialize.h"

#include "Order.h"
#include "OrderSet.h"

#include "Serialize.ipp"
#include <boost/serialization/version.hpp>



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
BOOST_CLASS_EXPORT(BombardOrder)
BOOST_CLASS_EXPORT(DeleteFleetOrder)
BOOST_CLASS_EXPORT(ChangeFocusOrder)
BOOST_CLASS_EXPORT(ResearchQueueOrder)
BOOST_CLASS_EXPORT(ProductionQueueOrder)
BOOST_CLASS_EXPORT(ShipDesignOrder)
BOOST_CLASS_EXPORT(ScrapOrder)
BOOST_CLASS_EXPORT(AggressiveOrder)
BOOST_CLASS_EXPORT(GiveObjectToEmpireOrder)
BOOST_CLASS_EXPORT(ForgetOrder)


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
        & BOOST_SERIALIZATION_NVP(m_fleet_names)
        & BOOST_SERIALIZATION_NVP(m_system_id)
        & BOOST_SERIALIZATION_NVP(m_fleet_ids)
        & BOOST_SERIALIZATION_NVP(m_ship_id_groups)
        & BOOST_SERIALIZATION_NVP(m_aggressives);
}

template <class Archive>
void FleetMoveOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet)
        & BOOST_SERIALIZATION_NVP(m_start_system)
        & BOOST_SERIALIZATION_NVP(m_dest_system)
        & BOOST_SERIALIZATION_NVP(m_route);
    if(version > 0) {
        ar & BOOST_SERIALIZATION_NVP(m_append);
    }else{
        m_append = false;
    }
}

BOOST_CLASS_VERSION(FleetMoveOrder, 1);

template <class Archive>
void FleetTransferOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_dest_fleet)
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
void BombardOrder::serialize(Archive& ar, const unsigned int version)
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
        & BOOST_SERIALIZATION_NVP(m_remove)
        & BOOST_SERIALIZATION_NVP(m_pause);
}

template <class Archive>
void ProductionQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_item)
        & BOOST_SERIALIZATION_NVP(m_number)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_index)
        & BOOST_SERIALIZATION_NVP(m_new_quantity)
        & BOOST_SERIALIZATION_NVP(m_new_blocksize)
        & BOOST_SERIALIZATION_NVP(m_new_index)
        & BOOST_SERIALIZATION_NVP(m_rally_point_id)
        & BOOST_SERIALIZATION_NVP(m_pause);
}

template <class Archive>
void ShipDesignOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order);
    ar  & BOOST_SERIALIZATION_NVP(m_design_id);
    ar  & BOOST_SERIALIZATION_NVP(m_delete_design_from_empire);
    ar  & BOOST_SERIALIZATION_NVP(m_create_new_design);
    ar  & BOOST_SERIALIZATION_NVP(m_move_design);
    ar  & BOOST_SERIALIZATION_NVP(m_update_name_or_description);
    ar  & BOOST_SERIALIZATION_NVP(m_name);
    ar  & BOOST_SERIALIZATION_NVP(m_description);
    ar  & BOOST_SERIALIZATION_NVP(m_designed_on_turn);
    ar  & BOOST_SERIALIZATION_NVP(m_hull);
    ar  & BOOST_SERIALIZATION_NVP(m_parts);
    ar  & BOOST_SERIALIZATION_NVP(m_is_monster);
    ar  & BOOST_SERIALIZATION_NVP(m_icon);
    ar  & BOOST_SERIALIZATION_NVP(m_3D_model);
    ar  & BOOST_SERIALIZATION_NVP(m_name_desc_in_stringtable);
    ar  & BOOST_SERIALIZATION_NVP(m_design_id_after);
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

template <class Archive>
void GiveObjectToEmpireOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id)
        & BOOST_SERIALIZATION_NVP(m_recipient_empire_id);
}

template <class Archive>
void ForgetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template <class Archive>
void Serialize(Archive& oa, const OrderSet& order_set)
{ oa << BOOST_SERIALIZATION_NVP(order_set); }
template void Serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& oa, const OrderSet& order_set);
template void Serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& oa, const OrderSet& order_set);

template <class Archive>
void Deserialize(Archive& ia, OrderSet& order_set)
{ ia >> BOOST_SERIALIZATION_NVP(order_set); }
template void Deserialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ia, OrderSet& order_set);
template void Deserialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ia, OrderSet& order_set);
