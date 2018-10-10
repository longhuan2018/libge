#include "stdafx.h"
#include "QuadTreePacket.h"
#include <sstream>

LIBGE_NAMESPACE_BEGINE
const unsigned char QuadTreeQuantum16::bytemaskBTG[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
const unsigned int kKeyholeMagicId = 0x7E2D;

static QuadtreeNumbering tree_numbering(5, true);
static QuadtreeNumbering root_numbering(4, false);

extern void convertEndian(LPVOID lpSrc, int size, LPVOID lpDst, bool littleEndian = false);

//////////////////////////////////////////////////////////////////////////
//
// Define Collecting Parameter classes for traversals
class Collector {
public:
	Collector() {}
	virtual ~Collector() {}
	virtual void Collect(const int node_index, const QuadtreePath &qt_path, const int subindex, const QuadTreeQuantum16* node) = 0;
private:
};

class ToStringCollector : public Collector {
public:
	ToStringCollector() {}
	~ToStringCollector() {}
	virtual void Collect(const int node_index, const QuadtreePath &qt_path, const int subindex, const QuadTreeQuantum16* node) 
	{
		node->ToString(ostr_, node_index, qt_path, subindex);
	}
	std::ostringstream &ostr() { return ostr_; }
private:
	std::ostringstream ostr_;
};

class DataReferenceCollector : public Collector {
public:
	DataReferenceCollector(QuadtreeDataReferenceGroup *refs, const QuadtreePath &path_prefix)
		: refs_(refs),
		path_prefix_(path_prefix) {
		assert(refs);
	}
	~DataReferenceCollector() {}
	virtual void Collect(const int node_index, const QuadtreePath &qt_path, const int subindex, const QuadTreeQuantum16* node) 
	{
		node->GetDataReferences(refs_, path_prefix_ + qt_path);
	}
private:
	QuadtreeDataReferenceGroup *refs_;  // not owned by this class
	const QuadtreePath path_prefix_;      // absolute path to this qt packet
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
void QuadTreeQuantum16::Cleanup() 
{
	_channel_type.clear();
	_channel_version.clear();
}

// Get references to all packets referred to in the quantum
// (including other quadtree packets) and append to result.

void QuadTreeQuantum16::GetDataReferences(QuadtreeDataReferenceGroup *references,const QuadtreePath &qt_path) const 
{
	if (references->qtp_refs && GetCacheNodeBit()) 
	{
		references->qtp_refs->push_back(QuadtreeDataReference(qt_path, _cnode_version, 0, 0));
	}
	if (references->img_refs && GetImageBit()) 
	{
		references->img_refs->push_back(QuadtreeDataReference(qt_path, _image_version, LayerType::LAYER_TYPE_IMAGERY, _image_data_provider));
	}
	if (references->ter_refs && GetTerrainBit()) 
	{
		references->ter_refs->push_back(QuadtreeDataReference(qt_path, _terrain_version, LayerType::LAYER_TYPE_TERRAIN, _terrain_data_provider));
	}
	if (references->vec_refs && GetDrawableBit()) 
	{
		unsigned short num_channels = _channel_type.size();
		for (unsigned short j = 0; j < num_channels; ++j) 
		{
			references->vec_refs->push_back(QuadtreeDataReference(qt_path, _channel_version[j], _channel_type[j], 0));
		}
	}
}

bool QuadTreeQuantum16::HasLayerOfType(LayerType layer_type) const 
{
	if (GetImageBit() &&
		layer_type == LayerType::LAYER_TYPE_IMAGERY) {
		return true;
	}
	if (GetTerrainBit() &&
		layer_type == LayerType::LAYER_TYPE_TERRAIN) {
		return true;
	}
	if (GetDrawableBit() &&
		layer_type == LayerType::LAYER_TYPE_VECTOR) {
		return true;
	}
	return false;
}

// ToString() - convert node contents to printable string
void QuadTreeQuantum16::ToString(std::ostringstream &str, const int node_index, const QuadtreePath &qt_path, const int subindex) const 
{
	str << "  node " << node_index
		<< "  s" << subindex
		<< " \"" << qt_path.AsString() << "\""
		<< "  iv = " << _image_version
		<< ", ip = " << (unsigned short)_image_data_provider
		<< ", tv = " << _terrain_version
		<< ", tp = " << (unsigned short)_terrain_data_provider
		<< ", c = " << _cnode_version
		<< ", flags = 0x" << std::hex
		<< static_cast<unsigned>(_children) << std::dec
		<< "(";

	if (GetImageBit())
		str << "I";
	if (GetTerrainBit())
		str << "T";
	if (GetDrawableBit())
		str << "V";
	if (GetCacheNodeBit())
		str << "C";
	for (int j = 0; j < 4; ++j) 
	{
		if (GetBit(j))
			str << "0123"[j];
	}
	str << ")" << std::endl;

	unsigned short num_channels = _channel_type.size();
	for (unsigned short j = 0; j < num_channels; ++j) {
		str << "    V" << j << ": layer = " << _channel_type[j]
			<< ", version = " << _channel_version[j] << std::endl;
	}
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// QuadTreePacket16
QuadTreePacket16::QuadTreePacket16()
{
	_magic_id = 0U;
	_data_type_id = 0U;
	_version = 0U;
	_data_instance_size = 0;
	_data_buffer_offset = 0;
	_data_buffer_size = 0;
	_meta_buffer_size = 0;
}

QuadTreePacket16::~QuadTreePacket16()
{
	clear();
}

void QuadTreePacket16::clear()
{
	_magic_id = 0U;
	_data_type_id = 0U;
	_version = 0U;
	_data_instance_size = 0;
	_data_buffer_offset = 0;
	_data_buffer_size = 0;
	_meta_buffer_size = 0;

	for (std::vector<QuadTreeQuantum16*>::iterator it = _data_instances.begin(); it != _data_instances.end(); it++)
	{
		QuadTreeQuantum16* quantum = *it;
		if (quantum != nullptr)
			delete quantum;
	}
	_data_instances.clear();
}

bool QuadTreePacket16::decode(const char* srcData, unsigned long size)
{
	_data_instances.clear();
	if (srcData == nullptr || size < 0)
		return false;

	const char* data = srcData;
	convertEndian((LPVOID)data, 4, &_magic_id, true); data += 4;
	if (_magic_id != kKeyholeMagicId)
		return false;

	int num_instances;
	convertEndian((LPVOID)data, 4, &_data_type_id, true); data += 4;
	convertEndian((LPVOID)data, 4, &_version, true); data += 4;
	convertEndian((LPVOID)data, 4, &num_instances, true); data += 4;
	convertEndian((LPVOID)data, 4, &_data_instance_size, true); data += 4;
	convertEndian((LPVOID)data, 4, &_data_buffer_offset, true); data += 4;
	convertEndian((LPVOID)data, 4, &_data_buffer_size, true); data += 4;
	convertEndian((LPVOID)data, 4, &_meta_buffer_size, true); data += 4;
	if (num_instances <= 0)
		return true;

	for (int i = 0; i < num_instances; i++)
	{
		QuadTreeQuantum16* quantum = new QuadTreeQuantum16();
		unsigned char kByteFiller;
		unsigned short kWordFiller;
		convertEndian((LPVOID)data, 1, &quantum->_children, true); data += 1;
		convertEndian((LPVOID)data, 1, &kByteFiller, true); data += 1;
		convertEndian((LPVOID)data, 2, &quantum->_cnode_version, true); data += 2;
		convertEndian((LPVOID)data, 2, &quantum->_image_version, true); data += 2;
		convertEndian((LPVOID)data, 2, &quantum->_terrain_version, true); data += 2;
		unsigned short num_channels;
		convertEndian((LPVOID)data, 2, &num_channels, true); data += 2;
		convertEndian((LPVOID)data, 2, &kWordFiller, true); data += 2;
		int type_offset;
		int version_offset;
		convertEndian((LPVOID)data, 4, &type_offset, true); data += 4;
		convertEndian((LPVOID)data, 4, &version_offset, true); data += 4;
		memcpy(quantum->_image_neighbors, data, sizeof(quantum->_image_neighbors)); data += sizeof(quantum->_image_neighbors);
		convertEndian((LPVOID)data, 1, &quantum->_image_data_provider, true); data += 1;
		convertEndian((LPVOID)data, 1, &quantum->_terrain_data_provider, true); data += 1;
		convertEndian((LPVOID)data, 2, &kWordFiller, true); data += 2;

		if (num_channels > 0)
		{
			quantum->_channel_type.resize(num_channels);
			quantum->_channel_version.resize(num_channels);
			memcpy(quantum->_channel_type.data(), srcData + _data_buffer_offset + type_offset, sizeof(unsigned short)*num_channels);
			memcpy(quantum->_channel_version.data(), srcData + _data_buffer_offset + version_offset, sizeof(unsigned short)*num_channels);
		}

		_data_instances.push_back(quantum);

// 		if (quantum->GetImageBit() || quantum->GetTerrainBit())
// 		{
// 			if (quantum->GetImageBit())
// 				printf("image_version: %ld, image_data_provider: %ld   ", quantum->_image_version, quantum->_image_data_provider);
// 
// 			if (quantum->GetTerrainBit())
// 				printf("terrain_version: %ld, terrain_data_provider: %ld  ", quantum->_terrain_version, quantum->_terrain_data_provider);
// 
// 			printf("\r\nimage_neighbors: ");
// 			for (int k = 0; k < 8; k++)
// 				printf("%d  ", quantum->_image_neighbors[k]);
// 			printf("\r\n");
// 		}
	}

	return true;
}

const QuadTreeQuantum16* QuadTreePacket16::FindNode(int node_index, bool root_node) const 
{
	int num = 0;
	QuadtreePath qt_path;
	return FindNodeImpl(node_index, root_node ? root_numbering : tree_numbering, &num, qt_path);
}

const QuadTreeQuantum16* QuadTreePacket16::FindNodeImpl(int node_index, const QuadtreeNumbering &numbering, int* num, const QuadtreePath qt_path) const 
{
	if (*num >= _data_instances.size())
		return nullptr;

	if (qt_path.Level() >= numbering.depth())
		return nullptr;

	const QuadTreeQuantum16* node = _data_instances.at(*num);	
	if (node_index == numbering.InorderToSubindex(numbering.TraversalPathToInorder(qt_path)))
		return node;

	// Descend to children, using children bits in the packet
	for (int i = 0; i < 4; ++i) 
	{
		if (node->GetBit(i)) 
		{
			const QuadtreePath new_qt_path(qt_path.Child(i));
			*num += 1;
			const QuadTreeQuantum16* child_node = FindNodeImpl(node_index, numbering, num, new_qt_path);
			if (child_node != NULL)
				return child_node;
		}
	}

	return nullptr;
}

void QuadTreePacket16::GetDataReferences(QuadtreeDataReferenceGroup *references, const QuadtreePath &path_prefix, const JpegCommentDate& jpeg_date, bool root_node)
{
	DataReferenceCollector collector(references, path_prefix);
	int node_index = 0;
	Traverser(&collector,
		root_node ? root_numbering : tree_numbering,//(path_prefix.Level() == 0) ? root_numbering : tree_numbering,
		&node_index,
		QuadtreePath());
}

bool QuadTreePacket16::HasLayerOfType(LayerType layer_type) const 
{
	if (layer_type == LayerType::LAYER_TYPE_IMAGERY_HISTORY) 
		return false;

	int node_count = _data_instances.size();
	for (int i = 0; i < node_count; ++i) 
	{
		const QuadTreeQuantum16* node = _data_instances.at(i);
		if (node!=nullptr && node->HasLayerOfType(layer_type)) 
			return true;
	}
	return false;
}

std::string QuadTreePacket16::ToString(bool root_node, bool detailed_info) const 
{
	ToStringCollector collector;
	collector.ostr() << _data_instances.size() << " nodes" << std::endl;
	int node_index = 0;
	Traverser(&collector, root_node ? root_numbering : tree_numbering, &node_index, QuadtreePath());
	return collector.ostr().str();
}

// Traverser - traverse the nodes of the quadtree packet with a
// Collecting Parameter
void QuadTreePacket16::Traverser(Collector *collector, const QuadtreeNumbering &numbering, int *node_indexp, const QuadtreePath &qt_path) const 
{
	if (*node_indexp >= _data_instances.size())
		return;

	const QuadTreeQuantum16* node = _data_instances.at(*node_indexp);
	if (qt_path.Level() >= numbering.depth())
		return;

	int subindex = numbering.InorderToSubindex(numbering.TraversalPathToInorder(qt_path));
	collector->Collect(*node_indexp, qt_path, subindex, node);

	// Descend to children, using children bits in the packet
	for (int i = 0; i < 4; ++i) 
	{
		if (node->GetBit(i)) 
		{
			const QuadtreePath new_qt_path(qt_path.Child(i));
			*node_indexp += 1;
			Traverser(collector, numbering, node_indexp, new_qt_path);
		}
	}
}

void QuadTreePacket16::CountNodesQTPR(int* nodenum, int num, int* curlevel, int* curpos, const QuadTreePacket16* qtpackets, int level) const 
{
	(*nodenum)++;
	int* nodepos = new int[num];

	memcpy(nodepos, curpos, sizeof(int)*num);

	// Traverse Children
	for (int i = 0; i < 4; i++) {
		// Check for children in any of the packets
		bool flag = false;
		for (int j = 0; j < num; j++) {
			// Check level first. Otherwise we'll do GetPtr()
			// on a potentially invalid index.
			if (curlevel[j] == level &&
				qtpackets[j]._data_instances.at(nodepos[j])->GetBit(i)) {
				curlevel[j]++;
				curpos[j]++;
				flag = true;
			}
		}
		if (flag)
			CountNodesQTPR(nodenum, num, curlevel, curpos, qtpackets, level + 1);
	}

	for (int j = 0; j < num; j++)
		if (curlevel[j] >= level)
			curlevel[j] = level - 1;
	delete[] nodepos;
	nodepos = nullptr;
}
LIBGE_NAMESPACE_END
