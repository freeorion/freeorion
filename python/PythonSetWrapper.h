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

    /* SetWrapper class encapsulates functions that expose the STL std::set<> class to Python in a limited,
       read-only fashion.  The set can be iterated through in Python, and printed. */
    template <typename ElementType>
    class SetWrapper {
    public:
        typedef typename std::set<ElementType> Set;
        typedef typename Set::const_iterator SetIterator;

        static unsigned int size(const Set& self) {
            return static_cast<unsigned int>(self.size());  // ignore warning http://lists.boost.org/Archives/boost/2007/04/120377.php
        }
        static bool empty(const Set& self) { return self.empty(); }
        static bool contains(const Set& self, const ElementType& item) { return self.find(item) != self.end(); }
        static unsigned int count(const Set& self, const ElementType& item) { return self.find(item) == self.end() ? 0u : 1u; }
        static SetIterator begin(const Set& self) { return self.begin(); }
        static SetIterator end(const Set& self) { return self.end(); }

        static std::string to_string(const Set& self) {
            std::string retval = "set([";
            for (SetIterator it = self.begin(); it != self.end(); ++it) {
                if (it != self.end() && it != self.begin()) {
                    retval += ", ";
                }
                try {
                    // attempt to cast directly to string (for ints and other built-in types)
                    std::string s = boost::lexical_cast<std::string>(*it);
                    retval += s;
                } catch (...) {
                    // cast failed, so default to unknown result
                    retval += "?";
                }
            }
            retval += "])";
            return retval;
        };

        static void Wrap(const std::string& python_name) {
            class_<Set>(python_name.c_str(), no_init)
                .def("__str__",         &to_string)
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
