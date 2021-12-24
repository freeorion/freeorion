#ifndef _UniverseObjectVisitor_h_
#define _UniverseObjectVisitor_h_


#include <memory>
#include "../util/Export.h"


class UniverseObject;
class Building;
class Fleet;
class Planet;
class Ship;
class System;
class Field;
class Fighter;


//! Base class for UniverseObject visitores.
//!
//! These visitors have Visit() overloads for each type in the UniversObject-based
//! class herarchy.  Calling Visit() returns the @p obj parameter, if some
//! predicate is true of that object.  Each UniverseObject subclass needs to
//! have an Accept(const UniverseObjectVisitor& visitor) method that consists
//! only of "visitor->Visit(this)".  Because of the specific-type overloads,
//! passing a UniverseObjectVisitor into the Accept() method of
//! a UniverseObject will cause the UniverseObjectVisitor's appropriate Visit()
//! method to be called.  Since the specific type of the @p obj parameter is
//! known within each Visit() method,@p obj can be accessed by type, without
//! using a dynamic_cast.  Note that is is therefore safe to static_cast
//! a UniversObject pointer that is returned from a UniverseObjectVisitor
//! subclass that only returns a nonzero for one specific UniverseObject
//! subclass (e.g. StationaryFleetVisitor<Planet>).  The default behavior of
//! all Visit() methods besides Visit(UniverseObject*) is to return the result
//! of a call to Visit(UniverseObject*).  This means that UniverseObjectVisitor
//! subclasses can override Visit(UniverseObject*) only, and calls to all
//! Visit() overloads will work.  The default return value for
//! Visit(UniverseObject*) is 0, so overridding any @p one Visit() method
//! besides this one will ensure that only UniverseObjects of a single subclass
//! are recognized by the visitor.
struct FO_COMMON_API UniverseObjectVisitor {
    virtual ~UniverseObjectVisitor() = default;

    virtual auto Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject>;
    virtual auto Visit(const std::shared_ptr<Building>& obj) const -> std::shared_ptr<UniverseObject>;
    virtual auto Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject>;
    virtual auto Visit(const std::shared_ptr<Planet>& obj) const -> std::shared_ptr<UniverseObject>;
    virtual auto Visit(const std::shared_ptr<Ship>& obj) const -> std::shared_ptr<UniverseObject>;
    virtual auto Visit(const std::shared_ptr<System>& obj) const -> std::shared_ptr<UniverseObject>;
    virtual auto Visit(const std::shared_ptr<Field>& obj) const -> std::shared_ptr<UniverseObject>;
    virtual auto Visit(const std::shared_ptr<Fighter>& obj) const -> std::shared_ptr<UniverseObject>;
};


#endif
