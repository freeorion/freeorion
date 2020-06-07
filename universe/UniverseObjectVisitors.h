#ifndef _UniverseObjectVisitors_h_
#define _UniverseObjectVisitors_h_


#include "UniverseObjectVisitor.h"


FO_COMMON_API extern const int ALL_EMPIRES;


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
    StationaryFleetVisitor(int empire = ALL_EMPIRES);

    virtual ~StationaryFleetVisitor();

    auto Visit(std::shared_ptr<Fleet> obj) const -> std::shared_ptr<UniverseObject> override;

    int const empire_id;
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
    OrderedMovingFleetVisitor(int empire = ALL_EMPIRES);

    virtual ~OrderedMovingFleetVisitor();

    auto Visit(std::shared_ptr<Fleet> obj) const -> std::shared_ptr<UniverseObject> override;

    int const empire_id;
};


//! Returns obj iff @a obj is a moving Fleet belonging to the given empire,
//! and that is moving between systems.
//!
//! If the given empire is ALL_EMPIRE, all moving fleets will be returned.
struct FO_COMMON_API MovingFleetVisitor : UniverseObjectVisitor
{
    MovingFleetVisitor(int empire = ALL_EMPIRES);

    virtual ~MovingFleetVisitor();

    auto Visit(std::shared_ptr<Fleet> obj) const -> std::shared_ptr<UniverseObject> override;

    int const empire_id;
};


//! Returns obj iff @a obj is owned by the empire with id @a empire_id.
struct FO_COMMON_API OwnedVisitor : UniverseObjectVisitor
{
    OwnedVisitor(int empire = ALL_EMPIRES);

    virtual ~OwnedVisitor();

    auto Visit(std::shared_ptr<UniverseObject> obj) const -> std::shared_ptr<UniverseObject> override;

    int const empire_id;
};


struct FO_COMMON_API HostileVisitor : UniverseObjectVisitor
{
    HostileVisitor(int viewing_empire, int owning_empire = ALL_EMPIRES);

    virtual ~HostileVisitor();

    auto Visit(std::shared_ptr<UniverseObject> obj) const -> std::shared_ptr<UniverseObject> override;

    int const viewing_empire_id;
    int const owning_empire_id;
};


#endif // _UniverseObjectVisitors_h_
