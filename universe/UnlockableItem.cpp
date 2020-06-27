#include "UnlockableItem.h"

#include "../util/CheckSums.h"
#include "../util/Logger.h"


std::string UnlockableItem::Dump(unsigned short ntabs) const {
    std::string retval = "Item type = ";
    switch (type) {
    case UIT_BUILDING:      retval += "Building";   break;
    case UIT_SHIP_PART:     retval += "ShipPart";   break;
    case UIT_SHIP_HULL:     retval += "ShipHull";   break;
    case UIT_SHIP_DESIGN:   retval += "ShipDesign"; break;
    case UIT_TECH:          retval += "Tech"    ;   break;
    default:                retval += "?"       ;   break;
    }
    retval += " name = \"" + name + "\"\n";
    return retval;
}

bool operator==(const UnlockableItem& lhs, const UnlockableItem& rhs) {
    return lhs.type == rhs.type &&
    lhs.name == rhs.name;
}

bool operator!=(const UnlockableItem& lhs, const UnlockableItem& rhs)
{ return !(lhs == rhs); }


namespace CheckSums {
    void CheckSumCombine(unsigned int& sum, const UnlockableItem& item) {
        TraceLogger() << "CheckSumCombine(Slot): " << typeid(item).name();
        CheckSumCombine(sum, item.type);
        CheckSumCombine(sum, item.name);
    }
}
