#include "ClientUniverse.h"
#include "UniverseObject.h"
#include "../GG/XML/XMLDoc.h"

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
