#include "ClientUniverse.h"
#include "UniverseObject.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
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
const double ClientUniverse::UNIVERSE_WIDTH =    1000.0;
const int    ClientUniverse::MIN_STAR_DISTANCE = 15;

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
        if (UniverseObject* obj = m_factory.GenerateObject(elem.Child("m_objects").Child(i)))
            Insert(obj, obj->ID());
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

int ClientUniverse::NearestSystem(System& target_sys)
{
   // get list of system ID's
   ConstObjectVec sys_obj_vec = FindObjects(IsSystem);
   int nearest_sys_id = 0;
   float nearest_dist = 2000;

   for (int sys_cnt = 0; sys_cnt < (int) sys_obj_vec.size(); sys_cnt++)
   {
      const System* temp_sys = dynamic_cast<const System*>(sys_obj_vec[sys_cnt]);

      if (temp_sys->ID() != target_sys.ID())
      {
         float temp_dist = pow((temp_sys->X()-target_sys.X()),2) + pow(temp_sys->Y()-target_sys.Y(),2);

         if (temp_dist < nearest_dist)
         {
            nearest_dist = temp_dist;
            nearest_sys_id = temp_sys->ID();
         }
         else if ((temp_dist == nearest_dist) && (temp_sys->ID() < nearest_sys_id))
         {
            nearest_dist = temp_dist;
            nearest_sys_id = temp_sys->ID();
         }
      } 
   }
   
   return nearest_sys_id;
}   

int ClientUniverse::NearestSystem(System& target_sys, System& prev_sys)
{
   // get list of system ID's
   ConstObjectVec sys_obj_vec = FindObjects(IsSystem);
   int nearest_sys_id = 0;
   float nearest_dist = 2000;
   float prev_dist = pow((prev_sys.X()-target_sys.X()),2) + pow(prev_sys.Y()-target_sys.Y(),2);

   for (int sys_cnt = 0; sys_cnt < (int) sys_obj_vec.size(); sys_cnt++)
   {
      const System* temp_sys = dynamic_cast<const System*>(sys_obj_vec[sys_cnt]);
      if ((temp_sys->ID() != target_sys.ID()) && (temp_sys->ID() != prev_sys.ID()))
      {
 
         float temp_dist = pow((temp_sys->X()-target_sys.X()),2) + pow(temp_sys->Y()-target_sys.Y(),2);
         if ((temp_dist > prev_dist) && (temp_dist < nearest_dist))
         {
            nearest_dist = temp_dist;
            nearest_sys_id = temp_sys->ID();
         }
         else if (temp_dist == prev_dist)
         {
            // special case: next system is same distance as prev_sys, but has higher ID
            if (temp_sys->ID() > prev_sys.ID())
            {
               // it's possible we've already found another system at the same distance also
               if ((nearest_dist != temp_dist) || (temp_sys->ID() < nearest_sys_id))
               {
                  nearest_dist = temp_dist;
                  nearest_sys_id = temp_sys->ID();
               }
            }
         }
        else if ((temp_dist == nearest_dist) && (temp_sys->ID() < nearest_sys_id))
         {
            nearest_dist = temp_dist;
            nearest_sys_id = temp_sys->ID();
         }
      } 
   }
   return nearest_sys_id;
}

int ClientUniverse::Insert(UniverseObject* obj, int obj_id)
{
   if (!obj)
   {
#ifdef FREEORION_BUILD_HUMAN
      HumanClientApp* client_app = HumanClientApp::GetApp();
      client_app->Logger().errorStream() << "ClientUniverse::Insert : Attempt to add object to object map failed because the object pointer is null.";
#endif
      return UniverseObject::INVALID_OBJECT_ID;
   }

   if (m_objects.find(obj_id) == m_objects.end())
   {
      // ID is unused, store object here
      m_objects[obj_id] = obj;
      return obj_id;
   }
      
   // ID is in use
#ifdef FREEORION_BUILD_HUMAN
   HumanClientApp* client_app = HumanClientApp::GetApp();
   client_app->Logger().errorStream() << "ClientUniverse::Insert : Attempt to add object to object map failed because ID " << obj_id << " is already in use.";
#endif
   return UniverseObject::INVALID_OBJECT_ID;         
}
