#include "common.h"
#include "scan.h"


// �ݹ�ɨ��path�������ļ�, �����ݿ��ļ��ȶ�, �������ݿ�
void ScanManager::ScanDir(const std::string& path) {
	vector<string> files;
	vector<string> dirs;
	DirList(path, files, dirs);

	// ���ļ���Ŀ¼����set(����)
	set<string> localFiles; // �����ļ�����
	localFiles.insert(files.begin(), files.end());
	localFiles.insert(dirs.begin(), dirs.end());

	set<string> dbFiles; // ���ݿ��ļ�����
	DataManager::GetInstance()->GetDbDoc(path, dbFiles);

	// ��ʼ�ȶ�, ���������ݿ�û�����, ��֮ɾ��
	set<string>::iterator localIt = localFiles.begin();
	set<string>::iterator dbIt = dbFiles.begin();

	while (localIt != localFiles.end() && dbIt != dbFiles.end()) {
		if (*localIt < *dbIt) { //˵��������, ���ݿ�û��, �����ݿ����
			//TODO: ���ݿ����
			DataManager::GetInstance()->InsertDbDoc(path, *localIt);
			localIt++;
		}
		else if (*localIt > *dbIt) { //���ݿ���, ����û��, �����ݿ�ɾ�� 
			//TODO: ���ݿ�ɾ��
			DataManager::GetInstance()->DeleteDbDoc(path, *dbIt);
			dbIt++;
		}
		else {
			localIt++;
			dbIt++;
		}
	}

	while (localIt != localFiles.end()) {
		//TODO: ���ݿ����
		DataManager::GetInstance()->InsertDbDoc(path, *localIt);
		localIt++;
	}

	while (dbIt != dbFiles.end()) {
		//TODO: ���ݿ�ɾ��
		DataManager::GetInstance()->DeleteDbDoc(path, *dbIt);
		dbIt++;
	}

	//�ݹ���Ŀ¼
	for (const auto& dir : dirs) {
		//ThreadPool::GetInstance()->AddTask(std::bind(&ScanManager::ScanDir, this,  path + "\\" + dir));
		//ScanDir(path + "\\" + dir);
		pool.AddTask(&ScanManager::ScanDir, this, path + "\\" + dir);
	}
}