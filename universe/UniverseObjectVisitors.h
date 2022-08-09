#ifndef _UniverseObjectVisitors_h_
#define _UniverseObjectVisitors_h_


#include "UniverseObjectVisitor.h"
#include "ConstantsFwd.h"
#include "UniverseObject.h"

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
template <typename S1, typename S2>
struct FO_COMMON_API NotInSetsVisitor : UniverseObjectVisitor
{
    NotInSetsVisitor(const S1& set1_, const S2& set2_) :
        set1(set1_),
        set2(set2_)
    {}
    auto Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject> override {
        int obj_id = obj->ID();
        if (set1.find(obj_id) != set1.end())
            return nullptr;
        return set2.find(obj_id) != set2.end() ? nullptr : obj;
    }

    const S1& set1;
    const S2& set2;
};


//! Returns obj iff the ID of @a obj is NOT in the passed-in set.
template <typename S>
struct FO_COMMON_API NotInSetVisitor : UniverseObjectVisitor
{
    explicit NotInSetVisitor(const S& set_) :
        set(set_)
    {}
    auto Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject> override
    { return set.find(obj->ID()) != set.end() ? nullptr : obj; }

    const S& set;
};

#endif
