#include "System.h"
#include "../GG/XML/XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdexcept>

System::System() : 
   UniverseObject(),
   m_star(INVALID_STARTYPE),
   m_orbits(0)
{
}

System::System(StarType star, int orbits, const std::string& name, double x, double y, 
               const std::set<int>& owners/* = std::set<int>()*/) : 
   UniverseObject(name, x, y, owners),
   m_star(star),
   m_orbits(orbits)
{
   if (m_star < INVALID_STARTYPE || NUM_STARTYPES < m_star)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with an invalid star type.");
   if (m_orbits < 0)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
}
   
System::System(StarType star, int orbits, const StarlaneMap& lanes_and_holes, 
               const std::string& name, double x, double y, const std::set<int>& owners/* = std::set<int>()*/) : 
   UniverseObject(name, x, y, owners),
   m_star(star),
   m_orbits(orbits),
   m_starlanes_wormholes(lanes_and_holes)
{
   if (m_star < INVALID_STARTYPE || NUM_STARTYPES < m_star)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with an invalid star type.");
   if (m_orbits < 0)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
}
   
System::System(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject"))
{
   if (elem.Tag() != "System")
      throw std::invalid_argument("Attempted to construct a System from an XMLElement that had a tag other than \"System\"");
   // TODO
}

System::~System()
{
   for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
      delete it->second;
   }
}
   
bool System::HasStarlaneTo(int id) const
{
   const_lane_iterator it = m_starlanes_wormholes.find(id);
   return (it == m_starlanes_wormholes.end() ? false : it->second == false);
}

bool System::HasWormholeTo(int id) const
{
   const_lane_iterator it = m_starlanes_wormholes.find(id);
   return (it == m_starlanes_wormholes.end() ? false : it->second == true);
}

GG::XMLElement System::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("System");

   element.AppendChild( UniverseObject::XMLEncode() );

   XMLElement star("m_star");
   star.SetAttribute( "value", lexical_cast<std::string>(m_star) );
   element.AppendChild(star);

   XMLElement orbits("m_orbits");
   orbits.SetAttribute( "value", lexical_cast<std::string>(m_orbits) );
   element.AppendChild(orbits);

   XMLElement orbit_map("m_objects");
   for(const_orbit_iterator itr= begin(); itr != end(); itr++)
   {
      XMLElement map_object("map_object");

      XMLElement orbit("orbit");
      orbit.SetAttribute( "value", lexical_cast<std::string>((*itr).first) );
      map_object.AppendChild(orbit);

      XMLElement orbit_obj_id("orbit_obj_id");
      orbit_obj_id.SetAttribute( "value", lexical_cast<std::string>((*itr).second->ID()) );
      map_object.AppendChild(orbit_obj_id);

      orbit_map.AppendChild(map_object);
   }
   element.AppendChild(orbit_map);

   XMLElement lane_map("m_starlanes_wormholes");
   for(const_lane_iterator itr= begin_lanes(); itr != end_lanes(); itr++)
   {
      XMLElement map_lane("map_lane");

      XMLElement system("system");
      system.SetAttribute( "value", lexical_cast<std::string>((*itr).first) );
      map_lane.AppendChild(system);

      XMLElement wormhole("wormhole");
      wormhole.SetAttribute( "value", lexical_cast<std::string>((*itr).second) );
      map_lane.AppendChild(wormhole);

      lane_map.AppendChild(map_lane);
   }
   element.AppendChild(lane_map);

   return element;
}

int System::Insert(UniverseObject* obj)
{
	int retval = -1;
    for (int orb = 0; orb < m_orbits; orb++)
    {
       if (m_objects.find(orb) == end())
       {
          m_objects.insert(std::pair<int, UniverseObject*>(orb, obj));
          obj->SetSystem(this);
          return orb;
       }
    }
    printf("No available orbits\n");
	return retval;
}

int System::Insert(UniverseObject* obj, int orbit)
{
    if (orbit < 0 || m_orbits < orbit)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
    m_objects.insert(std::pair<int, UniverseObject*>(orbit, obj));
    obj->SetSystem(this);
	return orbit;
}

UniverseObject* System::Remove(int id)
{
   UniverseObject* retval = 0;
   for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
      if (it->second->ID() == id) {
         retval = it->second;
         retval->SetSystem(0);
         m_objects.erase(it);
      }
   }
   return retval;
}

bool System::Delete(int id)
{
   UniverseObject* obj = Remove(id);
   delete obj;
   return obj;
}

UniverseObject* System::Object(int id)
{
   UniverseObject* retval = 0;
   for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
      if (it->second->ID() == id) {
         retval = it->second;
      }
   }
   return retval;
}

void System::AddStarlane(int id)
{
   m_starlanes_wormholes[id] = false;
}

void System::AddWormhole(int id)
{
   m_starlanes_wormholes[id] = true;
}

bool System::RemoveStarlane(int id)
{
   bool retval = false;
   if (retval = HasStarlaneTo(id)) {
      m_starlanes_wormholes.erase(id);
   }
   return retval;
}

bool System::RemoveWormhole(int id)
{
   bool retval = false;
   if (retval = HasWormholeTo(id)) {
      m_starlanes_wormholes.erase(id);
   }
   return retval;
}

void System::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
}

void System::PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps)
{
}

void System::XMLMerge(const GG::XMLElement& elem)
{
}

