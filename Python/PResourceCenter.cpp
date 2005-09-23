#include "ResourceCenter.h"
#include "Meter.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {

struct ResourceCenterWrap : ResourceCenter, wrapper<ResourceCenter>
{
    ResourceCenterWrap(const Meter& p, UniverseObject* o)
	: ResourceCenter(p,o) {}

        void AdjustMaxMeters() 
	{
	    if (override o = this->get_override("AdjustMaxMeters"))
		o();
	    else
		ResourceCenter::AdjustMaxMeters();
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
		ResourceCenter::PopGrowthProductionResearchPhase();
	}
    void default_PopGrowthProductionResearchPhase()
	{
	    this->PopGrowthProductionResearchPhase();
	}

};
}

void ExportResourceCenter()
{
    class_<ResourceCenterWrap, boost::noncopyable>("ResourceCenter",
						   init<const Meter&,UniverseObject*>())

	// Accesors

	.add_property("PrimaryFocus",
		      &ResourceCenter::PrimaryFocus,
		      &ResourceCenter::SetPrimaryFocus)
	.add_property("SecondaryFocus",
		      &ResourceCenter::SecondaryFocus,
		      &ResourceCenter::SetSecondaryFocus)
	.add_property("FarmingMeter",
		      make_function((Meter&(ResourceCenter::*)())
				    &ResourceCenter::FarmingMeter,
				    return_internal_reference<>()))
	.add_property("IndustryMeter",
		      make_function((Meter&(ResourceCenter::*)())
				    &ResourceCenter::IndustryMeter,
				    return_internal_reference<>()))
	.add_property("ResearchMeter",
		      make_function((Meter&(ResourceCenter::*)())
				    &ResourceCenter::ResearchMeter,
				    return_internal_reference<>()))
	.add_property("TradeMeter",
		      make_function((Meter&(ResourceCenter::*)())
				    &ResourceCenter::TradeMeter,
				    return_internal_reference<>()))
	.add_property("ConstructionMeter",
		      make_function((Meter&(ResourceCenter::*)())
				    &ResourceCenter::ConstructionMeter,
				    return_internal_reference<>()))
	.def("GetMeter",
	     (Meter*(ResourceCenter::*)(MeterType))
	     &ResourceCenter::GetMeter,
	     return_internal_reference<>())
	.add_property("FarmingPoints",
		      &ResourceCenter::FarmingPoints)
	.add_property("IndustryPoints",
		      &ResourceCenter::IndustryPoints)
	.add_property("MiningPoints",
		      &ResourceCenter::MiningPoints)
	.add_property("ResearchPoints",
		      &ResourceCenter::ResearchPoints)
	.add_property("TradePoints",
		      &ResourceCenter::TradePoints)
	.def("ProjectedCurrent",&ResourceCenter::ProjectedCurrent)


	// Virtual functions

	.def("AdjustMaxMeters",
	     &ResourceCenter::AdjustMaxMeters,
	     &ResourceCenterWrap::default_AdjustMaxMeters)
	.def("PopGrowthProductionResearchPhase",
	     &ResourceCenter::PopGrowthProductionResearchPhase,
	     &ResourceCenterWrap::default_PopGrowthProductionResearchPhase)
	;

}

