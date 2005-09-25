#include "Tech.h"

#include <boost/python.hpp>

using namespace boost::python;

void ExportTech()
{
    class_<Tech,boost::noncopyable>("Tech",no_init)
	.add_property("Name", make_function(&Tech::Name,
					    return_value_policy<copy_const_reference>()))
	.add_property("Description", make_function(&Tech::Description,
						   return_value_policy<copy_const_reference>()))
	.add_property("Type", &Tech::Type)
	.add_property("Category", make_function(&Tech::Category,
						return_value_policy<copy_const_reference>()))
	.add_property("ResearchCost", &Tech::ResearchCost)
	.add_property("ResearchTurns", &Tech::ResearchTurns)

// 	.add_property("Effects", make_function(&Tech::Effects,
// 					       return_internal_reference<>()))

	.add_property("Prerequisites", make_function(&Tech::Prerequisites,
						     return_internal_reference<>()))
	.add_property("Graphic", make_function(&Tech::Graphic,
					       return_internal_reference<>()))
	.add_property("UnlockedItems", make_function(&Tech::UnlockedItems,
						     return_internal_reference<>()))
	.add_property("UnlockedTechs", make_function(&Tech::UnlockedTechs,
						     return_internal_reference<>()))
	;
}
