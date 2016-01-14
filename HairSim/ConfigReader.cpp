#include "ConfigReader.h"
#include <fstream>
#include <iostream>
using namespace std;

bool IsSpace(char c)//判断是不是空格
{
	if (' ' == c || '\t' == c)
		return true;
	return false;
}

bool IsCommentChar(char c)//判断是不是注释符
{
	switch (c) {
	case COMMENT_CHAR:
		return true;
	default:
		return false;
	}
}

void Trim(string & str)//去除字符串的首尾空格
{
	if (str.empty()) {
		return;
	}
	int i, start_pos, end_pos;
	for (i = 0; i < str.size(); ++i) {
		if (!IsSpace(str[i])) {
			break;
		}
	}
	if (i == str.size()) { // 全部是空白字符串
		str = "";
		return;
	}

	start_pos = i;

	for (i = str.size() - 1; i >= 0; --i) {
		if (!IsSpace(str[i])) {
			break;
		}
	}
	end_pos = i;

	str = str.substr(start_pos, end_pos - start_pos + 1);
}

//参数name，配置项的名字
//返回值，对应配置项name的value值
string ConfigReader::getValue(const string & name)
{
	string line;
	string new_line;
	while (getline(*infile, line))
	{
		if (line.empty())
		{
			return "";
		}
		int start_pos = 0, end_pos = line.size() - 1, pos;
		if ((pos = line.find(COMMENT_CHAR)) != -1)
		{
			if (0 == pos)
			{  // 行的第一个字符就是注释字符
				return "";
			}
			end_pos = pos - 1;
		}
		new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // 预处理，删除注释部分 
		if ((pos = new_line.find('=')) == -1)
		{
			return "";  // 没有=号
		}
		string na = new_line.substr(0, pos);
		Trim(na);
		if (na == name)
		{
			string value = new_line.substr(pos + 1, end_pos + 1 - (pos + 1));
			Trim(value);
			return  value;
		}
	}
	return "";
}

ConfigReader::ConfigReader(const string & filename)
{
	infile = new ifstream(filename.c_str());
	if (!infile)
	{
		cout << "无法打开配置文件" << endl;
	}
}
