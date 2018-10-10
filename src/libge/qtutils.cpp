#include "stdafx.h"
#include "qtutils.h"

#include <math.h>
#include <algorithm>
#include <iostream>  // NOLINT(readability/streams)
#include <string>
#include <vector>
#undef max
#undef min

LIBGE_NAMESPACE_BEGINE

/**
* Fills in all of the mercator addresses that correspond to nodes that touch
* the lat/lng bounding box of the node specifed by the given
* plate carree address. This could be optimized more by doing
* more of the work inline.
*/
void ConvertFlatToMercatorQtAddresses(std::string flat_qtaddress, std::vector<std::string>* mercator_qtaddresses) 
{
	// x, y, z correspond to their Maps query usage (column, row, and LOD).
	unsigned int x;
	unsigned int y;
	unsigned int z;

	ConvertFromQtNode(flat_qtaddress, &x, &y, &z);
	// Dimension of the grid is 2^z by 2^z.
	double max_ypos = static_cast<double>(1 << z);
	double y_value = static_cast<double>(y);

	// Calculate top and bottom latitude of tile.
	// We use a 360 degree span because only the middle section of
	// the grid is used for plate carree. The entire grid actually
	// spans (-180,180), but (-180,-90) and (90,180) are black.
	double min_lat = 180.0 - (360.0 * y_value / max_ypos);
	double max_lat = 180.0 - (360.0 * (y_value + 1.0) / max_ypos);

	// Find the corresponding y range on a Mercator map.
	// Be aggressive in inclusion, and allow unfound tiles
	// to just be ignored.
	unsigned int y_bottom = LatToYPos(min_lat, z, true);
	unsigned int y_top = LatToYPos(max_lat, z, true);
	// Add a Mercator quadtree address for each value of y.
	for (unsigned int y_next = y_bottom; y_next <= y_top; y_next++) {
		mercator_qtaddresses->push_back(ConvertToQtNode(x, y_next, z));
	}
}

/**
* Returns the y position on a grid at given depth corresponding
* to the given latitude.
* yPos is an integer in the range of [0, 2^z).
*/
unsigned int LatToYPos(double lat, unsigned int z, bool is_mercator) 
{
	double y;
	double min_y;
	double max_y;
	unsigned int y_off;
	unsigned int max_ypos = 1 << z;

	// y is inverted (montonic downwards)
	lat = -lat;
	if (is_mercator) {
		y = MercatorLatToY(lat);
		min_y = -PI;
		max_y = PI;
		y_off = 0;
	}
	else {
		y = lat;
		min_y = -90.0;
		max_y = 90.0;
		// For non-Mercator, we are using only half the y space.
		max_ypos >>= 1;
		// Imagery starts a quarter of the way down the grid.
		y_off = max_ypos >> 1;
	}

	// Force legal y pos for illegal inputs beyond extremes.
	if (y >= max_y) {
		return max_ypos - 1 + y_off;
	}
	else if (y < min_y) {
		return y_off;
	}
	else {
		return (y - min_y) / (max_y - min_y) * max_ypos + y_off;
	}
}

double YPosToLat(unsigned int y, unsigned int z, bool is_mercator)
{
	double min_y;
	double max_y;
	unsigned int y_off;
	unsigned int max_ypos = 1 << z;

	if (is_mercator) {
		min_y = -PI;
		max_y = PI;
		y_off = 0;
	}
	else {
		min_y = -90.0;
		max_y = 90.;
		// For non-Mercator, we are using only half the y space.
		max_ypos >>= 1;
		// Imagery starts a quarter of the way down the grid.
		y_off = max_ypos >> 1;
	}

	if (y >= max_ypos)
	{
		return min_y;
	}
	else if (y <=y_off )
	{
		return max_y;
	}
	else
	{
		double lat = (y - y_off)*(max_y - min_y) / max_ypos + min_y;
		if ( is_mercator )
			MercatorYToLat(lat);
		return -lat;
	}
}

/**
* Returns the y position associated with the given latitude.
* y is a double in the range of (-pi, pi).
*/
double MercatorLatToY(double lat) 
{
	if (lat >= MAX_MERCATOR_LATITUDE) 
	{
		return PI;
	}
	else if (lat <= -MAX_MERCATOR_LATITUDE) 
	{
		return -PI;
	}

	return log(tan(PI / 4.0 + lat / 360.0 * PI));
}

double MercatorLngToX(double lng)
{
	if (lng >= MAX_MERCATOR_LONGITUDE)
	{
		return PI;
	}
	else if (lng <= -MAX_MERCATOR_LONGITUDE)
	{
		return -PI;
	}

	return lng;
}

/**
* Returns the latitude associated with the given y location.
* y is a double in the range of (-pi, pi).
*/
double MercatorYToLat(double y) 
{
	if (y >= PI) {
		return MAX_MERCATOR_LATITUDE;
	}
	else if (y <= -PI) {
		return -MAX_MERCATOR_LATITUDE;
	}

	return (atan(exp(y)) - PI / 4.0) * 360.0 / PI;
}

double MercatorXToLng(double x)
{
	if (x >= PI) {
		return MAX_MERCATOR_LONGITUDE;
	}
	else if (x <= -PI) {
		return -MAX_MERCATOR_LONGITUDE;
	}

	return x;
}

/**
* Returns the position on a Mercator grid at given depth (z) corresponding
* to the normalized, linear y value.
* y is a double in the range of (-pi, pi).
* yPos is an integer in the range of [0, 2^z).
*/
unsigned int YToYPos(double y, unsigned int z) 
{
	// Use signed to check for underflow.
	int max_ypos = 1 << z;
	int ypos = max_ypos * ((y + PI) / (2.0 * PI));
	if (ypos < 0) {
		return 0;
	}
	else if (ypos >= max_ypos) 
	{
		return max_ypos - 1;
	}
	else {
		return static_cast<unsigned int>(ypos);
	}
}

/**
* Returns the latitude that will appear half way between the two given
* latitudes on a Mercator map.
*/
double BisectLatitudes(double south, double north, bool is_mercator) 
{
	if (is_mercator) 
	{
		double y1 = MercatorLatToY(south);
		double y2 = MercatorLatToY(north);
		return MercatorYToLat((y1 + y2) / 2.0);
	}
	else {
		return (south + north) / 2.0;
	}
}

/**
* Helper for converting from map space to qtnode address.
* Return empty string if input is invalid.
*/
std::string ConvertToQtNode(unsigned int x, unsigned int y, unsigned int z) 
{
	std::string qtnode = "0";
	// Half the width or height of the map coordinates at the target LOD.
	// I.e. the size of the top-level quadrants.
	unsigned int  half_ndim = 1 << (z - 1);
	for (unsigned int i = 0; i < z; ++i) 
	{
		// Choose quadtree address char based on quadrant that x, y fall into.
		if ((y >= half_ndim) && (x < half_ndim)) {
			qtnode += "0";
			y -= half_ndim;
		}
		else if ((y >= half_ndim) && (x >= half_ndim)) {
			qtnode += "1";
			y -= half_ndim;
			x -= half_ndim;
		}
		else if ((y < half_ndim) && (x >= half_ndim)) {
			qtnode += "2";
			x -= half_ndim;
		}
		else {
			qtnode += "3";
		}

		// Cut in half for the next level of quadrants.
		half_ndim >>= 1;
	}

	// x and y should be cleared by the end.
	if (x || y) 
	{
		return "";
	}

	return qtnode;
}

/**
* Helper for converting from qtnode address to map space.
* Expects leading "0". Returns MAX_LEVEL for zoom level if there
* is an error.
*/
void ConvertFromQtNode(const std::string& qtnode, unsigned int* x, unsigned int* y, unsigned int* z) 
{
	const char* ptr = qtnode.c_str();
	*z = qtnode.size();
	if (*z == 0) 
	{
		*z = MAX_LEVEL;
		return;
	}
	// LOD is the length of the quadtree address - 1.
	*z -= 1;

	if (*ptr != '0') 
	{
		*z = MAX_LEVEL;
		return;
	}

	// For each quadtree address character, add 1 bit of info to x and y
	// location.
	*x = 0;
	*y = 0;
	// Half the width or height of the map coordinates at the target LOD.
	// I.e. the size of the top-level quadrants.
	unsigned int half_ndim = 1 << (*z - 1);
	for (unsigned int i = 0; i < *z; ++i) 
	{
		switch (*++ptr) {
		case '0':
			*y += half_ndim;
			break;

		case '1':
			*x += half_ndim;
			*y += half_ndim;
			break;

		case '2':
			*x += half_ndim;
			break;

		case '3':
			// No change
			break;

		default:
			*z = MAX_LEVEL;
			return;
		}

		// Cut in half for the next level of quadrants.
		half_ndim >>= 1;
	}
}

bool QtNodeBounds(const std::string& name, bool is_mercator, double* minY, double* minX, double* maxY, double* maxX, unsigned int* level)
{
	if (name.empty() || minX == nullptr || minY == nullptr || maxX == nullptr || maxY == nullptr || level == nullptr)
		return false;

	*minX = -180.0;
	*maxY = 180.0;
	*maxX = 180.0;
	*minY = -180.0;	
	*level = name.size() - 1;
	for (int i = 1; i < name.size(); i++)
	{
		switch (name.at(i))
		{
		case '0':
		{
			*maxY = (*maxY + *minY) / 2.0;
			*maxX = (*minX + *maxX) / 2.0;
		}
		break;

		case '1':
		{
			*maxY = (*maxY + *minY) / 2.0;
			*minX = (*minX + *maxX) / 2.0;
		}
		break;

		case '2':
		{
			*minY = (*maxY + *minY) / 2.0;
			*minX = (*minX + *maxX) / 2.0;
		}
		break;

		case '3':
		{
			*minY = (*maxY + *minY) / 2.0;
			*maxX = (*minX + *maxX) / 2.0;
		}
		break;

		default:
			break;
		}
	}

	if (is_mercator)
	{
		LatLonToMeters(*minY, *minX, *minX, *minY);
		LatLonToMeters(*maxY, *maxX, *maxX, *maxY);
	}

	return true;
}

std::string ConvertToQtNode(double y, double x, unsigned int level, bool is_mercator)
{
	if (level >= MAX_LEVEL)
		return "";

	double minX= -180.0;
	double maxY = 180.0;
	double maxX = 180.0;
	double minY = -180.0;
	if (is_mercator)
	{
		MetersToLatLon(x, y, y, x);
		/*minX = -MAX_MERCATOR_FLAT_X;
		maxY = MAX_MERCATOR_FLAT_Y;
		maxX = MAX_MERCATOR_FLAT_X;
		minY = -MAX_MERCATOR_FLAT_Y;*/
	}

	if (x<minX || x>maxX || y<minY || y>maxY)
		return "";
	
	std::string name;
	name.resize(level + 1);
	memset(name._Myptr(), '0', level + 1);
	for (unsigned int i = 1U; i <= level; i++)
	{
		if (x >= minX && x < (minX + maxX) / 2.0 && y >= minY && y < (maxY + minY) / 2.0)
		{
			maxY = (maxY + minY) / 2.0;
			maxX = (minX + maxX) / 2.0;
			name[i] = '0';
		}
		else if (x >= (minX + maxX) / 2.0 && x < maxX && y >= minY && y < (maxY + minY) / 2.0)
		{
			maxY = (maxY + minY) / 2.0;
			minX = (minX + maxX) / 2.0;
			name[i] = '1';
		}
		else if (x >= (minX + maxX) / 2.0 && x < maxX && y >= (maxY + minY) / 2.0 && y < maxY)
		{
			minY = (maxY + minY) / 2.0;
			minX = (minX + maxX) / 2.0;
			name[i] = '2';
		}
		else if (x >= minX && x < (minX + maxX) / 2.0 && y >= (maxY + minY) / 2.0 && y < maxY)
		{
			minY = (maxY + minY) / 2.0;
			maxX = (minX + maxX) / 2.0;
			name[i] = '3';
		}
		else
			return "";
	}

	return name;
}

std::vector<std::string> ConvertToQtNode(double minY, double minX, double maxY, double maxX, unsigned int level, bool is_mercator)
{
	unsigned int minx = std::numeric_limits<unsigned int>::max();
	unsigned int miny = std::numeric_limits<unsigned int>::max();
	unsigned int maxx = std::numeric_limits<unsigned int>::min();
	unsigned int maxy = std::numeric_limits<unsigned int>::min();
	unsigned int x, y, z;
	bool xOut = false;
	bool yOut = false;
	std::string name = ConvertToQtNode(minY, minX, level, is_mercator);
	if (!name.empty())
	{
		ConvertFromQtNode(name, &x, &y, &z);
		minx = __min(minx, x);
		maxx = __max(maxx, x);
		miny = __min(miny, y);
		maxy = __max(maxy, y);

		double tmpMinX, tmpMinY, tmpMaxX, tmpMaxY;
		unsigned int tmpLevel;
		QtNodeBounds(name, is_mercator, &tmpMinY, &tmpMinX, &tmpMaxY, &tmpMaxX, &tmpLevel);
		xOut = tmpMaxX < maxX;
		yOut = tmpMaxY < maxY;
	}

	if ( xOut )
	{
		name = ConvertToQtNode(minY, maxX, level, is_mercator);
		if (!name.empty())
		{
			ConvertFromQtNode(name, &x, &y, &z);
			minx = __min(minx, x);
			maxx = __max(maxx, x);
			miny = __min(miny, y);
			maxy = __max(maxy, y);
		}
	}
	
	if ( yOut )
	{
		name = ConvertToQtNode(maxY, minX, level, is_mercator);
		if (!name.empty())
		{
			ConvertFromQtNode(name, &x, &y, &z);
			minx = __min(minx, x);
			maxx = __max(maxx, x);
			miny = __min(miny, y);
			maxy = __max(maxy, y);
		}

		if (xOut)
		{
			name = ConvertToQtNode(maxY, maxX, level, is_mercator);
			if (!name.empty())
			{
				ConvertFromQtNode(name, &x, &y, &z);
				minx = __min(minx, x);
				maxx = __max(maxx, x);
				miny = __min(miny, y);
				maxy = __max(maxy, y);
			}
		}
	}

	std::vector<std::string> names;
	for (x = minx; x <= maxx; x++)
	{
		for (y = miny; y <= maxy; y++)
		{
			name = ConvertToQtNode(x, y, level);
			if (!name.empty())
				names.push_back(name);
		}
	}

	return names;
}

const int TILE_SIZE = 256;
const double originShift = 2 * PI * 6378137.0 / 2.0;
const double initialResolution = 2 * PI * 6378137.0 / (double)TILE_SIZE;
void LatLonToMeters(double lat, double lon, double& mx, double& my)
{
	mx = lon * originShift / 180.0;
	my = log(tan((90.0 + lat) * PI / 360.0)) / (PI / 180.0);
	my = my * originShift / 180.0;
}

void MetersToLatLon(double mx, double my, double& lat, double& lon)
{
	lon = (mx / originShift) * 180.0;
	lat = (my / originShift) * 180.0;
	lat = 180.0 / PI * (2.0 * atan(exp(lat * PI / 180.0)) - PI / 2.0);
}

void PixelsToMeters(double px, double py, int zoom, double& mx, double& my)
{
	double res = initialResolution / pow(2.0, zoom);
	mx = px * res - originShift;
	my = py * res - originShift;
}

void MetersToPixels(double mx, double my, int zoom, double& px, double& py)
{
	double res = initialResolution / pow(2.0, zoom);
	px = (mx + originShift) / res;
	py = (my + originShift) / res;
}

void PixelsToTile(double px, double py, int& tileX, int& tileY)
{
	tileX = ceil(px / (double)TILE_SIZE) - 1;
	tileY = ceil(py / (double)TILE_SIZE) - 1;
}

void MetersToTile(double mx, double my, int zoom, int& tileX, int& tileY)
{
	double px = 0.0;
	double py = 0.0;
	MetersToPixels(mx, my, zoom, px, py);
	PixelsToTile(px, py, tileX, tileY);
}

void GoogleTile(int tx, int ty, int zoom, int& tileX, int& tileY)
{
	tileX = tx;
	tileY = pow(2.0, zoom) - 1 - ty;
}

void LatLon2GoogleTile(double lat, double lon, int zoom, int& tileX, int& tileY)
{
	double mx = 0.0;
	double my = 0.0;
	LatLonToMeters(lat, lon, mx, my);
	MetersToTile(mx, my, zoom, tileX, tileY);
	GoogleTile(tileX, tileY, zoom, tileX, tileY);
}

void TileBounds(int tx, int ty, int zoom, double& minx, double& miny, double& maxx, double& maxy)
{
	PixelsToMeters(tx*(double)TILE_SIZE, ty*(double)TILE_SIZE, zoom, minx, miny);
	PixelsToMeters((tx + 1)*(double)TILE_SIZE, (ty + 1)*(double)TILE_SIZE, zoom, maxx, maxy);
}

void GoogleTileLatLonBounds(int tileX, int tileY, int zoom, double& minlat, double& minlon, double& maxlat, double& maxlon)
{
	double minx, miny, maxx, maxy;
	TileBounds(tileX, tileY, zoom, minx, miny, maxx, maxy);
	MetersToLatLon(minx, miny, minlat, minlon);
	MetersToLatLon(maxx, maxy, maxlat, maxlon);
}
LIBGE_NAMESPACE_END