//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_TEXTHELPER_H
#define NETIFEDISPATCHER_TEXTHELPER_H
#include <iostream>
#include <string>
#include <vector>
namespace Netife{
    class TextHelper {
    public:
        static std::vector<std::string> split(const std::string &str, const std::string &pattern);
    };
}

#endif //NETIFEDISPATCHER_TEXTHELPER_H