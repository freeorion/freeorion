#ifndef PYTHON_SETWRAPPER_H
#define PYTHON_SETWRAPPER_H

#include <set>
#include <string>

#include <boost/python.hpp>


namespace FreeOrionPython {
    /* SetWrapper class encapsulates functions that expose the STL std::set<>
     * class to Python in a limited, read-only fashion.  The set can be iterated
     * through in Python, and printed. */
    template <typename SetT>
    class SetWrapper {
    public:
        using Set = SetT;
        using Key = typename Set::key_type;
        using Compare = typename Set::key_compare;
        using SetIterator = typename Set::const_iterator;

        static unsigned int size(const Set& self) {
            return static_cast<unsigned int>(self.size());  // ignore warning http://lists.boost.org/Archives/boost/2007/04/120377.php
        }
        static bool         empty(const Set& self) { return self.empty(); }
        static bool         contains(const Set& self, const Key& item) { return self.contains(item); }
        static SetIterator  begin(const Set& self) { return self.begin(); }
        static SetIterator  end(const Set& self) { return self.end(); }

        static void Wrap(const std::string& python_name) {
            namespace py = boost::python;

            py::class_<Set>(python_name.c_str(), py::no_init)
                .def("__len__",         &size)
                .def("empty",           &empty)
                .def("__contains__",    &contains)
                .def("__iter__",        py::iterator<Set>())
            ;
        }
    };
}
#endif
