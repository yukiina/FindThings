#pragma once 
#include "common.h"

// 数据模块  

// 封装sqlite常用操作
class SqliteManager {
public:
	SqliteManager() : _db(nullptr) {}
	~SqliteManager() {
		Close();
	}
	void Open(const string& dbPath, const string& dbName);
	void Close();
	void ExecSql(const string& sql);
	void GetTable(const string& sql, int& row, int& col, char**& result);

	SqliteManager(const SqliteManager&) = delete;
	SqliteManager& operator=(const SqliteManager&) = delete;
private:
	sqlite3* _db;
};

//RAII 自动释放 GetTable中申请的内存 char** result
class AutoGetTable {
public:
	AutoGetTable(SqliteManager& dbObject, const string& sql, int& row, int& col, char**& result)
		: _ppObject(nullptr)
	{
		dbObject.GetTable(sql, row, col, result);
		_ppObject = result;
	}

	~AutoGetTable() {
		if (_ppObject) {
			TRACE_LOG("GetTable资源释放成功");
			sqlite3_free_table(_ppObject);
		}
	}
	AutoGetTable(const AutoGetTable&) = delete;
	AutoGetTable& operator=(const AutoGetTable&) = delete;
private:
	
	char** _ppObject;
};



// 数据管理模块
class DataManager {
public:
	static DataManager* GetInstance() 
	{
		static DataManager dm;
		return &dm;
	}
	void Init();
	void GetDbDoc(const string& filePath, set<string>& dbSet);
	void InsertDbDoc(const string& filePath, const string& fileName);
	void DeleteDbDoc(const string& filePath, const string& fileName);
	void Search(const string& key, vector<std::pair<string, string>>& paths);
	void SplitHighLight(const string& key, const string& str, string& prefix
		, string& highStr, string& suffix);
	void ClearInsertVector();
private:
	DataManager() { Init(); }
	DataManager(const DataManager&) = delete;
	DataManager& operator=(const DataManager&) = delete;

	std::mutex _mutex;

	SqliteManager _sqlManager;
	vector<pair<string, string>> delVector;
	vector<pair<string, string>> insertVector;
};
