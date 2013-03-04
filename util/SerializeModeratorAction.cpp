#include "Serialize.h"

#include "../util/ModeratorAction.h"

#include "Serialize.ipp"


// exports for boost serialization of polymorphic ModeratorAction hierarchy
BOOST_CLASS_EXPORT(Moderator::DestroyUniverseObject)
BOOST_CLASS_EXPORT(Moderator::SetOwner)


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
