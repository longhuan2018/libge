// libge.cpp : 定义 DLL 应用程序的导出函数。
//
#ifndef _LIBGE_H
#define _LIBGE_H

#include "export.h"
#include <assert.h>
#include <string>
#include <vector>
#include "QuadTreePacket.h"
#include "Terrain.h"
#include "CacheManager.h"

LIBGE_NAMESPACE_BEGINE
class LIBGE_API CLibGEHelper
{
public:
	typedef enum
	{
		FALTFILE_IMAGE = 0,
		FALTFILE_HISTORY_IMAGE,
		FALTFILE_TERRAIN,
		FALTFILE_Q2TREE,
		FALTFILE_QPTREE,
		FALTFILE_LAYER,
		FALTFILE_LAYER_3D,
		FALTFILE_TEXTURE,
		FALTFILE_UNKNOWN = 0xFF
	}EFaltfileType;
public:
	CLibGEHelper();
	virtual ~CLibGEHelper();

public:
	std::string cachePath() { return _cachePath; }
	void cachePath(const std::string& path) { _cachePath = path; }

public:
	int  getDBRoot();
	std::string getImage(double minX, double minY, double maxX, double maxY, unsigned int level, unsigned int rasterXSize, unsigned int rasterYSize, bool is_mercator = false);
	std::string getImage(unsigned int x, unsigned int y, unsigned level, int version, bool is_mercator = false);
	std::string getImage(const char* name, int version, bool is_mercator = false);
	std::string getTerrain(unsigned int x, unsigned int y, unsigned level, int version, int* pCols = nullptr, int* pRows = nullptr, bool is_mercator = false);
	std::string getTerrain(const char* name, int version, int* pCols = nullptr, int* pRows = nullptr, bool is_mercator = false);
	std::string getTerrain(double minX, double minY, double maxX, double maxY, unsigned int level, unsigned int rasterXSize, unsigned int rasterYSize, bool is_mercator = false);

protected:	
	bool geauth();
	QuadTreePacket16* getQuadtree(unsigned int x, unsigned int y, unsigned level, int version);
	QuadTreePacket16* getQuadtree(const char* name, int version);
	std::string randomServerURL();
	std::string randomGEAuth();
	int getVersion(const char* name, CacheManager::ETableType type);
	std::string getGEName(const std::string& strUrl);
	EFaltfileType faltFileType(const std::string& strUrl);	
	int Get(const std::string & strUrl, std::string & strResponse, bool useSession);
	int Post(const std::string & strUrl, const char* szPost, int postSize, std::string & strResponse, bool useSession);
	std::string getFlatfile(const std::string& url, const std::string& key, CacheManager::ETableType type);

	std::string UnPackGEZlib(char* srcData, unsigned long srcSize);
	bool  DecodeImage(const char* srcData, unsigned long srcSize, const char* filePath);
	Terrain* DecodeTerrain(const char* name, const char* srcData, unsigned long srcSize);
	QuadTreePacket16* DecodeQuadtree(const char* srcData, unsigned long srcSize);

public:
	static void Initialize();
	static void UnInitialize();

private:
	std::string _cachePath;
	std::string _baseURL;
	std::string _hl;
	std::string _gl;
	char _crypt_key[1024];
	std::vector<std::string> _SessionIds;
	std::vector<std::string> _serverURLs;
	std::vector<std::string> _geAuths;
	unsigned short _version;

	CacheManager _cacheManager;
};
LIBGE_NAMESPACE_END
#endif //_LIBGE_H

