#include "System.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#ifdef FREEORION_BUILD_SERVER
#include "../server/ServerApp.h"
#else
#ifdef FREEORION_BUILD_HUMAN
#include "../client/human/HumanClientApp.h"
#else
#include "../client/AI/AIClientApp.h"
#endif
#endif

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
   using GG::XMLElement;

   if (elem.Tag() != "System")
      throw std::invalid_argument("Attempted to construct a System from an XMLElement that had a tag other than \"System\"");

   Visibility vis = (Visibility) lexical_cast<int> ( elem.Child("visibility").Attribute("value") );

   m_star = (StarType) lexical_cast<int> ( elem.Child("m_star").Attribute("value") );

   if (vis == FULL_VISIBILITY)
   {
      m_orbits = lexical_cast<int> ( elem.Child("m_orbits").Attribute("value") );
      
      XMLElement orbit_map = elem.Child("m_objects");
      for(int i=0; i<orbit_map.NumChildren(); i++)
      {
         XMLElement map_object = orbit_map.Child(i);

         int map_key = lexical_cast<int> (map_object.Child("orbit").Attribute("value") );
         int obj_id = lexical_cast<int> (map_object.Child("orbit_obj_id").Attribute("value") );
         
         Insert(obj_id, map_key);
      }

      XMLElement lane_map = elem.Child("m_starlanes_wormholes");
      for(int i=0; i<lane_map.NumChildren(); i++)
      {
         XMLElement map_lane = lane_map.Child(i);

         int system = lexical_cast<int> (map_lane.Child("system").Attribute("value") );
         bool wormhole = lexical_cast<bool> (map_lane.Child("wormhole").Attribute("value") );

         if (wormhole)
         {
            AddWormhole(system);
         } 
         else 
         {
            AddStarlane(system);
         }
      }
   }      
}

System::~System()
{
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

UniverseObject::Visibility System::Visible(int empire_id) const
{
   // if system is has been explored it is fully visible, if not it
   // will be partially visible
#ifdef FREEORION_BUILD_SERVER 
   ServerApp* server_app = ServerApp::GetApp();
   Empire* empire = (server_app->Empires()).Lookup(empire_id);
       
   if (empire->HasExploredSystem(ID()))
   {
      return FULL_VISIBILITY;
   }
#endif
   return PARTIAL_VISIBILITY;
}


GG::XMLElement System::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("System");

   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<std::string>(FULL_VISIBILITY) );
   element.AppendChild(visibility);

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
      orbit_obj_id.SetAttribute( "value", lexical_cast<std::string>((*itr).second) );
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


GG::XMLElement System::XMLEncode(int empire_id) const
{
   // Partial visibility version.  Displays only the star type.

   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("System");

   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<std::string>(PARTIAL_VISIBILITY) );
   element.AppendChild(visibility);

   // use partial encode so owners are not visible
   element.AppendChild( UniverseObject::XMLEncode(empire_id) );

   XMLElement star("m_star");
   star.SetAttribute( "value", lexical_cast<std::string>(m_star) );
   element.AppendChild(star);

   return element;
}


int System::Insert(UniverseObject* obj)
{
	int retval = -1;

    // look for an empty orbit to place into
    for (int orb = 0; orb < m_orbits; orb++)
    {
       if (m_objects.find(orb) == end())
       {
          m_objects.insert(std::pair<int, int>(orb, obj->ID()));
          obj->SetSystem(this);
          return orb;
       }
    }

    // if no empty orbits, add a new one
    m_objects.insert(std::pair<int, int>(m_orbits, obj->ID()));
    m_orbits++;
    
	return retval;
}

int System::Insert(UniverseObject* obj, int orbit)
{
    if (orbit < 0 || m_orbits < orbit)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
    m_objects.insert(std::pair<int, int>(orbit, obj->ID()));
    obj->SetSystem(this);
	return orbit;
}


int System::Insert(int obj_id, int orbit)
{
    if (orbit < 0 || m_orbits < orbit)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
    m_objects.insert(std::pair<int, int>(orbit, obj_id));
	return orbit;
}

bool System::Remove(int id)
{
   UniverseObject* retval = 0;
   for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
      if (it->second == id) {
         retval->SetSystem(0);
         m_objects.erase(it);
         return true;
      }
   }
   return false;
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


