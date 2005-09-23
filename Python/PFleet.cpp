#include "System.h"
#include "Fleet.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace 
{
struct FleetWrap : Fleet, wrapper<Fleet>
{
    FleetWrap() : Fleet() {}
    FleetWrap(const std::string& n, double x, double y, int o)
	: Fleet(n,x,y,o) {}
        
// 	Meter* GetMeter(MeterType type)
// 	{
// 	    if (override o = this->get_override("GetMeter"))
// 		return o(type);
// 	    return Fleet::GetMeter(type);
// 	}
// 	Meter* default_GetMeter(MeterType type) {
// 	    return this->Fleet::GetMeter(type);
// 	}

    Visibility GetVisibility(int id) const
	{
	    if (override o = this->get_override("GetVisibility"))
		return o(id);
	    return Fleet::GetVisibility(id);
	}
    Visibility default_GetVisibility(int id) const
	{
	    return this->Fleet::GetVisibility(id);
	}
	
    UniverseObject* Accept(const UniverseObjectVisitor& vis) const
	{
	    if (override o = this->get_override("Accept"))
		return o(vis);
	    return Fleet::Accept(vis);
	}
    UniverseObject* default_Accept(const UniverseObjectVisitor& vis) const
	{
	    return this->Fleet::Accept(vis);
	}

    void AddOwner(int id)
	{
	    if (override o = this->get_override("AddOwner"))
		o(id);
	    else
		Fleet::AddOwner(id);
	}
    void default_AddOwner(int id)
	{
	    this->Fleet::AddOwner(id);
	}

    void RemoveOwner(int id)
	{
	    if (override o = this->get_override("RemoveOwner"))
		o(id);
	    else
		Fleet::RemoveOwner(id);
	}
    void default_RemoveOwner(int id)
	{
	    this->Fleet::RemoveOwner(id);
	}

	
    void AddSpecial(const std::string& name)
	{
	    if (override o = this->get_override("AddSpecial"))
		o(name);
	    else
		Fleet::AddSpecial(name);
	}
    void default_AddSpecial(const std::string& name)
	{
	    this->Fleet::AddSpecial(name);
	}
	
    void RemoveSpecial(const std::string& name)
	{
	    if (override o = this->get_override("RemoveSpecial"))
		o(name);
	    else
		Fleet::RemoveSpecial(name);
	}
    void default_RemoveSpecial(const std::string& name)
	{
	    return this->Fleet::RemoveSpecial(name);
	}

    void MovementPhase()
	{
	    this->get_override("MovementPhase")();
	}
    void default_MovementPhase()
	{
	    this->Fleet::MovementPhase();
	}

    void AdjustMaxMeters()
	{
	    if (override o = this->get_override("AdjustMaxMeters"))
		o();
	    Fleet::AdjustMaxMeters();
	}
    void default_AdjustMaxMeters()
	{
	    this->Fleet::AdjustMaxMeters();
	}
	
    void PopGrowthProductionResearchPhase()
	{
	    this->get_override("PopGrowthProductionResearchPhase");
	}
    void default_PopGrowthProductionResearchPhase()
	{
	    this->Fleet::PopGrowthProductionResearchPhase();
	}
};
}

void ExportFleet()
{
    class_<FleetWrap,bases<UniverseObject>,boost::noncopyable>("Fleet",init<>())
	.def(init<const std::string&,double,double,int>(args("name","x","y","owner")))

        // Accessors

	.add_property("TravelRoute",
		      make_function(&Fleet::TravelRoute,return_internal_reference<>()))
	.add_property("ETA",&Fleet::ETA)
	.add_property("FinalDestinationID",&Fleet::FinalDestinationID)
	.add_property("FinalDestination",
		      make_function(&Fleet::FinalDestination,return_internal_reference<>()))
	.add_property("PreviousSystemID",&Fleet::PreviousSystemID)
	.add_property("NextSystemID",&Fleet::NextSystemID)
	.add_property("CanChangeDirectionEnRoute",&Fleet::CanChangeDirectionEnRoute)
	.add_property("HasArmedShips",&Fleet::HasArmedShips)
	.add_property("NumShips",&Fleet::NumShips)
	.add_property("ContainsShip",&Fleet::ContainsShip)

	// Mutators

	.def("SetRoute",&Fleet::SetRoute, with_custodian_and_ward<2,1>())
	.def("AddShip",&Fleet::AddShip)
	.def("RemoveShip",&Fleet::RemoveShip)
	.def("AddShips",&Fleet::AddShips)
	.def("RemoveShips",&Fleet::RemoveShips)
	.def("DeleteShips",&Fleet::DeleteShips)
	.def("__iter__",iterator<Fleet>())
	

        // virtual methods

#define WRAP_VIRT(x) .def(#x,&Fleet::x,&FleetWrap::default_##x)
//	WRAP_VIRT(GetMeter)
	WRAP_VIRT(GetVisibility)
	.def("Accept", &Fleet::Accept, &FleetWrap::default_Accept, return_internal_reference<>())
	WRAP_VIRT(AddOwner)
	WRAP_VIRT(RemoveOwner)
	WRAP_VIRT(AddSpecial)
	WRAP_VIRT(RemoveSpecial)
	WRAP_VIRT(MovementPhase)
	WRAP_VIRT(AdjustMaxMeters)
	WRAP_VIRT(PopGrowthProductionResearchPhase)
	;
}
