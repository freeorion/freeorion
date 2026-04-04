#ifndef _Visibility_h_
#define _Visibility_h_

#include <algorithm>
#include <map>
#include <optional>
#include <vector>
#include <boost/container/flat_map.hpp>
#include "ConstantsFwd.h"
#include "EnumsFwd.h"



//! Degrees of visibility an Empire can have for an UniverseObject.
//! Determines how much information the empire gets about the object.
FO_ENUM(
    (Visibility),
    ((INVALID_VISIBILITY, -1))
    ((VIS_NO_VISIBILITY))
    ((VIS_BASIC_VISIBILITY))
    ((VIS_PARTIAL_VISIBILITY))
    ((VIS_FULL_VISIBILITY))
    ((NUM_VISIBILITIES))
)


class Visibilities {
    using int_vis_map_t = boost::container::flat_map<int, Visibility>;
    int_vis_map_t ids_vis;

public:
    using const_iterator = int_vis_map_t::const_iterator;
    using const_reference = int_vis_map_t::const_reference;
    using reference = const_reference;
    using const_pointer = int_vis_map_t::const_pointer;
    using pointer = const_pointer;

    Visibilities() noexcept = default;
    explicit Visibilities(std::string_view str);
    Visibilities(std::pair<int, Visibility> val) :
        ids_vis({val})
    {}

    [[nodiscard]] const_iterator begin() const noexcept { return ids_vis.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return ids_vis.end(); }
    [[nodiscard]] bool empty() const noexcept { return ids_vis.empty(); }

    Visibility Get(int id) const {
        auto it = ids_vis.find(id);
        if (it == ids_vis.end())
            return Visibility::VIS_NO_VISIBILITY;
        return it->second;
    }
    std::optional<Visibility> GetIfSet(int id) const {
        auto it = ids_vis.find(id);
        if (it == ids_vis.end())
            return std::nullopt;
        return it->second;
    }

    std::size_t Size() const noexcept { return ids_vis.size(); }
    std::size_t SizeInMemory() const noexcept { return ids_vis.capacity() * sizeof(decltype(ids_vis)::value_type); }

    void Set(int id, Visibility vs) { ids_vis.insert_or_assign(id, vs); }
    void Set(const Visibilities& rhs) {
        ids_vis.reserve(ids_vis.size() + rhs.Size());
        for (auto& [id, vs] : rhs.ids_vis)
            ids_vis.insert_or_assign(id, vs);
    }
    // set, but don't lower value if already present. return final vis value.
    Visibility SetOrIncrease(int id, Visibility vs) {
        auto [it, is_new] = ids_vis.try_emplace(id, vs);
        if (!is_new && it->second < vs)
            it->second = vs;
        return it->second;
    }

    std::string ToString() const;

    template <typename Archive>
    friend void serialize(Archive&, Visibilities&, unsigned int const);
};
using EmpireObjectVisibilityMap = std::map<int, Visibilities>; ///< map from empire id to ObjectVisibilityMap for that empire


struct ObjVisTurns {
    int obj_id = INVALID_OBJECT_ID;
    int basic = INVALID_GAME_TURN;
    int partial = INVALID_GAME_TURN;
    int full = INVALID_GAME_TURN;

    constexpr ObjVisTurns() noexcept = default;
    constexpr explicit ObjVisTurns(int obj_id_) noexcept :
        obj_id(obj_id_)
    {}
    constexpr ObjVisTurns(int obj_id_, int basic_turn, int partial_turn, int full_turn) noexcept :
        obj_id(obj_id_), basic(basic_turn), partial(partial_turn), full(full_turn)
    {}
    constexpr ObjVisTurns(int obj_id_, Visibility vis, int turn) noexcept :
        obj_id(obj_id_),
        basic(vis >= Visibility::VIS_BASIC_VISIBILITY ? turn : INVALID_GAME_TURN),
        partial(vis >= Visibility::VIS_PARTIAL_VISIBILITY ? turn : INVALID_GAME_TURN),
        full(vis >= Visibility::VIS_FULL_VISIBILITY ? turn : INVALID_GAME_TURN)
    {}

    constexpr bool operator<(const ObjVisTurns& rhs) noexcept { return obj_id < rhs.obj_id; };
    constexpr bool operator==(const ObjVisTurns& rhs) noexcept { return obj_id == rhs.obj_id; };

    constexpr int& operator[](Visibility vis) noexcept {
        switch (vis) {
        case Visibility::VIS_FULL_VISIBILITY:    return full;
        case Visibility::VIS_PARTIAL_VISIBILITY: return partial;
        default:                                 return basic;
        }
    }
    [[nodiscard]] constexpr int operator[](Visibility vis) const noexcept {
        switch (vis) {
        case Visibility::VIS_FULL_VISIBILITY:    return full;
        case Visibility::VIS_PARTIAL_VISIBILITY: return partial;
        default:                                 return basic;
        }
    }

    [[nodiscard]] constexpr bool contains(Visibility vis) const noexcept { return operator[](vis) != INVALID_GAME_TURN; }
    [[nodiscard]] constexpr bool empty() const noexcept { return obj_id == INVALID_OBJECT_ID || basic == INVALID_GAME_TURN; }

    // sets turn for \a vis and lower visibilities to \a turn
    constexpr void SetVisTurnsCascade(Visibility vis, int turn) noexcept {
        switch (vis) {
        case Visibility::VIS_FULL_VISIBILITY:    full = std::max(full, turn); [[fallthrough]];
        case Visibility::VIS_PARTIAL_VISIBILITY: partial = std::max(partial, turn); [[fallthrough]];
        case Visibility::VIS_BASIC_VISIBILITY:   basic = std::max(basic, turn); break;
        default: break;
        }
    }
};
using EmpireObjectVisibilityTurnsVecMap = std::map<int, std::vector<ObjVisTurns>>;

#endif
