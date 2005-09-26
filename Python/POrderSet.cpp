#include "OrderSet.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {

int OrderSet_IssueOrder_Wrap(OrderSet& os, std::auto_ptr<Order> op)
{
    int retval = os.IssueOrder(op.get());
    op.release();
    return retval;
}

int OrderSet_AddOrder_Wrap(OrderSet& os, std::auto_ptr<Order> op)
{
    int retval = os.AddOrder(op.get());
    op.release();
    return retval;
}

}

void ExportOrderSet()
{
    class_<OrderSet>("OrderSet")

	// Accessors
	.def("ExamineOrder",&OrderSet::ExamineOrder,
	     return_internal_reference<>())

	// Mutators

	.def("IssueOrder",&OrderSet_IssueOrder_Wrap)
	.def("AddOrder",&OrderSet_AddOrder_Wrap)
	.def("RecindOrder",&OrderSet::RecindOrder)
	.def("Reset",&OrderSet::Reset)

	.def("__iter__",iterator<const OrderSet,return_internal_reference<> >())
	;
}

