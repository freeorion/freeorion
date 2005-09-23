#include "PopCenter.h"
#include "Meter.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {
struct PopCenterWrap : PopCenter, wrapper<PopCenter>
{

    PopCenterWrap(UniverseObject* o,double m1, double m2)
	: PopCenter(o,m1,m2) {}
    PopCenterWrap(int r,UniverseObject* o,double m1,double m2)
	: PopCenter(r,o,m1,m2) {}

    void AdjustMaxMeters() 
	{
	    if (override o = this->get_override("AdjustMaxMeters"))
		o();
	    else
		PopCenter::AdjustMaxMeters();
	}
    void default_AdjustMaxMeters()
	{
	    this->AdjustMaxMeters();
	}

    void PopGrowthProductionResearchPhase()
	{
	    if (override o = this->get_override("PopGrowthProductionResearchPhase"))
		o();
	    else
		PopCenter::PopGrowthProductionResearchPhase();
	}
    void default_PopGrowthProductionResearchPhase()
	{
	    this->PopGrowthProductionResearchPhase();
	}
};
}

void ExportPopCenter() 
{
    class_<PopCenterWrap, boost::noncopyable>("PopCenter",
					      init<UniverseObject*,double,double>())
	.def(init<int,UniverseObject*,double,double>())

	// Accessors

	.add_property("PopulationMeter",
		      make_function(
			  (Meter&(PopCenter::*)())
			  &PopCenter::PopulationMeter,
			  return_internal_reference<>()))
	.add_property("HealthMeter",
		      make_function(
			  (Meter&(PopCenter::*)())
			  &PopCenter::HealthMeter,
			  return_internal_reference<>()))
	.add_property("PopPoints",&PopCenter::PopPoints)
	.add_property("MaxPop",&PopCenter::MaxPop)
	.add_property("PopGrowth",&PopCenter::PopGrowth)
	.add_property("Race",&PopCenter::Race,&PopCenter::SetRace)
	.add_property("Inhabitants",&PopCenter::Inhabitants)
	.add_property("PopDensity",&PopCenter::PopDensity)
	.add_property("AvailableFood",
		      &PopCenter::AvailableFood,
		      &PopCenter::SetAvailableFood)
	.add_property("FuturePopGrowth",&PopCenter::FuturePopGrowth)
	.add_property("FuturePopGrowthMax",&PopCenter::FuturePopGrowthMax)

	// Mutators

	.def("AdjustPop",&PopCenter::AdjustPop)
	.def("Reset",&PopCenter::Reset)

	// Virtual functions

	.def("AdjustMaxMeters",
	     &PopCenter::AdjustMaxMeters,
	     &PopCenterWrap::default_AdjustMaxMeters)
	.def("PopGrowthProductionResearchPhase",
	     &PopCenter::PopGrowthProductionResearchPhase,
	     &PopCenterWrap::default_PopGrowthProductionResearchPhase)
	;
    
	
}

