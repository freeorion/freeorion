#include "Building.h"
#include "Planet.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {
    
struct BuildingWrap : Building, wrapper<Building>
{
    BuildingWrap() : Building() 
	{}

    BuildingWrap(int e,const std::string& b,int p)
	: Building(e,b,p)
	{}
    
    
// 	Meter* GetMeter(MeterType type)
// 	{
// 	    if (override o = this->get_override("GetMeter"))
// 		return o(type);
// 	    return Building::GetMeter(type);
// 	}
// 	Meter* default_GetMeter(MeterType type) {
// 	    return this->Building::GetMeter(type);
// 	}

    Visibility GetVisibility(int id) const
	{
	    if (override o = this->get_override("GetVisibility"))
		return o(id);
	    return Building::GetVisibility(id);
	}
    Visibility default_GetVisibility(int id) const
	{
	    return this->Building::GetVisibility(id);
	}
	
    UniverseObject* Accept(const UniverseObjectVisitor& vis) const
	{
	    if (override o = this->get_override("Accept"))
		return o(vis);
	    return Building::Accept(vis);
	}
    UniverseObject* default_Accept(const UniverseObjectVisitor& vis) const
	{
	    return this->Building::Accept(vis);
	}

    void AddOwner(int id)
	{
	    if (override o = this->get_override("AddOwner"))
		o(id);
	    else
		Building::AddOwner(id);
	}
    void default_AddOwner(int id)
	{
	    this->Building::AddOwner(id);
	}

    void RemoveOwner(int id)
	{
	    if (override o = this->get_override("RemoveOwner"))
		o(id);
	    else
		Building::RemoveOwner(id);
	}
    void default_RemoveOwner(int id)
	{
	    this->Building::RemoveOwner(id);
	}

	
    void AddSpecial(const std::string& name)
	{
	    if (override o = this->get_override("AddSpecial"))
		o(name);
	    else
		Building::AddSpecial(name);
	}
    void default_AddSpecial(const std::string& name)
	{
	    this->Building::AddSpecial(name);
	}
	
    void RemoveSpecial(const std::string& name)
	{
	    if (override o = this->get_override("RemoveSpecial"))
		o(name);
	    else
		Building::RemoveSpecial(name);
	}
    void default_RemoveSpecial(const std::string& name)
	{
	    return this->Building::RemoveSpecial(name);
	}

    void MovementPhase()
	{
	    this->get_override("MovementPhase")();
	}
    void default_MovementPhase()
	{
	    this->Building::MovementPhase();
	}

    void AdjustMaxMeters()
	{
	    if (override o = this->get_override("AdjustMaxMeters"))
		o();
	    Building::AdjustMaxMeters();
	}
    void default_AdjustMaxMeters()
	{
	    this->Building::AdjustMaxMeters();
	}
	
    void PopGrowthProductionResearchPhase()
	{
	    this->get_override("PopGrowthProductionResearchPhase");
	}
    void default_PopGrowthProductionResearchPhase()
	{
	    this->Building::PopGrowthProductionResearchPhase();
	}
};
}



void ExportBuilding()
{
    class_<Building,bases<UniverseObject>,boost::noncopyable>("Building",init<>())
	.def(init<int,const std::string&,int>())
	.add_property("BuildingType",
		      make_function(&Building::GetBuildingType,
				    return_internal_reference<>()))
	.add_property("Operating", &Building::Operating)
	.add_property("PlanetID", &Building::PlanetID, &Building::SetPlanetID)
	.add_property("Planet",
		      make_function(&Building::GetPlanet,
				    return_internal_reference<>()))
	.def("Activate",&Building::Activate)

	// virtual functions

#define WRAP_VIRT(x) .def(#x,&Building::x,&BuildingWrap::default_##x)
	// WRAP_VIRT(GetMeter)
	WRAP_VIRT(GetVisibility)
	.def("Accept", &Building::Accept, &BuildingWrap::default_Accept, return_internal_reference<>())
	WRAP_VIRT(AddOwner)
	WRAP_VIRT(RemoveOwner)
	WRAP_VIRT(AddSpecial)
	WRAP_VIRT(RemoveSpecial)
	WRAP_VIRT(MovementPhase)
	WRAP_VIRT(AdjustMaxMeters)
	WRAP_VIRT(PopGrowthProductionResearchPhase)
	;
    
}
