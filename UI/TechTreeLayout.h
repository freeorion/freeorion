#ifndef _TechTreeLayout_h_
#define _TechTreeLayout_h_
#include <map>
#include <list>
#include <string>
#include <vector>

#include <GG/PtRect.h>
#include "../universe/Tech.h"


/**
 * This class stores internal information for layouting the tech graph.
 * The whole space is organized in a table.
 * Every tech is assign a tech level, or depth, which is the maximum distance it is from start techs.
 * Every tech level (m_depth) is one column.
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
    TechTreeLayout();
    ~TechTreeLayout();
    void                        AddNode(const std::string& tech, GG::X width, GG::Y height);
    const Node*                 GetNode(const std::string & name) const;
    void                        AddEdge(const std::string & parent, const std::string & child);
    const GG::X                 GetWidth() const;
    const GG::Y                 GetHeight() const;
    double                      GetWidth(const std::string & name ) const;
    double                      GetHeight(const std::string & name ) const;
    void                        Clear();
    void                        DoLayout( double column_width, double row_height, double x_margin);
    const std::vector<Edge*>&   GetOutEdges(const std::string & name) const;
    void                        Debug() const;
private:
    double                          m_width; //width of the complete graph
    double                          m_height; //height of the complete graph
    int                             m_row_count;
    int                             m_column_count;
    std::map< std::string, Node*>   m_node_map; //map name->node for external access by name
    std::vector<Node*>              m_nodes; // list of nodes for sorting
};

class TechTreeLayout::Edge {
public:
    Edge(const std::string& from, const std::string& to);
    ~Edge();
    const std::string&  GetTechFrom() const;
    const std::string&  GetTechTo() const;
    void                ReadPoints(std::vector<std::pair<double,double>> & points) const;
    void                AddPoint(double x, double y);
    void                Debug() const;
private:
    std::vector<std::pair<double, double>>  m_points;   // point list of connection
    std::string                             m_from;     //source tech
    std::string                             m_to;       //destination tech
};

class TechTreeLayout::Node {
    friend class TechTreeLayout;
    friend class Column;
public:
    Node(const std::string& tech, GG::X width, GG::Y height);
    ~Node();

    const GG::X                 GetX() const;
    const GG::Y                 GetY() const;
    int                         GetDepth() const;
    const std::string&          GetTech() const;
    const std::vector<Edge*>&   GetOutEdges() const;
    int                         GetNumberOfChildren() const;
    int                         GetNumberOfParents() const;
    void                        Debug() const;
    bool                        IsFinalNode() const;
    bool                        IsStartNode() const;
    int                         GetWeight() const { return m_weight; };

    bool operator<(const Node& y) const;

private:
    Node(Node* parent, Node* child, std::vector<Node*>& nodes);

    bool    Wobble(Column & column);
    bool    IsPlaceHolder() const;
    void    AddChild(Node* node);
    void    SetDepthRecursive(int depth);
    double  CalculateFamilyDistance(int row);
    void    CreatePlaceHolder(std::vector<Node*>& nodes);
    void    DoLayout(std::vector<Column> & row_index, bool cat);
    void    CalculateCoordinate(double column_width, double row_height);
    void    CreateEdges(double x_margin, double column_width, double row_height);

    int                 m_depth = -1;   // depth 1 available at beginning 2 one requisite etc
    int                 m_row = -1;     // layout row, every node is organized in a straight tabelle system
    std::string         m_tech;         // name
    double              m_x = 0.0;      // left border
    double              m_y = 0.0;      // top border
    double              m_width;        // width
    double              m_height;       // height
    bool                m_place_holder; // is place holder
    int                 m_children_rows = 0; // height in cells for layouting
    std::vector<Node*>  m_parents;      // parents
    std::vector<Node*>  m_children;     // children
    Node*               m_primary_child = nullptr; // primary child for layout
    std::vector<Edge*>  m_out_edges;    // outgoing edges
    const int           m_weight;       // height in rows

};

class TechTreeLayout::Column {
public:
    Column();
    bool            Fit(int index, TechTreeLayout::Node* node);
    bool            PlaceClosestFreeIndex(int index, TechTreeLayout::Node* node);
    int             ClosestFreeIndex(int index, TechTreeLayout::Node* node);
    bool            Place(int index, TechTreeLayout::Node* node);
    bool            Move(int to, TechTreeLayout::Node* node);
    Node*           Seek(Node* m, int direction);
    bool            Swap(Node* m, Node* n);
    unsigned int    Size();
private:
    std::vector<TechTreeLayout::Node*> m_column;
};
#endif
