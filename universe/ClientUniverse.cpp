#include "ClientUniverse.h"

#include "UniverseObject.h"

#include "Fleet.h"

#include "Planet.h"

#include "Ship.h"

#include "XMLDoc.h"



#ifdef FREEORION_BUILD_HUMAN

#include "../client/human/HumanClientApp.h"

#endif



#include <stdexcept>

#include <cmath>



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

   using GG::XMLElement;



   if (elem.Tag() != "Universe")

      throw std::invalid_argument("Attempted to construct a ClientUniverse from an XMLElement that had a tag other than \"Universe\"");



   for(int i=0; i<elem.NumChildren(); i++)

   {

      XMLElement uni_obj = elem.Child(i);



      if (elem.Tag() == "Fleet")

      {

         Fleet* fleet = new Fleet(elem);

         Insert(fleet, fleet->ID());

      }

      else if (elem.Tag() == "Planet")

      {

         Planet* planet = new Planet(elem);

         Insert(planet, planet->ID());

      }

      else if (elem.Tag() == "Ship")

      {

         Ship* ship = new Ship(elem);

         Insert(ship, ship->ID());

      }

      else if (elem.Tag() == "System")

      {

         System* system = new System(elem);

         Insert(system, system->ID());

      }

      else

      {

#ifdef FREEORION_BUILD_HUMAN

         HumanClientApp* client_app = HumanClientApp::GetApp();

         client_app->Logger().errorStream() << "ClientUniverse::ClientUniverse : Attempt to add object to object map failed because element tag -" << elem.Tag() << "- is unknown.";

#endif

      }

   }

}



ClientUniverse::~ClientUniverse()

{

   for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {

      delete it->second;

   }

}



void ClientUniverse::PopulateUniverse(const GG::XMLElement& elem)

{

   using GG::XMLElement;



   // wipe out anything present in the object map

   for(ObjectMap::iterator itr= m_objects.begin(); itr != m_objects.end(); itr++)

   {

      delete itr->second;

      m_objects.erase(itr);

   }



   if (elem.Tag() != "Universe")

      throw std::invalid_argument("Attempted to construct a ClientUniverse from an XMLElement that had a tag other than \"Universe\"");



   for(int i=0; i<elem.NumChildren(); i++)

   {

      XMLElement uni_obj = elem.Child(i);



      if (elem.Tag() == "Fleet")

      {

         Fleet* fleet = new Fleet(elem);

         Insert(fleet, fleet->ID());

      }

      else if (elem.Tag() == "Planet")

      {

         Planet* planet = new Planet(elem);

         Insert(planet, planet->ID());

      }

      else if (elem.Tag() == "Ship")

      {

         Ship* ship = new Ship(elem);

         Insert(ship, ship->ID());

      }

      else if (elem.Tag() == "System")

      {

         System* system = new System(elem);

         Insert(system, system->ID());

      }

      else

      {

#ifdef FREEORION_BUILD_HUMAN

         HumanClientApp* client_app = HumanClientApp::GetApp();

         client_app->Logger().errorStream() << "ClientUniverse::ClientUniverse : Attempt to add object to object map failed because element tag -" << elem.Tag() << "- is unknown.";

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

   using GG::XMLElement;



   XMLElement element("Universe");



   XMLElement object_map("m_objects");

   for(const_iterator itr= begin(); itr != end(); itr++)

   {

      XMLElement univ_object("univ_object");

      univ_object.AppendChild( (*itr).second->XMLEncode() );

      object_map.AppendChild(univ_object);

   }

   element.AppendChild(object_map);



   return element;

}



GG::XMLElement ClientUniverse::XMLEncode(int empire_id) const

{

   using GG::XMLElement;



   XMLElement element("Universe");

  

   XMLElement object_map("m_objects");

   for(const_iterator itr= begin(); itr != end(); itr++)

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



// predicate for FindOjbects pred itr

bool IsSystem(const UniverseObject* obj) { return dynamic_cast<const System*>(obj); }



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

         float temp_dist = sqrt(pow((temp_sys->X()-target_sys.X()),2) + pow(temp_sys->Y()-target_sys.Y(),2));



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

   float prev_dist = sqrt(pow((prev_sys.X()-target_sys.X()),2) + pow(prev_sys.Y()-target_sys.Y(),2));



   for (int sys_cnt = 0; sys_cnt < (int) sys_obj_vec.size(); sys_cnt++)

   {

      const System* temp_sys = dynamic_cast<const System*>(sys_obj_vec[sys_cnt]);

      if ((temp_sys->ID() != target_sys.ID()) && (temp_sys->ID() != prev_sys.ID()))

      {

 

         float temp_dist = sqrt(pow((temp_sys->X()-target_sys.X()),2) + pow(temp_sys->Y()-target_sys.Y(),2));

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

   if (obj == NULL)

   {

#ifdef FREEORION_BUILD_HUMAN

      HumanClientApp* client_app = HumanClientApp::GetApp();

      client_app->Logger().errorStream() << "ClientUniverse::Insert : Attempt to add object to object map failed because the object pointer is NULL.";

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

