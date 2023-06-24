#include "Serialize.h"

#include "../universe/Fleet.h"
#include "Order.h"
#include "OrderSet.h"

#include "Serialize.ipp"
#include <boost/serialization/version.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>
#include "Logger.h"


BOOST_CLASS_EXPORT(Order)
BOOST_CLASS_VERSION(Order, 1)
BOOST_CLASS_EXPORT(RenameOrder)
BOOST_CLASS_EXPORT(NewFleetOrder)
BOOST_CLASS_VERSION(NewFleetOrder, 2)
BOOST_CLASS_EXPORT(FleetMoveOrder)
BOOST_CLASS_VERSION(FleetMoveOrder, 2)
BOOST_CLASS_EXPORT(FleetTransferOrder)
BOOST_CLASS_EXPORT(AnnexOrder)
BOOST_CLASS_EXPORT(ColonizeOrder)
BOOST_CLASS_EXPORT(InvadeOrder)
BOOST_CLASS_EXPORT(BombardOrder)
BOOST_CLASS_EXPORT(ChangeFocusOrder)
BOOST_CLASS_EXPORT(PolicyOrder)
BOOST_CLASS_VERSION(PolicyOrder, 2)
BOOST_CLASS_EXPORT(ResearchQueueOrder)
BOOST_CLASS_EXPORT(ProductionQueueOrder)
BOOST_CLASS_VERSION(ProductionQueueOrder, 2)
BOOST_CLASS_EXPORT(ShipDesignOrder)
BOOST_CLASS_VERSION(ShipDesignOrder, 1)
BOOST_CLASS_EXPORT(ScrapOrder)
BOOST_CLASS_EXPORT(AggressiveOrder)
BOOST_CLASS_VERSION(AggressiveOrder, 1)
BOOST_CLASS_EXPORT(GiveObjectToEmpireOrder)
BOOST_CLASS_EXPORT(ForgetOrder)


template <typename Archive>
void Order::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_empire);
    // m_executed is intentionally not serialized so that orders always
    // deserialize with m_execute = false.  See class comment for OrderSet.
    if constexpr (Archive::is_loading::value) {
        if (version < 1) {
            bool dummy_executed;
            ar  & boost::serialization::make_nvp("m_executed", dummy_executed);
        }
    }
}

template <typename Archive>
void RenameOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <typename Archive>
void NewFleetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet_name)
        & BOOST_SERIALIZATION_NVP(m_fleet_id)
        & BOOST_SERIALIZATION_NVP(m_ship_ids);
    if (version < 2) {
        bool aggressive = false;
        ar  & boost::serialization::make_nvp("m_aggressive", aggressive);
        m_aggression = aggressive ? FleetDefaults::FLEET_DEFAULT_ARMED : FleetDefaults::FLEET_DEFAULT_UNARMED;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_aggression);
    }
}

template <typename Archive>
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

template <typename Archive>
void FleetTransferOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_dest_fleet)
        & BOOST_SERIALIZATION_NVP(m_add_ships);
}

template <typename Archive>
void AnnexOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_planet);
}

template <typename Archive>
void ColonizeOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship)
        & BOOST_SERIALIZATION_NVP(m_planet);
}

template <typename Archive>
void InvadeOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship)
        & BOOST_SERIALIZATION_NVP(m_planet);
}

template <typename Archive>
void BombardOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship)
        & BOOST_SERIALIZATION_NVP(m_planet);
}

template <typename Archive>
void ChangeFocusOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_planet)
        & BOOST_SERIALIZATION_NVP(m_focus);
}

template <typename Archive>
void PolicyOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_policy_name)
        & BOOST_SERIALIZATION_NVP(m_category)
        & BOOST_SERIALIZATION_NVP(m_adopt)
        & BOOST_SERIALIZATION_NVP(m_slot);
    if (version >= 2) // otherwise, default false is fine
        ar  & BOOST_SERIALIZATION_NVP(m_revert);
}

template <typename Archive>
void ResearchQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_position)
        & BOOST_SERIALIZATION_NVP(m_remove)
        & BOOST_SERIALIZATION_NVP(m_pause);
}

template <typename Archive>
void ProductionQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_item);

    int m_number, m_index, m_pause, m_split_incomplete, m_dupe, m_use_imperial_pp;
    if (version < 2)
        ar  & BOOST_SERIALIZATION_NVP(m_number);

    ar  & BOOST_SERIALIZATION_NVP(m_location);

    if (version < 2)
        ar  & BOOST_SERIALIZATION_NVP(m_index);

    ar  & BOOST_SERIALIZATION_NVP(m_new_quantity)
        & BOOST_SERIALIZATION_NVP(m_new_blocksize)
        & BOOST_SERIALIZATION_NVP(m_new_index)
        & BOOST_SERIALIZATION_NVP(m_rally_point_id);

    if (version < 2) {
        ar  & BOOST_SERIALIZATION_NVP(m_pause)
            & BOOST_SERIALIZATION_NVP(m_split_incomplete)
            & BOOST_SERIALIZATION_NVP(m_dupe)
            & BOOST_SERIALIZATION_NVP(m_use_imperial_pp);
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_action);
    }

    if (version < 2 && Archive::is_loading::value) {
        // would need to generate action and UUID from deserialized values. instead generate an invalid order. will break partial-turn saves.
        m_uuid = boost::uuids::nil_generator()();
        m_uuid2 = boost::uuids::nil_generator()();
        m_action = ProdQueueOrderAction::INVALID_PROD_QUEUE_ACTION;

    } else {
        // Serialization of m_uuid as a primitive doesn't work as expected from
        // the documentation.  This workaround instead serializes a string
        // representation.
        if constexpr (Archive::is_saving::value) {
            auto string_uuid = boost::uuids::to_string(m_uuid);
            ar & BOOST_SERIALIZATION_NVP(string_uuid);
            auto string_uuid2 = boost::uuids::to_string(m_uuid2);
            ar & BOOST_SERIALIZATION_NVP(string_uuid2);

        } else {
            std::string string_uuid;
            ar & BOOST_SERIALIZATION_NVP(string_uuid);
            std::string string_uuid2;
            ar & BOOST_SERIALIZATION_NVP(string_uuid2);
            try {
                m_uuid = boost::lexical_cast<boost::uuids::uuid>(string_uuid);
                m_uuid2 = boost::lexical_cast<boost::uuids::uuid>(string_uuid2);
            } catch (const boost::bad_lexical_cast&) {
                ErrorLogger() << "Error casting to UUIDs from strings: " << string_uuid
                              << " and " << string_uuid2 << ".  ProductionOrder will be invalid";
                m_uuid = boost::uuids::nil_generator()();
                m_uuid2 = boost::uuids::nil_generator()();
                m_action = ProdQueueOrderAction::INVALID_PROD_QUEUE_ACTION;
            }
        }
    }
}

template <typename Archive>
void ShipDesignOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order);
    ar  & BOOST_SERIALIZATION_NVP(m_design_id);

    if (version >= 1) {
        // Serialization of m_uuid as a primitive doesn't work as expected from
        // the documentation.  This workaround instead serializes a string
        // representation.
        if constexpr (Archive::is_saving::value) {
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
    } else if constexpr (Archive::is_loading::value) {
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

template <typename Archive>
void ScrapOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template <typename Archive>
void AggressiveOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id);
    if (version < 1) {
        bool aggressive = false;
        ar  & boost::serialization::make_nvp("m_aggression", aggressive);
        m_aggression = aggressive ? FleetAggression::FLEET_AGGRESSIVE : FleetAggression::FLEET_DEFENSIVE;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_aggression);
    }
}

template <typename Archive>
void GiveObjectToEmpireOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id)
        & BOOST_SERIALIZATION_NVP(m_recipient_empire_id);
}

template <typename Archive>
void ForgetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template <typename Archive>
void Serialize(Archive& oa, const OrderSet& order_set)
{ oa << BOOST_SERIALIZATION_NVP(order_set); }
template void Serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& oa, const OrderSet& order_set);
template void Serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& oa, const OrderSet& order_set);

template <typename Archive>
void Deserialize(Archive& ia, OrderSet& order_set)
{ ia >> BOOST_SERIALIZATION_NVP(order_set); }
template void Deserialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ia, OrderSet& order_set);
template void Deserialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ia, OrderSet& order_set);
