#include "System.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace 
{
struct SystemWrap : System, wrapper<System>
{
    SystemWrap() : System() {}
    SystemWrap(StarType s,int o,const std::string&n,double x,double y,const std::set<int>&ow=std::set<int>())
	: System(s,o,n,x,y,ow) {}
//    SystemWrap(StarType s,int o,const StarlaneMap &l,const std::string&n,double x,double y,const std::set<int>&ow=std::set<int>())
//	: System(s,o,l,n,x,y,ow) {}

// 	Meter* GetMeter(MeterType type)
// 	{
// 	    if (override o = this->get_override("GetMeter"))
// 		return o(type);
// 	    return System::GetMeter(type);
// 	}
// 	Meter* default_GetMeter(MeterType type) {
// 	    return this->System::GetMeter(type);
// 	}

    Visibility GetVisibility(int id) const
	{
	    if (override o = this->get_override("GetVisibility"))
		return o(id);
	    return System::GetVisibility(id);
	}
    Visibility default_GetVisibility(int id) const
	{
	    return this->System::GetVisibility(id);
	}
	
    UniverseObject* Accept(const UniverseObjectVisitor& vis) const
	{
	    if (override o = this->get_override("Accept"))
		return o(vis);
	    return System::Accept(vis);
	}
    UniverseObject* default_Accept(const UniverseObjectVisitor& vis) const
	{
	    return this->System::Accept(vis);
	}

    void AddOwner(int id)
	{
	    if (override o = this->get_override("AddOwner"))
		o(id);
	    else
		System::AddOwner(id);
	}
    void default_AddOwner(int id)
	{
	    this->System::AddOwner(id);
	}

    void RemoveOwner(int id)
	{
	    if (override o = this->get_override("RemoveOwner"))
		o(id);
	    else
		System::RemoveOwner(id);
	}
    void default_RemoveOwner(int id)
	{
	    this->System::RemoveOwner(id);
	}

	
    void AddSpecial(const std::string& name)
	{
	    if (override o = this->get_override("AddSpecial"))
		o(name);
	    else
		System::AddSpecial(name);
	}
    void default_AddSpecial(const std::string& name)
	{
	    this->System::AddSpecial(name);
	}
	
    void RemoveSpecial(const std::string& name)
	{
	    if (override o = this->get_override("RemoveSpecial"))
		o(name);
	    else
		System::RemoveSpecial(name);
	}
    void default_RemoveSpecial(const std::string& name)
	{
	    return this->System::RemoveSpecial(name);
	}

    void MovementPhase()
	{
	    this->get_override("MovementPhase")();
	}
    void default_MovementPhase()
	{
	    this->System::MovementPhase();
	}

    void AdjustMaxMeters()
	{
	    if (override o = this->get_override("AdjustMaxMeters"))
		o();
	    System::AdjustMaxMeters();
	}
    void default_AdjustMaxMeters()
	{
	    this->System::AdjustMaxMeters();
	}
	
    void PopGrowthProductionResearchPhase()
	{
	    this->get_override("PopGrowthProductionResearchPhase");
	}
    void default_PopGrowthProductionResearchPhase()
	{
	    this->System::PopGrowthProductionResearchPhase();
	}
};
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Insert_overloads,Insert,1,2)

void ExportSystem()
{
    class_<SystemWrap,bases<UniverseObject>,boost::noncopyable>("System",init<>())
    
	.def(init<StarType,int,const std::string&,double,double,optional<const std::set<int>&> >(args("star","orbits","name","x","y","owners")))
//	.def(init<StarType,int,const StarlaneMap&,const std::string&,double,double,optional<const std::set<int>&> >(args("star","orbits","lanes_and_holes","name","x","y","owners")))

	// Accessors
	.add_property("Star",&System::Star,&System::SetStarType)
	.add_property("Orbits",&System::Orbits)
	.add_property("Starlanes",&System::Starlanes)
	.add_property("Wormholes",&System::Wormholes)
	.add_property("HasStarlaneTo",&System::HasStarlaneTo)
	.add_property("HasWormholeTo",&System::HasWormholeTo)
	.def("FindObjectIDs",(System::ObjectIDVec (System::*)(const UniverseObjectVisitor&)const)&System::FindObjectIDs)
 	.add_property("Orbits",range((System::orbit_iterator(System::*)())&System::begin,
			    (System::orbit_iterator(System::*)())&System::end))
 	.add_property("Lanes",range((System::lane_iterator(System::*)())&System::begin_lanes,
			   (System::lane_iterator(System::*)())&System::end_lanes))

	// Mutators

	.def("Insert",(int(System::*)(UniverseObject*))&System::Insert)
	.def("Insert",(int(System::*)(UniverseObject*,int))&System::Insert)
	.def("Insert",(int(System::*)(int,int))&System::Insert)
	.def("Remove",&System::Remove)
	.def("AddStarlane",&System::AddStarlane)
	.def("AddWormhole",&System::AddWormhole)
	.def("RemoveStarlane",&System::RemoveStarlane)
	.def("RemoveWormhole",&System::RemoveWormhole)
	.def("FindObjects",
	     (System::ObjectVec(System::*)(const UniverseObjectVisitor&))
	     &System::FindObjects)
	.def("FindObjectsInOrbit",
	     (System::ObjectVec(System::*)(int,const UniverseObjectVisitor&))
	      &System::FindObjectsInOrbit)
	
        // virtual methods

#define WRAP_VIRT(x) .def(#x,&System::x,&SystemWrap::default_##x)
	// WRAP_VIRT(GetMeter)
	WRAP_VIRT(GetVisibility)
	.def("Accept", &System::Accept, &SystemWrap::default_Accept, return_internal_reference<>())
	WRAP_VIRT(AddOwner)
	WRAP_VIRT(RemoveOwner)
	WRAP_VIRT(AddSpecial)
	WRAP_VIRT(RemoveSpecial)
	WRAP_VIRT(MovementPhase)
	WRAP_VIRT(AdjustMaxMeters)
	WRAP_VIRT(PopGrowthProductionResearchPhase)
	;
}
