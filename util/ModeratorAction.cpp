#include "ModeratorAction.h"

#include "AppInterface.h"
#include "../universe/Universe.h"

#include <boost/lexical_cast.hpp>

/////////////////////////////////////////////////////
// Moderator::DestroyUniverseObject
/////////////////////////////////////////////////////

void Moderator::DestroyUniverseObject::Execute() const
{ GetUniverse().RecursiveDestroy(m_object_id); }

std::string Moderator::DestroyUniverseObject::Dump() const {
    std::string retval = "Moderator::DestroyUniverseObject object_id = "
                       + boost::lexical_cast<std::string>(m_object_id);
    return retval;
}

