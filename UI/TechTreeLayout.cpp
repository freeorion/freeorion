#include "TechTreeLayout.h"

#include "../universe/Tech.h"
#include "../util/Logger.h"
#include <algorithm>
#include <cmath>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

namespace  {
    const int NODE_CELL_HEIGHT = 2;
    const int LINE_CELL_HEIGHT = 1;
}


//////////////////
// class Column //
//////////////////
bool TechTreeLayout::Column::Fit(int index, TechTreeLayout::Node* node) {
    if (0 >= index)
        return false;

    int size = column.size();
    if (index + node->weight > size)
        column.resize(index + node->weight, nullptr);

    for (int j = index + node->weight; j-->index; ) {
        if (column[j] != nullptr && column[j] != node)
            return false;
    }
    return true;
}

int TechTreeLayout::Column::ClosestFreeIndex(int index, TechTreeLayout::Node* node) {
    // search for free node nearest to ideal index:
    // start at ideal, then search in adjacent indices above and below
    // in order of increasing distance to find a free spot.
    int step = 0;
    int i = index;
    while (true) {
        i = i - step;
        step = step + 1;
        if (Fit(i, node))
            return i;

        i = i + step;
        step = step + 1;
        if (Fit(i, node))
            return i;
    }
    return -1; //should never happen :-)
}

bool TechTreeLayout::Column::Place(int index, TechTreeLayout::Node* node) {
    if (Fit(index, node)) {
        for (int i = index + node->weight; i-->index; )
            column[i] = node;

        node->row = index;
        return true;
    }
    return false;
}


////////////////
// class Edge //
////////////////
TechTreeLayout::Edge::Edge(const std::string& from, const std::string& to) :
    m_points(std::vector<std::pair<double, double>>()),
    m_from(from),
    m_to(to)
{ assert(GetTech(from) && GetTech(to)); }

const std::string& TechTreeLayout::Edge::GetTechFrom() const
{ return m_from; }

const std::string& TechTreeLayout::Edge::GetTechTo() const
{ return m_to; }

void TechTreeLayout::Edge::AddPoint(double x, double y)
{ m_points.push_back(std::pair<double, double>(x, y)); }

void TechTreeLayout::Edge::ReadPoints(std::vector<std::pair<double, double>>& points) const {
    for (const auto& p : m_points)
        points.push_back(p);
}

void TechTreeLayout::Edge::Debug() const {
    DebugLogger() << "Edge " << m_from << "-> " << m_to << ": ";
    for (const auto& p : m_points)
        DebugLogger() << "(" << p.first << "," << p.second << ") ";
    DebugLogger() << "\n";
}

/**
 * releases all resources
 */
TechTreeLayout::~TechTreeLayout() {
    for (Node* node : m_nodes)
        delete node;
}

/**
 * creates and initialises all nodes
 * @param column_width width of each column
 * @param row_height height of each row
 * @param x_margin horizontal part of arrow before changing direction to child node
 */
void TechTreeLayout::DoLayout(double column_width, double row_height, double x_margin) {
    assert(column_width > 0 && row_height > 0);
    double internal_height = row_height / NODE_CELL_HEIGHT; // node has NODE_CELL_HEIGHT rows internally
    // 1. Set depth of all nodes
    for (Node* node : m_nodes)
         node->fillDepth();

    // 2. Insert placeholder nodes between display nodes, whose depth difference
    //    exceeds 1.
    auto initial_nodes = m_nodes;
    for (auto node : initial_nodes) {
        for (auto current_child : node->children) {
            auto current_parent = node;

            while (current_parent->depth + 1 < current_child->depth) {
                auto placeholder = new Node(current_parent, current_child);
                m_nodes.push_back(placeholder);

                boost::range::replace(current_child->parents, current_parent, placeholder);
                boost::range::replace(current_parent->children, current_child, placeholder);

                if (current_parent->primary_child == current_child)
                    current_parent->primary_child = placeholder;

                current_parent = placeholder;
            }
        }
    }

    // 3. Sort nodes by depth level, category and name
    boost::range::sort(m_nodes, [](Node* l, Node* r) -> bool { return *l < *r; });

    // 4. Separate tree nodes into layers by depth
    unsigned int max_node_depth = (!m_nodes.empty()) ? m_nodes.back()->depth : 0;
    std::vector<std::vector<Node*>> tree_layers(max_node_depth + 1);
    for (Node* node : m_nodes)
        tree_layers[node->depth].push_back(node);

    // 5. Find widest tree layer
    unsigned int widest_layer_idx = 0;
    for (auto cur_layer_idx : boost::irange(size_t{}, tree_layers.size()))
        if (tree_layers[widest_layer_idx].size() < tree_layers[cur_layer_idx].size())
            widest_layer_idx = cur_layer_idx;

    // 6. Process layers starting from the widest layer
    std::vector<unsigned int> layer_order;
    layer_order.reserve(tree_layers.size());
    layer_order.push_back(widest_layer_idx);
    int offset = 0;
    while (layer_order.size() < tree_layers.size()) {
        ++offset;
        if (0 <= widest_layer_idx - offset)
            layer_order.push_back(widest_layer_idx - offset);
        if (widest_layer_idx + offset < tree_layers.size())
            layer_order.push_back(widest_layer_idx + offset);
    }

    // 7. Place nodes on layer columns trying to place relative nodes close
    //    together.
    std::vector<Column> row_index = std::vector<Column>(tree_layers.size());
    for (int layer : layer_order) {
        std::string current_category;
        for (Node* node : tree_layers[layer]) {
            if (node->row != -1)
                continue;

            // find average row index of node's children and parents

            // 2. place node
            int index = 0;
            int count = 0;
            //check children
            for (auto child : node->children) {
                if (child->row != -1) {
                    index += child->row;
                    count++;
                }
            }
            //check parents
            for (auto parent : node->parents) {
                if (parent->row != -1) {
                    index += parent->row;
                    count++;
                }
            }

            // if any parents or children have been placed, put this node in next free
            // space after the ideal node.  if no parents or children have been placed,
            // put node at start of row
            int index_offset = (count != 0) ? index / count : 1;
            row_index[node->depth].Place(row_index[node->depth].ClosestFreeIndex(index_offset, node), node);

            const Tech* node_tech = GetTech(node->tech_name);
            current_category = node_tech ? node_tech->Category() : "";
        }
    }

    // optimize layout, every node gets a rating if moving would shorten the distance to it's family
    // if the movement is possible either if the place if free or the neighbour has the opposite wish
    bool movement = true;
    while (movement) {
        movement = false;

        for (auto node : m_nodes) {
            auto& column = row_index[node->depth];

            double dist = node->CalculateFamilyDistance(node->row);

            //try to find free space arround optimal position
            int closest_free_index = column.ClosestFreeIndex(static_cast<int>(node->row + dist + 0.5), node);

            //check if that space is better
            double improvement = std::abs(dist)
                               - std::abs(node->CalculateFamilyDistance(closest_free_index));

            if (improvement > 0.25) {
                if (column.Fit(closest_free_index, node)) {
                    for (int i = node->row + node->weight; i --> node->row; )
                        column.column[i] = nullptr;

                    column.Place(closest_free_index, node);

                    movement = true;
                    goto continue_shuffle;
                }
            }

            // no free space found, but might be able to swap positions with another node

            // find neighbour
            int direction = (dist > 0) ? 1 : -1;
            Node* neighbour_node = nullptr;
            int pivot = node->row;

            // find next node adjacent to node in requested direction
            while (!neighbour_node || neighbour_node == node) {
                if (pivot < 0 || pivot >= column.column.size()) {
                    neighbour_node = nullptr;
                    break;
                }

                neighbour_node = column.column[pivot];
                pivot += direction;
            }

            // try to switch node with neighbour node
            if (neighbour_node) {
                improvement = std::abs(dist)
                            + std::abs(neighbour_node->CalculateFamilyDistance(neighbour_node->row))
                            - std::abs(neighbour_node->CalculateFamilyDistance(node->row))
                            - std::abs(node->CalculateFamilyDistance(neighbour_node->row));

                if (improvement > 0.25) { // 0 produces endless loop
                    if (node->weight == neighbour_node->weight) {
                        for (int i = 0; i < node->weight; i++) {
                            column.column[node->row + i] = neighbour_node;
                            column.column[neighbour_node->row + i] = node;
                        }

                        std::swap(node->row, neighbour_node->row);

                        movement = true;
                        goto continue_shuffle;
                    }
                }
            }
        }

        continue_shuffle:;
    }

    //4.d. count used rows and columns
    unsigned int column_count = row_index.size();
    unsigned int row_count = 0;
    for (int i = row_index.size(); i-->0;) {
        unsigned int cur_row_count = 0;
        for (unsigned int j = row_index[i].column.size(); j --> 0; ) {
            if (row_index[i].column[j]) {
                cur_row_count = j;
                break;
            }
        }
        row_count = std::max(row_count, cur_row_count);
    }
    //4.e. set size
    for (int i = m_nodes.size(); i --> 0 ; )
         m_nodes[i]->CalculateCoordinate(column_width, internal_height);

    m_width = column_count * column_width;
    m_height = row_count * internal_height;

    //5. create edges
    for (int i = m_nodes.size(); i --> 0 ; )
        m_nodes[i]->CreateEdges(x_margin, column_width, internal_height);
}

const GG::X TechTreeLayout::GetWidth() const
{ return GG::X(static_cast<int>(m_width)); }

const GG::Y TechTreeLayout::GetHeight() const
{ return GG::Y(static_cast<int>(m_height)); }

const TechTreeLayout::Node* TechTreeLayout::GetNode(const std::string & name) const {
    auto item = m_node_map.find(name);
    if (item == m_node_map.end()) {
        DebugLogger() << "TechTreeLayout::getNode: missing node " << name << "\n";
        Debug();
        throw "node missing";
    } else {
        return (*item).second;
    }
}

void TechTreeLayout::AddNode(const std::string& tech, GG::X width, GG::Y height) {
    assert(width > 0 && height > 0 && GetTech(tech));
    auto node = new TechTreeLayout::Node(tech, width, height);
    //DebugLogger() << "Adding Node: " << node << " for tech " << tech;
    m_nodes.push_back(node);
    m_node_map[tech] = node;
}

void TechTreeLayout::AddEdge(const std::string& parent, const std::string& child) {
    auto p = m_node_map.find(parent);
    auto c = m_node_map.find(child);
    assert(p != m_node_map.end() && c != m_node_map.end());
    p->second->AddChild(c->second);
}

const std::vector<TechTreeLayout::Edge*>& TechTreeLayout::GetOutEdges(const std::string& name) const {
    auto item = m_node_map.find(name);
    if (item == m_node_map.end()) {
        DebugLogger() << "TechTreeLayout::getNode: missing node " << name << "\n";
        Debug();
        throw "node missing";
    } else {
        return (*item).second->outgoing_edges;
    }
}

void TechTreeLayout::Debug() const {
    for (Node* node : m_nodes)
        node->Debug();
}

void TechTreeLayout::Clear() {
    //!!! IMPORTANT !!! Node have to be delete in order depth ascending or we will access freed memory!
    for (Node* node : m_nodes)
        delete node;
    m_nodes.clear();
    m_node_map.clear();
}

////////////////
// class Node //
////////////////
/**
 * creates a node for that tech
 */
TechTreeLayout::Node::Node(const std::string& tech, GG::X width, GG::Y height) :
    weight(NODE_CELL_HEIGHT),
    tech_name(tech),
    place_holder(false),
    m_width(Value(width)),
    m_height(Value(height))
{ assert(width > 0 && height > 0 && GetTech(tech)); }

/**
 * recursively creates dummy nodes between parent and child
 */
TechTreeLayout::Node::Node(Node* parent, Node* child) :
    weight(LINE_CELL_HEIGHT),
    tech_name(),
    place_holder(true),
    m_width(0),
    m_height(0)
{
    // copy parent/child connectivity
    depth = parent->depth + 1;
    tech_name = child->tech_name;
    parents.push_back(parent);
    children.push_back(child);
    primary_child = child;
}

TechTreeLayout::Node::~Node() {
    children.clear();
    parents.clear();
    for (Edge* out_edge : outgoing_edges)
        delete out_edge;
    outgoing_edges.clear();
}

const GG::X TechTreeLayout::Node::GetX() const
{ return GG::X(static_cast<int>(m_x)); }

const GG::Y TechTreeLayout::Node::GetY() const
{ return GG::Y(static_cast<int>(m_y)); }

void TechTreeLayout::Node::CalculateCoordinate(double column_width, double row_height) {
    m_x = (static_cast<double>(depth) - 0.5) * column_width;
    m_y = row * row_height;
}

double TechTreeLayout::Node::CalculateFamilyDistance(int row) {
    int familysize = parents.size() + children.size();
    if (familysize == 0)
        return 0;

    double distance = 0;
    for (const Node* node : children)
        if (node)
            distance += std::abs(node->row - row);
    for (const Node* node : parents)
        if (node)
            distance += std::abs(node->row - row);

    return distance / familysize;
}

bool TechTreeLayout::Node::operator<(const TechTreeLayout::Node& y) const {
    if (depth == y.depth) {
        const Tech* this_node_tech = GetTech(tech_name);
        assert(this_node_tech);
        const Tech* y_node_tech = GetTech(y.tech_name);
        assert(y_node_tech);

        if (this_node_tech->Category() == y_node_tech->Category())
            return tech_name < y.tech_name;
        return this_node_tech->Category() < y_node_tech->Category();
    }
    return depth < y.depth;
}

void TechTreeLayout::Node::AddChild(Node* node) {
    //DebugLogger() << "Node::AddChild this node: " << this << " for tech: " << tech_name << "  with child node: " << node << "  for tech: " << node->tech_name;
    assert(node);
    children.push_back(node);
    node->parents.push_back(this);
    //Note: primary_child is used as sorting criteria
    // nodes with the same child get drawn "together"
    // so we need to choose wisely which node is our heir
    // 1. we prefere childs with the same category
    // 2. we order lexically
    if (!primary_child) {
        //no choice
        primary_child = node;
        return;
    }

    if (node->place_holder)
        return;

    //reaching this case means we will get crossing lines
    //we decide which node is the better child for layout decisions
    const Tech* this_node_tech = GetTech(tech_name);
    assert(this_node_tech);
    const Tech* child_node_tech = GetTech(primary_child->tech_name);
    assert(child_node_tech);
    const Tech* input_node_tech = GetTech(node->tech_name);
    assert(input_node_tech);

    int n = ((child_node_tech->Category() == this_node_tech->Category()) ? 0 : 1)
        //binary case 00 01 10 11
        +  ((input_node_tech->Category() == this_node_tech->Category()) ? 0 : 2);
    switch (n) {
    case 0: //both categories fit
        if (node->tech_name < primary_child->tech_name)
            primary_child = node;
        break;
    case 1:  //only primary_child category fits
        //keep primary_child
        break;
    case 2: //only node category fits
        primary_child = node;
        break;
    case 3: //both categories are wrong
        if (input_node_tech->Category() < child_node_tech->Category())
            primary_child = node;
        //else keep child
        break;
    }
}

void TechTreeLayout::Node::fillDepth() {
    for (Node* child : children) {
        child->depth = std::max(child->depth, this->depth + 1);
        child->fillDepth();
    }
}

void TechTreeLayout::Node::CreateEdges(double x_margin, double column_width, double row_height) {
    assert(column_width > 0);
    for (int i = children.size(); i --> 0; ) {
        //find next real node and create coordinates
        Node* next = children[i];
        while (next->place_holder) {
            next->CalculateCoordinate(column_width, row_height);
            next = next->primary_child;
        }
        const std::string& to = next->tech_name;
        //create drawing path
        next = children[i];
        auto edge = new Edge(tech_name, to);
        //from, line start
        edge->AddPoint(m_x, m_y + m_height / 2); // start on the left side of the node
        edge->AddPoint(m_x + m_width + x_margin, m_y + m_height / 2);
        //draw line until a real tech is reached
        while (next->place_holder) {
            //horizontal line bypassing the placeholder
            edge->AddPoint(next->m_x - 2 * x_margin, next->m_y);
            edge->AddPoint(next->m_x + m_width + x_margin, next->m_y);
            next = next->primary_child;
        }
        //to, line end
        edge->AddPoint(next->m_x - 2 * x_margin, next->m_y + next->m_height / 2); //double space for arrow
        edge->AddPoint(next->m_x, next->m_y + next->m_height / 2); // the end has to be exact for the arrow head
        //store drawing path
        outgoing_edges.push_back(edge);
    }
}

void TechTreeLayout::Node::Debug() const {
    DebugLogger() << "Tech - " << tech_name << " (" << m_x << "," << m_y << ") #" << depth << "\n";
    DebugLogger() << "  Parents - ";
    for (int i = parents.size(); i --> 0; )
        DebugLogger() << parents[i]->tech_name << "#" << parents[i]->depth;

    DebugLogger() << "\n";
    DebugLogger() << "  Children - ";
    for (int i = children.size(); i --> 0; )
        DebugLogger() << children[i]->tech_name << "#" << children[i]->depth;;

    DebugLogger() << "\n";

    for (int i = outgoing_edges.size(); i-->0; ) {
        DebugLogger() << "     - ";
        outgoing_edges[i]->Debug();
    }
}
