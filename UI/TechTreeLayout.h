#ifndef _TechTreeLayout_h_
#define _TechTreeLayout_h_

#include <map>
#include <string>
#include <vector>

#include <GG/PtRect.h>


/**
 * This class stores internal information for layouting the tech graph.
 * The whole space is organized in a table.
 * Every tech is assign a tech level, or depth, which is the maximum distance it is from start techs.
 * Every tech level (depth) is one column.
 * The first column has depth 1 techs, 2nd column has depth 2 tech, etc.
 * An initial column is filled with techs grouped by category (may be middle or first, or ? column)
 * Nodes in adjacent columns are placed near the average of children or parent nodes in adjacent columns
 * For children or parents that don't lie in the next column we generate dummy
 * techs that are rendered as horizontal line (part of the future connection
 * arrow) that connect the low tech with the high tech. This allows complex
 * connection lines.
 * Dummy techs are directly processed in order to keep them as straight as possible.
 *
 *
 * sample (without category)
 * A->DE B->D C->F D->F E->F
 * A(1) D(2) E(2) B(1) D(2) C(1) F(3)
 * C(1)->F(3) changed to C(1)->G*(2)->F(3) (*Dummy)
 * 1. Column We sort the start Techs
 *    A   |     | 
 *    B   |     | 
 *    C   |     | 
 *   C has a Dummy child, which is placed at once
 *    A   |     | 
 *    B   |     | 
 *    C   |  G* | 
 * 2. Column
 *   D has parents A, B. The average is 1 a free place
 *    A   |  D  | 
 *    B   |     | 
 *    C   |  G* | 
 *   E has parents A. The average is 1 but already used, it's place in 2
 *    A   |  D  | 
 *    B   |  E  | 
 *    C   |  G* | 
 * 3. Column
 *   F has parents G*, D, E note that C was replaced with a dummy. The average is 2 a free place
 *    A   |  D  | 
 *    B   |  E  | F
 *    C   |  G* | 
 *
 * now we create the arrows
 *    A-\-/->D--\
 *    B-/ \->E--->F
 *    C -----G*-/ 
 * 
 * author Lathanda
 */
class TechTreeLayout {
public:
    class Node;

    class Edge;

    class Column;

    ~TechTreeLayout();

    void AddNode(std::string const & tech, GG::X width, GG::Y height);

    auto GetNode(std::string const & name) const -> Node const *;

    auto GetWidth(std::string const & name) const -> double;

    auto GetHeight(std::string const & name) const;

    void AddEdge(std::string const & parent, std::string const & child);

    auto GetOutEdges(std::string const & name) const -> std::vector<Edge*> const &;

    auto GetWidth() const -> GG::X const;

    auto GetHeight() const -> GG::Y const;

    void Clear();

    void DoLayout( double column_width, double row_height, double x_margin);

    void Debug() const;

private:
    //! width of the complete graph
    double m_width = 0;

    //! height of the complete graph
    double m_height = 0;

    int m_row_count = 0;

    int m_column_count = 0;

    //! map name->node for external access by name
    std::map<std::string, Node*> m_node_map;

    //! list of nodes for sorting
    std::vector<Node*> m_nodes;
};


class TechTreeLayout::Edge {
public:
    Edge(std::string const & from, std::string const & to);

    auto GetTechFrom() const -> std::string const &;

    auto GetTechTo() const -> std::string const &;

    void ReadPoints(std::vector<std::pair<double,double>> & points) const;

    void AddPoint(double x, double y);

    void Debug() const;

private:
    //! point list of connection
    std::vector<std::pair<double, double>> m_points;

    //! source tech
    std::string m_from;

    //! destination tech
    std::string m_to;
};


class TechTreeLayout::Node {
    friend class TechTreeLayout;

    friend class Column;

public:
    Node(std::string const & tech, GG::X width, GG::Y height);

    ~Node();

    auto GetX() const -> GG::X const;

    auto GetY() const -> GG::Y const;

    void Debug() const;

    auto operator<(Node const & y) const -> bool;

private:
    Node(Node* parent, Node* child);

    auto Wobble(Column & column) -> bool;

    void AddChild(Node* node);

    void fillDepth();

    auto CalculateFamilyDistance(int row) -> double;

    void CalculateCoordinate(double column_width, double row_height);

    void CreateEdges(double x_margin, double column_width, double row_height);

    std::vector<Node*> parents;

    std::vector<Node*> children;

    unsigned int depth = 1;

    //! layout row, every node is organized in a straight tabelle system
    int row = -1;

    //! height in rows
    const int weight;

    std::string tech_name;

    bool place_holder;

    // primary child for layout
    Node* primary_child = nullptr;

    std::vector<Edge*> outgoing_edges;

    //! left border
    double m_x = 0.0;

    //! top border
    double m_y = 0.0;

    //! width
    double m_width;

    //! height
    double m_height;
};


class TechTreeLayout::Column {
public:
    auto Fit(int index, TechTreeLayout::Node* node) -> bool;

    auto ClosestFreeIndex(int index, TechTreeLayout::Node* node) -> int;

    auto Place(int index, TechTreeLayout::Node* node) -> bool;

    std::vector<TechTreeLayout::Node*> column;
};

#endif
