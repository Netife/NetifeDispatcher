//
// Created by Administrator on 2023/5/1.
//

#include "logger.h"
using namespace std;
void Netife::logger::RegisterCommonLogger(const std::string& name) {
    el::Logger* loaderLogger = el::Loggers::getLogger(name);
    el::Configurations conf;
    conf.setGlobally(el::ConfigurationType::Format, "[%datetime{%H:%m:%s}][%level][%logger] %msg");
    conf.setGlobally(el::ConfigurationType::Filename, "logs\\%datetime{%Y-%M-%d}.log");
    el::Loggers::reconfigureLogger(name, conf);
}
