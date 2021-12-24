#ifndef _UniverseObjectVisitors_h_
#define _UniverseObjectVisitors_h_


#include "UniverseObjectVisitor.h"
#include "ConstantsFwd.h"

//! Returns obj iff @a obj is a Fleet belonging to the given empire that is
//! parked at a System, not under orders to move.
//!
//! If the given empire is ALL_EMPIRES, all orderd moving fleets will be
//! returned.
//
//! Note that it is preferable to use this functor on System searches, rather
//! than Universe ones.
struct FO_COMMON_API StationaryFleetVisitor : UniverseObjectVisitor
{
    explicit StationaryFleetVisitor(int empire = ALL_EMPIRES) :
        empire_id(empire)
    {}
    auto Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject> override;

    int const empire_id = ALL_EMPIRES;
};


//! Returns obj iff @a obj is a Fleet belonging to the given empire, and that
//! is under orders to move, but is not yet moving.
//!
//! If the given empire is ALL_EMPIRES, all stationary fleets will be returned.
//!
//! Note that it is preferable to use this functor on System searches, rather
//! than Universe ones.
struct FO_COMMON_API OrderedMovingFleetVisitor : UniverseObjectVisitor
{
    explicit OrderedMovingFleetVisitor(int empire = ALL_EMPIRES) :
        empire_id(empire)
    {}
    auto Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject> override;

    int const empire_id = ALL_EMPIRES;
};


//! Returns obj iff @a obj is a moving Fleet belonging to the given empire,
//! and that is moving between systems.
//!
//! If the given empire is ALL_EMPIRE, all moving fleets will be returned.
struct FO_COMMON_API MovingFleetVisitor : UniverseObjectVisitor
{
    explicit MovingFleetVisitor(int empire = ALL_EMPIRES) :
        empire_id(empire)
    {}
    auto Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject> override;

    int const empire_id = ALL_EMPIRES;
};


//! Returns obj iff @a obj is owned by the empire with id @a empire_id.
struct FO_COMMON_API OwnedVisitor : UniverseObjectVisitor
{
    explicit OwnedVisitor(int empire = ALL_EMPIRES) :
        empire_id(empire)
    {}
    auto Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject> override;

    int const empire_id = ALL_EMPIRES;
};


struct FO_COMMON_API HostileVisitor : UniverseObjectVisitor
{
    explicit HostileVisitor(int viewing_empire, int owning_empire = ALL_EMPIRES) :
        viewing_empire_id(viewing_empire),
        owning_empire_id(owning_empire)
    {}
    auto Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject> override;

    int const viewing_empire_id = ALL_EMPIRES;
    int const owning_empire_id = ALL_EMPIRES;
};


#endif
