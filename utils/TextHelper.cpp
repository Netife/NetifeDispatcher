//
// Created by Administrator on 2023/5/1.
//

#include "TextHelper.h"
using namespace std;
namespace Netife{
    std::vector <string> TextHelper::split(const std::string &str, const std::string &pattern) {
        vector<string> res;
        if(str.empty())
            return res;
        string strs = str + pattern;
        size_t pos = strs.find(pattern);

        while(pos != std::string::npos)
        {
            string temp = strs.substr(0, pos);
            res.push_back(temp);
            //去掉已分割的字符串,在剩下的字符串中进行分割
            strs = strs.substr(pos+1, strs.size());
            pos = strs.find(pattern);
        }

        return res;
    }
}