#include "ModeratorAction.h"

#include "AppInterface.h"
#include "../universe/Universe.h"

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
