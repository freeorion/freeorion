#include "../Empire/Empire.h"
#include "../universe/ShipDesign.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(PTIQ_overload, PlaceTechInQueue, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(PBIQ_overload, PlaceBuildInQueue, 4,5)

}

void ExportEmpire()
{
    class_<Empire,boost::noncopyable>("Empire",
		   init<const std::string&,const std::string&,int,const GG::Clr&,int>())

	.def("UpdateResourcePool", &Empire::UpdateResourcePool)
	.def("UpdateResearchQueue", &Empire::UpdateResearchQueue)
	.def("UpdateProductionQueue", &Empire::UpdateProductionQueue)

	// Accessors 

	.add_property("Name", make_function(&Empire::Name,
					    return_value_policy<copy_const_reference>())
		      , &Empire::SetName)
	.add_property("PlayerName", make_function(&Empire::PlayerName,
						  return_value_policy<copy_const_reference>())
		      , &Empire::SetPlayerName)
	.add_property("EmpireID", &Empire::EmpireID)
	.add_property("Color", make_function(&Empire::Color, 
					     return_internal_reference<>())
		      , &Empire::SetColor)
	.add_property("HomeworldID", &Empire::HomeworldID)
	.add_property("CapitolID", &Empire::CapitolID)
	.def("GetShipDesign", &Empire::GetShipDesign, return_internal_reference<>())
	.def("ResearchableTech", &Empire::ResearchableTech)
	.add_property("ResearchQueue", make_function(&Empire::GetResearchQueue,
						     return_internal_reference<>()))
	.def("ResearchStatus", &Empire::ResearchStatus)
	.add_property("AvailableTechs", make_function(&Empire::AvailableTechs,
						      return_internal_reference<>()))
	.def("TechAvailable", &Empire::TechAvailable)
	.add_property("AvailableBuildingTypes", make_function(&Empire::AvailableBuildingTypes,
							      return_internal_reference<>()))
	.def("BuildingTypeAvailable", &Empire::BuildingTypeAvailable)
	.add_property("ProductionQueue", make_function(&Empire::GetProductionQueue,
						       return_internal_reference<>()))
	.def("ProductionStatus", &Empire::ProductionStatus)
	.def("ProductionCostAndTime", &Empire::ProductionCostAndTime)
	.def("BuildableItem",&Empire::BuildableItem)
	.def("HasExploredSystem", &Empire::HasExploredSystem)
	.add_property("NumSitRepEntries", &Empire::NumSitRepEntries)
	
	// Iterators
	
	.add_property("Techs", range(&Empire::TechBegin, &Empire::TechEnd))
	.add_property("BuildingTypes", range(&Empire::BuildingTypeBegin, &Empire::BuildingTypeEnd))
	.add_property("Explored", range(&Empire::ExploredBegin, &Empire::ExploredEnd))
	.add_property("ShipDesigns", range(&Empire::ShipDesignBegin, &Empire::ShipDesignEnd))
	.add_property("SitReps", range(&Empire::SitRepBegin, &Empire::SitRepEnd))
	// RESPOOLS MISSING
	.add_property("ProductionPoints", &Empire::ProductionPoints)


	// Mutators

	.def("PlaceTechInQueue", &Empire::PlaceTechInQueue, PTIQ_overload(args("tech","pos")))
	.def("RemoveTechFromQueue", &Empire::RemoveTechFromQueue)
	.def("PlaceBuildInQueue", &Empire::PlaceBuildInQueue, PBIQ_overload(args("build_type","name","number","location","pos")))
	.def("SetBuildQuantity", &Empire::SetBuildQuantity)
	.def("MoveBuildWithinQueue", &Empire::MoveBuildWithinQueue)
	.def("RemoveBuildFromQueue", &Empire::RemoveBuildFromQueue)
	.def("AddTech", &Empire::AddTech)
	.def("UnlockItem", &Empire::UnlockItem)
	.def("AddBuildingType", &Empire::AddBuildingType)
	.def("RefineBuildingType", &Empire::RefineBuildingType)
	.def("ClearRefinements", &Empire::ClearRefinements)
	.def("AddExploredSystem", &Empire::AddExploredSystem)
	.def("AddShipDesign", &Empire::AddShipDesign)
	.def("AddSitRepEntry", &Empire::AddSitRepEntry)
	.def("RemoveTech", &Empire::RemoveTech)
	.def("LockItem", &Empire::LockItem)
	.def("RemoveBuildingType", &Empire::RemoveBuildingType)
	.def("ClearSitRep", &Empire::ClearSitRep)
	.def("CheckResearchProgress", &Empire::CheckResearchProgress)
	.def("CheckProductionProgress", &Empire::CheckProductionProgress)
	
	;
}
