/****************************************************************************
*   原作:  符星
*   日期:  2013-4-14
*   目的:  读取配置文件的信息，以string的形式返回
*   要求:  配置文件的格式，以#作为行注释，配置的形式是key = value，中间可有空格，也可没有空格
*****************************************************************************/
#pragma once
#include <map>
#include <string>
#define COMMENT_CHAR '#'//注释符

class ParamDict :
	public std::map<std::string, std::string>
{
public:
};

class ConfigReader
{
	typedef std::string string;
private:
    std::ifstream *infile = nullptr;
	string fileName;

public:
    //参数filename，配置文件的名字
    ConfigReader(const string & filename);
    ~ConfigReader(void){ if (infile) delete infile; }

    //参数name，配置项的名字
    //返回值，对应配置项name的value值
    string getValue(const string & name);
    void getParamDict(ParamDict& dict);

    void close();
};

