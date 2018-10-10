#pragma once
#include "QuadtreeNumbering.h"
#include "JpegCommentDate.h"
#include <vector>

LIBGE_NAMESPACE_BEGINE
class LIBGE_API QuadtreeDataReference 
{
public:
	QuadtreeDataReference()
		: version_(0),
		channel_(0),
		provider_(0),
		jpeg_date_() {
	}
	QuadtreeDataReference(const QuadtreePath &qt_path,
		unsigned short version,
		unsigned short channel,
		unsigned short provider)
		: qt_path_(qt_path),
		version_(version),
		channel_(channel),
		provider_(provider),
		jpeg_date_() {
	}
	QuadtreeDataReference(const QuadtreePath &qt_path,
		unsigned short version,
		unsigned short channel,
		unsigned short provider,
		const JpegCommentDate& jpeg_date)
		: qt_path_(qt_path),
		version_(version),
		channel_(channel),
		provider_(provider),
		jpeg_date_(jpeg_date) {
	}
	inline QuadtreePath qt_path() const { return qt_path_; }
	inline unsigned short version() const { return version_; }
	inline unsigned short channel() const { return channel_; }
	inline unsigned short provider() const { return provider_; }
	inline const JpegCommentDate& jpeg_date() const {
		return jpeg_date_;
	}
	inline bool IsHistoricalImagery() const {
		return channel_ == LayerType::LAYER_TYPE_IMAGERY_HISTORY &&
			!jpeg_date_.IsCompletelyUnknown();
	}

	inline void set_qt_path(const QuadtreePath &qt_path) { qt_path_ = qt_path; }
	inline void set_version(unsigned short version) { version_ = version; }
	inline void set_channel(unsigned short channel) { channel_ = channel; }
	inline void set_provider(unsigned short provider) { provider_ = provider; }

	inline bool operator==(const QuadtreeDataReference &other) const {
		return qt_path_ == other.qt_path_
			&& channel_ == other.channel_
			&& version_ == other.version_
			&& provider_ == other.provider_;
	}
	inline bool operator!=(const QuadtreeDataReference &other) const {
		return !(*this == other);
	}
	inline bool operator>(const QuadtreeDataReference &other) const {
		return qt_path_ > other.qt_path_;
	}

	static const size_t kSerialSize = sizeof(QuadtreePath) + 3 * sizeof(unsigned short);
private:
	QuadtreePath qt_path_;                // path to the data
	unsigned short version_;
	unsigned short channel_;                      // zero for non-vector
	unsigned short provider_;
	JpegCommentDate jpeg_date_;
};

class LIBGE_API QuadtreeDataReferenceGroup
{
public:
	QuadtreeDataReferenceGroup(std::vector<QuadtreeDataReference> *qtp_refs_,
		std::vector<QuadtreeDataReference> *qtp2_refs_,
		std::vector<QuadtreeDataReference> *img_refs_,
		std::vector<QuadtreeDataReference> *ter_refs_,
		std::vector<QuadtreeDataReference> *vec_refs_)
		: qtp_refs(qtp_refs_),
		qtp2_refs(qtp_refs_),
		img_refs(img_refs_),
		ter_refs(ter_refs_),
		vec_refs(vec_refs_) 
	{
	}
	~QuadtreeDataReferenceGroup() {}

	void Reset() 
	{
		qtp_refs->clear();
		qtp2_refs->clear();
		img_refs->clear();
		ter_refs->clear();
		vec_refs->clear();
	}

public:
	// Note: these vectors are not owned by this structure
	std::vector<QuadtreeDataReference> *qtp_refs;
	std::vector<QuadtreeDataReference> *qtp2_refs;
	std::vector<QuadtreeDataReference> *img_refs;
	std::vector<QuadtreeDataReference> *ter_refs;
	std::vector<QuadtreeDataReference> *vec_refs;
};

class LIBGE_API QuadTreeQuantum16
{
public:
	// Those are bit for children nodes.
	int  GetBit(int bit) const { return (_children & bytemaskBTG[bit]) != 0; }
	void SetBit(int bit)   { _children |= bytemaskBTG[bit]; }
	void ClearBit(int bit) { _children &= ~bytemaskBTG[bit]; }

	// CacheNodeBit indicates a node on last level.
	// client does not process children info for these,
	// since we don't actually have info for the children.
	// As a result, no need to set any of the children bits for
	// cache nodes, since client will simply disregard them.
	int  GetCacheNodeBit() const { return (_children & bytemaskBTG[4]) != 0; }
	void SetCacheNodeBit()   { _children |= bytemaskBTG[4]; }
	void ClearCacheNodeBit() { _children &= ~bytemaskBTG[4]; }

	// Set if this node contains vector data.
	int  GetDrawableBit() const { return (_children & bytemaskBTG[5]) != 0; }
	void SetDrawableBit()   { _children |= bytemaskBTG[5]; }
	void ClearDrawableBit() { _children &= ~bytemaskBTG[5]; }

	// Set if this node contains image data.
	int  GetImageBit() const { return (_children & bytemaskBTG[6]) != 0; }
	void SetImageBit()   { _children |= bytemaskBTG[6]; }
	void ClearImageBit() { _children &= ~bytemaskBTG[6]; }

	// Set if this node contains terrain data.
	int  GetTerrainBit() const { return (_children & bytemaskBTG[7]) != 0; }
	void SetTerrainBit()   { _children |= bytemaskBTG[7]; }
	void ClearTerrainBit() { _children &= ~bytemaskBTG[7]; }

public:
	// Make default ctor public - datapacket needs it to call vector ctor.
	// Use one of the Init() functions to intialize the structure
	// with proper number of channels.
	QuadTreeQuantum16() { }
	~QuadTreeQuantum16() { Cleanup(); }

	// Frees any allocated data - channels
	void Cleanup();

	// Return a readable string describing the quadtree node (matches the output
	// of the KhQuadTreeNodeProtoBuf class).
	void ToString(std::ostringstream &str,
		const int node_index,
		const QuadtreePath &qt_path,
		const int subindex) const;

	// Get references to all packets referred to in the quantum
	// (including other quadtree packets) and append to refs.
	void GetDataReferences(QuadtreeDataReferenceGroup *references,
		const QuadtreePath &qt_path) const;

	// Return true if a layer of the specified type exists in this node.
	bool HasLayerOfType(LayerType layer_type) const;

public:
	unsigned char _children;
	unsigned short _cnode_version;  // cachenode version
	unsigned short _image_version;
	unsigned short _terrain_version;
	signed char  _image_neighbors[8];
	unsigned char _image_data_provider;
	unsigned char _terrain_data_provider;
	std::vector<unsigned short> _channel_type;
	std::vector<unsigned short> _channel_version;

protected:
	static const unsigned char bytemaskBTG[];
};

class Collector;
class LIBGE_API QuadTreePacket16
{
public:
	QuadTreePacket16();
	virtual ~QuadTreePacket16();

public:
	void clear();
	bool decode(const char* data, unsigned long size);
	const QuadTreeQuantum16* FindNode(int node_index, bool root_node) const;
	void GetDataReferences(QuadtreeDataReferenceGroup *references, const QuadtreePath &path_prefix, const JpegCommentDate& jpeg_date, bool root_node);
	std::string ToString(bool root_node, bool detailed_info) const;
	bool HasLayerOfType(LayerType layer_type) const;

protected:
	const QuadTreeQuantum16* FindNodeImpl(int node_index, const QuadtreeNumbering &numbering, int* num, const QuadtreePath qt_path) const;
	// Traverse node using a Collecting Parameter
	void Traverser(Collector *collector, const QuadtreeNumbering &numbering, int *node_indexp, const QuadtreePath &qt_path) const;

	// Count nodes in packets
	void CountNodesQTPR(int* nodenum, int num, int* curlevel, int* curpos, const QuadTreePacket16* qtpackets, int level) const;

protected:
	unsigned int _magic_id;
	unsigned int _data_type_id;
	unsigned int _version;	
	int _data_instance_size;
	int _data_buffer_offset;
	int _data_buffer_size;
	int _meta_buffer_size;
	std::vector<QuadTreeQuantum16*> _data_instances;
};
LIBGE_NAMESPACE_END
