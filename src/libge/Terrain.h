#pragma once
#include "export.h"
#include <assert.h>
#include <string>
#include <vector>
#include <map>

LIBGE_NAMESPACE_BEGINE
// Terrain mesh
class LIBGE_API MeshVertex
{
public:
	double x;
	double y;
	float z;
};

class LIBGE_API MeshFace
{
public:
	unsigned short a, b, c;
};

class LIBGE_API Mesh
{
public:
	Mesh();
	~Mesh();

public:
	void Reset();
	inline int source_size() const { return source_size_; }
	inline double ox() const { return ox_; }
	inline double oy() const { return oy_; }
	inline double dx() const { return dx_; }
	inline double dy() const { return dy_; }
	inline int num_points() const { return num_points_; }
	inline int num_faces() const { return num_faces_; }
	inline int level() const { return level_; }
	inline const MeshVertex &Vertex(size_t i) { return vertices_.at(i); }
	inline const MeshFace &Face(size_t i) { return faces_.at(i); }

	bool decode(const char* srcData, unsigned long size, unsigned long& offset);

private:
	int source_size_;
	double ox_;
	double oy_;
	double dx_;
	double dy_;
	int num_points_;
	int num_faces_;
	int level_;
	std::vector<MeshVertex> vertices_;
	std::vector<MeshFace> faces_;
};

class LIBGE_API Meshs : public std::vector<Mesh>
{
public:
	Meshs() {}
	Meshs(const std::string& name)
		: qtNode_(name)
	{
	}

public:
	const std::string& name() { return qtNode_; }
	void name(const std::string& name) { qtNode_ = name; }

	//const std::vector<Mesh>& meshs() { return meshs_; } const
	//std::vector<Mesh>& meshs() { return meshs_; }

private:
	std::string qtNode_;
	//std::vector<Mesh> meshs_;
};

class LIBGE_API Terrain : public std::vector<Meshs>
{
public:
	Terrain() {}
	Terrain(const std::string& name);	
	~Terrain();

public:
	const std::string& name() { return qtNode_; }
	void name(const std::string& name) { qtNode_ = name; }
	bool decode(const char* srcData, unsigned long size);
	std::string toDEM(int index, int& nCols, int& nRows, bool isMercator = false);
	std::string toDEM(Meshs& meshs, int& nCols, int& nRows, bool isMercator = false);

public:
	std::string qtNode_;
};
LIBGE_NAMESPACE_END
