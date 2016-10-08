#include "Serialize.h"

#include "ModeratorAction.h"

#include "Serialize.ipp"


BOOST_CLASS_EXPORT(Moderator::DestroyUniverseObject)
BOOST_CLASS_EXPORT(Moderator::SetOwner)
BOOST_CLASS_EXPORT(Moderator::AddStarlane)
BOOST_CLASS_EXPORT(Moderator::RemoveStarlane)
BOOST_CLASS_EXPORT(Moderator::CreateSystem)
BOOST_CLASS_EXPORT(Moderator::CreatePlanet)


template <class Archive>
void Moderator::ModeratorAction::serialize(Archive& ar, const unsigned int version)
{}

template void Moderator::ModeratorAction::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Moderator::ModeratorAction::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Moderator::ModeratorAction::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Moderator::ModeratorAction::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Moderator::DestroyUniverseObject::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template void Moderator::DestroyUniverseObject::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Moderator::DestroyUniverseObject::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Moderator::DestroyUniverseObject::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Moderator::DestroyUniverseObject::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Moderator::SetOwner::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_object_id)
        & BOOST_SERIALIZATION_NVP(m_new_owner_empire_id);
}

template void Moderator::SetOwner::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Moderator::SetOwner::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Moderator::SetOwner::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Moderator::SetOwner::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Moderator::AddStarlane::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_id_1)
        & BOOST_SERIALIZATION_NVP(m_id_2);
}

template void Moderator::AddStarlane::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Moderator::AddStarlane::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Moderator::AddStarlane::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Moderator::AddStarlane::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Moderator::RemoveStarlane::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_id_1)
        & BOOST_SERIALIZATION_NVP(m_id_2);
}

template void Moderator::RemoveStarlane::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Moderator::RemoveStarlane::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Moderator::RemoveStarlane::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Moderator::RemoveStarlane::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Moderator::CreateSystem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_star_type);
}

template void Moderator::CreateSystem::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Moderator::CreateSystem::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Moderator::CreateSystem::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Moderator::CreateSystem::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Moderator::CreatePlanet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_system_id)
        & BOOST_SERIALIZATION_NVP(m_planet_type)
        & BOOST_SERIALIZATION_NVP(m_planet_size);
}

template void Moderator::CreatePlanet::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Moderator::CreatePlanet::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Moderator::CreatePlanet::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Moderator::CreatePlanet::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);
