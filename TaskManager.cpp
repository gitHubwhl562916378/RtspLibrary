#include "TaskManager.h"

TaskManager::TaskManager(int thr_num):
    service_(thr_num),work_(service_)
{
    for(int i = 0; i < thr_num; i++){
        threads_.emplace_back(std::thread([this]{
            service_.run();
        }));
    }
}

TaskManager::~TaskManager()
{
    service_.stop();
    for(auto iter = threads_.begin(); iter != threads_.end();)
    {
        if(iter->joinable()){
            iter->join();
        }
        iter = threads_.erase(iter);
    }
}
