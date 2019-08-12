#include "common.h"
#include "scan.h"


// 递归扫描path中所有文件, 和数据库文件比对, 更新数据库
void ScanManager::ScanDir(const std::string& path) {
	vector<string> files;
	vector<string> dirs;
	DirList(path, files, dirs);

	// 把文件和目录加入set(排序)
	set<string> localFiles; // 本地文件集合
	localFiles.insert(files.begin(), files.end());
	localFiles.insert(dirs.begin(), dirs.end());

	set<string> dbFiles; // 数据库文件集合
	DataManager::GetInstance()->GetDbDoc(path, dbFiles);

	// 开始比对, 本地有数据库没有添加, 反之删除
	set<string>::iterator localIt = localFiles.begin();
	set<string>::iterator dbIt = dbFiles.begin();

	while (localIt != localFiles.end() && dbIt != dbFiles.end()) {
		if (*localIt < *dbIt) { //说明本地有, 数据库没有, 向数据库添加
			//TODO: 数据库添加
			DataManager::GetInstance()->InsertDbDoc(path, *localIt);
			localIt++;
		}
		else if (*localIt > *dbIt) { //数据库有, 本地没有, 从数据库删除 
			//TODO: 数据库删除
			DataManager::GetInstance()->DeleteDbDoc(path, *dbIt);
			dbIt++;
		}
		else {
			localIt++;
			dbIt++;
		}
	}

	while (localIt != localFiles.end()) {
		//TODO: 数据库添加
		DataManager::GetInstance()->InsertDbDoc(path, *localIt);
		localIt++;
	}

	while (dbIt != dbFiles.end()) {
		//TODO: 数据库删除
		DataManager::GetInstance()->DeleteDbDoc(path, *dbIt);
		dbIt++;
	}

	//递归子目录
	for (const auto& dir : dirs) {
		//ThreadPool::GetInstance()->AddTask(std::bind(&ScanManager::ScanDir, this,  path + "\\" + dir));
		//ScanDir(path + "\\" + dir);
		pool.AddTask(&ScanManager::ScanDir, this, path + "\\" + dir);
	}
}