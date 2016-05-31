#include "EnumText.h"

#include "util/i18n.h"

namespace{
    const std::string EMPTY_STRING;
}

const std::string& TextForGalaxySetupSetting(GalaxySetupOption gso) {
    switch (gso) {
        case GALAXY_SETUP_NONE:     return UserString("GSETUP_NONE");
        case GALAXY_SETUP_LOW:      return UserString("GSETUP_LOW");
        case GALAXY_SETUP_MEDIUM:   return UserString("GSETUP_MEDIUM");
        case GALAXY_SETUP_HIGH:     return UserString("GSETUP_HIGH");
        case GALAXY_SETUP_RANDOM:   return UserString("GSETUP_RANDOM");
        default:                    return EMPTY_STRING;
    }
}

const std::string& TextForGalaxyShape(Shape shape) {
    switch (shape) {
        case SPIRAL_2:      return UserString("GSETUP_2ARM");
        case SPIRAL_3:      return UserString("GSETUP_3ARM");
        case SPIRAL_4:      return UserString("GSETUP_4ARM");
        case CLUSTER:       return UserString("GSETUP_CLUSTER");
        case ELLIPTICAL:    return UserString("GSETUP_ELLIPTICAL");
        case DISC:          return UserString("GSETUP_DISC");
        case BOX:           return UserString("GSETUP_BOX");
        case IRREGULAR:     return UserString("GSETUP_IRREGULAR");
        case RING:          return UserString("GSETUP_RING");
        case RANDOM:        return UserString("GSETUP_RANDOM");
        default:            return EMPTY_STRING;
    }
}

const std::string& TextForAIAggression(Aggression a) {
    switch (a) {
        case BEGINNER:      return UserString("GSETUP_BEGINNER");
        case TURTLE:        return UserString("GSETUP_TURTLE");
        case CAUTIOUS:      return UserString("GSETUP_CAUTIOUS");
        case TYPICAL:       return UserString("GSETUP_TYPICAL");
        case AGGRESSIVE:    return UserString("GSETUP_AGGRESSIVE");
        case MANIACAL:      return UserString("GSETUP_MANIACAL");
        default:            return EMPTY_STRING;
    }
}
