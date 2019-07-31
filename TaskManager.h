#pragma once
#include <thread>
#include "boost/asio/io_service.hpp"

class TaskManager
{
public:
    explicit TaskManager(int thr_num);
    boost::asio::io_service& GetIOService(){return service_;}
    virtual ~TaskManager();

private:
    boost::asio::io_service service_;
    boost::asio::io_service::work work_;
    std::vector<std::thread> threads_;
};
