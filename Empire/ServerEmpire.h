
#ifndef _FREEORION_SERVEREMPIRE_H_
#define _FREEORION_SERVEREMPIRE_H_

#ifndef _FREEORION_EMPIREMANAGER_H_
#include "EmpireManager.h"
#endif

/**
* ServerEmpire is the Server version of the EmpireManager
*
*/
class ServerEmpire : public EmpireManager
{
public:
   
   /** \name Constructors */ //@{
   /// Default Constructor
   /** Initializes the empire ID counter */
   ServerEmpire();
   //@}
   
    /** 
       Creates an empire with the specified properties and
       returns a pointer to it, after setting it up.
       homeID is the ID of the planet which is the empire's homeworld
       the empire will be created, and the given planetID added to its
       list of owned planets.
       this will only set up the data in Empire.  It is the caller's 
       responsibility to make sure that universe updates planet ownership.
       I do this because GameCore may want to call this under a variety of
       circumstances, and I do not want it to be too specific.
    */
    Empire* CreateEmpire(std::string& name, GG::Clr color, int planetID, Empire::ControlStatus state);

    
    /**
      removes all traces of the empire with the given ID.
      and deallocates that empire.  Pointers, references, and iterators
      to that empire will be invalidated.
      
      Nothing happens if an empire with the specified ID does not exist.
      
      Again, this method does not do anything to the universe,
      that is GameCore's responsibility.
      
      This method returns true if the empire was removed, false if it
      doesn't exist.
    */
    bool RemoveEmpire(int ID);
    
    
    /**
    * Creates an XMLElement representing an XML diff between the
    * present state of the empires and the state at the beginning of 
    * the current turn.
    *
    * The returned element can be passed to the 
    * ClientManager::HandleEmpireUpdate() method to bring the ClientManager
    * in sync with the server manager.  
    *
    * When this method is called, the stored XMLElements for the empires at
    * the beginning of the turn are replaced by fresh elements representing
    * their current states.
    */
	GG::XMLElement CreateClientEmpireUpdate(int EmpireID);
    
    /**
    * Creates an XMLElement representing the list of sitrep events
    * for the empire with the given ID.  The returned element can be 
    * sent to the client and decoded to put the proper sitrep events in
    * the client's queue
    */
	GG::XMLElement CreateClientSitrepUpdate(int EmpireID);
    
private:

    /**
    * A set of XMLElements for the empires, which contain their states
    * at the beginning of this turn.  These are used by 
    * CreateClientEmpireUpdate() to produce an update XMLElement to
    * send off to the client.
    *
    * This list is repopulated with the current empire states
    * whenever CreateClientEmpireUpdate() is called
    */
    std::list<GG::XMLElement> m_last_turn_elements;
    
    /// The next ID value that will be used for creating empires
    int m_next_id;
 
};

#endif
