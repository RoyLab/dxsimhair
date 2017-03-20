#pragma once
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "macros.h"

namespace XR
{
	typedef std::map<std::string, std::string> ParameterDictionary;

	const char _COMMENT_CHAR = '#';//注释符

	namespace
	{
		bool IsSpace(char c)//判断是不是空格
		{
			if (' ' == c || '\t' == c)
				return true;
			return false;
		}

		bool IsCommentChar(char c)//判断是不是注释符
		{
			switch (c) {
			case _COMMENT_CHAR:
				return true;
			default:
				return false;
			}
		}

		void Trim(std::string & str)//去除字符串的首尾空格
		{
			if (str.empty()) {
				return;
			}
			int i, start_pos, end_pos;
			for (i = 0; i < (int)str.size(); ++i) {
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
	}

	class ConfigReader
	{
		typedef std::string string;
		typedef std::ifstream ifstream;
	private:
		ifstream *infile = nullptr;
		string fileName;

	public:
		//参数filename，配置文件的名字
		ConfigReader(const string & filename)
		{
			infile = new std::ifstream(filename.c_str());
			if (!infile)
			{
				std::cout << "无法打开配置文件" << std::endl;
			}
		}
		~ConfigReader(void)
		{
			if (infile)
			{
				if (infile->is_open())
					infile->close();
				delete infile;
			}
		}

		//参数name，配置项的名字
		//返回值，对应配置项name的value值
		string getValue(const string & name)
		{
			assert(infile->is_open());
			string line;
			string new_line;
			while (getline(*infile, line))
			{
				if (line.empty() || line == "\n\r" || line == "\n")
				{
					continue;
				}

				int start_pos = 0, end_pos = line.size() - 1, pos;
				if ((pos = line.find(_COMMENT_CHAR)) != -1)
				{
					if (0 == pos)
					{  // 行的第一个字符就是注释字符
						continue;
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

		void getParamDict(ParameterDictionary& dict)
		{
			string line;
			string new_line;
			while (getline(*infile, line))
			{
				if (line.empty() || line == "\n\r" || line == "\n")
				{
					continue;
				}

				int start_pos = 0, end_pos = line.size() - 1, pos;
				if ((pos = line.find(_COMMENT_CHAR)) != -1)
				{
					if (0 == pos)
					{  // 行的第一个字符就是注释字符
						continue;
					}
					end_pos = pos - 1;
				}
				new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // 预处理，删除注释部分 
				if ((pos = new_line.find('=')) == -1)
				{
					continue;  // 没有=号
				}
				string na = new_line.substr(0, pos);
				Trim(na);
				string value = new_line.substr(pos + 1, end_pos + 1 - (pos + 1));
				Trim(value);
				dict[na] = value;
			}
		}

		void close()
		{
			if (infile && infile->is_open())
				infile->close();
		}
	};

}


