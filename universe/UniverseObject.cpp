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

    try {
        Visibility vis = Visibility(lexical_cast<int>(elem.Child("vis").Text()));

        m_id = lexical_cast<int>(elem.Child("m_id").Text());
        m_x = lexical_cast<double>(elem.Child("m_x").Text());
        m_y = lexical_cast<double>(elem.Child("m_y").Text());
        m_system_id = lexical_cast<int>(elem.Child("m_system_id").Text());

        if (vis == PARTIAL_VISIBILITY || vis == FULL_VISIBILITY) {
            m_name = elem.Child("m_name").Text();
            m_owners = GG::ContainerFromString<std::set<int> >(elem.Child("m_owners").Text());
        }
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in UniverseObject::UniverseObject(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
}

UniverseObject::~UniverseObject()
{
}

System* UniverseObject::GetSystem() const
{
    return dynamic_cast<System*>(GetUniverse().Object(m_system_id));
}

bool UniverseObject::Unowned() const 
{
    return m_owners.empty();
}

bool UniverseObject::OwnedBy(int empire) const 
{
    return m_owners.find(empire) != m_owners.end();
}

bool UniverseObject::WhollyOwnedBy(int empire) const 
{
    return m_owners.size() == 1 && m_owners.find(empire) != m_owners.end();
}

UniverseObject::Visibility UniverseObject::GetVisibility(int empire_id) const
{
    return (empire_id == Universe::ALL_EMPIRES || m_owners.find(empire_id) != m_owners.end()) ? FULL_VISIBILITY : NO_VISIBILITY;
}

GG::XMLElement UniverseObject::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
    // limited visibility object -- no owner info
   using GG::XMLElement;
   using boost::lexical_cast;

   Visibility vis = GetVisibility(empire_id);
   
   XMLElement retval("UniverseObject");
   retval.AppendChild(XMLElement("vis", lexical_cast<std::string>(vis)));
   retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
   retval.AppendChild(XMLElement("m_x", lexical_cast<std::string>(m_x)));
   retval.AppendChild(XMLElement("m_y", lexical_cast<std::string>(m_y)));
   retval.AppendChild(XMLElement("m_system_id", lexical_cast<std::string>(m_system_id)));
   if (vis == PARTIAL_VISIBILITY || vis == FULL_VISIBILITY) {
      retval.AppendChild(XMLElement("m_name", m_name));
      retval.AppendChild(XMLElement("m_owners", GG::StringFromContainer<std::set<int> >(m_owners)));
   }
   return retval;
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
