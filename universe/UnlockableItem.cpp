#include "UnlockableItem.h"

#include "../util/CheckSums.h"
#include "../util/Logger.h"


std::string UnlockableItem::Dump(uint8_t) const {
    std::string retval = "Item type = ";
    switch (type) {
    case UnlockableItemType::UIT_BUILDING:    retval += "Building";   break;
    case UnlockableItemType::UIT_SHIP_PART:   retval += "ShipPart";   break;
    case UnlockableItemType::UIT_SHIP_HULL:   retval += "ShipHull";   break;
    case UnlockableItemType::UIT_SHIP_DESIGN: retval += "ShipDesign"; break;
    case UnlockableItemType::UIT_TECH:        retval += "Tech"    ;   break;
    default:                                  retval += "?"       ;   break;
    }
    retval += " name = \"" + name + "\"\n";
    return retval;
}

namespace CheckSums {
    void CheckSumCombine(uint32_t& sum, const UnlockableItem& item) {
        TraceLogger() << "CheckSumCombine(Slot): " << typeid(item).name();
        CheckSumCombine(sum, item.type);
        CheckSumCombine(sum, item.name);
    }
}
