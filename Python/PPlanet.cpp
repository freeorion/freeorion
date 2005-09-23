#include "Planet.h"
#include "Building.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace 
{
struct PlanetWrap : Planet, wrapper<Planet>
{
    PlanetWrap() : Planet() {}
    PlanetWrap(PlanetType type, PlanetSize size)
	: Planet(type,size) {}
        
// 	Meter* GetMeter(MeterType type)
// 	{
// 	    if (override o = this->get_override("GetMeter"))
// 		return o(type);
// 	    return Planet::GetMeter(type);
// 	}
// 	Meter* default_GetMeter(MeterType type) {
// 	    return this->Planet::GetMeter(type);
// 	}

    Visibility GetVisibility(int id) const
	{
	    if (override o = this->get_override("GetVisibility"))
		return o(id);
	    return Planet::GetVisibility(id);
	}
    Visibility default_GetVisibility(int id) const
	{
	    return this->Planet::GetVisibility(id);
	}
	
    UniverseObject* Accept(const UniverseObjectVisitor& vis) const
	{
	    if (override o = this->get_override("Accept"))
		return o(vis);
	    return Planet::Accept(vis);
	}
    UniverseObject* default_Accept(const UniverseObjectVisitor& vis) const
	{
	    return this->Planet::Accept(vis);
	}

    void AddOwner(int id)
	{
	    if (override o = this->get_override("AddOwner"))
		o(id);
	    else
		Planet::AddOwner(id);
	}
    void default_AddOwner(int id)
	{
	    this->Planet::AddOwner(id);
	}

    void RemoveOwner(int id)
	{
	    if (override o = this->get_override("RemoveOwner"))
		o(id);
	    else
		Planet::RemoveOwner(id);
	}
    void default_RemoveOwner(int id)
	{
	    this->Planet::RemoveOwner(id);
	}

	
    void AddSpecial(const std::string& name)
	{
	    if (override o = this->get_override("AddSpecial"))
		o(name);
	    else
		Planet::AddSpecial(name);
	}
    void default_AddSpecial(const std::string& name)
	{
	    this->Planet::AddSpecial(name);
	}
	
    void RemoveSpecial(const std::string& name)
	{
	    if (override o = this->get_override("RemoveSpecial"))
		o(name);
	    else
		Planet::RemoveSpecial(name);
	}
    void default_RemoveSpecial(const std::string& name)
	{
	    return this->Planet::RemoveSpecial(name);
	}

    void MovementPhase()
	{
	    this->get_override("MovementPhase")();
	}
    void default_MovementPhase()
	{
	    this->Planet::MovementPhase();
	}

    void AdjustMaxMeters()
	{
	    if (override o = this->get_override("AdjustMaxMeters"))
		o();
	    Planet::AdjustMaxMeters();
	}
    void default_AdjustMaxMeters()
	{
	    this->Planet::AdjustMaxMeters();
	}
	
    void PopGrowthProductionResearchPhase()
	{
	    this->get_override("PopGrowthProductionResearchPhase");
	}
    void default_PopGrowthProductionResearchPhase()
	{
	    this->Planet::PopGrowthProductionResearchPhase();
	}
};
}

void ExportPlanet()
{
    class_<PlanetWrap,bases<Building>,boost::noncopyable>("Planet",init<>())

	.def(init<PlanetType,PlanetSize>(args("type","size")))

        // Accessors

	.add_property("Type",&Planet::Type,&Planet::SetType)
	.add_property("Size",&Planet::Size,&Planet::SetSize)
	.add_property("Buildings",
		      make_function(&Planet::Buildings,return_value_policy<copy_const_reference>()))
	.add_property("AvailableTrade",&Planet::AvailableTrade)
	.add_property("BuildingCosts",&Planet::BuildingCosts)
	.def("ContainsBuilding",&Planet::ContainsBuilding)
	.def("__contains__",&Planet::ContainsBuilding) //for convinience
	.add_property("IsAboutToBeColonized",&Planet::IsAboutToBeColonized)
	.add_property("DefBases",&Planet::DefBases, &Planet::AdjustDefBases)
	
	// Mutators

	.def("AddBuilding",&Planet::AddBuilding)
	.def("RemoveBuilding",&Planet::RemoveBuilding)
	.def("DeleteBuilding",&Planet::DeleteBuilding)
	.def("SetAvailableTrade",&Planet::SetAvailableTrade)
	.def("Reset",&Planet::Reset)
	.def("Conquer",&Planet::Conquer,args("conqueror"))
	.def("SetIsAboutToBeColonized",&Planet::SetIsAboutToBeColonized)
	.def("ResetIsAboutToBeColonized",&Planet::ResetIsAboutToBeColonized)

        // virtual methods

#define WRAP_VIRT(x) .def(#x,&Planet::x,&PlanetWrap::default_##x)
	// WRAP_VIRT(GetMeter)
	WRAP_VIRT(GetVisibility)
	.def("Accept", &Planet::Accept, &PlanetWrap::default_Accept, return_internal_reference<>())
	WRAP_VIRT(AddOwner)
	WRAP_VIRT(RemoveOwner)
	WRAP_VIRT(AddSpecial)
	WRAP_VIRT(RemoveSpecial)
	WRAP_VIRT(MovementPhase)
	WRAP_VIRT(AdjustMaxMeters)
	WRAP_VIRT(PopGrowthProductionResearchPhase)
	;
}
