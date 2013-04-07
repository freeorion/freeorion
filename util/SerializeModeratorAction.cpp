#include "Serialize.h"

#include "../util/ModeratorAction.h"

#include "Serialize.ipp"


// exports for boost serialization of polymorphic ModeratorAction hierarchy
BOOST_CLASS_EXPORT(Moderator::DestroyUniverseObject)
BOOST_CLASS_EXPORT(Moderator::SetOwner)
BOOST_CLASS_EXPORT(Moderator::CreateSystem)
BOOST_CLASS_EXPORT(Moderator::CreatePlanet)


template <class Archive>
void Moderator::ModeratorAction::serialize(Archive& ar, const unsigned int version)
{}

template void Moderator::ModeratorAction::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void Moderator::ModeratorAction::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void Moderator::DestroyUniverseObject::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template void Moderator::DestroyUniverseObject::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void Moderator::DestroyUniverseObject::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void Moderator::SetOwner::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_object_id)
        & BOOST_SERIALIZATION_NVP(m_new_owner_empire_id);
}

template void Moderator::SetOwner::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void Moderator::SetOwner::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void Moderator::AddStarlane::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_id_1)
        & BOOST_SERIALIZATION_NVP(m_id_2);
}

template void Moderator::CreatePlanet::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void Moderator::CreatePlanet::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);


template <class Archive>
void Moderator::CreateSystem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_star_type);
}

template void Moderator::CreateSystem::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void Moderator::CreateSystem::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void Moderator::CreatePlanet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_system_id)
        & BOOST_SERIALIZATION_NVP(m_planet_type)
        & BOOST_SERIALIZATION_NVP(m_planet_size);
}

template void Moderator::CreatePlanet::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void Moderator::CreatePlanet::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);
