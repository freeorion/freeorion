#include "Serialize.h"

#include "../Empire/InfluenceQueue.h"
#include "../Empire/ProductionQueue.h"
#include "../Empire/ResearchQueue.h"
#include "../util/Logger.h"

#include "Serialize.ipp"
#include <boost/lexical_cast.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/random_generator.hpp>

template <typename Archive>
void ResearchQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(name);
    ar  & BOOST_SERIALIZATION_NVP(empire_id);
    ar  & BOOST_SERIALIZATION_NVP(allocated_rp);
    ar  & BOOST_SERIALIZATION_NVP(turns_left);
    ar  & BOOST_SERIALIZATION_NVP(paused);
}

template void ResearchQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void ResearchQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue);
    ar  & BOOST_SERIALIZATION_NVP(m_projects_in_progress);
    ar  & BOOST_SERIALIZATION_NVP(m_total_RPs_spent);
    ar  & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void ResearchQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void ProductionQueue::ProductionItem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(build_type);
    ar  & BOOST_SERIALIZATION_NVP(name);
    ar  & BOOST_SERIALIZATION_NVP(design_id);
}

template void ProductionQueue::ProductionItem::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void ProductionQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(item);
    ar  & BOOST_SERIALIZATION_NVP(empire_id);
    ar  & BOOST_SERIALIZATION_NVP(ordered);
    ar  & BOOST_SERIALIZATION_NVP(remaining);
    ar  & BOOST_SERIALIZATION_NVP(blocksize);
    ar  & BOOST_SERIALIZATION_NVP(location);
    ar  & BOOST_SERIALIZATION_NVP(allocated_pp);
    ar  & BOOST_SERIALIZATION_NVP(progress);
    ar  & BOOST_SERIALIZATION_NVP(progress_memory);
    ar  & BOOST_SERIALIZATION_NVP(blocksize_memory);
    ar  & BOOST_SERIALIZATION_NVP(turns_left_to_next_item);
    ar  & BOOST_SERIALIZATION_NVP(turns_left_to_completion);
    ar  & BOOST_SERIALIZATION_NVP(rally_point_id);
    ar  & BOOST_SERIALIZATION_NVP(paused);
    ar  & BOOST_SERIALIZATION_NVP(allowed_imperial_stockpile_use);

    if (Archive::is_loading::value && version < 3) {
        to_be_removed = false;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(to_be_removed);
    }

    if constexpr (Archive::is_saving::value) {
        // Serialization of uuid as a primitive doesn't work as expected from
        // the documentation.  This workaround instead serializes a string
        // representation.
        auto string_uuid = boost::uuids::to_string(uuid);
        ar & BOOST_SERIALIZATION_NVP(string_uuid);

    } else if (Archive::is_loading::value && version < 2) {
        // assign a random ID to this element so that future-issued orders can refer to it
        uuid = boost::uuids::random_generator()();

    } else {
        // convert string back into UUID
        std::string string_uuid;
        ar & BOOST_SERIALIZATION_NVP(string_uuid);

        try {
            uuid = boost::lexical_cast<boost::uuids::uuid>(string_uuid);
        } catch (const boost::bad_lexical_cast&) {
            uuid = boost::uuids::random_generator()();
        }
    }
}

BOOST_CLASS_VERSION(ProductionQueue::Element, 3)

template void ProductionQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);



namespace {
    constexpr auto to_uid = [](const auto& i) noexcept { return UniverseObjectID{i}; };
    constexpr auto to_int_value = [](const auto& i) noexcept -> int { return Value(i); };

    using id_flat_set = boost::container::flat_set<UniverseObjectID>;
    using id_flat_set_float_map = std::map<id_flat_set, float>;

    id_flat_set ToUniverseObjectIDFlatSet(const auto& in)
    { return in | range_transform(to_uid) | range_to<id_flat_set>(); }

    id_flat_set_float_map ToIDFlatSetFloatMap(const auto& in) {
        id_flat_set_float_map retval;
        for (const auto& [keys, vals] : in) {
            retval.emplace(std::piecewise_construct,
                           std::forward_as_tuple(ToUniverseObjectIDFlatSet(keys)),
                           std::forward_as_tuple(vals));
        }
        return retval;
    }
}

static_assert(boost::serialization::version<ProductionQueue>::value > 0);

template <typename Archive>
void ProductionQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue);
    ar  & BOOST_SERIALIZATION_NVP(m_projects_in_progress);

    using boost::serialization::make_nvp;

    if (Archive::is_loading::value && version < 1) {
        std::map<std::set<int>, float> temp1;
        ar &  make_nvp("m_object_group_allocated_pp", temp1);
        m_object_group_allocated_pp = ToIDFlatSetFloatMap(temp1);
        std::map<std::set<int>, float> temp2;
        ar &  make_nvp("m_object_group_allocated_stockpile_pp", temp2);
        m_object_group_allocated_stockpile_pp = ToIDFlatSetFloatMap(temp2);

    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_object_group_allocated_pp);
        ar  & BOOST_SERIALIZATION_NVP(m_object_group_allocated_stockpile_pp);
    }

    ar  & BOOST_SERIALIZATION_NVP(m_expected_new_stockpile_amount);
    ar  & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void ProductionQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void InfluenceQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(name);
    ar  & BOOST_SERIALIZATION_NVP(empire_id);
    ar  & BOOST_SERIALIZATION_NVP(allocated_ip);
    ar  & BOOST_SERIALIZATION_NVP(paused);
}

template void InfluenceQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void InfluenceQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue);
    ar  & BOOST_SERIALIZATION_NVP(m_projects_in_progress);
    ar  & BOOST_SERIALIZATION_NVP(m_total_IPs_spent);
    ar  & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void InfluenceQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);
