#include "ModeratorAction.h"

#include "AppInterface.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"

#include <boost/lexical_cast.hpp>
#include <boost/serialization/export.hpp>


Moderator::ModeratorAction::ModeratorAction()
{ Logger().debugStream() << "ModeratorAction::ModeratorAction()"; }

/////////////////////////////////////////////////////
// Moderator::DestroyUniverseObject
/////////////////////////////////////////////////////
Moderator::DestroyUniverseObject::DestroyUniverseObject()
{ Logger().debugStream() << "DestroyUniverseObject::DestroyUniverseObject()"; }

Moderator::DestroyUniverseObject::DestroyUniverseObject(int object_id) :
    m_object_id(object_id)
{ Logger().debugStream() << "DestroyUniverseObject::DestroyUniverseObject(" << m_object_id << ")"; }

void Moderator::DestroyUniverseObject::Execute() const
{ GetUniverse().RecursiveDestroy(m_object_id); }

std::string Moderator::DestroyUniverseObject::Dump() const {
    std::string retval = "Moderator::DestroyUniverseObject object_id = "
                       + boost::lexical_cast<std::string>(m_object_id);
    return retval;
}

/////////////////////////////////////////////////////
// Moderator::SetOwner
/////////////////////////////////////////////////////
Moderator::SetOwner::SetOwner()
{ Logger().debugStream() << "SetOwner::SetOwner()"; }

Moderator::SetOwner::SetOwner(int object_id, int new_owner_empire_id) :
    m_object_id(object_id),
    m_new_owner_empire_id(new_owner_empire_id)
{ Logger().debugStream() << "SetOwner::SetOwner(" << m_object_id << ", " << m_new_owner_empire_id << ")"; }

void Moderator::SetOwner::Execute() const {
    UniverseObject* obj = GetUniverseObject(m_object_id);
    if (!obj) {
        Logger().errorStream() << "Moderator::SetOwner::Execute couldn't get object with id: " << m_object_id;
        return;
    }
    obj->SetOwner(m_new_owner_empire_id);
}

std::string Moderator::SetOwner::Dump() const {
    std::string retval = "Moderator::SetOwner object_id = "
                       + boost::lexical_cast<std::string>(m_object_id)
                       + " new_owner_empire_id = "
                       + boost::lexical_cast<std::string>(m_new_owner_empire_id);
    return retval;
}
