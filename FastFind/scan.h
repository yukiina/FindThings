#pragma once
#include "common.h"
#include "data.h"
#include "ThreadPool.h"

// É¨ÃèÄ£¿é
class ScanManager {
public:
	void ScanDir(const std::string& path);
	void StartScan() {
		//while (1) {
			ScanDir("D:\\");
			DataManager::GetInstance()->ClearInsertVector();
			//std::this_thread::sleep_for(std::chrono::seconds(5));
		//}
	}

	static ScanManager* CreateInstance() {
		static ScanManager scan;
		static std::thread th(&ScanManager::StartScan, &scan);
		th.detach();
		return &scan;
	}
private:
	ScanManager() {}
	ScanManager(const ScanManager&) = delete;
	ScanManager& operator=(const ScanManager&) = delete;
	ThreadPool pool;
	//DataManager _dm;
	
};




