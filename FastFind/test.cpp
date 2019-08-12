#include "common.h"
#include "scan.h"
#include "data.h"


// 测试 
void TestDirList() {
	vector<string> files;
	vector<string> dirs;
	DirList("C:\\Users\\lenovo\\Desktop\\C++", files, dirs);

	cout << "文件: " << endl;
	for (const auto& file : files) {
		cout << file << endl;
	}
	cout << endl;

	cout << "目录: " << endl;
	for (const auto& dir : dirs) {
		cout << dir << endl;
	}
	cout << endl;
}
void TestScanDir() {
	//ScanManager sa;
	//sa.ScanDir("E:\\");
}
void TestDb() {
	SqliteManager sm;
	sm.Open(".\\", "test.db");
	string sql = "create table tb_files(id INTEGER PRIMARY KEY AUTOINCREMENT" \
		", file_path text, file_name text)";

	int row = 0, col = 0;
	sql = "select* from tb_files";
	
	char** dbResult;
	AutoGetTable ag(sm, sql, row, col, dbResult);

	printf("查到%d条记录\n", row);
	int index = col;
	for (int i = 0; i < row; i++) {
		//printf("%s\n", dbResult[i]);
		for (int j = 0; j < col; j++) {
			printf(" %s  --->  %s\n", dbResult[j], dbResult[index]);
			index++;
		}
		printf("----------------------------------------------------------\n");
	}

}
void TestScan() {
	//ScanManager scan;
	//scan.ScanDir("D:\\");
}

void TestSerach() {
	///clock_t start, end;
	//start = clock();
	ScanManager* scan = ScanManager::CreateInstance();
	//scan->ScanDir("D:\\");
	//end = clock();
	//printf("Use Time:%f\n", ((double)(end - start) / CLOCKS_PER_SEC));

	string key;
	vector<pair<string, string>> paths;
	cout << "输入搜索的关键字: ";
	while (cin >> key) {
		paths.clear();
		DataManager::GetInstance()->Search(key, paths);
		cout << "-----------------------------------------------------------------------------------" << endl;
		printf("%-50s %-50s\n", "文件名", "路径");
		for (const auto& e : paths) {
			//cout << "path: " << e.first << " ---> " << "fileName: " << e.second << endl;
			//printf("%-50s %-50s\n", e.second.c_str(), e.first.c_str());
			string prefix, high, suffix;
			DataManager::GetInstance()->SplitHighLight(key, e.second, prefix, high, suffix);
			if (prefix.empty() && high.empty() && suffix.empty()) 
				continue;
			cout << prefix;
			ColorPrintf(&high[0]);
			cout << suffix;

			for (size_t i = e.second.size(); i < 50; i++)
			{
				cout << " ";
			}
			printf("%-50s\n", e.first.c_str());
			
		}

		
		cout << "-----------------------------------------------------------------------------------" << endl;
		cout << "输入搜索的关键字: ";
	}
}

//void TestColor() {
//	
//
//	{
//		string key = "xagy";
//		string str = "西安工业大学";
//		
//		string prefix, high, suffix;
//		SplitHighLight(key, str, prefix, high, suffix);
//			
//		cout << prefix;
//		ColorPrintf(&high[0]);
//		cout << suffix << endl;
//	}
//

int main() {
	
	TestSerach();
	system("pause");
	return 0;
}