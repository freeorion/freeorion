#include "Ship.h"
#include "Fleet.h"
#include "Building.h"
#include "ShipDesign.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace 
{
struct ShipWrap : Ship, wrapper<Ship>
{
    ShipWrap() : Ship() {}
    ShipWrap(int e,const std::string& d)
	: Ship(e,d) {}
// 	Meter* GetMeter(MeterType type)
// 	{
// 	    if (override o = this->get_override("GetMeter"))
// 		return o(type);
// 	    return Ship::GetMeter(type);
// 	}
// 	Meter* default_GetMeter(MeterType type) {
// 	    return this->Ship::GetMeter(type);
// 	}

    Visibility GetVisibility(int id) const
	{
	    if (override o = this->get_override("GetVisibility"))
		return o(id);
	    return Ship::GetVisibility(id);
	}
    Visibility default_GetVisibility(int id) const
	{
	    return this->Ship::GetVisibility(id);
	}
	
    UniverseObject* Accept(const UniverseObjectVisitor& vis) const
	{
	    if (override o = this->get_override("Accept"))
		return o(vis);
	    return Ship::Accept(vis);
	}
    UniverseObject* default_Accept(const UniverseObjectVisitor& vis) const
	{
	    return this->Ship::Accept(vis);
	}

    void AddOwner(int id)
	{
	    if (override o = this->get_override("AddOwner"))
		o(id);
	    else
		Ship::AddOwner(id);
	}
    void default_AddOwner(int id)
	{
	    this->Ship::AddOwner(id);
	}

    void RemoveOwner(int id)
	{
	    if (override o = this->get_override("RemoveOwner"))
		o(id);
	    else
		Ship::RemoveOwner(id);
	}
    void default_RemoveOwner(int id)
	{
	    this->Ship::RemoveOwner(id);
	}

	
    void AddSpecial(const std::string& name)
	{
	    if (override o = this->get_override("AddSpecial"))
		o(name);
	    else
		Ship::AddSpecial(name);
	}
    void default_AddSpecial(const std::string& name)
	{
	    this->Ship::AddSpecial(name);
	}
	
    void RemoveSpecial(const std::string& name)
	{
	    if (override o = this->get_override("RemoveSpecial"))
		o(name);
	    else
		Ship::RemoveSpecial(name);
	}
    void default_RemoveSpecial(const std::string& name)
	{
	    return this->Ship::RemoveSpecial(name);
	}

    void MovementPhase()
	{
	    this->get_override("MovementPhase")();
	}
    void default_MovementPhase()
	{
	    this->Ship::MovementPhase();
	}

    void AdjustMaxMeters()
	{
	    if (override o = this->get_override("AdjustMaxMeters"))
		o();
	    Ship::AdjustMaxMeters();
	}
    void default_AdjustMaxMeters()
	{
	    this->Ship::AdjustMaxMeters();
	}
	
    void PopGrowthProductionResearchPhase()
	{
	    this->get_override("PopGrowthProductionResearchPhase");
	}
    void default_PopGrowthProductionResearchPhase()
	{
	    this->Ship::PopGrowthProductionResearchPhase();
	}
};
}

void ExportShip()
{
    class_<ShipWrap,bases<UniverseObject>,boost::noncopyable>("Ship",init<>())
	.def(init<int,const std::string&>(args("empire_id","design_name")))

	// Accessors

	.add_property("Design",
		      make_function(&Ship::Design,
				    return_value_policy<reference_existing_object>()))
	.add_property("FleetID",&Ship::FleetID,&Ship::SetFleetID)
	.add_property("Fleet",
		      make_function(&Ship::GetFleet,
				    return_value_policy<reference_existing_object>()))
	.add_property("IsArmed",&Ship::IsArmed)

        // virtual methods

#define WRAP_VIRT(x) .def(#x,&Ship::x,&ShipWrap::default_##x)
	// WRAP_VIRT(GetMeter)
	WRAP_VIRT(GetVisibility)
	.def("Accept", &Ship::Accept, &ShipWrap::default_Accept, return_internal_reference<>())
	WRAP_VIRT(AddOwner)
	WRAP_VIRT(RemoveOwner)
	WRAP_VIRT(AddSpecial)
	WRAP_VIRT(RemoveSpecial)
	WRAP_VIRT(MovementPhase)
	WRAP_VIRT(AdjustMaxMeters)
	WRAP_VIRT(PopGrowthProductionResearchPhase)
	;
}
