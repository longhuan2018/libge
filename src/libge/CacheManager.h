#pragma once
#include "export.h"
#include "sqlite3.h"
#include <string>

LIBGE_NAMESPACE_BEGINE
class LIBGE_API CacheManager
{
public:
	typedef enum 
	{
		TYPE_IMAGE = 0,
		TYPE_TERRAIN,
		TYPE_QUADTREE,
		TYPE_VECTOR,
		TYPE_UNKNOWN
	}ETableType;

public:
	typedef int(*sqlite3_callback)(void*, int, char**, char**);

public:
	CacheManager();
	~CacheManager();

public:
	static CacheManager& GetInstance() { return _instance; }

public:
	sqlite3* GetSqlite() { return m_pSqlite; }
	bool Open(const char* lpszFile);
	void Close();

	// 事务处理
	// 开始事务处理
	bool BeginTransaction();

	// 提交事务
	bool Commit();

	// 回滚事务
	bool Rollback();

	// 执行SQL语句
	bool SQLExec(const char* lpszSQL, sqlite3_callback xCallback, void *pArg, char **pzErrMsg);
	sqlite3_stmt* SQLExec(const char* lpszSQL, char **pzErrMsg);

	// 非查询执行语句
	bool ExecNoQuery(const char* lpszSQL, char **pzErrMsg);

	bool AddProvider(unsigned int provider, const std::string& name, unsigned int type);
	bool AddVersion(const std::string& name, unsigned int x, unsigned int y, unsigned int level, unsigned int version, unsigned int channel, unsigned int provider, const std::string& tileDate, ETableType type);
	unsigned int GetVersion(const std::string& name, ETableType type);
	unsigned int GetVersion(unsigned int x, unsigned int y, unsigned int level, ETableType type);

	bool AddFaltfile(const std::string& url, const std::string data, ETableType type);
	std::string GetFaltfile(const std::string& url, ETableType type);

protected:
	sqlite3* _open(const char* lpszFile);
	std::string versionTableName(ETableType type);
	std::string faltFileTableName(ETableType type);

protected:
	sqlite3* m_pSqlite;

private:
	sqlite3_stmt* m_pQuadtreeInsertStmt;

	// runtime
	sqlite3_stmt* m_pRuntimeInsertStmt;
	void* m_hRuntimeMutex;

	// 查询Tile
	sqlite3_stmt* m_pTileSelectStmt;
	void* m_hSelectMutex;

private:
	static CacheManager _instance;
};
LIBGE_NAMESPACE_END
