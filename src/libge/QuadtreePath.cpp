#include "stdafx.h"
#include "QuadtreePath.h"
#include <algorithm>
#undef min
#undef max

LIBGE_NAMESPACE_BEGINE

const unsigned int QuadtreePath::kMaxLevel = 24;
const unsigned int QuadtreePath::kStoreSize = sizeof(unsigned long long);
const unsigned int QuadtreePath::kChildCount = 4;

const unsigned int QuadtreePath::kLevelBits = 2;       // bits per level in packed path_
const unsigned long long QuadtreePath::kLevelBitMask = 0x03;
const unsigned int QuadtreePath::kTotalBits = 64;      // total storage bits
const unsigned long long QuadtreePath::kPathMask = ~(~unsigned long long(0) >> (QuadtreePath::kMaxLevel * QuadtreePath::kLevelBits));
const unsigned long long QuadtreePath::kLevelMask = ~QuadtreePath::kPathMask;

// Construct from level, row, col.  Code adapated from kbf.cpp

QuadtreePath::QuadtreePath(unsigned int level, unsigned int row, unsigned int col) : path_(0) {
	static const unsigned long long order[][2] = { { 0, 3 }, { 1, 2 } };

	assert(level <= kMaxLevel);

	for (unsigned int j = 0; j < level; ++j) {
		unsigned int right = 0x01 & (col >> (level - j - 1));
		unsigned int top = 0x01 & (row >> (level - j - 1));
		path_ |= order[right][top] << (kTotalBits - ((j + 1) * kLevelBits));
	}

	path_ |= level;
}

// The generation sequence is
// ***  +----+----+
// ***  | 2  | 3  |
// ***  +----+----+
// ***  | 0  | 1  |
// ***  +----+----+
// where as the quad sequence is
// ***  +----+----+
// ***  | 3  | 2  |
// ***  +----+----+
// ***  | 0  | 1  |
// ***  +----+----+
// So to convert a QuadtreePath to generation sequence need to twiddle all 2's
// to 3 and all 3's to 2's.
unsigned long long QuadtreePath::GetGenerationSequence() const {
	const unsigned int level = Level();
	unsigned long long sequence = path_;
	unsigned long long check_for_2_or_3_mask = ((unsigned long long)0x1) << (kTotalBits - 1);
	unsigned long long interchange_2_or_3_mask = ((unsigned long long)0x01) << (kTotalBits - 2);

	for (unsigned int j = 0; j < level; ++j, check_for_2_or_3_mask >>= 2,
		interchange_2_or_3_mask >>= 2) {
		if (sequence & check_for_2_or_3_mask) {
			sequence ^= interchange_2_or_3_mask;
		}
	}
	return sequence;
}


void QuadtreePath::FromBranchlist(unsigned int level, const unsigned char blist[]) {
	assert(level <= kMaxLevel);

	for (unsigned int j = 0; j < level; ++j) {
		path_ |= (blist[j] & kLevelBitMask) << (kTotalBits - ((j + 1) * kLevelBits));
	}
	path_ |= level;
}


// Construct from blist (binary or ASCII - ignores all but lower 2
// bits of each level, depends on fact that lower 2 bits of '0', '1',
// '2', and '3' are same as binary representation)
QuadtreePath::QuadtreePath(unsigned int level, const unsigned char blist[]) :
path_(0) {
	FromBranchlist(level, blist);
}

// like above, but as convenience works with std::string
QuadtreePath::QuadtreePath(const std::string &blist) : path_(0) {
	FromBranchlist(blist.size(), (unsigned char*)&blist[0]);
}


QuadtreePath::QuadtreePath(const QuadtreePath &other, unsigned int level) : path_(0)
{
	unsigned int lev = std::min(level, other.Level());
	path_ = other.PathBits(lev) | lev;
	assert(IsValid());
}

std::string
QuadtreePath::AsString(void) const {
	std::string result;
	result.resize(Level());
	for (unsigned int i = 0U; i < Level(); ++i) {
		result[i] = '0' + LevelBitsAtPos(i);
	}
	return result;
}

// Extract level, row, column (adapted from kbf.cpp)

void QuadtreePath::GetLevelRowCol(unsigned int *level, unsigned int *row, unsigned int *col) const {
	static const unsigned int rowbits[] = { 0x00, 0x00, 0x01, 0x01 };
	static const unsigned int colbits[] = { 0x00, 0x01, 0x01, 0x00 };

	unsigned int row_val = 0U;
	unsigned int col_val = 0U;

	for (unsigned int j = 0U; j < Level(); ++j) {
		unsigned int level_bits = LevelBitsAtPos(j);
		row_val = (row_val << 1) | (rowbits[level_bits]);
		col_val = (col_val << 1) | (colbits[level_bits]);
	}

	*level = Level();
	*row = row_val;
	*col = col_val;
}

// Preorder comparison operator

bool QuadtreePath::operator<(const QuadtreePath &other) const {
	unsigned int minlev = (Level() < other.Level()) ? Level() : other.Level();

	// if same up to min level, then lower level comes first,
	// otherwise just do integer compare (most sig. bits are lower levels).
	unsigned long long mask = ~(~unsigned long long(0) >> (minlev * kLevelBits));
	if (mask & (path_ ^ other.path_)) {   // if differ at min level
		return PathBits() < other.PathBits();
	}
	else {                              // else lower level is parent
		return Level() < other.Level();
	}
}

// Advance to next node in same level, return false at end of level

bool QuadtreePath::AdvanceInLevel() {
	unsigned long long path_bits = PathBits();
	unsigned long long path_mask = PathMask(Level());
	if (path_bits != path_mask) {
		path_ += unsigned long long(1) << (kTotalBits - Level()*kLevelBits);
		assert(IsValid());
		return true;
	}
	else {
		return false;
	}
}

// Advance to next node in preorder, return false if no more nodes.
// Only nodes at levels <= max_level are generated.

bool QuadtreePath::Advance(unsigned int max_level) {
	assert(max_level > 0U);
	assert(Level() <= max_level);
	if (Level() < max_level) {
		*this = Child(0);
		return true;
	}
	else {
		while (WhichChild() == kChildCount - 1) {
			*this = Parent();
		}
		return AdvanceInLevel();
	}
}

// Return path to parent

QuadtreePath QuadtreePath::Parent() const {
	assert(Level() > 0);
	unsigned int new_level = Level() - 1U;

	return QuadtreePath((path_ & (kPathMask << kLevelBits*(kMaxLevel - new_level)))
		| new_level);
}

// Return path to child (must be in range [0..3])

QuadtreePath QuadtreePath::Child(unsigned int child) const {
	assert(Level() <= kMaxLevel);
	assert(child <= 3);
	unsigned int new_level = Level() + 1U;
	return QuadtreePath(PathBits()
		| unsigned long long(child) << (kTotalBits - new_level*kLevelBits)
		| new_level);
}

bool QuadtreePath::IsAncestorOf(const QuadtreePath &other) const {
	if (Level() <= other.Level()) {
		return PathBits(Level()) == other.PathBits(Level());
	}
	else {
		return false;
	}
}


QuadtreePath QuadtreePath::RelativePath(const QuadtreePath &parent,
	const QuadtreePath &child)
{
	assert(parent.IsAncestorOf(child));
	unsigned int levelDiff = child.Level() - parent.Level();
	return QuadtreePath((child.PathBits() << (parent.Level() * kLevelBits)) |
		levelDiff);
}

bool QuadtreePath::ChildTileCoordinates(int tile_width,
	const QuadtreePath& child_qpath,
	int* row, int* column,
	int* width) const {
	if (!IsAncestorOf(child_qpath)) {
		return false;
	}

	QuadtreePath relative_qpath = QuadtreePath::RelativePath(*this, child_qpath);
	*width = tile_width;
	*row = 0;
	*column = 0;
	// We will stop if we get to 1 pixel wide subtree...at that point we're done.
	for (unsigned int level = 0U; level < relative_qpath.Level() && *width > 1U; ++level) {
		int quad = relative_qpath[level];
		*width >>= 1;
		if (quad == 0) {
			*row += *width;
		}
		else if (quad == 1) {
			*row += *width;
			*column += *width;
		}
		else if (quad == 2) {
			*column += *width;
		}  // quad == 3 is the upper left (image origin).
	}
	return true;
}

QuadtreePath QuadtreePath::Concatenate(const QuadtreePath sub_path) const {
	unsigned long long level = Level() + sub_path.Level();
	assert(level <= kMaxLevel);
	return QuadtreePath((path_ & kPathMask)
		| ((sub_path.path_ & kPathMask) >> Level()*kLevelBits)
		| level);
}

unsigned int QuadtreePath::operator[](unsigned int position) const
{
	assert(position < Level());
	return LevelBitsAtPos(position);
}

LIBGE_NAMESPACE_END