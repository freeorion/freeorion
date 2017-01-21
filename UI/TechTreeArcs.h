#ifndef TECHTREEARCS_H
#define TECHTREEARCS_H

#include <memory>
#include <set>
#include <string>

class TechTreeLayout;

/// This class is responsible for drawing the lines
/// of a TechTreeLayout on screen.
class TechTreeArcs {
public:
    TechTreeArcs();

    TechTreeArcs(TechTreeArcs& other) = delete;

    TechTreeArcs(const TechTreeLayout& layout, const std::set<std::string>& techs_to_show);

    ~TechTreeArcs();

    TechTreeArcs& operator=(TechTreeArcs& other) = delete;

    void Render(double scale);

    /// Clears the arcs
    void Reset();
    /// Recalculates the arcs
    void Reset(const TechTreeLayout& layout, const std::set<std::string>& techs_to_show);

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

#endif // TECHTREEARCS_H
