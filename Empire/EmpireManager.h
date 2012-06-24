// -*- C++ -*-
#ifndef _EmpireManager_h_
#define _EmpireManager_h_

#include "../universe/Enums.h"

#ifndef _GG_Clr_h_
#include <GG/Clr.h>
#endif

#include <boost/serialization/access.hpp>

#include <map>
#include <set>
#include <string>

class Empire;

/** Maintains all of the Empire objects that exist in the application. */
class EmpireManager {
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
    const Empire*       Lookup(int ID) const;

    const_iterator      begin() const;
    const_iterator      end() const;

    /** Returns whether the empire with ID \a id has been eliminated, or false
      * if no such empire exists. */
    bool                Eliminated(int id) const;

    DiplomaticStatus    GetDiplomaticStatus(int empire1, int empire2) const;

    std::string         Dump() const;
    //@}

    /** \name Mutators */ //@{
    /** Returns the empire whose ID is \a id, or 0 if none exists. */
    Empire*         Lookup(int id);

    iterator        begin();
    iterator        end();

    void            BackPropegateMeters();

    /** Marks the empire with ID \a id as eliminated, and cleans up that empire
      * if it exists (or does nothing if that empire doesn't exist).  Cleanup
      * involves clearing queues, resetting the capital, and cleaning up other
      * state info not relevant to an eliminated empire. */
    void            EliminateEmpire(int id);

    void            SetDiplomaticStatus(int empire1, int empire2, DiplomaticStatus status);
    void            ResetDiplomacy();

    /** Creates and inserts an empire with the specified properties and returns
      * a pointer to it.  This will only set up the data in Empire.  It is the
      * caller's responsibility to make sure that universe updates planet
      * ownership. */
    Empire*         CreateEmpire(int empire_id, const std::string& name, const std::string& player_name,
                                 const GG::Clr& color);

    /** Removes and deletes all empires from the manager. */
    void            Clear();
    //@}


private:
    /** Adds the given empire to the manager. */
    void            InsertEmpire(Empire* empire);

    std::map<int, Empire*>                          m_empire_map;
    std::set<int>                                   m_eliminated_empires;
    std::map<std::pair<int, int>, DiplomaticStatus> m_empire_diplomatic_statuses;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // _EmpireManager_h_
