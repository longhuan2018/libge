#pragma once
#include "export.h"
#include <assert.h>
#include <string>



#include <string>
#include <vector>

LIBGE_NAMESPACE_BEGINE

const double PI = 3.14159265358979;
const unsigned int MAX_LEVEL = 24;
const double MAX_MERCATOR_LONGITUDE = 180.0;
const double MAX_MERCATOR_LATITUDE  = 85.051128779806589;
const double MAX_MERCATOR_FLAT_X = 20037508.342789244;
const double MAX_MERCATOR_FLAT_Y = MAX_MERCATOR_FLAT_X;

const double khEarthCircumference = 40075160.00;
const double khEarthCircumferencePerDegree = khEarthCircumference / 360.00;

// This value is defined because GDAL gives 40075016.6855784 / 2.0 for 180 deg
// longitude for Mercator_1SP projection.
//const double khEarthCircumference = 40075016.6855784;
//const double khEarthCircumferencePerDegree = khEarthCircumference / 360.00;

/**
* Fills in all of the mercator addresses that correspond to nodes that touch
* the lat/lng bounding box of the node specifed by the given
* plate carree address.
*/
void LIBGE_API ConvertFlatToMercatorQtAddresses(std::string flat_qtaddress, std::vector<std::string>* mercator_qtaddresses);

/**
* Returns the y position on a grid at given depth corresponding
* to the given latitude.
*/
unsigned int LIBGE_API LatToYPos(double lat, unsigned int z, bool is_mercator);
double LIBGE_API YPosToLat(unsigned int y, unsigned int z, bool is_mercator);


/**
* Returns the normalized, linear y associated with the given latitude.
* Y is in the range of (-pi, pi) for latitudes in the
* range of (-MAX_MERCATOR_LATITUDE, MAX_MERCATOR_LATITUDE).
*/
double LIBGE_API MercatorLatToY(double lat);
double LIBGE_API MercatorLngToX(double lng);

/**
* Returns the latitude associated with the given normalized, linear y.
* Y is in the range of (-pi, pi) for latitudes in the
* range of (-MAX_MERCATOR_LATITUDE, MAX_MERCATOR_LATITUDE).
*/
double LIBGE_API MercatorYToLat(double y);
double LIBGE_API MercatorXToLng(double x);

/**
* Returns the position on a Mercator grid at given depth (z) corresponding
* to the normalized, linear y value.
* Y is in the range (-pi, pi) and return value is in
* the range [0, 2^z-1].
*/
unsigned int LIBGE_API YToYPos(double y, unsigned int z);

/**
* Returns the latitude that will appear half way between the two given
* latitudes on a Mercator map.
*/
double LIBGE_API BisectLatitudes(double south, double north, bool is_mercator);

/**
* Helper for converting from map space to qtnode address.
* @param x Requested column for map tile.
* @param y Requested row for map tile.
* @param z Requested zoom level for map.
*/
std::string LIBGE_API ConvertToQtNode(unsigned int x, unsigned int y, unsigned int z);

/**
* Helper for converting to map space from a qtnode address.
* @param qtnode Quadtree address to be converted to map coordinates.
* @param x Column for map tile.
* @param y Row for map tile.
* @param z Zoom level for map.
*/
void LIBGE_API ConvertFromQtNode(const std::string& qtnode, unsigned int* x, unsigned int* y, unsigned int* z);

bool LIBGE_API QtNodeBounds(const std::string& name, bool is_mercator, double* minY, double* minX, double* maxY, double* maxX, unsigned int* level);
std::string LIBGE_API ConvertToQtNode(double y, double x, unsigned int level, bool is_mercator);
std::vector<std::string> LIBGE_API ConvertToQtNode(double minY, double minX, double maxY, double maxX, unsigned int level, bool is_mercator);

void LIBGE_API LatLonToMeters(double lat, double lon, double& mx, double& my);
void LIBGE_API MetersToLatLon(double mx, double my, double& lat, double& lon);
void LIBGE_API LatLon2GoogleTile(double lat, double lon, int zoom, int& tileX, int& tileY);
void LIBGE_API GoogleTileLatLonBounds(int tileX, int tileY, int zoom, double& minlat, double& minlon, double& maxlat, double& maxlon);
LIBGE_NAMESPACE_END
