#include "UniverseObject.h"

#include "../util/AppInterface.h"
#include "System.h"
#include "Universe.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
#include <stdexcept>

#include <stdexcept>


// static(s)
const double UniverseObject::INVALID_POSITION =  -100000.0;
const int    UniverseObject::INVALID_OBJECT_ID = -1;
const int    UniverseObject::MAX_ID            = 2000000000;

UniverseObject::UniverseObject() : 
   m_id(INVALID_OBJECT_ID),
   m_x(INVALID_POSITION),
   m_y(INVALID_POSITION),
   m_system_id(INVALID_OBJECT_ID)
{
}

UniverseObject::UniverseObject(const std::string name, double x, double y, 
                               const std::set<int>& owners/* = std::set<int>()*/) : 
   m_id(INVALID_OBJECT_ID),
   m_name(name),
   m_x(x),
   m_y(y),
   m_owners(owners),
   m_system_id(INVALID_OBJECT_ID)
{
   if (m_x < 0.0 || Universe::UniverseWidth() < m_x || m_y < 0.0 || Universe::UniverseWidth() < m_y)
      throw std::invalid_argument("UniverseObject::UniverseObject : Attempted to create an object \"" + m_name + "\" off the map area.");
}

UniverseObject::UniverseObject(const GG::XMLElement& elem)
{
   using GG::XMLElement;

   if (elem.Tag() != "UniverseObject")
      throw std::invalid_argument("Attempted to construct a UniverseObject from an XMLElement that had tag: \"" +  elem.Tag() + "\"." );

   Visibility vis = (Visibility) lexical_cast<int> ( elem.Child("visibility").Attribute("value") );

   m_id = lexical_cast<int> ( elem.Child("m_id").Attribute("value") );
   m_name = elem.Child("m_name").Text();
   m_x = lexical_cast<double> ( elem.Child("m_x").Attribute("value") );
   m_y = lexical_cast<double> ( elem.Child("m_y").Attribute("value") );
   m_system_id = lexical_cast<int> ( elem.Child("m_system_id").Attribute("value") );

   if (vis == FULL_VISIBILITY)
   {
      XMLElement owners = elem.Child("m_owners");
      for(int i=0; i<owners.NumChildren(); i++)
      {
          m_owners.insert(  lexical_cast<int> (owners.Child(i).Attribute("value") ) );
      }
   }       
}

UniverseObject::~UniverseObject()
{
}

System* UniverseObject::GetSystem() const
{
    return dynamic_cast<System*>(GetUniverse().Object(m_system_id));
}

GG::XMLElement UniverseObject::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   
   XMLElement element("UniverseObject");
   
   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<std::string>(FULL_VISIBILITY) );
   element.AppendChild(visibility);

   XMLElement ID("m_id");
   ID.SetAttribute( "value", lexical_cast<std::string>(m_id) );
   element.AppendChild(ID);
   
   XMLElement name("m_name");
   name.SetText(Name());
   element.AppendChild(name);
   
   XMLElement x("m_x");
   x.SetAttribute( "value", lexical_cast<std::string>(m_x) );
   element.AppendChild(x);

   XMLElement y("m_y");
   y.SetAttribute( "value", lexical_cast<std::string>(m_y) );
   element.AppendChild(y);

   XMLElement owners("m_owners");
   for(std::set<int>::const_iterator itr=m_owners.begin(); itr != m_owners.end(); itr++)
   {
      XMLElement owner("owner"+lexical_cast<std::string>(*itr));
      owner.SetAttribute( "value", lexical_cast<std::string>(*itr) );
      owners.AppendChild(owner);
   }
   element.AppendChild(owners);

   XMLElement system_id("m_system_id");
   system_id.SetAttribute( "value", lexical_cast<std::string>(m_system_id) );
   element.AppendChild(system_id);

   return element;
}



GG::XMLElement UniverseObject::XMLEncode(int empire_id) const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   
   XMLElement element("UniverseObject");
   
   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<std::string>(PARTIAL_VISIBILITY) );
   element.AppendChild(visibility);
   
   XMLElement ID("m_id");
   ID.SetAttribute( "value", lexical_cast<std::string>(m_id) );
   element.AppendChild(ID);
   
   XMLElement name("m_name");
   name.SetText(Name());
   element.AppendChild(name);

   XMLElement x("m_x");
   x.SetAttribute( "value", lexical_cast<std::string>(m_x) );
   element.AppendChild(x);

   XMLElement y("m_y");
   y.SetAttribute( "value", lexical_cast<std::string>(m_y) );
   element.AppendChild(y);

   XMLElement system_id("m_system_id");
   system_id.SetAttribute( "value", lexical_cast<std::string>(m_system_id) );
   element.AppendChild(system_id);


   return element;
}


void UniverseObject::Move(double x, double y)
{
   if (m_x + x < 0.0 || Universe::UniverseWidth() < m_x + x || m_y + y < 0.0 || Universe::UniverseWidth() < m_y + y)
      throw std::runtime_error("UniverseObject::Move : Attempted to move object \"" + m_name + "\" off the map area.");
   m_x += x;
   m_y += y;
   m_changed_sig();
}

void UniverseObject::MoveTo(double x, double y)
{
   if (x < 0.0 || Universe::UniverseWidth() < x || y < 0.0 || Universe::UniverseWidth() < y)
      throw std::invalid_argument("UniverseObject::MoveTo : Attempted to place object \"" + m_name + "\" off the map area.");
   m_x = x;
   m_y = y;
   m_changed_sig();
}

void UniverseObject::AddOwner(int id)    
{
  m_owners.insert(id); 
  m_changed_sig();
}

void UniverseObject::RemoveOwner(int id)
{
  m_owners.erase(id);
  m_changed_sig();
}
