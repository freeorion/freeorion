#ifndef _UnlockableItem_h_
#define _UnlockableItem_h_


#include "../util/Enum.h"
#include "../util/Export.h"
#include <compare>
#include <cstdint>


/** types of items that can be unlocked for empires */
FO_ENUM(
    (UnlockableItemType),
    ((INVALID_UNLOCKABLE_ITEM_TYPE, -1))
    ((UIT_BUILDING))               ///< a kind of Building
    ((UIT_SHIP_PART))              ///< a kind of ship part (which are placed into hulls to make designs)
    ((UIT_SHIP_HULL))              ///< a ship hull (into which parts are placed)
    ((UIT_SHIP_DESIGN))            ///< a complete ship design
    ((UIT_TECH))                   ///< a technology
    ((UIT_POLICY))                 ///< an imperial policy
    ((NUM_UNLOCKABLE_ITEM_TYPES))  ///< keep last, the number of types of unlockable item
)


//! Specifies single item of game content that may be unlocked for an empire.
//! The @a type field stores the type of item that is being unlocked, such as
//! building or ship component, and the  @a name field contains the name of the
//! actual item (e.g. (UnlockableItemType::UIT_BUILDING, "Superfarm") or
//! (UnlockableItemType::UIT_SHIP_PART, "Death Ray")).
struct FO_COMMON_API UnlockableItem {
    UnlockableItem() = default;

    template <typename S>
    UnlockableItem(UnlockableItemType type_, S&& name_) :
        type(type_),
        name(std::forward<S>(name_))
    {}

#if defined(__cpp_impl_three_way_comparison) && (!defined(__clang_major__) || (__clang_major__ >= 11)) && (!defined(__GNUC__) || (__GNUC__ >= 10) || (defined(__clang_major__)))
#  if !defined(__cpp_lib_three_way_comparison)
    [[nodiscard]] std::strong_ordering operator<=>(const UnlockableItem&) const noexcept = default;
#  else
    [[nodiscard]] auto operator<=>(const UnlockableItem&) const noexcept = default;
#  endif
#else
    [[nodiscard]] bool operator==(const UnlockableItem&) const noexcept { return type == rhs.type && name == rhs.name; }
    [[nodiscard]] bool operator<(const UnlockableItem& rhs) const noexcept { return type < rhs.type || (type == rhs.type && name < rhs.name); }
#endif

    //! Returns a data file format representation of this object
    auto Dump(uint8_t ntabs = 0) const -> std::string;

    UnlockableItemType type = UnlockableItemType::INVALID_UNLOCKABLE_ITEM_TYPE; //! The kind of item this is
    std::string name; //! the exact item this is
};

namespace CheckSums {
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, const UnlockableItem& item);
}


#endif
