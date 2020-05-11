#include "Parse.h"


namespace YAML {
    bool convert<GG::Clr>::decode(const Node& node, GG::Clr& rhs) {
        if (!node.IsScalar())
            return false;

        try {
            rhs = GG::HexClr(node.as<std::string>());
        }
        catch(const std::exception& e) {
            return false;
        }

        return true;
    }
}
