#include "System.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;


#include <stdexcept>


System::System() : 
   UniverseObject(),
   m_star(INVALID_STAR_TYPE),
   m_orbits(0)
{
}

System::System(StarType star, int orbits, const std::string& name, double x, double y, 
               const std::set<int>& owners/* = std::set<int>()*/) : 
   UniverseObject(name, x, y, owners),
   m_star(star),
   m_orbits(orbits)
{
   if (m_star < INVALID_STAR_TYPE || NUM_STAR_TYPES < m_star)
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
   if (m_star < INVALID_STAR_TYPE || NUM_STAR_TYPES < m_star)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with an invalid star type.");
   if (m_orbits < 0)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
}
   
System::System(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject")),
   m_orbits(0)
{
    using GG::XMLElement;

    if (elem.Tag().find( "System" ) == std::string::npos )
        throw std::invalid_argument("Attempted to construct a System from an XMLElement that had a tag other than \"System\"");

    try {
        m_star = lexical_cast<StarType>(elem.Child("m_star").Text());

        Visibility vis = Visibility(lexical_cast<int>(elem.Child("UniverseObject").Child("vis").Text()));
        if (vis == PARTIAL_VISIBILITY || vis == FULL_VISIBILITY) {
            m_orbits = lexical_cast<int>(elem.Child("m_orbits").Text());
            m_objects = GG::MultimapFromString<int, int>(elem.Child("m_objects").Text());
            m_starlanes_wormholes = GG::MapFromString<int, bool>(elem.Child("m_starlanes_wormholes").Text());
        }      
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in System::System(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
}

System::~System()
{
}
   
int System::Starlanes() const
{
    int retval = 0;
    for (const_lane_iterator it = begin_lanes(); it != end_lanes(); ++it) {
        if (!it->second)
            ++retval;
    }
    return retval;
}

int System::Wormholes() const
{
    int retval = 0;
    for (const_lane_iterator it = begin_lanes(); it != end_lanes(); ++it) {
        if (it->second)
            ++retval;
    }
    return retval;
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

UniverseObject::Visibility System::GetVisibility(int empire_id) const
{
    // if system is at least partially owned by this empire it is fully visible, if it has been explored it is partially visible, 
    // and otherwise it will be partially visible
    Empire* empire = 0;
    if (empire_id == Universe::ALL_EMPIRES || OwnedBy(empire_id))
        return FULL_VISIBILITY;
    else if ((empire = Empires().Lookup(empire_id)) && empire->HasExploredSystem(ID()))
        return PARTIAL_VISIBILITY;
    else
        return NO_VISIBILITY;
}


GG::XMLElement System::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   Visibility vis = GetVisibility(empire_id);

   XMLElement retval("System" + boost::lexical_cast<std::string>(ID()));
   retval.AppendChild(UniverseObject::XMLEncode(empire_id)); // use partial encode so owners are not visible
   retval.AppendChild(XMLElement("m_star", lexical_cast<std::string>(m_star)));
   if (vis == PARTIAL_VISIBILITY) {
      retval.AppendChild(XMLElement("m_orbits", lexical_cast<std::string>(m_orbits)));
      retval.AppendChild(XMLElement("m_objects", GG::StringFromMultimap<int, int>(PartiallyVisibleObjects(empire_id))));
      retval.AppendChild(XMLElement("m_starlanes_wormholes", GG::StringFromMap<int, bool>(m_starlanes_wormholes)));
   } else if (vis == FULL_VISIBILITY) {
      retval.AppendChild(XMLElement("m_orbits", lexical_cast<std::string>(m_orbits)));
      retval.AppendChild(XMLElement("m_objects", GG::StringFromMultimap<int, int>(m_objects)));
      retval.AppendChild(XMLElement("m_starlanes_wormholes", GG::StringFromMap<int, bool>(m_starlanes_wormholes)));
   }
   return retval;
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
    obj->MoveTo(X(), Y());
    StateChangedSignal()();
	return Insert(obj->ID(), orbit);
}


int System::Insert(int obj_id, int orbit)
{
    if (orbit < -1)
        throw std::invalid_argument("System::Insert() : Attempted to place an object in an orbit less than -1");
    if (!GetUniverse().Object(obj_id))
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
         GetUniverse().Object(it->second)->SetSystem(INVALID_OBJECT_ID);
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

void System::AddOwner(int id)
{
}

void System::RemoveOwner(int id)
{
}

void System::MovementPhase( )
{
}

void System::PopGrowthProductionResearchPhase( )
{
}

System::ObjectMultimap System::PartiallyVisibleObjects(int empire_id) const
{
    ObjectMultimap retval;
    const Universe& universe = GetUniverse();
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (universe.Object(it->second)->GetVisibility(empire_id) <= PARTIAL_VISIBILITY)
            retval.insert(*it);
    }
    return retval;
}