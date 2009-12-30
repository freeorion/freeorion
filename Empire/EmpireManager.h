// -*- C++ -*-
#ifndef _EmpireManager_h_
#define _EmpireManager_h_

#include "../universe/Enums.h"

#ifndef _GG_Clr_h_
#include <GG/Clr.h>
#endif

#include <list>
#include <map>
#include <string>

class Empire;

/** Maintains all of the Empire objects that exist in the application. */
class EmpireManager
{
public:
    /// Iterator over Empires
    typedef std::map<int, Empire*>::iterator iterator; 

    /// Const Iterator over Empires
    typedef std::map<int, Empire*>::const_iterator const_iterator;

    /** \name Structors */ //@{
    virtual ~EmpireManager(); ///< virtual dtor
    const EmpireManager& operator=(EmpireManager& rhs); ///< assignment operator (move semantics)
    //@}

    /** \name Accessors */ //@{
    /** Returns the empire whose ID is \a ID, or 0 if none exists. */
    const Empire* Lookup(int ID) const;

    const_iterator begin() const;
    const_iterator end() const;
    //@}

    /** \name Mutators */ //@{
    /** Returns the empire whose ID is \a ID, or 0 if none exists. */
    Empire*     Lookup(int ID);

    iterator    begin();
    iterator    end();

    /** Creates and inserts an empire with the specified properties and returns a pointer to it.  \a planet_ID is the ID
        of the planet which is the empire's homeworld the empire will be created.  This will only set up the data in
        Empire.  It is the caller's responsibility to make sure that universe updates planet ownership. */
    Empire*     CreateEmpire(int id, const std::string& name, const std::string& player_name, const GG::Clr& color, int planet_ID);

    /** Adds the given empire to the manager. */
    void        InsertEmpire(Empire* empire);

    /** Removes the given empire from the manager. */
    void        RemoveEmpire(Empire* empire);

    /** Removes and deletes all empires from the manager. */
    void        RemoveAllEmpires();

    /** Removes all traces of the empire with the given ID.  and deallocates that empire.  Again, this method does not
       do anything to the universe.  This method returns true if the empire was removed, false if it doesn't exist. */
    bool        EliminateEmpire(int id);
    //@}


private:
    std::map<int, Empire*> m_empire_map;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void EmpireManager::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_empire_map);
}

#endif // _EmpireManager_h_
