#include "TechTreeLayout.h"

#include "../universe/Tech.h"
#include "../util/Logger.h"
#include <algorithm>
#include <cmath>

namespace  {
    constexpr int NODE_CELL_HEIGHT = 2;
    constexpr int LINE_CELL_HEIGHT = 1;
}


//////////////////
// class Column //
//////////////////
bool TechTreeLayout::Column::Fit(int index, TechTreeLayout::Node* node) {
    if (0 >= index)
        return false;
    std::size_t idx = static_cast<std::size_t>(index);

    const auto size = column.size();
    if (idx + node->weight > size)
        column.resize(idx + node->weight, nullptr);

    for (auto j = idx + node->weight; j --> idx;) {
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
        for (int i = index + node->weight; i-->index;)
            column[i] = node;

        node->row = index;
        return true;
    }
    return false;
}


////////////////
// class Edge //
////////////////
TechTreeLayout::Edge::Edge(std::string from, std::string to, uint32_t points) :
    m_from(std::move(from)),
    m_to(std::move(to))
{
    m_points.reserve(points);
    assert(GetTech(m_from) && GetTech(m_to));
}

void TechTreeLayout::Edge::Debug() const {
    DebugLogger() << "Edge " << m_from << "-> " << m_to << ": ";
    for (const auto& [f, s] : m_points)
        DebugLogger() << "(" << f << "," << s << ") ";
    DebugLogger() << "\n";
}

TechTreeLayout::~TechTreeLayout() {
    for (Node* node : m_nodes)
        delete node;
}

/** creates and initialises all nodes
  * @param column_width width of each column
  * @param row_height height of each row
  * @param x_margin horizontal part of arrow before changing direction to child node */
void TechTreeLayout::DoLayout(double column_width, double row_height, double x_margin) {
    assert(column_width > 0 && row_height > 0);
    double internal_height = row_height / NODE_CELL_HEIGHT; // node has NODE_CELL_HEIGHT rows internally
    //1. set all node depths from root parents
    for (Node* node : m_nodes)
        if (node->parents.empty())
            node->SetDepthRecursive(0);    // also sets all children's depths

    // find max node depth
    int max_node_depth = 0;
    for (Node* node : m_nodes)
        max_node_depth = std::max(max_node_depth, node->depth);

    //2. create placeholder nodes
    DebugLogger() << "TechTreeLayout::DoLayout creating placeholder nodes...";
    std::vector<Node*> raw_nodes = m_nodes; // just iterator over initial nodes, not also over the placeholders
    for (Node* node : raw_nodes)
        node->CreatePlaceHolder(m_nodes);

    //3. put nodes into containers for each depth column
    std::vector<std::vector<Node*>> tree_layers(max_node_depth + 1);
    for (Node* node : m_nodes) {
        assert((node->depth >= 0) && (node->depth < static_cast<int>(tree_layers.size())));
        tree_layers[node->depth].push_back(node);
    }

    // Sort nodes within each layer
    for (auto& layer : tree_layers)
        std::sort(layer.begin(), layer.end(), [](Node* l, Node* r) -> bool { return l && r && (*l < *r); });

    //4. do layout
    std::vector<Column> row_index = std::vector<Column>(tree_layers.size());

    // in what order do columns receive nodes?
    std::vector<int> column_order;
    column_order.reserve(tree_layers.size());
    // start with column with most nodes, progess outwards from it
    int first_column = 0;
    unsigned int max_column_nodes = 0;
    for (unsigned int i = 0; i < tree_layers.size(); ++i) {
        if (tree_layers[i].size() > max_column_nodes) {
            max_column_nodes = tree_layers[i].size();
            first_column = i;
        }
    }
    // progress outwards from initial column
    column_order.push_back(first_column);
    int next_column = column_order[0] + 1;
    int prev_column = column_order[0] - 1;
    while (column_order.size() < tree_layers.size()) {
        if (prev_column >= 0) {
            column_order.push_back(prev_column);
            prev_column--;
        }
        if (next_column < static_cast<int>(tree_layers.size())) {
            column_order.push_back(next_column);
            next_column++;
        }
    }
    // distribute tech nodes over the table, one column at a time
    for (int column : column_order) {
        std::string current_category;
        for (Node* node : tree_layers[column]) {
            const Tech* node_tech = GetTech(node->tech_name);
            const std::string& node_category = node_tech ? node_tech->Category() : "";
            bool new_category = node_category != current_category;
            node->DoLayout(row_index, new_category);
            current_category = node_category;
        }
    }
    // optimize layout, every node gets a rating if moving would shorten the distance to it's family
    // if the movement is possible either if the place if free or the neighbour has the opposite wish
    bool movement = true;
    while (movement) {
        movement = false;
        for (unsigned int i = m_nodes.size(); i --> 0;) {
            if (m_nodes[i]->Wobble(row_index[m_nodes[i]->depth])) {
                movement = true;
                break;
            }
        }
    }
    //4.d. count used rows and columns
    unsigned int column_count = row_index.size();
    unsigned int row_count = 0;
    for (int i = row_index.size(); i-->0;) {
        unsigned int cur_row_count = 0;
        for (unsigned int j = row_index[i].column.size(); j --> 0;) {
            if (row_index[i].column[j]) {
                cur_row_count = j;
                break;
            }
        }
        row_count = std::max(row_count, cur_row_count);
    }
    //4.e. set size
    for (int i = m_nodes.size(); i --> 0;)
         m_nodes[i]->CalculateCoordinate(column_width, internal_height);

    m_width = column_count * column_width;
    m_height = row_count * internal_height;

    //5. create edges
    for (int i = m_nodes.size(); i --> 0;)
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
    assert(width > GG::X0 && height > GG::Y0 && GetTech(tech));
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

const std::vector<TechTreeLayout::Edge>& TechTreeLayout::GetOutEdges(const std::string& name) const {
    auto item = m_node_map.find(name);
    if (item == m_node_map.end()) {
        DebugLogger() << "TechTreeLayout::getNode: missing node " << name << "\n";
        Debug();
        throw "node missing";
    } else {
        return item->second->outgoing_edges;
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
TechTreeLayout::Node::Node(std::string tech, GG::X width, GG::Y height) :
    weight(NODE_CELL_HEIGHT),
    tech_name(std::move(tech)),
    m_width(Value(width)),
    m_height(Value(height))
{ assert(width > GG::X0 && height > GG::Y0 && GetTech(tech_name)); }

/**
 * recursively creates dummy nodes between parent and child
 */
TechTreeLayout::Node::Node(Node* parent, Node* child, std::vector<Node*>& nodes) :
    weight(LINE_CELL_HEIGHT),
    place_holder(true)
{
    assert(parent != 0 && child != 0);
    // ensure passed in nodes are valid
    if (!parent)
        ErrorLogger() << "Node::Node passed null parent";
    if (!child)
        ErrorLogger() << "Node::Node passed null child";
    if (!parent || !child)
        return;

    //DebugLogger() << "Node::Node given parent " << parent
    //                       << " and child: " << child;

    //DebugLogger() << "Node::Node given parent with depth " << parent->depth
    //                       << "  and child with depth: " << child->depth;

    // ensure there is space to insert node between parent and child nodes
    if (child->depth <= parent->depth + 1) {
        ErrorLogger() << "no space to insert a dummy node!";
        depth = child->depth;
        return;
    }

    //DebugLogger() << "Node::Node adding dummy node: " << this
    //                       << "  between parent node tech: " << parent->tech_name
    //                       << "  and child node tech: " << child->tech_name;

    // add node to main node bookkeeping
    nodes.push_back(this);

    // copy parent/child connectivity
    depth = parent->depth + 1;
    tech_name = child->tech_name;
    parents.push_back(parent);
    children.push_back(child);
    primary_child = child;

    // update child's parents to point to this node instead of input parent
    for (Node*& child_parent_ref : child->parents) {
        if (child_parent_ref == parent)
            child_parent_ref = this;
    }

    // update parent's child node pointers to instead point to this node
    for (Node*& parent_child_ref : parent->children) {
        if (parent_child_ref == child)
            parent_child_ref = this;
    }
    if (parent->primary_child == child)
        parent->primary_child = this;
}

const GG::X TechTreeLayout::Node::GetX() const
{ return GG::X(static_cast<int>(m_x)); }

const GG::Y TechTreeLayout::Node::GetY() const
{ return GG::Y(static_cast<int>(m_y)); }

void TechTreeLayout::Node::CalculateCoordinate(double column_width, double row_height) {
    m_x = (static_cast<double>(depth) - 0.5) * column_width;
    m_y = row * row_height;
}

double TechTreeLayout::Node::CalculateFamilyDistance(int row_) {
    int familysize = parents.size() + children.size();
    if (familysize == 0)
        return 0;

    double distance = 0;
    for (const Node* node : children)
        if (node)
            distance += std::abs(node->row - row_);
    for (const Node* node : parents)
        if (node)
            distance += std::abs(node->row - row_);

    return distance / familysize;
}

bool TechTreeLayout::Node::Wobble(Column& column) {
    double dist, new_dist, s_dist, new_s_dist;
    dist = CalculateFamilyDistance(row);

    //try to find free space arround optimal position
    int closest_free_index = column.ClosestFreeIndex(static_cast<int>(row + dist + 0.5), this);

    //check if that space is better
    new_dist = CalculateFamilyDistance(closest_free_index);
    double improvement = std::abs(dist) - std::abs(new_dist);
    if (improvement > 0.25) {
        if (column.Fit(closest_free_index, this)) {
            for (int i = row + weight; i --> row;)
                column.column[i] = nullptr;

            column.Place(closest_free_index, this);

            //std::cout << m_name << ":" << dist << " -> " << new_dist <<"\n";
            return true;
        }
    }

    // no free space found, but might be able to swap positions with another node

    // find neighbour
    int direction = (dist > 0) ? 1 : -1;
    Node* n = nullptr;
    int i = row;

    // find next node adjacent to node this in requested direction
    while (!n || n == this) {
        if (i < 0 || i >= static_cast<int>(column.column.size())) {
            n = nullptr;
            break;
        }

        n = column.column[i];
        i += direction;
    }

    // try to switch node with neighbour node
    if (n) {
        s_dist     = n->CalculateFamilyDistance(n->row);
        new_s_dist = n->CalculateFamilyDistance(   row);
        new_dist   =    CalculateFamilyDistance(n->row);
        improvement = std::abs(dist) + std::abs(s_dist) - std::abs(new_dist) - std::abs(new_s_dist);
        if (improvement > 0.25) { // 0 produces endless loop
            if (weight == n->weight) {
                for (int ii = 0; ii < weight; ii++) {
                    column.column[static_cast<std::size_t>(row + ii)] = n;
                    column.column[static_cast<std::size_t>(n->row + ii)] = this;
                }
                int t_row = row;
                row = n->row;
                n->row = t_row;
                //std::cout << "(S)" << m_name  << ":" << dist << " -> " << new_dist << " & "<< n->m_name  << ":" << s_dist << " -> " << new_s_dist << "\n";
                return true;
            }
        }
    }
    return false;
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

void TechTreeLayout::Node::SetDepthRecursive(int depth_) {
    this->depth = std::max(depth_, this->depth);
    // set children's depths
    for (Node* node : children)
        node->SetDepthRecursive(this->depth + 1);
}

void TechTreeLayout::Node::CreatePlaceHolder(std::vector<Node*>& nodes) {
    //DebugLogger() << "Creating PlaceHolder for node " << this;
    //DebugLogger().flush();
    //DebugLogger() << "  for tech: " << tech_name;
    //DebugLogger().flush();
    //DebugLogger() << "  which has " << children.size() << " children:";
    //DebugLogger().flush();
    //for (const Node* child : children) {
    //    DebugLogger() << "      child: " << child << " with tech: " << child->tech_name;
    //}


    for (Node* child : children) {
        //DebugLogger() << "   processing child: " << child << " with tech: " << child->tech_name;


        Node* current_parent_node = this;

        //DebugLogger() << "Dummy nodes from " << this->tech_name << " to child: " << child->tech_name;
        //int dummy_nodes_added = 0;
        while (current_parent_node->depth + 1 < child->depth) {
            // there is at least one column gap beween the horizontal positions
            // of this node and this child node.
            //
            // to fill the gap visually, create dummy node(s) in the columns
            // between them.
            //DebugLogger() << "next column depth: " << current_parent_node->depth + 1
            //                       << "  child_depth: " << child->depth;
            //DebugLogger() << "current_parent_node: " << current_parent_node
            //                       << " child: " << child;
            auto dummy_node = new Node(current_parent_node, child, nodes);
            //DebugLogger() << "new dummy node depth: " << dummy_node->depth;
            current_parent_node = dummy_node;
            //++dummy_nodes_added;
        }
        //DebugLogger() << "done adding dummy nodes.  current_parent node depth + 1: " << current_parent_node->depth + 1 << "  child depth: " << child->depth;
        //if (dummy_nodes_added > 0) {
        //    DebugLogger() << "added " << dummy_nodes_added << " dummy nodes for from tech " << tech_name;
        //}

        //DebugLogger() << " node now has " << children.size() << " children:";
        //DebugLogger().flush();
        //for (Node* child : children) {
        //    DebugLogger() << "      child: " << child << " with tech: " << child->tech_name;
        //}
    }
}

void TechTreeLayout::Node::DoLayout(std::vector<Column>& row_index, bool cat) {
    //assert(row_height > 0 && column_width > 0 && row_index != 0);
    if (row != -1) return; //already done

    // find average row index of node's children and parents

    // 2. place node
    int index = 0;
    int count = 0;
    //check children
    for (int i = children.size(); i --> 0;) {
        if (children[i]->row != -1) {
            index += children[i]->row;
            count++;
        }
    }
    //check parents
    for (auto i = parents.size(); i --> 0;) {
        if (parents[i]->row != -1) {
            index += parents[i]->row;
            count++;
        }
    }
    if (static_cast<int>(row_index.size()) < depth + 1)
        row_index.resize(static_cast<std::size_t>(depth) + 1);

    // if any parents or children have been placed, put this node in next free
    // space after the ideal node.  if no parents or children have been placed,
    // put node at start of row
    int index_offset = (count != 0) ? index / count : 1;
    row_index[depth].Place(row_index[depth].ClosestFreeIndex(index_offset, this), this);
}

void TechTreeLayout::Node::CreateEdges(double x_margin, double column_width, double row_height) {
    assert(column_width > 0);
    for (int i = children.size(); i --> 0;) {
        //find next real node and create coordinates
        Node* next = children[i];
        uint32_t placeholders = 0;
        while (next->place_holder) {
            next->CalculateCoordinate(column_width, row_height);
            next = next->primary_child;
            ++placeholders;
        }

        //create drawing path
        next = children[i];
        if (!next) {
            ErrorLogger() << "TechTreeLayout::Node::CreateEdges bad edge!";
            continue;
        }
        auto& edge = outgoing_edges.emplace_back(tech_name, next->tech_name, placeholders + 2);

        //from, line start
        edge.AddPoint(m_x, m_y + m_height / 2); // start on the left side of the node
        edge.AddPoint(m_x + m_width + x_margin, m_y + m_height / 2);

        //draw line until a real tech is reached
        while (next->place_holder) {
            //horizontal line bypassing the placeholder
            edge.AddPoint(next->m_x - 2 * x_margin, next->m_y);
            edge.AddPoint(next->m_x + m_width + x_margin, next->m_y);
            next = next->primary_child;
        }

        //to, line end
        edge.AddPoint(next->m_x - 2 * x_margin, next->m_y + next->m_height / 2); //double space for arrow
        edge.AddPoint(next->m_x, next->m_y + next->m_height / 2); // the end has to be exact for the arrow head
    }
}

void TechTreeLayout::Node::Debug() const {
    DebugLogger() << "Tech - " << tech_name << " (" << m_x << "," << m_y << ") #" << depth << "\n";
    DebugLogger() << "  Parents - ";
    for (int i = parents.size(); i --> 0;)
        DebugLogger() << parents[i]->tech_name << "#" << parents[i]->depth;

    DebugLogger() << "\n";
    DebugLogger() << "  Children - ";
    for (int i = children.size(); i --> 0;)
        DebugLogger() << children[i]->tech_name << "#" << children[i]->depth;

    DebugLogger() << "\n";

    for (int i = outgoing_edges.size(); i-->0;) {
        DebugLogger() << "     - ";
        outgoing_edges[i].Debug();
    }
}
