#include "TechTreeLayout.h"
#include "../util/AppInterface.h"
#include <algorithm>

namespace  {
    const int NODE_CELL_HEIGHT = 2;
    const int LINE_CELL_HEIGHT = 1;
    const int CATEGORY_SEPARATOR_HEIGHT = 6;

//////////////////////////
// struct NodePointerCmp //
//////////////////////////
    struct NodePointerCmp{
        bool operator ()(const TechTreeLayout::Node* x, const TechTreeLayout::Node* y) const {
            return (*x) < (*y);
        }
    };
}//namespace
//////////////////
// class Column //
//////////////////
TechTreeLayout::Column::Column() : 
    m_column() {
}
bool TechTreeLayout::Column::Fit(int index, TechTreeLayout::Node* node) {
    int size = m_column.size();
    if (index + node->m_weight > size) {
        m_column.resize(index + node->m_weight, 0);
    } 
    if (0 < index) {
        for ( int j = index + node->m_weight; j-->index; ) {
            if ( m_column[j] != 0 && m_column[j] != node ) {
                return false;
            }
        }
        return true;
    }
    return false;
}
bool TechTreeLayout::Column::PlaceNextFree(int index, TechTreeLayout::Node* node) {
    int step = 0;
    int i = index;
    while (true) {
        i = i - step;
        step = step + 1;
        if (Place(i, node)) {
            return true;
        }
        i = i + step;
        step = step + 1;
        if (Place(i, node)) {
            return true;
        }
    }
    return false; //should never happen :-)
}
int TechTreeLayout::Column::NextFree(int index, TechTreeLayout::Node* node) {
    int step = 0;
    int i = index;
    while (true) {
        i = i - step;
        step = step + 1;
        if (Fit(i, node)) {
            return i;
        }
        i = i + step;
        step = step + 1;
        if (Fit(i, node)) {
            return i;
        }
    }
    return -1; //should never happen :-)
}
bool TechTreeLayout::Column::Place(int index, TechTreeLayout::Node* node) {
    if (Fit(index, node)) {
        for ( int i = index + node->m_weight; i-->index; ) {
            m_column[i] = node;
        }
        node->m_row = index;
        return true;
    } else {
        return false;
    }
}
bool TechTreeLayout::Column::Move(int to, TechTreeLayout::Node* node) {
    if (Fit(to, node)) {
        for ( int i = node->m_row + node->m_weight; i-->node->m_row; ) {
            m_column[i] = 0;
        }
        Place(to, node);
        return true;
    } else {
        return false;
    }
}
unsigned int TechTreeLayout::Column::Size() {
    for (unsigned int i = m_column.size(); i --> 0; ) {
        if (m_column[i] != 0) {
            return i;
        }
    }
    return 0;
}
TechTreeLayout::Node* TechTreeLayout::Column::Seek(Node* m, int direction) {
    Node* n = 0;
    int i = m->m_row;
    while(n == 0 || n == m) {
        if (i < 0 || i >= static_cast<int>(m_column.size())) {
            return 0;
        }
        n = m_column[i];
        i += direction;
    }
    return n;
}
bool TechTreeLayout::Column::Swap(Node* m, Node* n) {
    if (m->m_weight != n->m_weight) {
        //if they have different size shifting would be needed
        return false;
    }
    for(int i = 0; i < m->m_weight; i++) {
        m_column[m->m_row + i] = n;
        m_column[n->m_row + i] = m;
    }
    int t_row = m->m_row;
    m->m_row = n->m_row;
    n->m_row = t_row;
    return true;
}

////////////////
// class Edge //
////////////////
TechTreeLayout::Edge::Edge(const std::string& from, const std::string& to) :
    m_points(std::vector<std::pair<double,double> >( )),
    m_from(from),
    m_to(to)
{
    assert(GetTech(from) && GetTech(to));
}
TechTreeLayout::Edge::~Edge( ) {
    m_points.clear( );
}
const std::string& TechTreeLayout::Edge::GetTechFrom( ) const {
    return m_from;
}
const std::string& TechTreeLayout::Edge::GetTechTo( ) const {
    return m_to;
}
void TechTreeLayout::Edge::AddPoint(double x, double y) {
    m_points.push_back(std::pair<double,double>(x, y));
}
void TechTreeLayout::Edge::ReadPoints(std::vector<std::pair<double,double> > & points) const {
    for(std::vector<std::pair<double,double> >::const_iterator p = m_points.begin(); p != m_points.end(); p++) {
        points.push_back(*p);
    }
}

void TechTreeLayout::Edge::Debug() const {
    Logger().debugStream() << "Edge " << m_from << "-> " << m_to << ": ";
    for(std::vector<std::pair<double,double> >::const_iterator p = m_points.begin(); p != m_points.end(); p++) {
        Logger().debugStream() << "(" << (*p).first << "," << (*p).second << ") ";
    }
    Logger().debugStream() << "\n";
}
///////////////////////
// class LayoutGraph //
///////////////////////
/**
 * creates a new empty graph
 */
TechTreeLayout::TechTreeLayout() :
    m_width(0),
    m_height(0),
    m_row_count(0),
    m_column_count(0),
    m_node_map(),
    m_nodes()
{}
/**
 * releases all resources
 */
TechTreeLayout::~TechTreeLayout( ) {
    for(std::vector<TechTreeLayout::Node*>::iterator p = m_nodes.begin(); p != m_nodes.end(); p++) {
        delete *p;
    }
    m_nodes.clear( );
    m_node_map.clear( );
}
/**
 * creates and initialises all nodes
 * @param column_width width of each column
 * @param row_height height of each row
 * @param x_margin horizontal part of arrow before changing direction to child node
 */
void TechTreeLayout::DoLayout( double column_width, double row_height, double x_margin) {
    assert(column_width > 0 && row_height > 0);
    double internal_height = row_height / NODE_CELL_HEIGHT; // node has NODE_CELL_HEIGHT rows internally
    //1. calculate depth
    for(int i = m_nodes.size(); i-->0;) {
        m_nodes[i]->CalculateDepth( );
    }
    //2. create placeholder nodes
    for(int i = m_nodes.size(); i-->0;) {
        m_nodes[i]->CreatePlaceHolder(m_nodes );
    }
    //3. sort nodes by depth category and other fancy stuff
    std::sort(m_nodes.begin(), m_nodes.end(), NodePointerCmp());

    //4. do layout
    //4.a. prepare layout data array
    std::vector<Column> row_index = std::vector<Column>();
    //4.b. distribute tech nodes over the table
    std::string cat = "";
    for(unsigned int i = 0; i < m_nodes.size(); i++) {
        const Tech* node_tech = GetTech(m_nodes[i]->GetTech());
        std::string& node_tech_cat = node_tech ? node_tech->Category() : "";
        bool new_cat = node_tech_cat != cat;
        m_nodes[i]->DoLayout( row_index, new_cat);
        cat = node_tech_cat;
    }
    //4.c. optimize layout, every node gets a rateing if moving would shorten the distance to it's family
    //     if the movement is possible eith if the place if free or the neighbour has the opposite wish
    bool movement = true;
    while (movement) {
        movement = false;
        for(unsigned int i = m_nodes.size(); i --> 0;) {
            if (m_nodes[i]->Wobble(row_index[m_nodes[i]->m_depth])) {
                movement = true;
                break;
            }
        }
    }
    //4.d. count used rows and columns
    unsigned int column_count = row_index.size();
    unsigned int row_count = 0;
    for(int i =row_index.size(); i-->0;) {
        row_count = std::max(row_count, row_index[i].Size());
    }
    //4.e. set size
     for(int i = m_nodes.size(); i --> 0 ; ) {
         m_nodes[i]->CalculateCoordinate(column_width, internal_height);
    }

    m_width = column_count * column_width;
    m_height = row_count * internal_height;

    //5. create edges
     for(int i = m_nodes.size(); i --> 0 ; ) {
        m_nodes[i]->CreateEdges( x_margin, column_width, internal_height );
    }
}

const GG::X TechTreeLayout::GetWidth( ) const {
    return GG::X(static_cast<int>(m_width));
}
const GG::Y TechTreeLayout::GetHeight( ) const {
    return GG::Y(static_cast<int>(m_height));
}
const TechTreeLayout::Node* TechTreeLayout::GetNode(const std::string & name) const {
    std::map< std::string, TechTreeLayout::Node*>::const_iterator item = m_node_map.find(name);
    if (item == m_node_map.end()) {
        Logger().debugStream() << "TechTreeLayout::getNode: missing node " << name << "\n";
        Debug();
        throw "node missing";
    } else {
        return (*item).second;
    }
}
void TechTreeLayout::AddNode(const std::string& tech, GG::X width, GG::Y height) {
    assert(width > 0 && height > 0 && GetTech(tech));
    TechTreeLayout::Node *node = new TechTreeLayout::Node(tech, width, height);
    m_nodes.push_back(node);
    m_node_map[tech] = node;
}
void TechTreeLayout::AddEdge(const std::string & parent, const std::string & child) {
    std::map< std::string, TechTreeLayout::Node*>::iterator p = m_node_map.find(parent);
    std::map< std::string, TechTreeLayout::Node*>::iterator c = m_node_map.find(child);
    assert(p != m_node_map.end() && c != m_node_map.end());

    (*p).second->AddChild((*c).second);
}
const std::vector<TechTreeLayout::Edge*> & TechTreeLayout::GetOutEdges( const std::string & name) const {
    std::map< std::string, TechTreeLayout::Node*>::const_iterator item = m_node_map.find(name);
    if (item == m_node_map.end()) {
        Logger().debugStream() << "TechTreeLayout::getNode: missing node " << name << "\n";
        Debug();
        throw "node missing";
    } else {
        return (*item).second->GetOutEdges();
    }
}
void TechTreeLayout::Debug() const {
    for(std::vector<TechTreeLayout::Node*>::const_iterator n = m_nodes.begin(); n != m_nodes.end(); n++) {
        (*n)->Debug( );
    }
}
void TechTreeLayout::Clear() {
    //!!! IMPORTANT !!! Node have to be delete in order m_depth ascending or we will access freed memory!
    for(std::vector<TechTreeLayout::Node*>::iterator p = m_nodes.begin(); p != m_nodes.end(); p++) {
        delete *p;
    }
    m_nodes.clear( );
    m_node_map.clear( );
}

////////////////
// class Node //
////////////////
/**
 * creates a node for that tech
 */
TechTreeLayout::Node::Node(const std::string& tech, GG::X width, GG::Y height) :
    m_weight(NODE_CELL_HEIGHT),
    m_depth(-1),
    m_row(-1),
    m_tech(tech),
    m_x(0),
    m_y(0),
    m_width(Value(width)),
    m_height(Value(height)),
    m_place_holder(false),
    m_children_rows(0),
    m_parents(),
    m_children(),
    m_child(0),
    m_out_edges()
{
    assert(width > 0 && height > 0 && ::GetTech(tech));
}
/**
 * creates dummie nodes between parent and child
 */
TechTreeLayout::Node::Node(Node *parent, Node *child, std::vector<Node*> & nodes) :
    m_weight(LINE_CELL_HEIGHT),
    m_depth(parent->m_depth + 1),
    m_row(-1),
    m_tech(child->m_tech),
    m_x(0),
    m_y(0),
    m_width(0),
    m_height(0),
    m_place_holder(true),
    m_children_rows(0),
    m_parents(),
    m_children(),
    m_child(0),
    m_out_edges()
{
    assert(parent != 0 && child != 0);
    if ( m_depth + 1 == child->m_depth ) {
        m_children.push_back(child);
        m_child = child;
    } else {
        Node *placeholder = new Node(this, child, nodes);
        m_children.push_back(placeholder);
        m_child = placeholder;
    }
    m_parents.push_back(parent);
    nodes.push_back(this);
}
TechTreeLayout::Node::~Node( ) {
    m_children.clear( );
    m_parents.clear( );
    m_out_edges.clear( );
}
unsigned int TechTreeLayout::Node::GetDepth() const {
    return m_depth;
}
const std::string& TechTreeLayout::Node::GetTech( ) const {
    return m_tech;
}
int TechTreeLayout::Node::GetNumberOfChildren( ) const {
    return m_children.size( );
}
int TechTreeLayout::Node::GetNumberOfParents( ) const {
    return m_parents.size( );
}
const std::vector<TechTreeLayout::Edge*>& TechTreeLayout::Node::GetOutEdges( ) const {
    return m_out_edges;
}
const GG::X TechTreeLayout::Node::GetX() const{
    return GG::X(static_cast<int>(m_x));
}
const GG::Y TechTreeLayout::Node::GetY() const {
    return GG::Y(static_cast<int>(m_y));
}
void TechTreeLayout::Node::CalculateCoordinate( double column_width, double row_height) {
    m_x = (static_cast<double>(m_depth) - 0.5) * column_width;
    m_y = m_row * row_height;
}
bool TechTreeLayout::Node::IsFinalNode( ) const {
    return m_children.empty( );
}
bool TechTreeLayout::Node::IsStartNode( ) const {
    return m_parents.empty( );
}
bool TechTreeLayout::Node::IsPlaceHolder( ) const {
    return m_place_holder;
}
double TechTreeLayout::Node::CalculateFamilyDistance(int row) {
    double distance = 0;
    int familysize = (m_parents.size() + m_children.size());
    if (familysize == 0) return 0;
    Node* node = 0;
    for(int i = m_children.size(); i-->0; ) {
        node = m_children[i];
        while (node->IsPlaceHolder()) {
            node = node->m_children[0];
        }
        distance      += (node->m_row - row);
    }
    for(int i = m_parents.size(); i-->0; ) {
        node = m_parents[i];
        while (node->IsPlaceHolder()) {
            node = node->m_parents[0];
        }
        distance      += (node->m_row - row);
    }
    if (std::abs(distance) > 1000) {
        //std::cout << "error " << m_name << "(" << distance << ")";
    }
    return distance / familysize;
}

bool TechTreeLayout::Node::Wobble(Column & column) {
    double dist, new_dist, s_dist, new_s_dist;
    dist = CalculateFamilyDistance(m_row);
    int direction = (dist > 0)?+1:-1;
    double improvement = 0;
    //try to find free space arround optimal position
    int nextfree = column.NextFree((int)(m_row+dist+0.5), this);
    //check if that space is better
    new_dist = CalculateFamilyDistance(nextfree);
    improvement = std::abs(dist) - std::abs(new_dist);
    if (improvement > 0) {
        if(column.Move(nextfree , this) ) {
            //std::cout << m_name << ":" << dist << " -> " << new_dist <<"\n";
            return true;
        }
    }

    //try to switch node with neighbour node
    //find neighbour
    Node* n = column.Seek(this, direction);
    if (n != 0) {
        s_dist     = n->CalculateFamilyDistance(n->m_row);
        new_s_dist = n->CalculateFamilyDistance(   m_row);
        new_dist   =    CalculateFamilyDistance(n->m_row);
        improvement = std::abs(dist) + std::abs(s_dist) - std::abs(new_dist) - std::abs(new_s_dist);
        if (improvement > 0.5) { //0 produces endless loop
            if (column.Swap(this, n)) {
                //std::cout << "(S)" << m_name  << ":" << dist << " -> " << new_dist << " & "<< n->m_name  << ":" << s_dist << " -> " << new_s_dist << "\n";
                return true;
            }
        }
    }
    return false;
}
bool TechTreeLayout::Node::operator<(const TechTreeLayout::Node & y) const {
    if (m_depth == y.m_depth) {
        const Tech* this_node_tech = ::GetTech(m_tech);
        assert(this_node_tech);
        const Tech* y_node_tech = ::GetTech(y.m_tech);
        assert(y_node_tech);

        if (this_node_tech->Category() == y_node_tech->Category()) {
            return m_tech <  y.m_tech;
        } else {
            return this_node_tech->Category() < y_node_tech->Category();
        }
    } else {
        return m_depth < y.m_depth;
    }
}
void TechTreeLayout::Node::AddChild(Node* node) {
    assert(node);
    m_children.push_back(node);
    node->m_parents.push_back(this);
    //Note: m_child is used as sorting criteria
    // nodes with the same child get drawn "together"
    // so we need to choose wisely which node is our heir 
    // 1. we prefere childs with the same category, or lexically
    // 2. we order lexically
    if (!m_child) {
        //no choice
        m_child = node;
        return;
    }

    if (node->m_place_holder)
        return;

    //reaching this case means we will get crossing lines
    //we decide which node is the better child for layout decisions
    const Tech* this_node_tech = ::GetTech(m_tech);
    assert(this_node_tech);
    const Tech* child_node_tech = ::GetTech(m_child->m_tech);
    assert(child_node_tech);
    const Tech* input_node_tech = ::GetTech(node->m_tech);
    assert(input_node_tech);

    int n = ((child_node_tech->Category() == this_node_tech->Category()) ? 0 : 1)
        //binary case 00 01 10 11
        +  ((input_node_tech->Category() == this_node_tech->Category()) ? 0 : 2);
    switch (n) {
    case 0: //both categories fit
        if (node->m_tech < m_child->m_tech) {
            m_child = node;
        }
        break;
    case 1:  //only m_child category fits
        //keep m_child 
        break;
    case 2: //only node category fits
        m_child = node;
        break;
    case 3: //both categories are wrong
        if (input_node_tech->Category() < child_node_tech->Category()) {
            m_child = node;
        } //else keep child
        break;
    }
}
void TechTreeLayout::Node::CalculateDepth( ) {
    if (!IsStartNode( ))
        return; // no start node

    //start node
    m_depth = 0;
    for(int i = m_children.size(); i-->0; ) {
        m_children[i]->CalculateDepth(1);
    }
}
void TechTreeLayout::Node::CalculateDepth( int depth ) {
    assert(depth >= 0);
    if ( m_depth < depth ) {
        m_depth = depth;
        for(int i = m_children.size(); i-->0; ) {
             m_children[i]->CalculateDepth(m_depth + 1);
        }
    }
}

void TechTreeLayout::Node::CreatePlaceHolder(std::vector<Node*> & nodes ) {
    for(int i = m_children.size(); i --> 0; ) {
        if (m_depth + 1 <  m_children[i]->m_depth) {
            m_children_rows += LINE_CELL_HEIGHT;
            //create a dummy
            Node* n = new Node(this,  m_children[i], nodes);
            //replace parent with dummy
            std::replace( m_children[i]->m_parents.begin(),  m_children[i]->m_parents.end(), this, n);
             m_children[i] = n;
        } else {
            m_children_rows += NODE_CELL_HEIGHT;
        }
    }
    m_children_rows = std::max(m_children_rows, NODE_CELL_HEIGHT);
}
void TechTreeLayout::Node::DoLayout( std::vector<Column> & row_index, bool cat ) {
    //assert(row_height > 0 && column_width > 0 && row_index != 0);
    if(m_row != -1) return; //already done
    // 2. place node
    int index = 0;
    int count = 0;
    //check children
    for(int i = m_children.size(); i --> 0; ) {
        if (m_children[i]->m_row != -1) {
            index += m_children[i]->m_row;
            count++;
        }
    }
    //check parents
    for(int i = m_parents.size(); i --> 0; ) {
        if (m_parents[i]->m_row != -1) {
            index += m_parents[i]->m_row;
            count++;
        }
    }
    if ((int)row_index.size() < m_depth + 1) row_index.resize(m_depth + 1);
    if (count != 0) {
        row_index[m_depth].PlaceNextFree(index / count, this);
    } else {
        row_index[m_depth].PlaceNextFree(1, this);
    }

    // 3. place holder children
/*    Node* n = 0;
    for(int i = m_children.size(); i --> 0; ) {
        n = m_children[i];
        while (n->m_place_holder) {
            if ((int)row_index.size() < n->m_depth + 1) row_index.resize(n->m_depth + 1);
            row_index[n->m_depth].PlaceNextFree(m_row, n);
            n = n->m_child;
        }
    }*/
}

void TechTreeLayout::Node::CreateEdges( double x_margin, double column_width, double row_height ) {
    assert(column_width > 0);
    for (int i = m_children.size(); i --> 0; ) {
        //find next real node and create coordinates
        Node* next = m_children[i];
        while (next->m_place_holder) { 
            next->CalculateCoordinate( column_width, row_height);
            next = next->m_child; 
        }
        const std::string& to = next->m_tech;
        //create drawing path
        next = m_children[i];
        Edge* edge = new Edge(m_tech, to);
        //from, line start
        edge->AddPoint(m_x + m_width / 2, m_y + m_height / 2); // start in the middle of the node
        edge->AddPoint(m_x + m_width + x_margin, m_y + m_height / 2);
        //draw line until a real tech is reached
        while (next->m_place_holder) {
            //horizontal line bypassing the placeholder
            edge->AddPoint(next->m_x - 2 * x_margin, next->m_y + row_height / 2);
            edge->AddPoint(next->m_x + m_width + x_margin, next->m_y + row_height / 2);
            next = next->m_child;
        }
        //to, line end
        edge->AddPoint(next->m_x - 2 * x_margin, next->m_y + next->m_height / 2); //double space for arrow
        edge->AddPoint(next->m_x, next->m_y + next->m_height / 2); // the end has to be exact for the arrow head
        //store drawing path
        m_out_edges.push_back(edge);
    }
}
void TechTreeLayout::Node::Debug() const {
    Logger().debugStream() << "Tech - " << m_tech << " (" << m_x << "," << m_y << ") #" << m_depth << "\n";
    Logger().debugStream() << "  Parents - ";
    for(int i = m_parents.size(); i --> 0; ) {
        Logger().debugStream() << m_parents[i]->m_tech << "#" << m_parents[i]->m_depth;
    }
    Logger().debugStream() << "\n";
    Logger().debugStream() << "  Children - ";
    for(int i = m_children.size(); i --> 0; ) {
        Logger().debugStream() << m_children[i]->m_tech << "#" << m_children[i]->m_depth;;
    }
    Logger().debugStream() << "\n";

    for (int i = m_out_edges.size(); i-->0; ) {
        Logger().debugStream() << "     - ";
        m_out_edges[i]->Debug();
    }
}
