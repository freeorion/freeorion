#include "UniverseObject.h"
#include "../GG/XML/XMLDoc.h"

#include <stdexcept>

UniverseObject::UniverseObject() : 
   m_id(INVALID_ID),
   m_x(INVALID_POSITION),
   m_y(INVALID_POSITION)
{
}

UniverseObject::UniverseObject(const std::string name, double x, double y, 
                               const std::set<int>& owners/* = std::set<int>()*/) : 
   m_id(INVALID_ID),
   m_name(name),
   m_x(x),
   m_y(y),
   m_owners(owners)
{
   if (m_x < 0.0 || 1000.0 < m_x || m_y < 0.0 || 1000.0 < m_y)
      throw std::invalid_argument("UniverseObject::UniverseObject : Attempted to create an object \"" + m_name + "\" off the map area.");
}

UniverseObject::UniverseObject(const GG::XMLElement& elem)
{
   if (elem.Tag() != "UniverseObject")
      throw std::invalid_argument("Attempted to construct a UniverseObject from an XMLElement that had a tag other than \"UniverseObject\"");
   // TODO
}

UniverseObject::~UniverseObject()
{
}

GG::XMLElement UniverseObject::XMLEncode() const
{
   // TODO
}

void UniverseObject::Move(double x, double y)
{
   if (m_x + x < 0.0 || 1000.0 < m_x + x || m_y + y < 0.0 || 1000.0 < m_y + y)
      throw std::runtime_error("UniverseObject::Move : Attempted to move object \"" + m_name + "\" off the map area.");
   m_x += x;
   m_y += y;
}

void UniverseObject::MoveTo(double x, double y)
{
   if (x < 0.0 || 1000.0 < x || y < 0.0 || 1000.0 < y)
      throw std::invalid_argument("UniverseObject::MoveTo : Attempted to place object \"" + m_name + "\" off the map area.");
   m_x = x;
   m_y = y;
}

void UniverseObject::XMLMerge(const GG::XMLElement& elem)
{
}

