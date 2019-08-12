#include "data.h"

void SqliteManager::Open(const string& dbPath, const string& dbName)
{
	path pathStr(dbPath);
	string fullPath;
	if (!exists(pathStr)) {

		ERROE_LOG("输入的路径不存在, 数据库创建在当前文件夹");
		fullPath = ".\\" + dbName;
	}
	else {

		fullPath = dbPath + "\\" + dbName;
	}


	if (sqlite3_open(fullPath.c_str(), &_db) == SQLITE_OK) {
		TRACE_LOG("数据库创建成功!!");
	}
	else {
		ERROE_LOG("数据库创建失败!!");
	}

}

void SqliteManager::Close()
{
	if (sqlite3_close(_db) == SQLITE_OK) {
		TRACE_LOG("数据库关闭成功!!");
	}
	else {
		ERROE_LOG("数据库关闭失败!!");
	}
}

void SqliteManager::ExecSql(const string& sql)
{
	char* errMsg;
	if (sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK) {
		TRACE_LOG("执行成功: %s", sql.c_str());
	}
	else {
		ERROE_LOG("执行失败: %s %s", sql.c_str(), errMsg);
	}

}

void SqliteManager::GetTable(const string& sql, int& row, int& col, char**& dbResult)
{
	char* errMsg;
	//int sqlite3_get_table(sqlite3*, const char *sql, char ***resultp, int *nrow, int *ncolumn, char **errmsg );
	if (sqlite3_get_table(_db, sql.c_str(), &dbResult, &row, &col, &errMsg) == SQLITE_OK) {
		TRACE_LOG("执行成功: %s", sql.c_str());
	}
	else {
		ERROE_LOG("执行失败: %s %s", sql.c_str(), errMsg);
	}

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void DataManager::Init()
{
	insertVector.reserve(1000);

	_sqlManager.Open(__DB_PATH__, __DB_NAME__);

	stringstream ssrm;
	ssrm << "create table if not exists " 
		 << __TB_NAME__ 
		 << "(id INTEGER PRIMARY KEY, path text, name text, name_pinyin text, name_initials text);";
	_sqlManager.ExecSql(ssrm.str());

	ssrm.clear();
	ssrm << "CREATE INDEX path_index ON " << __TB_NAME__ << "(path);";
	_sqlManager.ExecSql(ssrm.str());
	//CREATE INDEX salary_index ON COMPANY(salary);
}

void DataManager::GetDbDoc(const string& filePath, set<string>& dbSet)
{
	stringstream ssrm;
	ssrm << "select name from " << __TB_NAME__
		 << " where path = \"" << filePath << "\";";

	int row = 0, col = 0;
	char** ppRes;

	std::unique_lock<std::mutex> ulc(_mutex);
	AutoGetTable ag(_sqlManager, ssrm.str(), row, col, ppRes);
	ulc.unlock();

	//printf("查到%d条记录\n", row);
	
	int index = col;
	for (int i = 0; i < row; i++) {
		//printf("%s\n", dbResult[i]);
		for (int j = 0; j < col; j++) {
			//printf(" %s  --->  %s\n", ppRes[j], ppRes[index]);
			dbSet.insert(ppRes[index]);
			index++;
		
		}
	}
}



void DataManager::InsertDbDoc(const string& filePath, const string& fileName)
{
	
	
	insertVector.push_back(std::make_pair(filePath, fileName));
	//std::lock_guard<std::mutex> lock(_mutex);
	

	if (insertVector.size() >= 1000) {
		string sql;
		sql += "begin;\n";
		for (const auto& e : insertVector) {
			string pinyin = ChineseConvertPinYinAllSpell(e.second);
			string initials = ChineseConvertPinYinInitials(e.second);
			stringstream ssrm;
			ssrm << "insert into " << __TB_NAME__
				<< " (path, name, name_pinyin, name_initials) values"
				<< "(\"" << e.first << "\", \"" << e.second << "\", \""
				<< pinyin << "\", \"" << initials << "\"" << ");";
			//_sqlManager.ExecSql(ssrm.str());
			sql += ssrm.str();
		}
		sql += "commit;\n";
		_sqlManager.ExecSql(sql);
		insertVector.clear();
	}
}

void DataManager::DeleteDbDoc(const string& filePath, const string& fileName)
{
	stringstream ssrm;
	ssrm << "delete from " << __TB_NAME__ 
		 << " where path = \"" << filePath 
		 << "\" and name = \"" << fileName << "\";";
	_sqlManager.ExecSql(ssrm.str());


	string _path = filePath;
	_path += "\\";
	_path += fileName;
	ssrm.clear();
	ssrm << "delete from " << __TB_NAME__
		 << " where path like '" << _path << "';";
	_sqlManager.ExecSql(ssrm.str());
}



// 模糊搜索 --> 返回 path-fileName 键值对
void DataManager::Search(const string& key, vector<pair<string, string>>& paths)
{
	//if (isEnglish(key)) {


		stringstream ssrm;
		string initials = ChineseConvertPinYinInitials(key);
		string pinyin = ChineseConvertPinYinAllSpell(key);
		ssrm << "select path, name from " << __TB_NAME__
			<< " where name_pinyin like \"%" << pinyin << "%\"or name_initials like \"%" << initials << "%\";";

		int row = 0, col = 0;
		char** ppRes;
		_sqlManager.ExecSql("PRAGMA case_sensitive_like=ON");

		std::unique_lock<std::mutex> ulc(_mutex);
		AutoGetTable ag(_sqlManager, ssrm.str(), row, col, ppRes);
		ulc.unlock();

		printf("查到%d条记录\n", row);
		int index = col;
		for (int i = 0; i < row; i++) {

			for (int j = 0; j < col; j += col) {
				//printf(" %s  --->  %s\n", ppRes[j], ppRes[index]);
				paths.push_back(std::make_pair(ppRes[index], ppRes[index + 1]));
				index += col;

			}
		}
	/*}
	else {
		
	}*/
}

void DataManager::SplitHighLight(const string& key, const string& str, string& prefix
	, string& highStr, string& suffix) {

	// 1. 直接匹配
	size_t pos = str.find(key);
	if (pos != string::npos) {
		prefix = str.substr(0, pos);
		highStr = key;
		suffix = str.substr(pos + key.size(), string::npos);
		return;
	}

	//2. 拼音匹配
	string pinyin = ChineseConvertPinYinAllSpell(str);
	pos = pinyin.find(key);
	if (pos != string::npos) {
		size_t strStart = 0;
		size_t strEnd = 0;
		size_t pyStart = 0;
		size_t keyStart = 0;

		char chinese[3] = { 0 };

		while (pyStart < pos) {
			//非汉字
			if (str[strStart] > 0 && str[strStart] < 128) {
				pyStart++;
				strStart++;
				continue;
			}
			chinese[0] = str[strStart];
			chinese[1] = str[strStart + 1];
			string charPinyin = ChineseConvertPinYinAllSpell(chinese);
			//中文特殊字符
			if (charPinyin.size() == 0) {
				strStart += 2;
				continue;
			}

			pyStart += charPinyin.size();
			strStart += 2;
		}
		prefix = str.substr(0, strStart);

		strEnd = strStart;
		while (keyStart < key.size()) {
			//非汉字
			if (str[strEnd] > 0 && str[strEnd] < 128) {
				keyStart++;
				strEnd++;
				continue;
			}
			chinese[0] = str[strEnd];
			chinese[1] = str[strEnd + 1];
			string charPinyin = ChineseConvertPinYinAllSpell(chinese);
			//中文特殊字符
			if (charPinyin.size() == 0) {
				strEnd += 2;
				continue;
			}
			keyStart += charPinyin.size();
			strEnd += 2;
		}
		highStr = str.substr(strStart, strEnd - strStart);
		suffix = str.substr(strEnd, string::npos);
		return;
	}


	// 3. 首字母匹配
	string initials = ChineseConvertPinYinInitials(str);
	pos = initials.find(key);

	size_t strStart = 0;
	size_t strEnd = 0;
	size_t inStart = 0;
	size_t keyStart = 0;

	if (pos == string::npos) return;

	while (inStart < pos) {
		//非汉字
		if (str[strStart] > 0 && str[strStart] < 128) {
			inStart++;
			strStart++;
			continue;
		}
		inStart += 1;
		strStart += 2;
	}
	prefix = str.substr(0, strStart);

	strEnd = strStart;
	while (keyStart < key.size()) {
		//非汉字
		if (str[strEnd] > 0 && str[strEnd] < 128) {
			keyStart++;
			strEnd++;
			continue;
		}
		keyStart += 1;
		strEnd += 2;
	}
	highStr = str.substr(strStart, strEnd - strStart);
	suffix = str.substr(strEnd, string::npos);
	return;
}

void DataManager::ClearInsertVector()
{
	//std::unique_lock<std::mutex> ulc(_mutex);
	if (!insertVector.empty()) {
		string sql;
		sql += "begin;\n";
		for (const auto& e : insertVector) {
			string pinyin = ChineseConvertPinYinAllSpell(e.second);
			string initials = ChineseConvertPinYinInitials(e.second);
			stringstream ssrm;
			ssrm << "insert into " << __TB_NAME__
				<< " (path, name, name_pinyin, name_initials) values"
				<< "(\"" << e.first << "\", \"" << e.second << "\", \""
				<< pinyin << "\", \"" << initials << "\"" << ");";
			//_sqlManager.ExecSql(ssrm.str());
			sql += ssrm.str();
		}
		sql += "commit;\n";
		_sqlManager.ExecSql(sql);
		insertVector.clear();
	}
}

