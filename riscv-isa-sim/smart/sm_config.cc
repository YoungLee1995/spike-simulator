#include "sm_config.h"

using namespace std;

sm_config_t::sm_config_t(const string &file_path) : m_cfg_file(file_path) {}

string sm_config_t::getPath() const
{
    return m_cfg_file;
}

vector<string> sm_config_t::splitString(const string &s, char delim)
{
    vector<string> elems;
    string item;
    istringstream ss(s);
    while (getline(ss, item, delim))
    {
        if (!item.empty())
        {
            elems.push_back(item);
        }
    }
    return elems;
}

bool sm_config_t::isDigitString(const string &s)
{
    return !s.empty() && all_of(s.begin(), s.end(), [](char c) { return isdigit(c); });
}

bool sm_config_t::isLetterString(const string &s)
{
    return !s.empty() && all_of(s.begin(), s.end(), [](char c) { return isalpha(c); });
}

int sm_config_t::toInt32(const string &s)
{
    try {
        return stoi(s);
    } catch (const exception &e) {
        assert(0 && "invalid int string");
        return 0;
    }
}

int64_t sm_config_t::toInt64(const string &s)
{
    try {
        return stoll(s);
    } catch (const exception &e) {
        assert(0 && "invalid int64 string");
        return 0;
    }
}

bool sm_config_t::toBool(const string &s)
{
    string lower_s = s;
    transform(lower_s.begin(), lower_s.end(), lower_s.begin(), ::tolower);
    
    if (lower_s == "1" || lower_s == "true" || lower_s == "yes" || lower_s == "on")
        return true;
    else if (lower_s == "0" || lower_s == "false" || lower_s == "no" || lower_s == "off")
        return false;
    else
        assert(0 && "invalid bool string");
    
    return false;
}

string sm_config_t::readValue(const string &para, const string &default_value)
{
    ifstream infile(m_cfg_file);
    if (!infile.is_open())
    {
        return default_value;
    }
    
    string line;
    while (getline(infile, line))
    {
        // 跳过注释行和空行
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
            
        vector<string> v = splitString(line, '=');
        if (v.size() != 2)
            continue;
            
        string key = v[0];
        string val = v[1];
        
        // 移除空格
        key.erase(remove_if(key.begin(), key.end(), ::isspace), key.end());
        val.erase(remove_if(val.begin(), val.end(), ::isspace), val.end());
        
        if (key == para)
        {
            return val;
        }
    }
    
    return default_value;
}

bool sm_config_t::hasKey(const string &para)
{
    string value = readValue(para);
    return !value.empty();
}

void sm_config_t::match(const string &para, int &value, int default_value)
{
    string val_str = readValue(para);
    if (val_str.empty())
    {
        value = default_value;
        return;
    }
    
    if (isDigitString(val_str))
    {
        value = toInt32(val_str);
    }
    else
    {
        value = default_value;
    }
}

void sm_config_t::match(const string &para, int64_t &value, int64_t default_value)
{
    string val_str = readValue(para);
    if (val_str.empty())
    {
        value = default_value;
        return;
    }
    
    if (isDigitString(val_str))
    {
        value = toInt64(val_str);
    }
    else
    {
        value = default_value;
    }
}

void sm_config_t::match(const string &para, bool &value, bool default_value)
{
    string val_str = readValue(para);
    if (val_str.empty())
    {
        value = default_value;
        return;
    }
    
    try {
        value = toBool(val_str);
    } catch (...) {
        value = default_value;
    }
}

void sm_config_t::match(const string &para, string &value, const string &default_value)
{
    string val_str = readValue(para);
    if (val_str.empty())
    {
        value = default_value;
    }
    else
    {
        value = val_str;
    }
}

int sm_config_t::getInt(const string &para, int default_value)
{
    int value = default_value;
    match(para, value, default_value);
    return value;
}

int64_t sm_config_t::getInt64(const string &para, int64_t default_value)
{
    int64_t value = default_value;
    match(para, value, default_value);
    return value;
}

bool sm_config_t::getBool(const string &para, bool default_value)
{
    bool value = default_value;
    match(para, value, default_value);
    return value;
}

string sm_config_t::getString(const string &para, const string &default_value)
{
    string value = default_value;
    match(para, value, default_value);
    return value;
}