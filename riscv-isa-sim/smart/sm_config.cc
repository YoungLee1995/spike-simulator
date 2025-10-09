#include "sm_config.h"

using namespace std;

sm_config::sm_config(const string &file_path) : m_cfg_file(file_path) {}

vector<string> sm_config::splitString(const string &s, char delim)
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

bool sm_config::isDigitString(const string &s)
{
    return !s.empty() && all_of(s.begin(), s.end(), [](char c)
                                { return isdigit(c); });
}

bool sm_config::isLetterString(const string &s)
{
    return !s.empty() && all_of(s.begin(), s.end(), [](char c)
                                { return isalpha(c); });
}

int sm_config::toInt32(const string &s)
{
    try
    {
        return stoi(s);
    }
    catch (const exception &e)
    {
        assert(0 && "invalid int string");
        return 0;
    }
}

int64_t sm_config::toInt64(const string &s)
{
    try
    {
        return stoll(s);
    }
    catch (const exception &e)
    {
        assert(0 && "invalid int64 string");
        return 0;
    }
}

bool sm_config::toBool(const string &s)
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

void sm_config::match(const string &para, int &value )
{
    value = 0;
    ifstream file(m_cfg_file, ios_base::in);
    if (!file.is_open())
    {
        cout << m_cfg_file << "open file error" << endl;
        return;
    }

    string line;
    vector<string> tokens;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        line = removeComments(line);
        tokens = splitString(line, ' ');
        if (para.compare(tokens[0]) == 0)
        {
            value = toInt32(tokens[1]);
            break;
        }
    }
    file.close();
}

void sm_config::match(const string &para, int64_t &value)
{
    value = 0;
    ifstream file(m_cfg_file, ios_base::in);
    if (!file.is_open())
    {
        cout << m_cfg_file << "open file error" << endl;
        return;
    }
    string line;
    vector<string> tokens;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        line = removeComments(line);
        tokens = splitString(line, ' ');
        if (para.compare(tokens[0]) == 0)
        {
            value = toInt64(tokens[1]);
            break;  
        }
    }
    file.close();
}

void sm_config::match(const string &para, bool &value)
{
    value = false;
    ifstream file(m_cfg_file, ios_base::in);
    if (!file.is_open())
    {
        cout << m_cfg_file << "open file error" << endl;
        return;
    }
    string line;
    vector<string> tokens;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        line = removeComments(line);
        tokens = splitString(line, ' ');
        if (para.compare(tokens[0]) == 0)
        {
            value = toBool(tokens[1]);
            break;  
        }
    }
    file.close();
}

void sm_config::match(const string &para, string &value)
{
    value = "";
    ifstream file(m_cfg_file, ios_base::in);
    if (!file.is_open())
    {
        cout << m_cfg_file << "open file error" << endl;
        return;
    }
    string line;
    vector<string> tokens;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        line = removeComments(line);
        tokens = splitString(line, ' ');
        if (para.compare(tokens[0]) == 0)
        {
            value = tokens[1];
            break;  
        }
    }
    file.close();
}
string sm_config::removeComments(const string &line)
{
    size_t pos_hash = line.find('#');
    size_t pos_semicolon = line.find(';');
    size_t pos = string::npos;

    if (pos_hash != string::npos && pos_semicolon != string::npos)
        pos = min(pos_hash, pos_semicolon);
    else if (pos_hash != string::npos)
        pos = pos_hash;
    else if (pos_semicolon != string::npos)
        pos = pos_semicolon;

    if (pos != string::npos)
        return line.substr(0, pos);
    else
        return line;
}

