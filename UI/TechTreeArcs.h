#ifndef TECHTREEARCS_H
#define TECHTREEARCS_H

#include <set>
#include <string>

class TechTreeLayout;
class TechTreeArcsImplementation;

/// This class is responsible for drawing the lines
/// of a TechTreeLayout on screen.
class TechTreeArcs {
public:
    TechTreeArcs();
    TechTreeArcs(const TechTreeLayout& layout, const std::set<std::string>& techs_to_show);
    ~TechTreeArcs();

    void Render(double scale);

    /// Clears the arcs
    void Reset();
    /// Recalculates the arcs
    void Reset(const TechTreeLayout& layout, const std::set<std::string>& techs_to_show);

private:
    /// Should be a c++11 unique_ptr, but we don't have c++11
    TechTreeArcsImplementation* m_impl;

    // Disable copying
    TechTreeArcs(TechTreeArcs& other);
    TechTreeArcs& operator=(TechTreeArcs& other);
};

#endif // TECHTREEARCS_H
