/// Meter DISABLED FOR NOW
#include "UniverseObject.h"
#include "Meter.h"
#include "System.h"

#include <boost/python.hpp>

using namespace boost::python;

// Wrapper classes
namespace {

    struct UniverseObjectWrap : UniverseObject, wrapper<UniverseObject>
    {
	UniverseObjectWrap() : UniverseObject() {}
	UniverseObjectWrap(const std::string name, double x, double y, const std::set<int>& owners = std::set<int>()) : UniverseObject(name,x,y,owners) {}

// 	Meter* GetMeter(MeterType type)
// 	{
// 	    if (override o = this->get_override("GetMeter"))
// 		return o(type);
// 	    return UniverseObject::GetMeter(type);
// 	}
// 	Meter* default_GetMeter(MeterType type) {
// 	    return this->UniverseObject::GetMeter(type);
// 	}

	Visibility GetVisibility(int id) const
	{
	    if (override o = this->get_override("GetVisibility"))
		return o(id);
	    return UniverseObject::GetVisibility(id);
	}
	Visibility default_GetVisibility(int id) const
	{
	    return this->UniverseObject::GetVisibility(id);
	}
	
	UniverseObject* Accept(const UniverseObjectVisitor& vis) const
	{
	    if (override o = this->get_override("Accept"))
		return o(vis);
	    return UniverseObject::Accept(vis);
	}
	UniverseObject* default_Accept(const UniverseObjectVisitor& vis) const
	{
	    return this->UniverseObject::Accept(vis);
	}

	void AddOwner(int id)
	{
	    if (override o = this->get_override("AddOwner"))
		o(id);
	    else
		UniverseObject::AddOwner(id);
	}
	void default_AddOwner(int id)
	{
	    this->UniverseObject::AddOwner(id);
	}

	void RemoveOwner(int id)
	{
	    if (override o = this->get_override("RemoveOwner"))
		o(id);
	    else
		UniverseObject::RemoveOwner(id);
	}
	void default_RemoveOwner(int id)
	{
	    this->UniverseObject::RemoveOwner(id);
	}

	
	void AddSpecial(const std::string& name)
	{
	    if (override o = this->get_override("AddSpecial"))
		o(name);
	    else
		UniverseObject::AddSpecial(name);
	}
	void default_AddSpecial(const std::string& name)
	{
	    this->UniverseObject::AddSpecial(name);
	}
	
	void RemoveSpecial(const std::string& name)
	{
	    if (override o = this->get_override("RemoveSpecial"))
		o(name);
	    else
		UniverseObject::RemoveSpecial(name);
	}
	void default_RemoveSpecial(const std::string& name)
	{
	    return this->UniverseObject::RemoveSpecial(name);
	}

	void MovementPhase()
	{
	    this->get_override("MovementPhase")();
	}

	void AdjustMaxMeters()
	{
	    if (override o = this->get_override("AdjustMaxMeters"))
		o();
	    UniverseObject::AdjustMaxMeters();
	}
	void default_AdjustMaxMeters()
	{
	    this->UniverseObject::AdjustMaxMeters();
	}
	
	void PopGrowthProductionResearchPhase()
	{
	    this->get_override("PopGrowthProductionResearchPhase");
	}
    };
}

void ExportUniverseObject()
{
    // Public types

    enum_<UniverseObject::Visibility>("Visibility")
	.value("FULL_VISIBILITY",UniverseObject::FULL_VISIBILITY)
	.value("PARTIAL_VISIBILITY",UniverseObject::PARTIAL_VISIBILITY)
	.value("NO_VISIBILITY",UniverseObject::NO_VISIBILITY)
	;

    // Class UniverseObject
    
    class_<UniverseObjectWrap, boost::noncopyable>("UniverseObject")
	.def(init<const std::string,double,double,optional<const std::set<int>&> >())
	//.def(init<const GG::XMLElement&>())
	

	// Accessors and attributes

        .def_readonly("INVALID_POSITION", &UniverseObject::INVALID_POSITION)
        .def_readonly("INVALID_OBJECT_ID", &UniverseObject::INVALID_OBJECT_ID)
        .def_readonly("MAX_ID", &UniverseObject::MAX_ID)

	.add_property("ID", &UniverseObject::ID, &UniverseObject::SetID)
        .add_property("Name",
		      make_function(&UniverseObject::Name, 
				    return_value_policy< copy_const_reference >()),
		      &UniverseObject::Rename)
	.add_property("X", &UniverseObject::X)
	.add_property("Y", &UniverseObject::Y)
	.add_property("Owners",
		      make_function(&UniverseObject::Owners,
				    return_internal_reference<>()))
	.def("GetSystem",&UniverseObject::GetSystem,return_value_policy<reference_existing_object>())
	.add_property("SystemID", &UniverseObject::SystemID,&UniverseObject::SetSystem)
	.add_property("Specials",
		      make_function(&UniverseObject::Specials,
				    return_internal_reference<>()))
	.add_property("Unowned", &UniverseObject::Unowned)
	.def("OwnedBy", &UniverseObject::OwnedBy)
	.def("WhollyOwnedBy", &UniverseObject::WhollyOwnedBy)
	.def("GetVisibility", &UniverseObject::GetVisibility, &UniverseObjectWrap::default_GetVisibility)
	.def("Accept", &UniverseObject::Accept, &UniverseObjectWrap::default_Accept, return_internal_reference<>())
	
	

// Mutators
	
	.def("Move",&UniverseObject::Move)
	.def("MoveTo",&UniverseObject::MoveTo)
//	.def("GetMeter",&UniverseObject::GetMeter, &UniverseObjectWrap::default_GetMeter)
	.def("AddOwner",&UniverseObject::AddOwner, &UniverseObjectWrap::default_AddOwner)
	.def("RemoveOwner",&UniverseObject::RemoveOwner, &UniverseObjectWrap::default_RemoveOwner)
	.def("SetSystem",&UniverseObject::SetSystem)
	.def("AddSpecial",&UniverseObject::AddSpecial,&UniverseObjectWrap::default_AddSpecial)
	.def("RemoveSpecial",&UniverseObject::RemoveSpecial,&UniverseObjectWrap::default_RemoveSpecial)
	.def("MovementPhase",pure_virtual(&UniverseObject::MovementPhase))
	.def("ResetMaxMeters",&UniverseObject::ResetMaxMeters)
	.def("AdjustMaxMeters",&UniverseObject::AdjustMaxMeters,&UniverseObjectWrap::default_AdjustMaxMeters)
	.def("ClampMeters",&UniverseObject::ClampMeters)
	.def("PopGrowthProductionResearchPhase",pure_virtual(&UniverseObject::PopGrowthProductionResearchPhase))
	;
    
}
