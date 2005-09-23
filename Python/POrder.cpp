#include "Order.h"

#include <boost/python.hpp>

using namespace boost::python;

struct OrderWrap : Order, wrapper<Order>
{
    OrderWrap() : Order() {}
    OrderWrap(int e) : Order(e) {}

    void ExecuteImpl() const
	{
	    this->get_override("ExecuteImpl")();
	}
};


void ExportOrder()
{
    class_<OrderWrap,boost::noncopyable>("Order")

	.def(init<int>())

	// Accessors

	.add_property("EmpireID", &Order::EmpireID)
	.def("Execute",&Order::Execute)
	.def("Undo",&Order::Undo)

	// Static Members

//	.def("InitOrderFactory",&Order::InitOrderFactory)
	//.staticmethod("InitOrderFactory")
	;


    class_<ResearchQueueOrder,bases<Order>,boost::noncopyable>("ResearchQueueOrder")
	.def(init<int,const std::string&,optional<int> >())
	;


    class_<RenameOrder,bases<Order>,boost::noncopyable>("RenameOrder")
	.def(init<int,int,const std::string&>())
	
	.add_property("ObjectID", &RenameOrder::ObjectID)
	.add_property("Name", make_function(&RenameOrder::Name,
					    return_internal_reference<>()))
	;


    class_<ProductionQueueOrder,bases<Order>,boost::noncopyable>("ProductionQueueOrder")
	.def(init<int,BuildType,const std::string&,int,int>())
	.def(init<int,int,int,bool>())
	.def(init<int,int,optional<int> >())
	;


    class_<NewFleetOrder, bases<Order>, boost::noncopyable>("NewFleetOrder")
	.def(init<int,const std::string&,const int,int,optional<int> >())
	.def(init<int,const std::string&,const int,double,double,optional<int> >())

	.add_property("FleetName", make_function(&NewFleetOrder::FleetName,
						 return_internal_reference<>()))
	.add_property("SystemID", &NewFleetOrder::SystemID)
	.add_property("Position", &NewFleetOrder::Position)
	.add_property("NewID", &NewFleetOrder::NewID)
	.add_property("ShipID", &NewFleetOrder::ShipID)
	;
    
    
    class_<FleetTransferOrder, bases<Order>, boost::noncopyable>("FleetTransferOrder")
	.def(init<int,int,int,const std::vector<int>&>())

	.add_property("SourceFleet", &FleetTransferOrder::SourceFleet)
	.add_property("DestinationFleet", &FleetTransferOrder::DestinationFleet)
	.add_property("Ships", make_function(&FleetTransferOrder::Ships,
					     return_internal_reference<>()))
	;


    class_<FleetMoveOrder,bases<Order>,boost::noncopyable>("FleetMoveOrder")
	.def(init<int,int,int,int>())

	.add_property("FleetID", &FleetMoveOrder::FleetID)
	.add_property("StartSystemID", &FleetMoveOrder::StartSystemID)
	.add_property("DestinationSystemID", &FleetMoveOrder::DestinationSystemID)
	.add_property("Route",make_function(&FleetMoveOrder::Route,
					    return_internal_reference<>()))
	.add_property("RouteLength",&FleetMoveOrder::RouteLength)
	;

    class_<FleetColonizeOrder,bases<Order>,boost::noncopyable>("FleetColonizeOrder")
	.def(init<int,int,int>())

	.add_property("PlanetID", &FleetColonizeOrder::PlanetID)
	.add_property("ShipID", &FleetColonizeOrder::ShipID)
	.def("ServerExecute",&FleetColonizeOrder::ServerExecute)
	;

    class_<DeleteFleetOrder,bases<Order>,boost::noncopyable>("DeleteFleetOrder")
	.def(init<int,int>())


	.add_property("FleetID", &DeleteFleetOrder::FleetID)
	;

    class_<ChangeFocusOrder,bases<Order>,boost::noncopyable>("ChangeFocusOrder")
	.def(init<int,int,FocusType,bool>())

	.add_property("PlanetID", &ChangeFocusOrder::PlanetID)

	;
}

