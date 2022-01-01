#ifndef _UniverseObjectVisitors_h_
#define _UniverseObjectVisitors_h_


#include "UniverseObjectVisitor.h"
#include "ConstantsFwd.h"

#include <set>

class EmpireManager;

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


//! Returns obj iff @a obj is unowned.
struct FO_COMMON_API UnownedVisitor : UniverseObjectVisitor
{
    auto Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject> override;
    auto Visit(const std::shared_ptr<System>& obj) const -> std::shared_ptr<UniverseObject> override;
};


struct FO_COMMON_API HostileVisitor : UniverseObjectVisitor
{
    explicit HostileVisitor(int viewing_empire, const EmpireManager& empires_) :
        hostile_to_empire_id(viewing_empire),
        empires(empires_)
    {}
    auto Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject> override;

    int const hostile_to_empire_id = ALL_EMPIRES;
    const EmpireManager& empires;
};


//! Returns obj iff the ID of @a obj is NOT in either of the passed-in sets.
struct FO_COMMON_API NotInSetsVisitor : UniverseObjectVisitor
{
    explicit NotInSetsVisitor(const std::set<int>& set1_, const std::set<int>& set2_) :
        set1(set1_),
        set2(set2_)
    {}
    explicit NotInSetsVisitor(const std::set<int>& set1_) :
        set1(set1_)
    {}
    auto Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject> override;

    static const inline std::set<int> EMPTY_SET = {};

    const std::set<int>& set1;
    const std::set<int>& set2 = EMPTY_SET;
};

#endif
