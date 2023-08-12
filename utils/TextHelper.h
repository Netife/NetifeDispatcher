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
        static std::string getBetween(const std::string &str, const std::string &left, const std::string &right);
        static std::vector<std::string> splitByBlankWithSkipBlank(const std::string &str);
        static std::vector<std::string> splitMulti(const std::string &str, const std::string &pattern);
        static std::string myToHex(const std::string& srcStr);
        static std::string myToBytes(const std::string& srcStr);
    };
}

#endif //NETIFEDISPATCHER_TEXTHELPER_H
