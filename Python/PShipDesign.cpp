#include "ShipDesign.h"

#include <boost/python.hpp>

using namespace boost::python;

void ExportShipDesign()
{
    class_<ShipDesign>("ShipDesign")
	.def_readwrite("empire",&ShipDesign::empire)
	.def_readwrite("name",&ShipDesign::name)
	.def_readwrite("description",&ShipDesign::description)
	.def_readwrite("cost",&ShipDesign::cost)
	.def_readwrite("speed",&ShipDesign::speed)
	.def_readwrite("colonize",&ShipDesign::colonize)
	.def_readwrite("attack",&ShipDesign::attack)
	.def_readwrite("defense",&ShipDesign::defense)
	.def_readwrite("graphic",&ShipDesign::graphic)
	;
    
}
