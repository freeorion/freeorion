#include "Serialize.h"

#include "../Empire/InfluenceQueue.h"
#include "../Empire/ProductionQueue.h"
#include "../Empire/ResearchQueue.h"

#include "Serialize.ipp"
#include <boost/lexical_cast.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/random_generator.hpp>


template <typename Archive>
void ResearchQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(empire_id)
        & BOOST_SERIALIZATION_NVP(allocated_rp)
        & BOOST_SERIALIZATION_NVP(turns_left)
        & BOOST_SERIALIZATION_NVP(paused);
}

template void ResearchQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void ResearchQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_total_RPs_spent)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void ResearchQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void ProductionQueue::ProductionItem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(build_type)
        & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(design_id);
}

template void ProductionQueue::ProductionItem::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void ProductionQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(item)
        & BOOST_SERIALIZATION_NVP(empire_id)
        & BOOST_SERIALIZATION_NVP(ordered)
        & BOOST_SERIALIZATION_NVP(remaining)
        & BOOST_SERIALIZATION_NVP(blocksize)
        & BOOST_SERIALIZATION_NVP(location)
        & BOOST_SERIALIZATION_NVP(allocated_pp)
        & BOOST_SERIALIZATION_NVP(progress)
        & BOOST_SERIALIZATION_NVP(progress_memory)
        & BOOST_SERIALIZATION_NVP(blocksize_memory)
        & BOOST_SERIALIZATION_NVP(turns_left_to_next_item)
        & BOOST_SERIALIZATION_NVP(turns_left_to_completion)
        & BOOST_SERIALIZATION_NVP(rally_point_id)
        & BOOST_SERIALIZATION_NVP(paused)
        & BOOST_SERIALIZATION_NVP(allowed_imperial_stockpile_use);

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

BOOST_CLASS_VERSION(ProductionQueue, 1)

namespace {
    auto Convert(const std::map<std::set<int>, float>& in) {
        std::map<boost::container::flat_set<int>, float> retval;
        for (const auto& v : in)
            retval.emplace(std::piecewise_construct,
                           std::forward_as_tuple(v.first.begin(), v.first.end()),
                           std::forward_as_tuple(v.second));
        return retval;
    }
}

template <typename Archive>
void ProductionQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress);
    if (version < 1) {
        std::map<std::set<int>, float> temp;
        ar  & boost::serialization::make_nvp("m_object_group_allocated_pp", temp);
        m_object_group_allocated_pp = Convert(temp);
        temp.clear();
        ar  & boost::serialization::make_nvp("m_object_group_allocated_stockpile_pp", temp);
        m_object_group_allocated_stockpile_pp = Convert(temp);

    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_object_group_allocated_pp)
            & BOOST_SERIALIZATION_NVP(m_object_group_allocated_stockpile_pp);
    }
    ar  & BOOST_SERIALIZATION_NVP(m_expected_new_stockpile_amount)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void ProductionQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void InfluenceQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(empire_id)
        & BOOST_SERIALIZATION_NVP(allocated_ip)
        & BOOST_SERIALIZATION_NVP(paused);
}

template void InfluenceQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
void InfluenceQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_total_IPs_spent)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void InfluenceQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);
