#ifndef _UniverseObject_h_
#define _UniverseObject_h_

#include <set>
#include <string>
#include <vector>

class System;
class SitRepEntry;
namespace GG {class XMLElement;};

/** the abstract base class for all objects in the universe.  The UniverseObject class itself has only an ID, a name, 
   a position, possibly a System in which it is, and zero or more owners.  The position can range from 0 (left) to 1000 
   (right) in X, and 0 (top) to 1000 (bottom) in Y.  This coordinate system was chosen to help with conversion to 
   and from screen coordinates, which originate at the upper-left corner of the screen and increase down and to the 
   right.  Each UniverseObject-derived class inherits serveral pure virtual members that perform its actions during 
   various game phases, such as the movement phase.  These subclasses must define what actions to perform during 
   those phases.*/
class UniverseObject
{
public:
   /** the three different visibility levels */
   enum Visibility 
   { 
      FULL_VISIBILITY,
      PARTIAL_VISIBILITY,
      NO_VISIBILITY
   };

   /** \name Structors */ //@{
   UniverseObject();    ///< default ctor
   
   /** general ctor.  \throw std::invalid_arugment May throw std::invalid_arugment if the either x or y coordinate 
      is outside the map area.*/
   UniverseObject(const std::string name, double x, double y, const std::set<int>& owners = std::set<int>());
   
   UniverseObject(const GG::XMLElement& elem); ///< ctor that constructs a UniverseObject object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a UniverseObject object
   virtual ~UniverseObject();   ///< dtor
   //@}
   
   /** \name Accessors */ //@{
   int                  ID() const     {return m_id;}       ///< returns the ID number of this object.  Each object in FreeOrion has a unique ID number.
   const std::string&   Name() const   {return m_name;}     ///< returns the name of this object; some valid objects will have no name
   double               X() const      {return m_x;}        ///< the X-coordinate of this object
   double               Y() const      {return m_y;}        ///< the Y-coordinate of this object
   const std::set<int>& Owners() const {return m_owners;}   ///< returns the set of IDs of Empires owning all or part of this object.  \note This may be empty or have an arbitrary number of elements.
   const System*        GetSystem() const {return m_system;}///< returns a pointer to the system in which this object can be found, or 0 if the object is not within any system

   virtual Visibility Visible(int empire_id) const = 0; ///< returns the visibility status of this universe object relative to the input empire.
   
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a UniverseObject object
   virtual GG::XMLElement XMLEncode(int empire_id) const; ///< constructs an XMLElement from a UniverseObject object with visibility limited relative to the input empire
   //@}
   
   /** \name Mutators */ //@{
   void SetID(int id)                     {m_id = id;}      ///< sets the ID number of the object to \a id
   void Rename(const std::string& name)   {m_name = name;}  ///< renames this object to \a name
   
   /** moves the object using the vector (x, y). \throw std::runtime_error May throw std::runtime_error if the result 
      of the move would place either coordinate outside the map area.*/
   void Move(double x, double y);
   
   /** moves the object to coordinates (x, y). \throw std::invalid_arugment May throw std::invalid_arugment if the 
      either coordinate of the move is outside the map area.*/
   void MoveTo(double x, double y);
   
   void AddOwner(int id)            {m_owners.insert(id);}  ///< adds the Empire with ID \a id to the list of owners of this object
   void RemoveOwner(int id)         {m_owners.erase(id);}   ///< removes the Empire with ID \a id to the list of owners of this object
   void SetSystem(const System* sys){m_system = sys;}       ///< assigns this object to a System
   
   /** performs the movement that this object is responsible for this object's actions during the movement phase of 
      a turn.  Called by ServerUniverse::MovementPhase().*/
   virtual void MovementPhase(std::vector<SitRepEntry>& sit_reps) = 0;
   
   /** performs the movement that this object is responsible for this object's actions during the pop growth/production/
      research phase of a turn.  Called by ServerUniverse::PopGrowthProductionResearchPhase().*/
   virtual void PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps) = 0;

   //@}
   
   static const double  INVALID_POSITION = -100000;   ///< the position in x and y at which default-constructed objects are placed
   static const int     INVALID_OBJECT_ID = 0;              ///< the ID number assigned to a UniverseObject upon construction; it is assigned an ID later when it is placed in the universe

private:
   int            m_id;
   std::string    m_name;
   double         m_x;
   double         m_y;
   std::set<int>  m_owners;
   const System*  m_system;
};

#endif // _UniverseObject_h_
