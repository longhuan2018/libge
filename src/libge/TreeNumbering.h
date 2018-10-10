#pragma once
#include "QuadtreePath.h"

LIBGE_NAMESPACE_BEGINE
class LIBGE_API TreeNumbering
{
public:
	TreeNumbering(int branching_factor, int depth, bool mangle_second_row);
	virtual ~TreeNumbering();

	// Return the total number of nodes in the tree
	int num_nodes() const {
		return num_nodes_;
	}

	int depth() const {
		return depth_;
	}

	int branching_factor() const {
		return branching_factor_;
	}

	// Return the inorder numbering for the given subindex numbering.
	int SubindexToInorder(int subindex) const {
		assert(InRange(subindex));
		return nodes_[subindex].subindex_to_inorder;
	}

	// Return the subindex numbering for the given inorder numbering.
	int InorderToSubindex(int inorder) const {
		assert(InRange(inorder));
		return nodes_[inorder].inorder_to_subindex;
	}

	// Return the inorder numbering for the given traversal path.
	int TraversalPathToInorder(const QuadtreePath path) const;
	int TraversalPathToInorder(const std::string &path) const {
		return TraversalPathToInorder(QuadtreePath(path));
	}

	// Return the subindex numbering for the given traversal path.
	int TraversalPathToSubindex(const QuadtreePath path) const;
	int TraversalPathToSubindex(const std::string &path) const {
		return TraversalPathToSubindex(QuadtreePath(path));
	}

	// Return the traversal path to the given inorder node.
	QuadtreePath InorderToTraversalPath(int inorder) const;

	QuadtreePath SubindexToTraversalPath(int subindex) const;

	// If this is the subindex of an interior node, fill in indices with
	// the subindex numbers of its child nodes and return true.  indices
	// must be at least branching_factor in length.
	//
	// If this is a leaf node, return false.
	bool GetChildrenSubindex(int subindex, int *indices) const;

	bool GetChildrenInorder(int inorder, int *indices) const;

	// Return the level of the node with the given subindex
	int GetLevelSubindex(int subindex) const {
		return GetLevelInorder(SubindexToInorder(subindex));
	}

	int GetLevelInorder(int inorder) const {
		assert(InRange(inorder));
		return nodes_[inorder].inorder_to_level;
	}

	// Return the index of this node's parent, or -1 if this node is the root
	int GetParentSubindex(int subindex) const;

	int GetParentInorder(int inorder) const;

protected:

	bool InRange(int num) const {
		return (0 <= num) && (num < num_nodes_);
	}

private:

	// base is added to every subindex during the traversal.
	// level is the level of the tree we're at (0 = top)
	// offset is the left-to-right offset of this node in this level of the tree.
	// left_index is the subindex of the leftmost node in this level.
	// num counts the nth node in the inorder traversal.
	void PrecomputeSubindexToInorder(int base,
		int level, int offset, int left_index,
		int *num);

	void PrecomputeInorderToParent();

	// Return the number of nodes in the tree at levels <= the given level
	int NodesAtLevels(int level) const;

	void PrecomputeNodesAtLevels();

	int depth_;
	int branching_factor_;
	int num_nodes_;
	struct NodeInfo {
		int subindex_to_inorder;
		int inorder_to_subindex;
		int inorder_to_level;
		int inorder_to_parent;
	};

	NodeInfo *nodes_;

	// Store the number of nodes in the tree at levels <= the given level
	int *nodes_at_levels_;
};
LIBGE_NAMESPACE_END
