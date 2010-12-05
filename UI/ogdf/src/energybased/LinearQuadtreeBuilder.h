#ifndef _LINEAR_QUADTREE__BUILDER_H_
#define _LINEAR_QUADTREE__BUILDER_H_

#include "FastUtils.h"
#include "LinearQuadtree.h"

namespace ogdf {

//! the builder for the LinearQuadtree
class LinearQuadtreeBuilder
{
public:
	//! constructor
	LinearQuadtreeBuilder(LinearQuadtree& treeRef) : tree(treeRef) { n = tree.numberOfPoints(); };
	
	//! the main build call
	void build();

	//! prepares the node and leaf layer at position l where r is the next position
	void prepareNodeAndLeaf(LinearQuadtree::PointID l, LinearQuadtree::PointID r);

	//! prepares the node and leaf layer from position begin until end (excluding end)
	void prepareTree(LinearQuadtree::PointID begin,  LinearQuadtree::PointID end);

	//! prepares the node and leaf layer for the complete tree from 0 to n (excluding n)
	void prepareTree();

	//! merges the node curr with curr's next node by appending the next nodes children to curr except the first one.
	void mergeWithNext(LinearQuadtree::NodeID curr);

	//! the new link-only recursive builder
	LinearQuadtree::NodeID buildHierarchy(LinearQuadtree::NodeID curr, __uint32 maxLevel);

	//! the main function for the new link-only recursive builder
	void buildHierarchy();

	//! used by restore chain
	inline void restorePushBackChain(LinearQuadtree::NodeID curr)
	{
		if (restoreChainLastNode) tree.setNextNode(restoreChainLastNode, curr); else firstInner = curr;
		restoreChainLastNode = curr;
		numInnerNodes++;
	};

	inline void restoreChain(LinearQuadtree::NodeID curr)
	{
		if (tree.isLeaf(curr)) 
			return;
		else
		{
			restoreChain(tree.child(curr,0));
			tree.setFirstPoint(curr, tree.firstPoint(tree.child(curr, 0)));
			restorePushBackChain(curr);
			for (__uint32 i = 1; i < tree.numberOfChilds(curr); i++) 
				restoreChain(tree.child(curr, i));

			__uint32 lastPoint = tree.firstPoint(tree.child(curr, tree.numberOfChilds(curr)-1)) + tree.numberOfPoints(tree.child(curr, tree.numberOfChilds(curr)-1));
			tree.setNumberOfPoints(curr, lastPoint - tree.firstPoint(curr));
		};
	};

	inline void restoreChain()
	{
		restoreChainLastNode = 0;
		numInnerNodes = 0;
		if (!tree.isLeaf(tree.root()))
			restoreChain(tree.root());
		if (restoreChainLastNode)
			tree.setNextNode(restoreChainLastNode, 0);
	};

	//! returns the level of the first common ancestor of a and b
	inline __uint32 CAL(LinearQuadtree::PointID a, LinearQuadtree::PointID b)
	{
		// 64 bit version
		if (a<0) return 64;
		if (b>=tree.numberOfPoints()) return 64;
		__uint32 res = (32-((mostSignificantBit(tree.mortonNr(a) ^ tree.mortonNr(b)))/2));
		return res;
	};
	
	LinearQuadtree::NodeID firstInner;
	LinearQuadtree::NodeID firstLeaf;

	LinearQuadtree::NodeID lastInner;
	LinearQuadtree::NodeID lastLeaf;
	__uint32 numInnerNodes;
	__uint32 numLeaves;

	LinearQuadtree& tree;
	LinearQuadtree::NodeID restoreChainLastNode;
	LinearQuadtree::PointID n;
};

} // end of namespace ogdf

#endif


