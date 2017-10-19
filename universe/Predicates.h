#ifndef _Predicates_h_
#define _Predicates_h_


#include "../util/Export.h"

#include <memory>


class UniverseObject;
class Building;
class Fleet;
class Planet;
class Ship;
class System;
class Field;
class Fighter;


FO_COMMON_API extern const int ALL_EMPIRES;

/** the base class for UniverseObject visitor classes.  These visitors have Visit() overloads for each type in the UniversObject-based
    class herarchy.  Calling Visit() returns the \a obj parameter, if some predicate is true of that object.  Each UniverseObject
    subclass needs to have an Accept(const UniverseObjectVisitor& visitor) method that consists only of "visitor->Visit(this)".  Because
    of the specific-type overloads, passing a UniverseObjectVisitor into the Accept() method of a UniverseObject will cause the
    UniverseObjectVisitor's appropriate Visit() method to be called.  Since the specific type of the \a obj parameter is known within
    each Visit() method, \a obj can be accessed by type, without using a dynamic_cast.  Note that is is therefore safe to static_cast a
    UniversObject pointer that is returned from a UniverseObjectVisitor subclass that only returns a nonzero for one specific
    UniverseObject subclass (e.g. StationaryFleetVisitor<Planet>).  The default behavior of all Visit() methods besides
    Visit(UniverseObject*) is to return the result of a call to Visit(UniverseObject*).  This means that UniverseObjectVisitor
    subclasses can override Visit(UniverseObject*) only, and calls to all Visit() overloads will work.  The default return value for
    Visit(UniverseObject*) is 0, so overridding any \a one Visit() method besides this one will ensure that only UniverseObjects
    of a single subclass are recognized by the visitor. */
struct FO_COMMON_API UniverseObjectVisitor {
    virtual std::shared_ptr<UniverseObject> Visit(std::shared_ptr<UniverseObject> obj) const;

    virtual std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Building> obj) const;

    virtual std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Fleet> obj) const;

    virtual std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Planet> obj) const;

    virtual std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Ship> obj) const;

    virtual std::shared_ptr<UniverseObject> Visit(std::shared_ptr<System> obj) const;

    virtual std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Field> obj) const;

    virtual std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Fighter> obj) const;

    virtual ~UniverseObjectVisitor();
};

/** returns obj iff \a obj is a Fleet belonging to the given empire object that is parked at a System, not under orders to move.  
    If the given empire is -1, all orderd moving fleets will be returned.  Note that it is preferable to use this functor on System
    searches, rather than Universe ones. */
struct FO_COMMON_API StationaryFleetVisitor : UniverseObjectVisitor
{
    StationaryFleetVisitor(int empire = ALL_EMPIRES);

    virtual ~StationaryFleetVisitor();

    std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Fleet> obj) const override;

    const int empire_id;
};

/** returns obj iff \a obj is a Fleet belonging to the given empire, and that is under orders to move, but is not yet moving. 
    If the given empire is -1, all stationary fleets will be returned.  Note that it is preferable to use this functor on System
    searches, rather than Universe ones. */
struct FO_COMMON_API OrderedMovingFleetVisitor : UniverseObjectVisitor
{
    OrderedMovingFleetVisitor(int empire = ALL_EMPIRES);

    virtual ~OrderedMovingFleetVisitor();

    std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Fleet> obj) const override;

    const int empire_id;
};

/** returns obj iff \a obj is a moving Fleet belonging to the given empire, and that is moving between systems.  
    If the given empire is -1, all moving fleets will be returned. */
struct FO_COMMON_API MovingFleetVisitor : UniverseObjectVisitor
{
    MovingFleetVisitor(int empire = ALL_EMPIRES);

    virtual ~MovingFleetVisitor();

    std::shared_ptr<UniverseObject> Visit(std::shared_ptr<Fleet> obj) const override;

    const int empire_id;
};

/** returns obj iff \a obj is owned by the empire with id \a empire, and \a obj is of type T. */
template <class T>
struct OwnedVisitor : UniverseObjectVisitor
{
    OwnedVisitor(int empire = ALL_EMPIRES);

    virtual ~OwnedVisitor()
    {}

    std::shared_ptr<UniverseObject> Visit(std::shared_ptr<T> obj) const override;

    const int empire_id;
};

template <class T>
OwnedVisitor<T>::OwnedVisitor(int empire) :
    empire_id(empire)
{}

template <class T>
std::shared_ptr<UniverseObject> OwnedVisitor<T>::Visit(std::shared_ptr<T> obj) const
{
    if (obj->OwnedBy(empire_id))
        return obj;
    return nullptr;
}

template <class T>
struct HostileVisitor : UniverseObjectVisitor
{
    HostileVisitor(int viewing_empire, int owning_empire = ALL_EMPIRES);

    virtual ~HostileVisitor()
    {}

    std::shared_ptr<UniverseObject> Visit(std::shared_ptr<T> obj) const override;

    const int viewing_empire_id;
    const int owning_empire_id;
};

template <class T>
HostileVisitor<T>::HostileVisitor(int viewing_empire, int owning_empire) :
    viewing_empire_id(viewing_empire),
    owning_empire_id(owning_empire)
{}

template <class T>
std::shared_ptr<UniverseObject> HostileVisitor<T>::Visit(std::shared_ptr<T> obj) const
{
    if (obj->HostileToEmpire(viewing_empire_id))
        return obj;
    return nullptr;
}

#endif // _Predicates_h_
