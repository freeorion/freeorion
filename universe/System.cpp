#include "System.h"
#include "XMLDoc.h"

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
         int orbit = lexical_cast<int>(map_object.Attribute("orbit"));
         int id = lexical_cast<int>(map_object.Attribute("id"));
         m_objects.insert(std::make_pair(orbit, id));
      }

      XMLElement lane_map = elem.Child("m_starlanes_wormholes");
      for(int i=0; i<lane_map.NumChildren(); i++)
      {
         XMLElement lane = lane_map.Child(i);
         int system = lexical_cast<int>(lane.Attribute("system"));
         bool wormhole = lexical_cast<bool>(lane.Attribute("wormhole"));
         m_starlanes_wormholes[system] = wormhole;
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

   Empire* empire = (Empires()).Lookup(empire_id);
       
   if (empire->HasExploredSystem(ID()))
   {
      return FULL_VISIBILITY;
   }

   return PARTIAL_VISIBILITY;
}


GG::XMLElement System::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   XMLElement element("System");

   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<string>(FULL_VISIBILITY) );
   element.AppendChild(visibility);

   element.AppendChild( UniverseObject::XMLEncode() );

   XMLElement star("m_star");
   star.SetAttribute( "value", lexical_cast<string>(m_star) );
   element.AppendChild(star);

   XMLElement orbits("m_orbits");
   orbits.SetAttribute( "value", lexical_cast<string>(m_orbits) );
   element.AppendChild(orbits);

   XMLElement orbit_map("m_objects");
   for(const_orbit_iterator itr = begin(); itr != end(); ++itr)
   {
      XMLElement map_object("map_object");
      map_object.SetAttribute("orbit", lexical_cast<string>(itr->first));
      map_object.SetAttribute("id", lexical_cast<string>(itr->second));
      orbit_map.AppendChild(map_object);
   }
   element.AppendChild(orbit_map);

   XMLElement lane_map("m_starlanes_wormholes");
   for(const_lane_iterator itr = begin_lanes(); itr != end_lanes(); ++itr)
   {
      XMLElement map_lane("map_lane");
      map_lane.SetAttribute("system", lexical_cast<string>(itr->first));
      map_lane.SetAttribute("wormhole", lexical_cast<string>(itr->second));
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

   XMLElement orbits("m_orbits");
   orbits.SetAttribute( "value", lexical_cast<std::string>(m_orbits) );
   element.AppendChild(orbits);

   XMLElement orbit_map("m_objects");
   for(const_orbit_iterator itr = begin(); itr != end(); ++itr)
   {
      XMLElement map_object("map_object");
      map_object.SetAttribute("orbit", lexical_cast<std::string>(itr->first));
      map_object.SetAttribute("id", lexical_cast<std::string>(itr->second));
      orbit_map.AppendChild(map_object);
   }
   element.AppendChild(orbit_map);

   XMLElement lane_map("m_starlanes_wormholes");
   for(const_lane_iterator itr = begin_lanes(); itr != end_lanes(); ++itr)
   {
      XMLElement map_lane("map_lane");
      map_lane.SetAttribute("system", lexical_cast<std::string>(itr->first));
      map_lane.SetAttribute("wormhole", lexical_cast<std::string>(itr->second));
      lane_map.AppendChild(map_lane);
   }
   element.AppendChild(lane_map);

   return element;
}

int System::Insert(UniverseObject* obj)
{
    return Insert(obj, -1);
}

int System::Insert(UniverseObject* obj, int orbit)
{
    if (!obj)
        throw std::invalid_argument("System::Insert() : Attempted to place a null object in a System");
    if (orbit < -1)
        throw std::invalid_argument("System::Insert() : Attempted to place an object in an orbit less than -1");
    obj->SetSystem(ID());
    StateChangedSignal()();
	return Insert(obj->ID(), orbit);
}


int System::Insert(int obj_id, int orbit)
{
    if (orbit < -1)
        throw std::invalid_argument("System::Insert() : Attempted to place an object in an orbit less than -1");

    const Universe& universe = GetUniverse();

    if (!universe.Object(obj_id))
        throw std::invalid_argument("System::Insert() : Attempted to place an object in a System, when the object is not already in the Universe");
    if (m_orbits <= orbit)
        m_orbits = orbit + 1;
    m_objects.insert(std::pair<int, int>(orbit, obj_id));
    StateChangedSignal()();
	return orbit;
}

bool System::Remove(int id)
{
   bool retval = false;
   for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
      if (it->second == id) {

         const Universe& universe = GetUniverse();

         const_cast<UniverseObject*>(universe.Object(it->second))->SetSystem(0);
         m_objects.erase(it);
         retval = true;
         StateChangedSignal()();
         break;
      }
   }
   return retval;
}

void System::AddStarlane(int id)
{
   m_starlanes_wormholes[id] = false;
   StateChangedSignal()();
}

void System::AddWormhole(int id)
{
   m_starlanes_wormholes[id] = true;
   StateChangedSignal()();
}

bool System::RemoveStarlane(int id)
{
   bool retval = false;
   if (retval = HasStarlaneTo(id)) {
      m_starlanes_wormholes.erase(id);
      StateChangedSignal()();
   }
   return retval;
}

bool System::RemoveWormhole(int id)
{
   bool retval = false;
   if (retval = HasWormholeTo(id)) {
      m_starlanes_wormholes.erase(id);
      StateChangedSignal()();
   }
   return retval;
}

void System::MovementPhase( )
{
}

void System::PopGrowthProductionResearchPhase( )
{
}


