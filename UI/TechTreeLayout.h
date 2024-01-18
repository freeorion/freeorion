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

    void AddNode(const std::string& tech, GG::X width, GG::Y height);
    auto GetNode(const std::string& name) const -> Node const *;
    void AddEdge(const std::string& parent, std::string const & child);
    auto GetOutEdges(const std::string& name) const -> const std::vector<Edge>&;
    auto GetWidth() const -> GG::X const;
    auto GetHeight() const -> GG::Y const;
    void Clear();
    void DoLayout( double column_width, double row_height, double x_margin);
    void Debug() const;

private:
    double m_width = 0; //! width of the complete graph
    double m_height = 0; //! height of the complete graph
    int m_row_count = 0;
    int m_column_count = 0;
    std::map<std::string, Node*> m_node_map; //! map name->node for external access by name
    std::vector<Node*> m_nodes; //! list of nodes for sorting
};


class TechTreeLayout::Edge {
public:
    [[nodiscard]] Edge(std::string from, std::string to, uint32_t points = 3);

    [[nodiscard]] auto& GetTechFrom() const noexcept { return m_from; }
    [[nodiscard]] auto& GetTechTo() const noexcept { return m_to; }
    [[nodiscard]] auto& Points() const noexcept { return m_points; }

    void AddPoint(double x, double y) { m_points.emplace_back(x, y); }
    void Debug() const;

private:
    std::vector<std::pair<double, double>> m_points; //! point list of connection
    std::string m_from; //! source tech
    std::string m_to;   //! destination tech
};


class TechTreeLayout::Node {
    friend class TechTreeLayout;
    friend class Column;

public:
    Node(std::string tech, GG::X width, GG::Y height);

    auto GetX() const -> GG::X const;
    auto GetY() const -> GG::Y const;
    void Debug() const;
    auto operator<(const Node& y) const -> bool;

private:
    Node(Node* parent, Node* child, std::vector<Node*>& nodes);

    auto Wobble(Column& column) -> bool;
    void AddChild(Node* node);
    void SetDepthRecursive(int depth);
    auto CalculateFamilyDistance(int row_) -> double;
    void CreatePlaceHolder(std::vector<Node*>& nodes);
    void DoLayout(std::vector<Column>& row_index, bool cat);
    void CalculateCoordinate(double column_width, double row_height);
    void CreateEdges(double x_margin, double column_width, double row_height);

    std::vector<Node*> parents;
    std::vector<Node*> children;
    int depth = -1; //! depth 1 available at beginning 2 one requisite etc
    int row = -1; //! layout row, every node is organized in a straight tabelle system
    const int weight; //! height in rows
    std::string tech_name;
    bool place_holder = false;
    Node* primary_child = nullptr; // primary child for layout
    std::vector<Edge> outgoing_edges;
    double m_x = 0.0;
    double m_y = 0.0;
    double m_width = 0.0;
    double m_height = 0.0;
};


class TechTreeLayout::Column {
public:
    auto Fit(int index, TechTreeLayout::Node* node) -> bool;

    auto ClosestFreeIndex(int index, TechTreeLayout::Node* node) -> int;

    auto Place(int index, TechTreeLayout::Node* node) -> bool;

    std::vector<TechTreeLayout::Node*> column;
};

#endif
