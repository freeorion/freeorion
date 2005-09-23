#include "OrderSet.h"

#include <boost/python.hpp>

using namespace boost::python;

void ExportOrderSet()
{
    class_<OrderSet>("OrderSet")

	// Accessors
	.def("ExamineOrder",&OrderSet::ExamineOrder,
	     return_internal_reference<>())

	// Mutators

	.def("IssueOrder",&OrderSet::IssueOrder)
	.def("AddOrder",&OrderSet::AddOrder)
	.def("RecindOrder",&OrderSet::RecindOrder)
	.def("Reset",&OrderSet::Reset)

	.def("__iter__",iterator<const OrderSet,return_internal_reference<> >())
	;
}

