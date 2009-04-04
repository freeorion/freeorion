// -*- C++ -*-
#ifndef _Predicates_h_
#define _Predicates_h_

class UniverseObject;
class Building;
class Fleet;
class Planet;
class Ship;
class System;

#include <boost/mpl/if.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include <string>

extern const int ALL_EMPIRES;

/** a more efficient replacement for dynamic_cast that only works for UniverseObject and its subclasses */
template <class T1, class T2>
T1 universe_object_cast(T2 ptr);

/** the base class for UniverseObject visitor classes.  These visitors have Visit() overloads for each type in the UniversObject-based
    class herarchy.  Calling Visit() returns the \a obj parameter, if some predicate is true of that object.  Each UniverseObject
    subclass needs to have an Accept(const UniverseObjectVisitor& visitor) method that consists only of "visitor->Visit(this)".  Because
    of the specific-type overloads, passing a UniverseObjectVisitor into the Accept() method of a UniverseObject will cause the
    UniverseObjectVisitor's appropriate Visit() method to be called.  Since the specific type of the \a obj parameter is known within
    each Visit() method, \a obj can be accessed by type, without using a dynamic_cast.  Note that is is therefore safe to static_cast a
    UniversObject pointer that is returned from a UniverseObjectVisitor subclass that only returns a nonzero for one specific
    UniverseObject subclass (e.g. UniverseObjectSubclassVisitor<Planet>).  The default behavior of all Visit() methods besides
    Visit(UniverseObject*) is to return the result of a call to Visit(UniverseObject*).  This means that UniverseObjectVisitor
    subclasses can override Visit(UniverseObject*) only, and calls to all Visit() overloads will work.  The default return value for
    Visit(UniverseObject*) is 0, so overridding any \a one Visit() method besides this one will ensure that only UniverseObjects
    of a single subclass are recognized by the visitor. */
struct UniverseObjectVisitor
{
    virtual UniverseObject* Visit(UniverseObject* obj) const;
    virtual UniverseObject* Visit(Building* obj) const;
    virtual UniverseObject* Visit(Fleet* obj) const;
    virtual UniverseObject* Visit(Planet* obj) const;
    virtual UniverseObject* Visit(Ship* obj) const;
    virtual UniverseObject* Visit(System* obj) const;
    virtual ~UniverseObjectVisitor();
};

/** returns obj iff \a obj is of type T */
template <class T>
struct UniverseObjectSubclassVisitor : UniverseObjectVisitor
{
    virtual UniverseObject* Visit(T* obj) const;
    virtual ~UniverseObjectSubclassVisitor() {}
};

/** returns obj iff \a obj is a Fleet belonging to the given empire object that is parked at a System, not under orders to move.  
    If the given empire is -1, all orderd moving fleets will be returned.  Note that it is preferable to use this functor on System
    searches, rather than Universe ones. */
struct StationaryFleetVisitor : UniverseObjectVisitor
{
    StationaryFleetVisitor(int empire = ALL_EMPIRES);
    virtual UniverseObject* Visit(Fleet* obj) const;
    virtual ~StationaryFleetVisitor();
    const int empire_id;
};

/** returns obj iff \a obj is a Fleet belonging to the given empire, and that is under orders to move, but is not yet moving. 
    If the given empire is -1, all stationary fleets will be returned.  Note that it is preferable to use this functor on System
    searches, rather than Universe ones. */
struct OrderedMovingFleetVisitor : UniverseObjectVisitor
{
    OrderedMovingFleetVisitor(int empire = ALL_EMPIRES);
    virtual UniverseObject* Visit(Fleet* obj) const;
    virtual ~OrderedMovingFleetVisitor();
    const int empire_id;
};

/** returns obj iff \a obj is a moving Fleet belonging to the given empire, and that is moving between systems.  
    If the given empire is -1, all moving fleets will be returned. */
struct MovingFleetVisitor : UniverseObjectVisitor
{
    MovingFleetVisitor(int empire = ALL_EMPIRES);
    virtual UniverseObject* Visit(Fleet* obj) const;
    virtual ~MovingFleetVisitor();
    const int empire_id;
};

/** returns obj iff \a obj is owned by the empire with id \a empire, and \a obj is of type T. */
template <class T>
struct OwnedVisitor : UniverseObjectVisitor
{
    OwnedVisitor(int empire = ALL_EMPIRES);
    virtual T* Visit(T* obj) const;
    virtual ~OwnedVisitor() {} 
    const int empire_id;
};

/** returns obj iff \a obj is owned by \a only the empire with id \a empire, and \a obj is of type T. */
template <class T>
struct WhollyOwnedVisitor : UniverseObjectVisitor
{
    WhollyOwnedVisitor(int empire = ALL_EMPIRES);
    virtual T* Visit(T* obj) const;
    virtual ~WhollyOwnedVisitor() {}
    const int empire_id;
};

// template implementations

template <class T1, class T2>
T1 universe_object_cast(T2 ptr)
{
    using namespace boost;

    typedef typename add_pointer<
        typename remove_const<
            typename remove_pointer<
                T2
            >::type
        >::type
    >::type T2ConstFreeType;
    typedef typename add_pointer<
        typename remove_const<
            typename remove_pointer<
                T1
            >::type
        >::type
    >::type T1ConstFreeType;
    typedef UniverseObjectSubclassVisitor<typename remove_pointer<T1ConstFreeType>::type> VisitorType;

    return static_cast<T1ConstFreeType>(ptr->Accept(VisitorType()));
}

template <class T>
UniverseObject* UniverseObjectSubclassVisitor<T>::Visit(T* obj) const
{
    return obj;
}

template <class T>
OwnedVisitor<T>::OwnedVisitor(int empire) :
    empire_id(empire)
{
}

template <class T>
T* OwnedVisitor<T>::Visit(T* obj) const
{
    if (obj->OwnedBy(empire_id))
        return obj;
    return 0;
}

template <class T>
WhollyOwnedVisitor<T>::WhollyOwnedVisitor(int empire) :
    empire_id(empire)
{
}

template <class T>
T* WhollyOwnedVisitor<T>::Visit(T* obj) const
{
    if (obj->WhollyOwnedBy(empire_id))
        return obj;
    return 0;
}

#endif // _Predicates_h_
