#include "stdafx.h"
#include "Terrain.h"
#include <algorithm>
#include "qtutils.h"
#include "gdal_priv.h"

LIBGE_NAMESPACE_BEGINE
//  - Taken from USGS publication: "Map Projections -- A Working Manual", pg 12
static const double khWGS84EquatorialRadius = 6378137.0;
static const double DEGREE_TO_METER = 111319.49079327358;

// NASA's mean 6371.01+/-0.02 km
static const double khEarthMeanRadius = 6371010.0;

static const int kNegativeElevationExponentBias = 32;
static const double kCompressedNegativeAltitudeThreshold = 1e-12;

// It really doesn't matter what this number is so long as it matches the
// client. 2.2 clients had 6371000 hard coded, so we'll pick the mean which
// is really close. Earlier and later clients look at the dbRoot for the
// planet radius. The dbRoot had better match this number.
static const double khEarthRadius = khEarthMeanRadius;
#define TORAD(a) ((a) * PI/180.0)
// Meters to Earth's radius conversion factor
const double EarthPlanetaryConstant = 1.0 / khEarthRadius;
// Meters to Marss radius conversion factor
const double MarsPlanetaryConstant = 2.94377391816E-7;
const double PlanetaryConstant = EarthPlanetaryConstant;
// Support negative elevations starting in the 5.0 client, but allow older
// clients to show negative elevations as essentially zero.  Divide negative
// elevations by a large negative factor so that old clients can handle them
// (essentially zero), but new clients will know to multiply very small
// elevation numbers by the factor to recover the original elevation.
// Note: the factor is negative since the client has a performance issue when
// taking even small negative elevations in.
const double NegativeElevationFactor = -pow(2, kNegativeElevationExponentBias);

char GOOGLE_EARTH_TERRAIN_KEY[] = { 0x0a, 0x02, 0x08, 0x01 };
const size_t kEmptyMeshHeaderSize = 16;

extern void convertEndian(LPVOID lpSrc, int size, LPVOID lpDst, bool littleEndian = false);



//////////////////////////////////////////////////////////////////////////
// Mesh
Mesh::Mesh() 
{ 
	Reset(); 
}

Mesh::~Mesh() 
{
}

void Mesh::Reset()
{
	source_size_ = 0;
	ox_ = 0.0;
	oy_ = 0.0;
	dx_ = 0.0;
	dy_ = 0.0;
	num_points_ = 0;
	num_faces_ = 0;
	level_ = 0;
	faces_.clear();
	vertices_.clear();
}

bool Mesh::decode(const char* srcData, unsigned long size, unsigned long& offset)
{
	if (srcData == nullptr || size <= 0U)
		return false;

	unsigned long dataOffset = kEmptyMeshHeaderSize;
	const char* data = srcData;
	convertEndian((LPVOID)data, 4, &source_size_, true); data += 4;
	if (source_size_ != 0)
	{
		unsigned int tileSize = 256;
		unsigned int SizeX = tileSize / 2 + 1;
		unsigned int SizeY = tileSize / 2 + 1;

		convertEndian((LPVOID)data, 8, &ox_, true); data += 8;
		convertEndian((LPVOID)data, 8, &oy_, true); data += 8;
		convertEndian((LPVOID)data, 8, &dx_, true); data += 8;
		convertEndian((LPVOID)data, 8, &dy_, true); data += 8;
		convertEndian((LPVOID)data, 4, &num_points_, true); data += 4;
		convertEndian((LPVOID)data, 4, &num_faces_, true); data += 4;
		convertEndian((LPVOID)data, 4, &level_, true); data += 4;
		ox_ *= 180.0;
		oy_ *= 180.0;
		dx_ *= 180.0;
		dy_ *= 180.0;
		/*
		printf("source_size:%d level:%d pts/faces:%d,%d orig:%.12f,%.12f dx/dy:%.12f,%.12f\r\n",
			source_size_,
			level_,
			num_points_,
			num_faces_,
			ox_, oy_,
			dx_, dy_);
		*/
		vertices_.resize(num_points_);
		for (int i = 0; i < num_points_; ++i)
		{
			unsigned char c_tmp;
			convertEndian((LPVOID)data, 1, &c_tmp, true); data += 1;
			vertices_[i].x = c_tmp*dx_ + ox_;
			convertEndian((LPVOID)data, 1, &c_tmp, true); data += 1;
			vertices_[i].y = c_tmp*dy_ + oy_;
			convertEndian((LPVOID)data, 4, &(vertices_[i].z), true); data += 4;
			vertices_[i].z /= PlanetaryConstant;
			//printf("x: %.12f y: %.12f z: %.12f\r\n", vertices_[i].x, vertices_[i].y, vertices_[i].z);
		}
		faces_.resize(num_faces_);
		for (int i = 0; i < num_faces_; ++i)
		{
			convertEndian((LPVOID)data, 2, &(faces_[i].a), true); data += 2;
			convertEndian((LPVOID)data, 2, &(faces_[i].b), true); data += 2;
			convertEndian((LPVOID)data, 2, &(faces_[i].c), true); data += 2;
		}

		dataOffset = source_size_ + 4;// 48 + num_points_ * 6 + num_faces_ * 6;
	}
	else
		Reset();

	offset += dataOffset;
	return source_size_ != 0;
}
//////////////////////////////////////////////////////////////////////////

Terrain::Terrain(const std::string& name)
	: qtNode_(name)
{
}

Terrain::~Terrain()
{
}

bool Terrain::decode(const char* srcData, unsigned long srcSize)
{
	clear();
	unsigned long offset = 0U;
	while (offset < srcSize)
	{
		if (memcmp(srcData + offset, GOOGLE_EARTH_TERRAIN_KEY, sizeof(GOOGLE_EARTH_TERRAIN_KEY)) == 0)
			break;

		Mesh mesh;
		if (mesh.decode(srcData + offset, srcSize, offset))
		{
			std::string name = ConvertToQtNode(mesh.Vertex(0).y, mesh.Vertex(0).x, mesh.level()-1, false);
			std::vector<Meshs>::iterator it = begin();
			for (; it != end(); it++)
			{
				if (_stricmp(name.c_str(), it->name().c_str())==0)
					break;
			}

			if (it != end())
				it->push_back(mesh);
			else
			{
				Meshs meshs(name);				
				meshs.push_back(mesh);
				push_back(meshs);
			}
		}
	}

	return size()>0;
}

bool Intersect1(double X1, double Y1, double X2, double Y2, double X3, double Y3, double X4, double Y4, double& X, double& Y)
{
	double a1 = Y2 - Y1;
	double b1 = X1 - X2;
	double c1 = Y1*(X2 - X1) + X1*(Y1 - Y2);

	double a2 = Y4 - Y3;
	double b2 = X3 - X4;
	double c2 = Y3*(X4 - X3) + X3*(Y3 - Y4);

	//如果两条直线平行
	if (fabs(a1*b2 - a2*b1) < 1e-6)	
		return false;	

	X = (b1*c2 - b2*c1) / (a1*b2 - a2*b1);
	Y = (a2*c1 - a1*c2) / (a1*b2 - a2*b1);

	if ((X - X1)*(X - X2) <= 1e-6 && (Y - Y1)*(Y - Y2) <= 1e-6 && (X - X3)*(X - X4) <= 1e-6 && (Y - Y3)*(Y - Y4) <= 1e-6)
		return true;
	return false;
}

std::string Terrain::toDEM(int index, int& nCols, int& nRows, bool isMercator)
{
	if (index >= size() || index<0)
		return "";

	std::vector<Meshs>::iterator it = begin() + index;
	return toDEM(*it, nCols, nRows, isMercator);
}

std::string Terrain::toDEM(Meshs& meshs, int& nCols, int& nRows, bool isMercator)
{
	if (meshs.size() <= 0)
		return "";

	double minx_;
	double maxx_;
	double miny_;
	double maxy_;
	unsigned int z;
	QtNodeBounds(meshs.name(), isMercator, &miny_, &minx_, &maxy_, &maxx_, &z);

	int size = sqrt(meshs.size())*128;
	nRows = size;
	nCols = size;
	double lfCellSizeX = (maxx_ - minx_) / static_cast<double>(size);
	double lfCellSizeY = (maxy_ - miny_) / static_cast<double>(size);
	double LBX = minx_;
	double LBY = miny_;
	double RTX = maxx_;
	double RTY = maxy_;
	double epsilon = 1e-5;
	if (!isMercator)
	{
		LatLonToMeters(miny_, minx_, LBX, LBY);
		LatLonToMeters(maxy_, maxx_, RTX, RTY);
	}

	lfCellSizeX = (RTX - LBX) / static_cast<double>(nCols);
	lfCellSizeY = (RTY - LBY) / static_cast<double>(nRows);
	float* pAltitude = new float[nRows*nCols];
	double noDataValue = -FLT_MAX;
	for (int i = 0; i < nRows*nCols; i++)
		pAltitude[i] = noDataValue;

	double X[3], Y[3], Z[3];
	for (Meshs::iterator it = meshs.begin(); it != meshs.end(); it++)
	{			
		for (int f = 0; f < it->num_faces(); f++)
		{
			const MeshFace& face = it->Face(f);
			X[0] = it->Vertex(face.a).x;
			Y[0] = it->Vertex(face.a).y;
			Z[0] = it->Vertex(face.a).z;
			X[1] = it->Vertex(face.b).x;
			Y[1] = it->Vertex(face.b).y;
			Z[1] = it->Vertex(face.b).z;
			X[2] = it->Vertex(face.c).x;
			Y[2] = it->Vertex(face.c).y;
			Z[2] = it->Vertex(face.c).z;

			{
				for (int i = 0; i < 3; i++)
					LatLonToMeters(Y[i], X[i], X[i], Y[i]);
			}

			double minx = DBL_MAX;
			double miny = DBL_MAX;
			double maxx = DBL_MIN;
			double maxy = DBL_MIN;
			for (int j = 0; j < 3; j++)
			{
				minx = __min(minx, X[j]);
				maxx = __max(maxx, X[j]);
				miny = __min(miny, Y[j]);
				maxy = __max(maxy, Y[j]);
			}

			//判断此三角形范围内是否存在DEM格网点
			int nLBX = (int)(0.5 + (minx - LBX) / lfCellSizeX);
			int nLBY = (int)(0.5 + (miny - LBY) / lfCellSizeY);
			int nRTX = (int)((maxx - LBX) / lfCellSizeX + 0.5);
			int nRTY = (int)((maxy - LBY) / lfCellSizeY + 0.5);
			nLBX = __min(__max(nLBX, 0), nCols-1);
			nRTX = __min(__max(nRTX, 0), nCols - 1);
			nLBY = __min(__max(nLBY, 0), nRows - 1);
			nRTY = __min(__max(nRTY, 0), nRows - 1);
			//为落在此三角形内的格网点内差高程
			for (int rowIndex = nLBY; rowIndex <= nRTY; rowIndex++)
			{
				if (rowIndex < 0 || rowIndex >= nRows)
					continue;

				double demY = LBY + rowIndex*lfCellSizeY;

				if (demY<miny || demY>maxy)
					continue;

				for (int colIndex = nLBX; colIndex <= nRTX; colIndex++)
				{
					if (colIndex < 0 || colIndex >= nCols)
					{
						continue;
					}
					double demX = LBX + colIndex*lfCellSizeX;

					if (demX<minx || demX>maxx)
						continue;

					double dScale = 1.0;
					double dx01 = X[1] - X[0];
					double dy01 = Y[1] - Y[0];
					double dx12 = X[2] - X[1];
					double dy12 = Y[2] - Y[1];
					double dx20 = X[0] - X[2];
					double dy20 = Y[0] - Y[2];
					double dx1 = demX - X[1];
					double dy1 = demY - Y[1];
					double dx2 = demX - X[2];
					double dy2 = demY - Y[2];
					double dx0 = demX - X[0];
					double dy0 = demY - Y[0];

					double v01 = dx01*dy1 - dx1*dy01;
					double v12 = dx12*dy2 - dx2*dy12;
					double v20 = dx20*dy0 - dx0*dy20;
					if (fabs(v01)*dScale < epsilon &&
						(demX - X[0])*(demX - X[1])*dScale <= epsilon &&
						(demY - Y[0])*(demY - Y[1])*dScale <= epsilon)
					{
						double L01 = sqrt((X[0] - X[1])*(X[0] - X[1]) + (Y[0] - Y[1])*(Y[0] - Y[1]));
						double L0D = sqrt((demX - X[0])*(demX - X[0]) + (demY - Y[0])*(demY - Y[0]));
						double L1D = sqrt((demX - X[1])*(demX - X[1]) + (demY - Y[1])*(demY - Y[1]));
						pAltitude[(nRows - 1 - rowIndex)*nCols + colIndex] = Z[0] * L1D / L01 + Z[1] * L0D / L01;
					}
					else if (fabs(v12)*dScale < epsilon &&
						(demX - X[1])*(demX - X[2])*dScale <= epsilon &&
						(demY - Y[1])*(demY - Y[2])*dScale <= epsilon)
					{
						double L12 = sqrt((X[2] - X[1])*(X[2] - X[1]) + (Y[2] - Y[1])*(Y[2] - Y[1]));
						double L2D = sqrt((demX - X[2])*(demX - X[2]) + (demY - Y[2])*(demY - Y[2]));
						double L1D = sqrt((demX - X[1])*(demX - X[1]) + (demY - Y[1])*(demY - Y[1]));
						pAltitude[(nRows - 1 - rowIndex)*nCols + colIndex] = Z[1] * L2D / L12 + Z[2] * L1D / L12;
					}
					else if (fabs(v20)*dScale < epsilon &&
						(demX - X[2])*(demX - X[0])*dScale <= epsilon &&
						(demY - Y[2])*(demY - Y[0])*dScale <= epsilon)
					{
						double L20 = sqrt((X[2] - X[0])*(X[2] - X[0]) + (Y[2] - Y[0])*(Y[2] - Y[0]));
						double L2D = sqrt((demX - X[2])*(demX - X[2]) + (demY - Y[2])*(demY - Y[2]));
						double L0D = sqrt((demX - X[0])*(demX - X[0]) + (demY - Y[0])*(demY - Y[0]));
						pAltitude[(nRows - 1 - rowIndex)*nCols + colIndex] = Z[0] * L2D / L20 + Z[2] * L0D / L20;
					}
					else if ((v01*dScale > epsilon&&v12*dScale > epsilon&&v20*dScale > epsilon) || (v01*dScale < epsilon&&v12*dScale < epsilon&&v20*dScale < epsilon))
					{
						double tempX, tempY;
						Intersect1(X[0], Y[0], demX, demY, X[1], Y[1], X[2], Y[2], tempX, tempY);

						double L12 = sqrt((X[2] - X[1])*(X[2] - X[1]) + (Y[2] - Y[1])*(Y[2] - Y[1]));
						double L1T = sqrt((tempX - X[1])*(tempX - X[1]) + (tempY - Y[1])*(tempY - Y[1]));
						double L2T = sqrt((tempX - X[2])*(tempX - X[2]) + (tempY - Y[2])*(tempY - Y[2]));
						double tempZ = Z[1] * L2T / L12 + Z[2] * L1T / L12;

						double L0T = sqrt((tempX - X[0])*(tempX - X[0]) + (tempY - Y[0])*(tempY - Y[0]));
						double L0D = sqrt((demX - X[0])*(demX - X[0]) + (demY - Y[0])*(demY - Y[0]));
						double LTD = sqrt((demX - tempX)*(demX - tempX) + (demY - tempY)*(demY - tempY));
						pAltitude[(nRows - 1 - rowIndex)*nCols + colIndex] = Z[0] * LTD / L0T + tempZ*L0D / L0T;
					}
				}
			}
		}
	}

	std::string imgData;
	imgData.resize(nCols*nRows*sizeof(float));
	memcpy(imgData._Myptr(), pAltitude, imgData.size());
	delete[] pAltitude; pAltitude = nullptr;
	return imgData;
}
LIBGE_NAMESPACE_END