// -*- C++ -*-
#ifndef _Universe_h_
#define _Universe_h_

#ifndef _SitRepEntry_h_
#include "../util/SitRepEntry.h"
#endif

#include <vector>
#include <map>
#include <string>
#include <set>

class System;
class UniverseObject;
namespace GG {class XMLElement;}


class Universe
{
protected:
   typedef std::map<int, UniverseObject*> ObjectMap; ///< the container type that is used internally to hold the objects in the universe; keyed by ID number
   
public:
   /** the types of universe shapes available in FeeOrion*/
   enum Shape {SPIRAL_2,      ///< a two-armed spiral galaxy
               SPIRAL_3,      ///< a three-armed spiral galaxy
               SPIRAL_4,      ///< a four-armed spiral galaxy
               CLUSTER,	      ///< a cluster galaxy
               ELLIPTICAL,    ///< an elliptical galaxy
               IRREGULAR,     ///< an irregular galaxy
               FROM_FILE,     ///< a galaxy loaded from a file
               GALAXY_SHAPES  ///< the number of shapes in this enum (leave this last)
              }; // other types TBD
   
    typedef ObjectMap::const_iterator            const_iterator;   ///< a const_iterator for iteration over the objects in the universe
    typedef ObjectMap::iterator                  iterator;
       
    typedef std::vector<const UniverseObject*>   ConstObjectVec;   ///< the return type of FindObjects()
    typedef std::vector<UniverseObject*>         ObjectVec;        ///< the return type of the non-const FindObjects()
    typedef std::vector<int>                     ObjectIDVec;      ///< the return type of FindObjectIDs()

    
    
   /** \name Structors */ //@{
   Universe(); ///< default ctor
   Universe(const GG::XMLElement& elem); ///< ctor that constructs a Universe object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Universe object
   virtual ~Universe(); ///< dtor
   //@}
   
   /** \name Accessors */ //@{
   const UniverseObject* Object(int id) const; ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists
   UniverseObject* Object(int id);  ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists
   
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

    iterator begin() { return m_objects.begin();}
    iterator end() { return m_objects.end();}

    const_iterator begin() const  {return m_objects.begin();}   ///< returns the begin const_iterator for the objects in the universe
    const_iterator end() const    {return m_objects.end();}     ///< returns the end const_iterator for the objects in the universe
   
    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a Universe object
    virtual GG::XMLElement XMLEncode(int empire_id) const; ///< constructs an XMLElement from a Universe object with visibility restrictions for the given empire
    //@}




    /** \name Mutators */ //@{
    void SetUniverse(const GG::XMLElement& elem ); ///< wipes out the current object map and sets the map to the XMLElement passed in.
    
     /** inserts object \a obj into the universe; returns the ID number assigned to the object, or -1 on failure.  
      \note Universe gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after 
      passing it to Insert().*/
    int               Insert(UniverseObject* obj);

     /** inserts object \a obj of given ID into the universe; returns true on proper insert , or false on failure.  
      \note Universe gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after 
      passing it to InsertID().
      Useful mostly for times when ID needs to be consistant on client and server*/
    bool               InsertID(UniverseObject* obj, int id );
   
    /** generates systems and planets, assigns homeworlds and populates them with people, industry and bases, and places starting fleets.  Uses predefined galaxy shapes.  */
    void              CreateUniverse(Shape shape, int size, int players, int ai_players);
    /** generates systems and planets, assigns homeworlds and populates them with people, industry and bases, and places starting fleets.  Uses an arbitrary bitmap image to determine galaxy shape. */
    void              CreateUniverse(const std::string& map_file, int size, int players, int ai_players);

    /** removes the object with ID number \a id from the universe, and returns it; returns 0 if there is no such object*/
    UniverseObject*   Remove(int id);
   
    /** removes the object with ID number \a id; returns true if such an object was found, false otherwise*/
    bool              Delete(int id);
    
      /** moves UniverseObjects that are in movement or are starting movement as a result of orders this turn.  This must be 
      called after all movement Orders have been processed but before combats are resolved.  Any SitRepEntrys that 
      are generated are pushed onto the end of \a sit_reps.*/
    void MovementPhase(std::vector<SitRepEntry>& sit_reps);
   
   /** grows pop, and executes production and research.  This must be called after all Orders have been processed on the 
      Universe and all combats are resolved.  Any SitRepEntrys that are generated are pushed onto the end of \a sit_reps.*/
    void PopGrowthProductionResearch(std::vector<SitRepEntry>& sit_reps);
    
    
    //@}

   /** defines the size of the galaxy map.  Does not measure absolute distances; the ratio between map coordinates and actual distance varies 
       depending on universe size */
    static const double UNIVERSE_WIDTH;

  int GenerateObjectID( );  ///< generates an object ID for a future object. Usually used by the server to service new ID requests

  typedef std::vector<std::vector<std::set<std::pair<double, double> > > > AdjacencyGrid;

protected:

    // factory class
    template <class T> class NumberedElementFactory
    {
    public:
        typedef T* (*Generator)(const GG::XMLElement&); ///< this defines the function signature for object generators

        /** \name Structors */ //@{
        NumberedElementFactory() {} ///< ctor
        //@}

        /** Generates objects whose tag name contains but is not limited to the generator name  */ //@{
        T* GenerateObject(const GG::XMLElement& elem) const ///< returns a heap-allocated subclass object of the appropriate type
        {
            T* retval = 0;

            for ( typename std::map<std::string, Generator>::const_iterator it = m_generators.begin(); it != m_generators.end(); ++it )
            {
                // is the string anywhere in the tag?
                std::string tag_name = elem.Tag();
                std::string tag_alpha = tag_name.substr(0, tag_name.find_first_of("0123456789") );

                if ( tag_alpha == it->first )
                {
                    retval = it->second(elem);
                    break;
                }
            }
            return retval;
        }
   
        /** \name Mutators */ //@{
        /** adds (or overrides) a new generator that can generate subclass objects described by \a name */
        void AddGenerator(const std::string& name, Generator gen) {m_generators[name] = gen;}
        //@}

   private:
        /** mapping from strings to functions that can create the type of object that corresponds to the string */
        std::map<std::string, Generator> m_generators;
   };

   void GenerateSpiralGalaxy(int arms, int stars, AdjacencyGrid& adjacency_grid);  ///< creates a spiral galaxy and stores the empire homeworlds in the homeworlds vector
   void GenerateEllipticalGalaxy(int stars, AdjacencyGrid& adjacency_grid);  ///< creates an eliptical galaxy and stores the empire homeworlds in the homeworlds vector
   void GenerateClusterGalaxy(int stars, AdjacencyGrid& adjacency_grid);  ///< creates an cluster galaxy and stores the empire homeworlds in the homeworlds vector
   void GenerateIrregularGalaxy(int stars, AdjacencyGrid& adjacency_grid);   ///< creates an irregular galaxy and stores the empire homeworlds in the homeworlds vector

   void GenerateHomeworlds(int players, std::vector<int>& homeworlds, AdjacencyGrid& adjacency_grid);  ///< Picks systems to host homeworlds, generates planets for them, stores the ID's of the homeworld planets into the homeworld vector

   void PopulateSystems();  ///< Will generate planets for all systems that have empty object maps (ie those that aren't homeworld systems)

   /// Will create empire objects, assign them homeworlds, setup the homeworld population, industry, and starting fleets
   /// NOTE: does nothing if executed client-side. This is a hack to deal with the 
   /// dependency on ServerEmpireManager -- jdb                     
   void GenerateEmpires(int players, std::vector<int>& homeworlds);

   ObjectMap m_objects;
   NumberedElementFactory<UniverseObject> m_factory;
   int m_last_allocated_id;
};






#endif // _Universe_h_

