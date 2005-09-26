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
    class_<OrderWrap,std::auto_ptr<OrderWrap>,boost::noncopyable>("Order")

	.def(init<int>())

	// Accessors

	.add_property("EmpireID", &Order::EmpireID)
	.def("Execute",&Order::Execute)
	.def("Undo",&Order::Undo)

	// Static Members

//	.def("InitOrderFactory",&Order::InitOrderFactory)
	//.staticmethod("InitOrderFactory")
	;
    implicitly_convertible<std::auto_ptr<OrderWrap>,std::auto_ptr<Order> >();


    class_<ResearchQueueOrder,std::auto_ptr<ResearchQueueOrder>,bases<Order>,
	boost::noncopyable>("ResearchQueueOrder")
	.def(init<int,const std::string&,optional<int> >())
	;
    implicitly_convertible<std::auto_ptr<ResearchQueueOrder>,std::auto_ptr<Order> >();

    class_<RenameOrder,std::auto_ptr<RenameOrder>,
	bases<Order>,boost::noncopyable>("RenameOrder")
	.def(init<int,int,const std::string&>())
	
	.add_property("ObjectID", &RenameOrder::ObjectID)
	.add_property("Name", make_function(&RenameOrder::Name,
					    return_internal_reference<>()))
	;
    implicitly_convertible<std::auto_ptr<RenameOrder>,std::auto_ptr<Order> >();

    class_<ProductionQueueOrder,std::auto_ptr<ProductionQueueOrder>,
bases<Order>,boost::noncopyable>("ProductionQueueOrder")
	.def(init<int,BuildType,const std::string&,int,int>())
	.def(init<int,int,int,bool>())
	.def(init<int,int,optional<int> >())
	;
    implicitly_convertible<std::auto_ptr<ProductionQueueOrder>,std::auto_ptr<Order> >();

    class_<NewFleetOrder, bases<Order>, std::auto_ptr<NewFleetOrder>, boost::noncopyable>("NewFleetOrder")
	.def(init<int,const std::string&,const int,int,optional<int> >()[with_custodian_and_ward<1,3>()])
	.def(init<int,const std::string&,const int,double,double,optional<int> >()[with_custodian_and_ward<1,3>()])

	.add_property("FleetName", make_function(&NewFleetOrder::FleetName,
						 return_internal_reference<>()))
	.add_property("SystemID", &NewFleetOrder::SystemID)
	.add_property("Position", &NewFleetOrder::Position)
	.add_property("NewID", &NewFleetOrder::NewID)
	.add_property("ShipID", &NewFleetOrder::ShipID)
	;
    implicitly_convertible<std::auto_ptr<NewFleetOrder>,std::auto_ptr<Order> >();
    
    class_<FleetTransferOrder, std::auto_ptr<FleetTransferOrder>,
	bases<Order>, boost::noncopyable>("FleetTransferOrder")
	.def(init<int,int,int,const std::vector<int>&>())

	.add_property("SourceFleet", &FleetTransferOrder::SourceFleet)
	.add_property("DestinationFleet", &FleetTransferOrder::DestinationFleet)
	.add_property("Ships", make_function(&FleetTransferOrder::Ships,
					     return_internal_reference<>()))
	;
    implicitly_convertible<std::auto_ptr<FleetTransferOrder>,std::auto_ptr<Order> >();

    class_<FleetMoveOrder,std::auto_ptr<FleetMoveOrder>,
	bases<Order>,boost::noncopyable>("FleetMoveOrder")
	.def(init<int,int,int,int>())

	.add_property("FleetID", &FleetMoveOrder::FleetID)
	.add_property("StartSystemID", &FleetMoveOrder::StartSystemID)
	.add_property("DestinationSystemID", &FleetMoveOrder::DestinationSystemID)
	.add_property("Route",make_function(&FleetMoveOrder::Route,
					    return_internal_reference<>()))
	.add_property("RouteLength",&FleetMoveOrder::RouteLength)
	;
    implicitly_convertible<std::auto_ptr<FleetMoveOrder>,std::auto_ptr<Order> >();
    
    class_<FleetColonizeOrder,std::auto_ptr<FleetColonizeOrder>,
	bases<Order>,boost::noncopyable>("FleetColonizeOrder")
	.def(init<int,int,int>())

	.add_property("PlanetID", &FleetColonizeOrder::PlanetID)
	.add_property("ShipID", &FleetColonizeOrder::ShipID)
	.def("ServerExecute",&FleetColonizeOrder::ServerExecute)
	;
    implicitly_convertible<std::auto_ptr<FleetColonizeOrder>,std::auto_ptr<Order> >();

    class_<DeleteFleetOrder,std::auto_ptr<DeleteFleetOrder>,
	bases<Order>,boost::noncopyable>("DeleteFleetOrder")
	.def(init<int,int>())
	.add_property("FleetID", &DeleteFleetOrder::FleetID)
	;
    implicitly_convertible<std::auto_ptr<DeleteFleetOrder>,std::auto_ptr<Order> >();

    class_<ChangeFocusOrder,std::auto_ptr<ChangeFocusOrder>,
	bases<Order>,boost::noncopyable>("ChangeFocusOrder")
	.def(init<int,int,FocusType,bool>())

	.add_property("PlanetID", &ChangeFocusOrder::PlanetID)

	;
    implicitly_convertible<std::auto_ptr<ChangeFocusOrder>,std::auto_ptr<Order> >();
}

