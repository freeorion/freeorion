#include "ClientUniverse.h"
#include "UniverseObject.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "XMLDoc.h"
#include "Predicates.h"

#ifdef FREEORION_BUILD_HUMAN
#include "../client/human/HumanClientApp.h"
#endif

#include <stdexcept>
#include <cmath>

namespace {
UniverseObject* NewFleet(const GG::XMLElement& elem)  {return new Fleet(elem);}
UniverseObject* NewPlanet(const GG::XMLElement& elem) {return new Planet(elem);}
UniverseObject* NewShip(const GG::XMLElement& elem)   {return new Ship(elem);}
UniverseObject* NewSystem(const GG::XMLElement& elem) {return new System(elem);}
}

// static(s)
const double ClientUniverse::UNIVERSE_WIDTH = 1000.0;

ClientUniverse::ClientUniverse()
{
    m_factory.AddGenerator("Fleet", &NewFleet);
    m_factory.AddGenerator("Planet", &NewPlanet);
    m_factory.AddGenerator("Ship", &NewShip);
    m_factory.AddGenerator("System", &NewSystem);
}
   
ClientUniverse::ClientUniverse(const GG::XMLElement& elem)
{
    m_factory.AddGenerator("Fleet", &NewFleet);
    m_factory.AddGenerator("Planet", &NewPlanet);
    m_factory.AddGenerator("Ship", &NewShip);
    m_factory.AddGenerator("System", &NewSystem);

    SetUniverse(elem);
}

ClientUniverse::~ClientUniverse()
{
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
        delete it->second;
}

void ClientUniverse::SetUniverse(const GG::XMLElement& elem)
{
    using GG::XMLElement;

    if (elem.Tag() != "ClientUniverse")
        throw std::invalid_argument("Attempted to construct a ClientUniverse from an XMLElement that had a tag other than \"ClientUniverse\"");

    // wipe out anything present in the object map
    for (ObjectMap::iterator itr= m_objects.begin(); itr != m_objects.end(); ++itr)
        delete itr->second;
    m_objects.clear();

    for (int i = 0; i < elem.Child("m_objects").NumChildren(); ++i) {
        if (UniverseObject* obj = m_factory.GenerateObject(elem.Child("m_objects").Child(i))) {
            m_objects[obj->ID()] = obj;
        } else {
#ifdef FREEORION_BUILD_HUMAN
            HumanClientApp::GetApp()->Logger().errorStream() << "ClientUniverse::SetUniverse : Failed while trying to factory-generate "
                "m_objects element #" << i << " of the incoming turn update universe.";
#endif
        }
    }
}


const UniverseObject* ClientUniverse::ClientUniverse::Object(int id) const
{
    const_iterator it = m_objects.find(id);
    return (it != m_objects.end() ? it->second : 0);
}

GG::XMLElement ClientUniverse::XMLEncode() const
{
    GG::XMLElement retval("ClientUniverse");

    GG::XMLElement temp("m_objects");
    for (const_iterator it = begin(); it != end(); ++it)
        temp.AppendChild(it->second->XMLEncode());
    retval.AppendChild(temp);
    return retval;
}

GG::XMLElement ClientUniverse::XMLEncode(int empire_id) const
{
   using GG::XMLElement;

   XMLElement element("ClientUniverse");

   XMLElement object_map("m_objects");
   for(const_iterator itr = begin(); itr != end(); ++itr)
   {
      // determine visibility
      UniverseObject::Visibility vis = (*itr).second->Visible(empire_id);
      if (vis == UniverseObject::FULL_VISIBILITY)
      {
         XMLElement univ_object("univ_object");
         univ_object.AppendChild( (*itr).second->XMLEncode() );
         object_map.AppendChild(univ_object);
      }
      else if (vis == UniverseObject::PARTIAL_VISIBILITY)
      {
         XMLElement univ_object("univ_object");
         univ_object.AppendChild( (*itr).second->XMLEncode(empire_id) );
         object_map.AppendChild(univ_object);
      }

      // for NO_VISIBILITY no element is added
   }
   element.AppendChild(object_map);

   return element;
}
