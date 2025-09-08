#ifndef _SM_CONFIG_H_
#define _SM_CONFIG_H_

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


class sm_config_t
{
public:
    sm_config_t(const std::string &file_path = "sm_system.cfg");
    ~sm_config_t() = default;

    std::string getPath() const;
    
    // 基础类型匹配函数（带默认值）
    void match(const std::string &para, int &value, int default_value = 0);
    void match(const std::string &para, int64_t &value, int64_t default_value = 0);
    void match(const std::string &para, bool &value, bool default_value = false);
    void match(const std::string &para, std::string &value, const std::string &default_value = "");
    
    // 新增：获取值而不是通过引用设置
    int getInt(const std::string &para, int default_value = 0);
    int64_t getInt64(const std::string &para, int64_t default_value = 0);
    bool getBool(const std::string &para, bool default_value = false);
    std::string getString(const std::string &para, const std::string &default_value = "");
    
    // 新增：检查参数是否存在
    bool hasKey(const std::string &para);
    
    // 工具函数
    static std::vector<std::string> splitString(const std::string &s, char delim);
    static bool isDigitString(const std::string &s);
    static bool isLetterString(const std::string &s);
    static int toInt32(const std::string &s);
    static int64_t toInt64(const std::string &s);
    static bool toBool(const std::string &s);

private:
    std::string m_cfg_file;
    
    // 内部读取函数
    std::string readValue(const std::string &para, const std::string &default_value = "");
};

#endif // _SM_CONFIG_H_