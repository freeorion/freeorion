// -*- C++ -*-
#ifndef _Predicates_h_
#define _Predicates_h_

class UniverseObject;

/** returns true iff \a obj is a System object. */
bool IsSystem(const UniverseObject* obj);

/** returns true iff \a obj is a Planet object. */
bool IsPlanet(const UniverseObject* obj);

/** returns true iff \a obj is a Fleet object. */
bool IsFleet(const UniverseObject* obj);

/** returns true iff \a obj is a Ship object. */
bool IsShip(const UniverseObject* obj);

/** returns true iff \a obj is a Fleet belonging to the given empire object that is under orders to move, but is not yet moving. 
    If the given empire is -1, all stationary fleets will be returned.  Note that it is preferable to use this functor on System
    searches, rather than Universe ones. */
struct IsStationaryFleetFunctor
{
    IsStationaryFleetFunctor(int empire) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const;
    const int empire_id;
};

/** returns true iff \a obj is a Fleet belonging to the given empire object that is parked at a System, not under orders to move.  
    If the given empire is -1, all orderd moving fleets will be returned.  Note that it is preferable to use this functor on System
    searches, rather than Universe ones. */
struct IsOrderedMovingFleetFunctor
{
    IsOrderedMovingFleetFunctor(int empire) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const;
    const int empire_id;
};

/** returns true iff \a obj is a moving Fleet belonging to the given empire object that is moving between systems.  
    If the given empire is -1, all moving fleets will be returned. */
struct IsMovingFleetFunctor
{
    IsMovingFleetFunctor(int empire) : empire_id(empire) {}
    bool operator()(const UniverseObject* obj) const;
    const int empire_id;
};

#endif // _Predicates_h_
