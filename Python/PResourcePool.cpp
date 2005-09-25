#include "Empire/ResourcePool.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {

struct ResourcePoolWrap : ResourcePool, wrapper<ResourcePool>
{
    GG::XMLElement XMLEncode() const
	{
	    return this->get_override("XMLEncode")();
	}
    
    double Stockpile() const
	{
	    if (override o = this->get_override("Stockpile"))
		return o();
	    else
		return ResourcePool::Stockpile();
	}
    double default_Stockpile() const
	{
	    return this->ResourcePool::Stockpile();
	}

    void SetStockpile(double d)
	{
	    if (override o = this->get_override("SetStockpile"))
		o(d);
	    else
		ResourcePool::SetStockpile(d);
	}
    void default_SetStockpile(double d)
	{
	    this->ResourcePool::SetStockpile(d);
	}

    // SortFunc is not needed, Python can do that, too
    virtual void PlanetChanged()
	{
	    this->get_override("PlanetChanged")();
	}
};
} // namespace

void ExportResourcePool()
{
    class_<ResourcePoolWrap, boost::noncopyable>("ResourcePool")

	.add_property("Planets", 
		      make_function(
			  (std::vector<Planet*>&(ResourcePool::*)())&ResourcePool::Planets,
			  return_internal_reference<>()),
		      &ResourcePool::SetPlanets)

	// virtual methods

	.def("Stockpile", &ResourcePool::Stockpile, &ResourcePoolWrap::default_Stockpile)
	.def("SetStockpile", &ResourcePool::SetStockpile, &ResourcePoolWrap::default_SetStockpile)

//	.def("PlanetChanged", pure_virtual(&ResourcePool::PlanetChanged))
	;
}

