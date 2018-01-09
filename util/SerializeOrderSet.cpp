#include "Serialize.h"

#include "Order.h"
#include "OrderSet.h"

#include "Serialize.ipp"
#include <boost/serialization/version.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>


BOOST_CLASS_EXPORT(Order)
BOOST_CLASS_VERSION(Order, 1)
BOOST_CLASS_EXPORT(RenameOrder)
BOOST_CLASS_EXPORT(NewFleetOrder)
BOOST_CLASS_VERSION(NewFleetOrder, 1)
BOOST_CLASS_EXPORT(FleetMoveOrder)
BOOST_CLASS_EXPORT(FleetTransferOrder)
BOOST_CLASS_EXPORT(ColonizeOrder)
BOOST_CLASS_EXPORT(InvadeOrder)
BOOST_CLASS_EXPORT(BombardOrder)
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
    ar  & BOOST_SERIALIZATION_NVP(m_empire);
    // m_executed is intentionally not serialized so that orders always
    // deserialize with m_execute = false.  See class comment for OrderSet.
    if (Archive::is_loading::value && version < 1) {
        bool dummy_executed;
        ar  & boost::serialization::make_nvp("m_executed", dummy_executed);
    }
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
        & BOOST_SERIALIZATION_NVP(m_fleet_id)
        & BOOST_SERIALIZATION_NVP(m_ship_ids)
        & BOOST_SERIALIZATION_NVP(m_aggressive);
}

template <class Archive>
void FleetMoveOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet)
        & BOOST_SERIALIZATION_NVP(m_dest_system)
        & BOOST_SERIALIZATION_NVP(m_route);
    if (version > 0) {
        ar & BOOST_SERIALIZATION_NVP(m_append);
    } else {
        m_append = false;
    }
}

BOOST_CLASS_VERSION(FleetMoveOrder, 2);

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
        & BOOST_SERIALIZATION_NVP(m_pause)
        & BOOST_SERIALIZATION_NVP(m_split_incomplete)
        & BOOST_SERIALIZATION_NVP(m_dupe)
        & BOOST_SERIALIZATION_NVP(m_use_imperial_pp);
}

template <class Archive>
void ShipDesignOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order);
    ar  & BOOST_SERIALIZATION_NVP(m_design_id);

    if (version >= 1) {
        // Serialization of m_uuid as a primitive doesn't work as expected from
        // the documentation.  This workaround instead serializes a string
        // representation.
        if (Archive::is_saving::value) {
            auto string_uuid = boost::uuids::to_string(m_uuid);
            ar & BOOST_SERIALIZATION_NVP(string_uuid);
        } else {
            std::string string_uuid;
            ar & BOOST_SERIALIZATION_NVP(string_uuid);
            try {
                m_uuid = boost::lexical_cast<boost::uuids::uuid>(string_uuid);
            } catch (const boost::bad_lexical_cast&) {
                m_uuid = boost::uuids::nil_generator()();
            }
        }
    } else if (Archive::is_loading::value) {
        m_uuid = boost::uuids::nil_generator()();
    }

    ar  & BOOST_SERIALIZATION_NVP(m_delete_design_from_empire);
    ar  & BOOST_SERIALIZATION_NVP(m_create_new_design);
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
}

BOOST_CLASS_VERSION(ShipDesignOrder, 1)

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
