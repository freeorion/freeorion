#include "ClientUniverse.h"
#include "UniverseObject.h"
#include "../GG/XML/XMLDoc.h"

#include <stdexcept>

ClientUniverse::ClientUniverse()
{
   // TODO
}
   
ClientUniverse::ClientUniverse(Shape shape, int stars, int players)
{
   // TODO
}

ClientUniverse::ClientUniverse(const std::string& map_file, int stars, int players)
{
   // TODO
}

ClientUniverse::ClientUniverse(const GG::XMLElement& elem)
{
   if (elem.Tag() != "ClientUniverse")
      throw std::invalid_argument("Attempted to construct a ClientUniverse from an XMLElement that had a tag other than \"ClientUniverse\"");
   // TODO
}

ClientUniverse::~ClientUniverse()
{
   for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
      delete it->second;
   }
}

const UniverseObject* ClientUniverse::ClientUniverse::Object(int id) const
{
   const_iterator it = m_objects.find(id);
   return (it != m_objects.end() ? it->second : 0);
}

GG::XMLElement ClientUniverse::XMLEncode() const
{
   GG::XMLElement retval;
   // TODO
   return retval;
}

void ClientUniverse::XMLMerge(const GG::XMLElement& elem)
{
   // TODO
}

