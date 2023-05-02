//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_LOGGER_H
#define NETIFEDISPATCHER_LOGGER_H
#include "../lib/log/easylogging++.h"

namespace Netife{
    class logger {
    public:
        static void RegisterCommonLogger(const std::string& name);
    };
}



#endif //NETIFEDISPATCHER_LOGGER_H
