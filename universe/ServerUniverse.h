#ifndef _ServerUniverse_h_
#define _ServerUniverse_h_

#ifndef _ClientUniverse_h_
#include "ClientUniverse.h"
#endif



class UniverseObject;
class SitRepEntry;
namespace GG {class XMLElement;}

/** ServerUniverse is the universe class used by the server.  ServerUniverse is essentially a container for 
   UniverseObjects; it allows the server modules access to pointers to these UniverseObjects, either by ID lookup 
   (with Object()), or by search using an arbitrary prediacte (with FindObjects()).  The ID numbers of 
   UniverseObjects are also freely available using arbitrary predicates and FindObjectIDs().  Objects may be added
   to the ServerUniverse using Insert().*/
class ServerUniverse : public ClientUniverse
{
public:
   typedef ObjectMap::iterator            iterator;   ///< an iterator for iteration over the objects in the universe
   typedef std::vector<UniverseObject*>   ObjectVec;  ///< the return type of FindObjects()
   
   /** \name Structors */ //@{
   ServerUniverse(); ///< default ctor
   ServerUniverse(const GG::XMLElement& elem); ///< ctor that constructs a ServerUniverse object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ServerUniverse object
   ~ServerUniverse(); ///< dtor
   //@}
   
   /** \name Mutators */ //@{
   /** inserts object \a obj into the universe; returns the ID number assigned to the object, or -1 on failure.  
      \note ServerUniverse gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after 
      passing it to Insert().*/
   int               Insert(UniverseObject* obj);
   
   /** generates systems and planets, assigns homeworlds and populates them with people, industry and bases, and places starting fleets.  Uses predefined galaxy shapes.  */
   void              CreateUniverse(Shape shape, int size, int players, int ai_players);
   /** generates systems and planets, assigns homeworlds and populates them with people, industry and bases, and places starting fleets.  Uses an arbitrary bitmap image to determine galaxy shape. */
   void              CreateUniverse(const std::string& map_file, int size, int players, int ai_players);

   /** removes the object with ID number \a id from the universe, and returns it; returns 0 if theree is no such object*/
   UniverseObject*   Remove(int id);
   
   /** removes the object with ID number \a id; returns true if such an object was found, false otherwise*/
   bool              Delete(int id);
   
   UniverseObject*   Object(int id); ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists
   
   /** returns all the objects that match \a pred.  Predicates used with this function must take a single UniverseObject* 
      parameter and must return a bool or a type for which there is a conversion to bool.*/
   template <class Pred>
   ObjectVec FindObjects(Pred pred)
   {
      ObjectVec retval;
      for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
         UniverseObject* o = it->second;
         if (pred(o))
            retval.push_back(o);
      }
      return retval;
   }
   
   /** moves UniverseObjects that are in movement or are starting movement as a result of orders this turn.  This must be 
      called after all movement Orders have been processed but before combats are resolved.  Any SitRepEntrys that 
      are generated are pushed onto the end of \a sit_reps.*/
   void MovementPhase(std::vector<SitRepEntry>& sit_reps);
   
   /** grows pop, and executes production and research.  This must be called after all Orders have been processed on the 
      Universe and all combats are resolved.  Any SitRepEntrys that are generated are pushed onto the end of \a sit_reps.*/
   void PopGrowthProductionResearch(std::vector<SitRepEntry>& sit_reps);

   iterator begin()  {return m_objects.begin();}   ///< returns the begin const_iterator for the objects in the universe
   iterator end()    {return m_objects.end();}     ///< returns the end const_iterator for the objects in the universe
   //@}
   
private:

   void GenerateSpiralGalaxy(int arms, int stars);  ///< creates a spiral galaxy and stores the empire homeworlds in the homeworlds vector
   void GenerateElipticalGalaxy(int stars);  ///< creates an eliptical galaxy and stores the empire homeworlds in the homeworlds vector
   void GenerateIrregularGalaxy(int stars);  ///< creates an irregular galaxy and stores the empire homeworlds in the homeworlds vector

   void GenerateHomeworlds(int players, int stars, std::vector<int>& homeworlds);  ///< Picks systems to host homeworlds, generates planets for them, stores the ID's of the homeworld planets into the homeworld vector

   void PopulateSystems();  ///< Will generate planets for all systems that have empty object maps (ie those that aren't homeworld systems)

   void GenerateEmpires(int players, int ai_players, std::vector<int>& homeworlds);  ///< Will create empire objects, assign them homeworlds, setup the homeworld population, industry, and starting fleets

   int m_last_allocated_id;
   
};

#endif // _ServerUniverse_h_
