#pragma once
#include "export.h"
#include <assert.h>
#include <string>

LIBGE_NAMESPACE_BEGINE
typedef enum _tagLayerType
{
	LAYER_TYPE_IMAGERY = 0,
	LAYER_TYPE_TERRAIN = 1,
	LAYER_TYPE_VECTOR = 2,
	LAYER_TYPE_IMAGERY_HISTORY = 3
}LayerType;

class LIBGE_API QuadtreePath
{
public:
	static const unsigned int kMaxLevel;   // number of levels represented
	static const unsigned int kStoreSize;  // bytes required to store
	static const unsigned int kChildCount; // number of children at each level

	QuadtreePath() : path_(0) {}
	QuadtreePath(unsigned int level, unsigned int row, unsigned int col);
	QuadtreePath(unsigned int level, const unsigned char blist[]);
	QuadtreePath(const std::string &blist);
	// copy other path, but prune at given level
	QuadtreePath(const QuadtreePath &other, unsigned int level);

	inline bool IsValid() const {         // level in range, no stray bits
		return Level() <= kMaxLevel
			&& (0 == (path_ & ~(PathMask(Level()) | kLevelMask)));
	}
	bool operator<(const QuadtreePath &other) const;
	inline bool operator>(const QuadtreePath &other) const { return other < *this; }
	inline bool operator==(const QuadtreePath &other) const {
		return path_ == other.path_;
	}
	inline bool operator!=(const QuadtreePath &other) const {
		return path_ != other.path_;
	}

	unsigned long long GetGenerationSequence() const;

	// Test for paths in postorder (normal ordering is preorder).  False
	// if path1 == path2.
	static inline bool IsPostorder(const QuadtreePath &path1,
		const QuadtreePath &path2) {
		return !path1.IsAncestorOf(path2)
			&& (path2.IsAncestorOf(path1) || path2 > path1);
	}

	void GetLevelRowCol(unsigned int *level, unsigned int *row, unsigned int *col) const;
	inline unsigned int Level() const { return path_ & kLevelMask; }
	QuadtreePath Parent() const;
	QuadtreePath Child(unsigned int child) const;
	inline unsigned int WhichChild() const {
		return (path_ >> (kTotalBits - Level()*kLevelBits)) & kLevelBitMask;
	}

	std::string AsString(void) const;

	// Advance to next node in same level, return false at end of level
	bool AdvanceInLevel();

	// Advance to next node in preorder, return false at end of nodes <=
	// specified max level.
	bool Advance(unsigned int max_level);

	// NOTE: this is inclusive (returns true if other is same path)
	bool IsAncestorOf(const QuadtreePath &other) const;

	// asserts parent.IsAncestorOf(child)
	static QuadtreePath RelativePath(const QuadtreePath &parent,
		const QuadtreePath &child);

	// Return the row, column and width of the subtile which the child_qpath
	// occupies. Note: the method does not go beyond a single pixel width for
	// the TileCoordinates, it will return the single pixel width and coordinate
	// in that case.
	// This is based on Earth quadtree tile pattern
	// +---+---+
	// | 3 | 2 |
	// +---+---+
	// | 0 | 1 |
	// +---+---+
	// tile_width: the width and height of the tile for this quadtree path.
	// child_qpath: the quadtree path of the child quad node for which we're
	//              computing the subimage.
	// row: the row index of the upper left part of the subimage within the
	//      current tile.
	// column: the column index of the upper left part of the subimage within the
	//      current tile.
	// width: the width and height of the subimage within the current tile.
	// returns false if the child_qpath is not a child of this path, true o.w.
	bool ChildTileCoordinates(int tile_width, const QuadtreePath& child_qpath,
		int* row, int* column, int* width) const;

	// Concatenate paths
	QuadtreePath Concatenate(const QuadtreePath sub_path) const;
	inline QuadtreePath operator+(const QuadtreePath other) const {
		return Concatenate(other);
	}

	// This is useful for converting a prefix of the QuadtreePath into
	// an array index at the specified level.
	// QuadtreePath("23121").AsIndex(4) -> (binary) 10110110 -> 182
	// QuadtreePath("31").AsIndex(2) -> (binary) 1101 -> 13
	// QuadtreePath("1").AsIndex(1) -> (binary) 01 -> 1
	// QuadtreePath("").AsIndex(0) -> 0
	inline unsigned long long AsIndex(unsigned int level) const {
		return (path_ >> (kTotalBits - level*kLevelBits));
	}

	// return the branch made at the specified position
	// --- example ---
	// QuadtreePath path("201");
	// path[0] -> 2
	// path[1] -> 0
	// path[2] -> 1
	unsigned int operator[](unsigned int position) const;

	// *************************************************************************
	// ***  Quadrant routines
	// ***    - quadrants are numbered like this
	// ***
	// ***  +----+----+
	// ***  | 2  | 3  |
	// ***  +----+----+
	// ***  | 0  | 1  |
	// ***  +----+----+
	// *************************************************************************
	// Gets offset for parent cells pixel_buffer(which is ordered from left to
	// right and then from bottom to top). Refer Minifytile.
	static unsigned int QuadToBufferOffset(unsigned int quad, unsigned int tileWidth,
		unsigned int tileHeight) {
		assert(quad < 4);
		switch (quad) {
		case 0:
			return 0;
		case 1:
			return tileWidth / 2;
		case 2:
			return (tileHeight * tileWidth) / 2;
		case 3:
			return ((tileHeight + 1) * tileWidth) / 2;
		}
		return 0;

	}

	// ***  +----+----+
	// ***  | 2  | 3  |
	// ***  +----+----+
	// ***  | 0  | 1  |
	// ***  +----+----+
	// find the tile in the next level (more detail) that maps to the
	// specified quad in range [0,3]
	// Better named MagnifyRowColToChildRowColForQuad.
	static void MagnifyQuadAddr(unsigned int inRow, unsigned int inCol, unsigned int inQuad,
		unsigned int &outRow, unsigned int &outCol)
	{
		assert(inQuad < 4);
		switch (inQuad) {
		case 0:
			outRow = inRow * 2;
			outCol = inCol * 2;
			break;
		case 1:
			outRow = inRow * 2;
			outCol = (inCol * 2) + 1;
			break;
		case 2:
			outRow = (inRow * 2) + 1;
			outCol = inCol * 2;
			break;
		case 3:
			outRow = (inRow * 2) + 1;
			outCol = (inCol * 2) + 1;
			break;
		}
	}

private:
	void FromBranchlist(unsigned int level, const unsigned char blist[]);
	QuadtreePath(unsigned long long path) : path_(path) { assert(IsValid()); }
	static const unsigned int kLevelBits;       // bits per level in packed path_
	static const unsigned long long kLevelBitMask;
	static const unsigned int kTotalBits;      // total storage bits
	static const unsigned long long kPathMask;
	static const unsigned long long kLevelMask;

	inline unsigned long long PathBits() const { return path_ & kPathMask; }
	inline unsigned long long PathMask(unsigned int level) const {
		return kPathMask << ((kMaxLevel - level) * kLevelBits);
	}
	// like PathBits(), but clamps to supplied level
	inline unsigned long long PathBits(unsigned int level) const { return path_ & PathMask(level); }
	inline unsigned int LevelBitsAtPos(unsigned int position) const {
		return (path_ >> (kTotalBits - (position + 1)*kLevelBits)) & kLevelBitMask;
	}

	friend class QuadtreePathUnitTest;
	// This is the only data which should be stored in an instance of
	// this class
	unsigned long long path_;
};

LIBGE_NAMESPACE_END

