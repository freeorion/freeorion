#include "ServerUniverse.h"
#include "UniverseObject.h"
#include "../GG/XML/XMLDoc.h"

ServerUniverse::ServerUniverse()
{
   // TODO
}

ServerUniverse::ServerUniverse(Shape shape, int stars, int players)
{
   // TODO
}

ServerUniverse::ServerUniverse(const std::string& map_file, int stars, int players)
{
   // TODO
}

ServerUniverse::ServerUniverse(const GG::XMLElement& elem) : 
   ClientUniverse(elem)
{
}

ServerUniverse::~ServerUniverse()
{
   // TODO
}

int ServerUniverse::Insert(UniverseObject* obj)
{
   int retval = -1;
   // TODO
   return retval;
}

UniverseObject* ServerUniverse::Remove(int id)
{
   UniverseObject* retval = 0;
   iterator it = m_objects.find(id);
   if (it != m_objects.end()) {
      retval = it->second;
      m_objects.erase(id);
   }
   return retval;
}
   
bool ServerUniverse::Delete(int id)
{
   UniverseObject* obj = Remove(id);
   delete obj;
   return obj;
}
   
UniverseObject* ServerUniverse::Object(int id)
{
   iterator it = m_objects.find(id);
   return (it != m_objects.end() ? it->second : 0);
}

void ServerUniverse::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}
   
void ServerUniverse::PopGrowthProductionResearch(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

