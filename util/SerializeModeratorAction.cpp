#include "Serialize.h"

#include "../util/ModeratorAction.h"

#include "Serialize.ipp"


// exports for boost serialization of polymorphic ModeratorAction hierarchy
BOOST_CLASS_EXPORT(Moderator::DestroyUniverseObject)


template <class Archive>
void Moderator::ModeratorAction::serialize(Archive& ar, const unsigned int version)
{}

template <class Archive>
void Moderator::DestroyUniverseObject::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}
