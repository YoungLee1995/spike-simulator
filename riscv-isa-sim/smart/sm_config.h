#ifndef _SM_CONFIG_H_
#define _SM_CONFIG_H_

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class sm_config
{
public:
    sm_config(const std::string &file_path = "sm_system.cfg") : m_cfg_file(file_path) {}
    ~sm_config() {}

    std::string getPath() { return m_cfg_file; }

    string removeComments(const string &line);

    // 基础类型匹配函数（带默认值）
    void match(const std::string &para, int &value);
    void match(const std::string &para, int64_t &value);
    void match(const std::string &para, bool &value);
    void match(const std::string &para, std::string &value);

    // 工具函数
    static std::vector<std::string> splitString(const std::string &s, char delim);
    static bool isDigitString(const std::string &s);
    static bool isLetterString(const std::string &s);
    static int toInt32(const std::string &s);
    static int64_t toInt64(const std::string &s);
    static bool toBool(const std::string &s);

private:
    std::string m_cfg_file;
};

#endif // _SM_CONFIG_H_