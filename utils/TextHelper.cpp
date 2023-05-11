//
// Created by Administrator on 2023/5/1.
//

#include <sstream>
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

    std::string TextHelper::getBetween(const string &str, const string &left, const string &right) {
        string result = "";
        size_t left_pos = str.find(left);
        if (left_pos != string::npos) { // left字符串存在于s中
            left_pos += left.length(); // left_pos调整为left后面的位置
            size_t right_pos = str.find(right, left_pos); // 从left_pos右侧开始查找right字符串
            if (right_pos != string::npos) { // right字符串存在于s中
                result = str.substr(left_pos, right_pos - left_pos); // 取出left_pos与right_pos之间的子串
            }
        }
        return result;
    }

    std::vector<std::string> TextHelper::splitByBlankWithSkipBlank(const string &str) {
        vector<string> res;
        stringstream ss(str);
        string temp;
        bool flag = false; // 判断是否在双引号内
        while (ss >> temp) {
            if (temp[0] == '\"') {
                flag = true;
                temp = temp.substr(1);
                res.push_back(temp);
                continue;
            }
            if (temp[temp.size()-1] == '\"') {
                flag = false;
                temp = temp.substr(0, temp.size()-1);
                res.back() += ' ' + temp; // 注意需要加上空格
                continue;
            }
            if (flag) {
                res.back() += ' ' + temp; // 注意需要加上空格
            } else {
                res.push_back(temp);
            }
        }
        return res;
    }
}