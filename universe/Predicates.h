// -*- C++ -*-
#ifndef _Predicates_h_
#define _Predicates_h_

#if 10*__GNUC__ + __GNUC_MINOR__ > 33
# ifndef _UniverseObject_h_
#  include "../universe/UniverseObject.h"
# endif
#else
  class UniverseObject;
#endif

/** returns true iff \a obj is a Fleet belonging to the given empire object that is under orders to move, but is not yet moving. 
    If the given empire is -1, all stationary fleets will be returned.  Note that it is preferable to use this functor on System
    searches, rather than Universe ones. */
struct IsStationaryFleetFunctor
{
    IsStationaryFleetFunctor(int empire = -1) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const;
    const int empire_id;
};

/** returns true iff \a obj is a Fleet belonging to the given empire object that is parked at a System, not under orders to move.  
    If the given empire is -1, all orderd moving fleets will be returned.  Note that it is preferable to use this functor on System
    searches, rather than Universe ones. */
struct IsOrderedMovingFleetFunctor
{
    IsOrderedMovingFleetFunctor(int empire = -1) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const;
    const int empire_id;
};

/** returns true iff \a obj is a moving Fleet belonging to the given empire object that is moving between systems.  
    If the given empire is -1, all moving fleets will be returned. */
struct IsMovingFleetFunctor
{
    IsMovingFleetFunctor(int empire = -1) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const;
    const int empire_id;
};

/** returns true iff the given empire is an owner of \a obj. */
struct IsOwnedByFunctor
{
    IsOwnedByFunctor(int empire = -1) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const;
    const int empire_id;
};

/** returns true iff the given empire is the only owner of \a obj. */
struct IsWhollyOwnedByFunctor
{
    IsWhollyOwnedByFunctor(int empire = -1) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const;
    const int empire_id;
};

/** returns true iff \a obj is owned by the empire with id \a empire, and \a obj is of type T. */
template <class T> 
struct IsOwnedObjectFunctor
{
    IsOwnedObjectFunctor(int empire) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const 
    {
        return obj->Owners().find(empire_id) != obj->Owners().end() && dynamic_cast<const T*>(obj);
    }
    const int empire_id;
};

#endif // _Predicates_h_
