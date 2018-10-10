#include "stdafx.h"
#include "CacheManager.h"
#include <sstream>

LIBGE_NAMESPACE_BEGINE
CacheManager CacheManager::_instance;
CacheManager::CacheManager()
{
	m_pSqlite = NULL;
	m_pQuadtreeInsertStmt = NULL;
	m_pRuntimeInsertStmt = NULL;
	m_pTileSelectStmt = NULL;
	m_hRuntimeMutex = CreateMutexA(NULL, FALSE, "RUNTIME_MATEX");
	m_hSelectMutex = CreateMutexA(NULL, FALSE, "RECORD_SELECT_TILE_MUTEX");
}

CacheManager::~CacheManager()
{
	Close();
	if (m_hSelectMutex != NULL)
		CloseHandle(m_hSelectMutex);
	m_hSelectMutex = NULL;

	if (m_hRuntimeMutex != NULL)
		CloseHandle(m_hRuntimeMutex);
	m_hRuntimeMutex = NULL;
}

sqlite3* CacheManager::_open(const char* lpszFile)
{
	if (lpszFile == NULL)
		return NULL;

	sqlite3* pSqlite = NULL;
	int rc = sqlite3_open(lpszFile, &pSqlite);
	if (rc != SQLITE_OK)
		return NULL;

	char* pError = NULL;
	rc = sqlite3_exec(pSqlite, "SELECT * FROM DBRoot5"/*"SELECT * FROM sqlite_master WHERE type='table' AND name='DBRoot5'"*/, NULL, pSqlite, &pError);
	if (rc != SQLITE_OK)
	{
		char* errMsg = NULL;
		rc = sqlite3_exec(pSqlite, "CREATE TABLE DBRoot5(id INTEGER PRIMARY KEY AUTOINCREMENT, version int)", NULL, NULL, &errMsg);
		if (rc != SQLITE_OK)
		{
			sqlite3_close(pSqlite);
			return NULL;
		}

		{
			rc = sqlite3_exec(pSqlite, "CREATE TABLE TProvider(id INTEGER PRIMARY KEY AUTOINCREMENT, provider int, name TEXT, type int)", NULL, NULL, &errMsg);
			if (rc != SQLITE_OK)
			{
				sqlite3_close(pSqlite);
				return NULL;
			}

			rc = sqlite3_exec(pSqlite, "CREATE INDEX  TProvider_idx2 ON TProvider(provider ASC, type ASC)", NULL, NULL, &errMsg);
			if (rc != SQLITE_OK)
			{
				sqlite3_close(pSqlite);
				return FALSE;
			}
		}
		
		for (int i = TYPE_IMAGE; i < TYPE_UNKNOWN; i++)
		{
			std::string tableName = faltFileTableName((ETableType)i);
			std::stringstream ssSQL;
			ssSQL << "CREATE TABLE " << tableName << "(id INTEGER PRIMARY KEY AUTOINCREMENT, url TEXT, data BLOB)";
			rc = sqlite3_exec(pSqlite, ssSQL.str().c_str(), NULL, NULL, &errMsg);
			if (rc != SQLITE_OK)
			{
				sqlite3_close(pSqlite);
				return FALSE;
			}

			ssSQL.str("");
			ssSQL.clear();
			ssSQL << "CREATE INDEX " << tableName << "_idx2 ON " << tableName << "(url ASC)";
			rc = sqlite3_exec(pSqlite, ssSQL.str().c_str(), NULL, NULL, &errMsg);
			if (rc != SQLITE_OK)
			{
				sqlite3_close(pSqlite);
				return FALSE;
			}
		}

		for (int i = TYPE_IMAGE; i < TYPE_UNKNOWN; i++)
		{
			std::string tableName = versionTableName((ETableType)i);
			std::stringstream ssSQL;
			ssSQL << "CREATE TABLE " << tableName << "(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, x int, y int, level int, version int, channel int, provider int, tile_date TEXT)";
			rc = sqlite3_exec(pSqlite, ssSQL.str().c_str(), NULL, NULL, &errMsg);
			if (rc != SQLITE_OK)
			{
				sqlite3_close(pSqlite);
				return FALSE;
			}

			ssSQL.str("");
			ssSQL.clear();
			ssSQL << "CREATE INDEX " << tableName << "_idx2 ON " << tableName << "(name ASC, x ASC, y ASC, level ASC, version ASC, channel ASC, provider ASC, tile_date ASC)";
			rc = sqlite3_exec(pSqlite, ssSQL.str().c_str(), NULL, NULL, &errMsg);
			if (rc != SQLITE_OK)
			{
				sqlite3_close(pSqlite);
				return FALSE;
			}
		}
	}

	return pSqlite;
}

bool CacheManager::Open(const char* lpszFile)
{
	Close();
	m_pSqlite = _open(lpszFile);
	return m_pSqlite != NULL;
}

void CacheManager::Close()
{
	if (m_pQuadtreeInsertStmt != NULL)
		sqlite3_finalize(m_pQuadtreeInsertStmt);
	m_pQuadtreeInsertStmt = NULL;

	if (m_pRuntimeInsertStmt != NULL)
		sqlite3_finalize(m_pRuntimeInsertStmt);
	m_pRuntimeInsertStmt = NULL;

	if (m_pTileSelectStmt != NULL)
		sqlite3_finalize(m_pTileSelectStmt);
	m_pTileSelectStmt = NULL;

	if (m_pSqlite != NULL)
		sqlite3_close(m_pSqlite);
	m_pSqlite = NULL;
}

// 事务处理
// 开始事务
bool CacheManager::BeginTransaction()
{
	if (m_pSqlite == NULL)
		return FALSE;

	return ExecNoQuery("BEGIN", NULL);
}

// 提交事务
bool CacheManager::Commit()
{
	if (m_pSqlite == NULL)
		return FALSE;

	return ExecNoQuery("COMMIT", NULL);
}

// 回滚事务
bool CacheManager::Rollback()
{
	if (m_pSqlite == NULL)
		return FALSE;

	return ExecNoQuery("rollback", NULL);
}

// 执行SQL语句
bool CacheManager::SQLExec(const char* lpszSQL, sqlite3_callback xCallback, void *pArg, char **pzErrMsg)
{
	if (m_pSqlite == NULL || lpszSQL == NULL)
		return FALSE;

	int rc = sqlite3_exec(m_pSqlite, lpszSQL, xCallback, pArg, pzErrMsg);
	return rc == SQLITE_OK;
}

sqlite3_stmt* CacheManager::SQLExec(const char* lpszSQL, char **pzErrMsg)
{
	if (m_pSqlite == NULL || lpszSQL == NULL)
		return nullptr;

	LPCSTR lpSQL = lpszSQL;
	sqlite3_stmt* stmt = NULL;
	if (sqlite3_prepare_v2(m_pSqlite, lpSQL, strlen(lpSQL), &stmt, NULL) != SQLITE_OK)
	{
		if (stmt)
			sqlite3_finalize(stmt);
		return nullptr;
	}

	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return nullptr;
	}
	return stmt;
}

// 非查询执行语句
bool CacheManager::ExecNoQuery(const char* lpszSQL, char **pzErrMsg)
{
	if (m_pSqlite == NULL || lpszSQL == NULL)
		return FALSE;

	LPCSTR lpSQL = lpszSQL;
	sqlite3_stmt* stmt = NULL;
	if (sqlite3_prepare_v2(m_pSqlite, lpSQL, strlen(lpSQL), &stmt, NULL) != SQLITE_OK)
	{
		if (stmt)
			sqlite3_finalize(stmt);
		return FALSE;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		sqlite3_finalize(stmt);
		return FALSE;
	}

	sqlite3_finalize(stmt);
	return TRUE;
}

std::string CacheManager::versionTableName(ETableType type)
{
	switch (type)
	{
	case TYPE_IMAGE:
		return "TImageVersion";

	case TYPE_TERRAIN:
		return "TTerrainVersion";

	case TYPE_QUADTREE:
		return "TQuadtreeVersion";
			
	case TYPE_VECTOR:
		return "TVectorVersion";

	default:
		break;
	}

	return "";
}

std::string CacheManager::faltFileTableName(ETableType type)
{
	switch (type)
	{
	case TYPE_IMAGE:
		return "TImageFaltfile";

	case TYPE_TERRAIN:
		return "TTerrainFaltfile";

	case TYPE_QUADTREE:
		return "TQuadtreeFaltfile";

	case TYPE_VECTOR:
		return "TVectorFaltfile";

	default:
		break;
	}

	return "";
}

bool CacheManager::AddProvider(unsigned int provider, const std::string& name, unsigned int type)
{
	if (m_pSqlite == NULL)
		return false;

	{
		std::stringstream ssSQL;
		ssSQL << "SELECT name FROM TProvider WHERE provider=" << provider;
		sqlite3_stmt* stmt = SQLExec(ssSQL.str().c_str(), nullptr);
		if (stmt != nullptr)
		{
			const char* text = (const char*)sqlite3_column_text(stmt, 0);
			sqlite3_finalize(stmt);
			if ( (text==nullptr && name.empty()) || (text!=nullptr && !name.empty() && _stricmp(text, name.c_str())==0) )
				return true;

			ssSQL.str("");
			ssSQL.clear();
			ssSQL << "UPDATE TProvider SET type = " << type << ", name = '" << name << "' WHERE provider=" << provider;
			return ExecNoQuery(ssSQL.str().c_str(), nullptr);
		}
	}

	{
		std::stringstream ssSQL;
		ssSQL << "Insert into TProvider(name, provider, type) values('" << name << "', " << provider << ", " << type << ")";
		return ExecNoQuery(ssSQL.str().c_str(), nullptr);
	}
}

bool CacheManager::AddVersion(const std::string& name, unsigned int x, unsigned int y, unsigned int level, unsigned int version, unsigned int channel, unsigned int provider, const std::string& tileDate, ETableType type)
{
	if (m_pSqlite == NULL)
		return false;

	std::string tableName = versionTableName(type);
	if (tableName.empty())
		return false;

	{
		std::stringstream ssSQL;
		ssSQL << "SELECT version FROM " << tableName << " WHERE name='" << name << "'";
		sqlite3_stmt* stmt = SQLExec(ssSQL.str().c_str(), nullptr);
		if (stmt != nullptr)
		{
			int v = sqlite3_column_int(stmt, 0);
			sqlite3_finalize(stmt);
			if (v == version)
				return true;

			ssSQL.str("");
			ssSQL.clear();
			ssSQL << "UPDATE " << tableName << " SET version = " << version << ", tile_date = '" << tileDate << "' WHERE name='" << name << "'";
			return ExecNoQuery(ssSQL.str().c_str(), nullptr);
		}
	}

	{		
		std::stringstream ssSQL;
		ssSQL << "Insert into " << tableName << "(name, x, y, level, version, channel, provider, tile_date) values('" << name << "', " << x << ", " << y << ", " << level << ", " << version << ", " << channel << ", " << provider << ", '" << tileDate << "')";
		return ExecNoQuery(ssSQL.str().c_str(), nullptr);
	}
}

unsigned int CacheManager::GetVersion(const std::string& name, ETableType type)
{
	if (m_pSqlite == NULL)
		return 0U;

	std::string tableName = versionTableName(type);
	if (tableName.empty())
		return 0U;

	std::stringstream ssSQL;
	ssSQL << "SELECT version FROM " << tableName << " WHERE name='" << name << "'";
	sqlite3_stmt* stmt = SQLExec(ssSQL.str().c_str(), nullptr);
	if (stmt != nullptr)
	{
		int v = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return v;
	}

	return 0U;
}

unsigned int CacheManager::GetVersion(unsigned int x, unsigned int y, unsigned int level, ETableType type)
{
	if (m_pSqlite == NULL)
		return 0U;

	std::string tableName = versionTableName(type);
	if (tableName.empty())
		return 0U;

	std::stringstream ssSQL;
	ssSQL << "SELECT version FROM " << tableName << " WHERE x=" << x << " AND y=" << y << " AND level=" << level;
	sqlite3_stmt* stmt = SQLExec(ssSQL.str().c_str(), nullptr);
	if (stmt != nullptr)
	{
		int v = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return v;
	}

	return 0U;
}

bool CacheManager::AddFaltfile(const std::string& url, const std::string data, ETableType type)
{
	if (m_pSqlite == NULL)
		return false;

	std::string tableName = faltFileTableName(type);
	if (tableName.empty())
		return false;

	{
		std::stringstream ssSQL;
		ssSQL << "SELECT url FROM " << tableName << " WHERE url='" << url << "'";
		sqlite3_stmt* stmt = SQLExec(ssSQL.str().c_str(), nullptr);
		if (stmt != nullptr)
		{
			sqlite3_finalize(stmt);
			return true;
		}
	}

	{
		sqlite3_stmt* pStmt = nullptr;
		std::stringstream ssSQL;
		ssSQL << "Insert into " << tableName << "(url, data) values(?, ?)";
		if (sqlite3_prepare_v2(m_pSqlite, ssSQL.str().c_str(), strlen(ssSQL.str().c_str()), &pStmt, NULL) != SQLITE_OK)
		{
			if (pStmt!=nullptr)
				sqlite3_finalize(pStmt);
			pStmt = NULL;
			return false;
		}

		sqlite3_bind_text(pStmt, 1, url.c_str(), url.size(), SQLITE_STATIC);
		sqlite3_bind_blob(pStmt, 2, data.c_str(), data.size(), SQLITE_STATIC);

		bool result = (sqlite3_step(pStmt) == SQLITE_DONE);
		sqlite3_finalize(pStmt);
		return result;
	}
}

std::string CacheManager::GetFaltfile(const std::string& url, ETableType type)
{
	std::string data;
	if (m_pSqlite == NULL)
		return data;

	std::string tableName = faltFileTableName(type);
	if (tableName.empty())
		return data;

	std::stringstream ssSQL;
	ssSQL << "SELECT data FROM " << tableName << " WHERE url='" << url << "'";
	sqlite3_stmt* stmt = SQLExec(ssSQL.str().c_str(), nullptr);
	if (stmt != nullptr)
	{
		int size = sqlite3_column_bytes(stmt, 0);
		if (size > 0)
		{
			data.resize(size);
			memcpy(data._Myptr(), sqlite3_column_blob(stmt, 0), size);
		}
		sqlite3_finalize(stmt);
	}

	return data;
}

LIBGE_NAMESPACE_END