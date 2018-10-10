#pragma once
#include "TreeNumbering.h"

LIBGE_NAMESPACE_BEGINE
class LIBGE_API QuadtreeNumbering : public TreeNumbering
{
public:
	QuadtreeNumbering(int depth, bool mangle_second_row);
	virtual ~QuadtreeNumbering();

	static const int kDefaultDepth;
	static const int kRootDepth;

	// Compute the level and (x, y) position within the level for the given
	// subindex.
	void SubindexToLevelXY(int subindex, int *level, int *x, int *y) const;

	// Return the subindex for the given level and (x, y) position.
	int LevelXYToSubindex(int level, int x, int y) const;

	static unsigned long long TraversalPathToGlobalNodeNumber(QuadtreePath path);
	static unsigned long long TraversalPathToGlobalNodeNumber(const std::string &path) {
		return TraversalPathToGlobalNodeNumber(QuadtreePath(path));
	}
	static QuadtreePath GlobalNodeNumberToTraversalPath(unsigned long long num);

	// Convert a traversal path to a quadset number, and a node within
	// the quadset.
	static void TraversalPathToQuadsetAndSubindex(QuadtreePath path,
		unsigned long long *quadset_num,
		int *subindex);
	static void TraversalPathToQuadsetAndSubindex(const std::string &path,
		unsigned long long *quadset_num,
		int *subindex) {
		TraversalPathToQuadsetAndSubindex(QuadtreePath(path),
			quadset_num,
			subindex);
	}

	static bool IsQuadsetRootLevel(unsigned int level);

	static QuadtreePath QuadsetAndSubindexToTraversalPath(unsigned long long quadset_num,
		int subindex);

	static void QuadsetAndSubindexToLevelRowColumn(unsigned long long quadset_num,
		int subindex,
		int *level, int *row, int *col);

	// Return the number of nodes (subindex values) in a quadset
	static int NumNodes(unsigned long long quadset_num);

	// Convert subindex to inorder numbering for a specified quadset
	static int QuadsetAndSubindexToInorder(unsigned long long quadset_num,
		int subindex);

	// Get numbering for given quadset
	static const QuadtreeNumbering &Numbering(unsigned long long quadset_num);

	// Given a tile's level, row and column, return its Maps tile name.
	static std::string LevelRowColumnToMapsTraversalPath(int level,
		int row,
		int col);

	// Given a tile's Maps name, find its level, row and column.
	static void MapsTraversalPathToLevelRowColumn(const std::string &path,
		int *level,
		int *row, int *col);

	static bool IsMapsTile(const std::string &key) {
		return !key.empty() && key[0] == 't';
	}

private:

	void PrecomputeSubindexToLevelXY();

	struct LevelXY {
		int level;
		int x, y;
	};

	LevelXY *subindex_to_levelxy_;
};
LIBGE_NAMESPACE_END
