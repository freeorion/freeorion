#ifndef _ClientUniverse_h_
#define _ClientUniverse_h_

#include "System.h"

#include <vector>
#include <map>
#include <string>



// Object ID's are segregated into separate ranges by object type, primarily
// to facilitate the creation of new objects by the client without ID-space
// collisions
#define INVALID_OBJECT_ID              0
#define MIN_SHIP_ID           1900000000
#define MAX_SHIP_ID           2100000000



class UniverseObject;
namespace GG {class XMLElement;}

/** ClientUniverse is the universe class used by the clients.  ClientUniverse is essentially a container for 
   UniverseObjects; it allows anyone access to const pointers to these UniverseObjects, either by ID lookup 
   (with Object()), or by search using an arbitrary prediacte (with FindObjects()).  The ID numbers of 
   UniverseObjects are also freely available using arbitrary predicates and FindObjectIDs().  Nothing may be 
   added to the ClientUniverse, and ClientUniverse's only mutator is XMLMerge(), which takes the XML diffs 
   of the Universe created by the server during turn processing and updates the ClientUniverse.*/
class ClientUniverse 
{
protected:
   typedef std::map<int, UniverseObject*> ObjectMap; ///< the container type that is used internally to hold the objects in the universe; keyed by ID number
   
public:
   /** the types of universe shapes available in FreeOrion*/
   enum Shape {SPIRAL_2,      ///< a two-armed spiral galaxy
               SPIRAL_3,      ///< a three-armed spiral galaxy
               SPIRAL_4,      ///< a four-armed spiral galaxy
               ELLIPTICAL,    ///< an elliptical galaxy
               IRREGULAR      ///< an irregular galaxy
              }; // other types TBD
   
   typedef ObjectMap::const_iterator            const_iterator;   ///< a const_iterator for iteration over the objects in the universe
   typedef std::vector<const UniverseObject*>   ConstObjectVec;   ///< the return type of FindObjects()
   typedef std::vector<int>                     ObjectIDVec;      ///< the return type of FindObjectIDs()

   /** \name Structors */ //@{
   ClientUniverse(); ///< default ctor
   ClientUniverse(Shape shape, int stars, int players); ///< ctor to create a universe from a shape, number of stars, and number of players; other params TBD
   ClientUniverse(const std::string& map_file, int stars, int players); ///< ctor to create a universe from a shape image file, number of stars, and number of players; other params TBD
   ClientUniverse(const GG::XMLElement& elem); ///< ctor that constructs a ClientUniverse object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ClientUniverse object
   virtual ~ClientUniverse(); ///< dtor
   //@}
   
   /** \name Accessors */ //@{
   const UniverseObject* Object(int id) const; ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists
   
   /** returns all the objects that match \a pred.  Predicates used with this function must take a single const 
      UniverseObject* parameter and must return a bool or a type for which there is a conversion to bool.*/
   template <class Pred>
   ConstObjectVec FindObjects(Pred pred) const
   {
      ConstObjectVec retval;
      for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
         const UniverseObject* o = it->second;
         if (pred(o))
            retval.push_back(o);
      }
      return retval;
   }

   /** returns the IDs of all the objects that match \a pred.  Predicates used with this function must take a single const 
      UniverseObject* parameter and must return a bool or a type for which there is a conversion to bool.*/
   template <class Pred>
   ObjectIDVec FindObjectIDs(Pred pred) const
   {
      ObjectIDVec retval;
      for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
         const UniverseObject* o = it->second;
         if (pred(o))
            retval.push_back(it->first);
      }
      return retval;
   }

   const_iterator begin() const  {return m_objects.begin();}   ///< returns the begin const_iterator for the objects in the universe
   const_iterator end() const    {return m_objects.end();}     ///< returns the end const_iterator for the objects in the universe
   
   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a ClientUniverse object
   //@}

   int NearestSystem(System& target_sys); ///< returns the nearest system to the target. In the case that more than 1 systems are of equal distance, uses system ID as the tie-breaker
   int NearestSystem(System& target_sys, System& prev_sys); ///< returns the nearest system to target beyond the prev.  Can be used iteratively to generate a list of nearest systems.  Will use system ID as tie-breaker for equally distant systems.  This can result in a system being returned that is of an equal distance as prev_sys.

   
   /** \name Mutators */ //@{
   void XMLMerge(const GG::XMLElement& elem); ///< updates the ClientUniverse object from an XMLElement object that represents the updates
   //@}

   
protected:
   ObjectMap m_objects;
};

#endif // _ClientUniverse_h_
