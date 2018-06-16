#ifndef PYTHON_SETWRAPPER_H
#define PYTHON_SETWRAPPER_H

#include <set>
#include <string>

#include <boost/python.hpp>


namespace FreeOrionPython {
    using boost::python::class_;
    using boost::python::no_init;
    using boost::python::def;
    using boost::python::iterator;

    /* SetWrapper class encapsulates functions that expose the STL std::set<>
     * class to Python in a limited, read-only fashion.  The set can be iterated
     * through in Python, and printed. */
    template <typename ElementType>
    class SetWrapper {
    public:
        typedef typename std::set<ElementType> Set;
        typedef typename Set::const_iterator SetIterator;

        static unsigned int size(const Set& self) {
            return static_cast<unsigned int>(self.size());  // ignore warning http://lists.boost.org/Archives/boost/2007/04/120377.php
        }
        static bool         empty(const Set& self) { return self.empty(); }
        static bool         contains(const Set& self, const ElementType& item) { return self.count(item); }
        static unsigned int count(const Set& self, const ElementType& item) { return self.count(item); }
        static SetIterator  begin(const Set& self) { return self.begin(); }
        static SetIterator  end(const Set& self) { return self.end(); }

        static void Wrap(const std::string& python_name) {
            class_<Set>(python_name.c_str(), no_init)
                .def("__len__",         &size)
                .def("size",            &size)
                .def("empty",           &empty)
                .def("__contains__",    &contains)
                .def("count",           &count)
                .def("__iter__",        iterator<Set>())
            ;
        }
    };
}
#endif
