#pragma once
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "macros.h"

namespace XR
{
	typedef std::map<std::string, std::string> ParameterDictionary;

	const char _COMMENT_CHAR = '#';//ע�ͷ�

	namespace
	{
		bool IsSpace(char c)//�ж��ǲ��ǿո�
		{
			if (' ' == c || '\t' == c)
				return true;
			return false;
		}

		bool IsCommentChar(char c)//�ж��ǲ���ע�ͷ�
		{
			switch (c) {
			case _COMMENT_CHAR:
				return true;
			default:
				return false;
			}
		}

		void Trim(std::string & str)//ȥ���ַ�������β�ո�
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
			if (i == str.size()) { // ȫ���ǿհ��ַ���
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
		//����filename�������ļ�������
		ConfigReader(const string & filename)
		{
			infile = new std::ifstream(filename.c_str());
			if (!infile)
			{
				std::cout << "�޷��������ļ�" << std::endl;
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

		//����name�������������
		//����ֵ����Ӧ������name��valueֵ
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
					{  // �еĵ�һ���ַ�����ע���ַ�
						continue;
					}
					end_pos = pos - 1;
				}
				new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // Ԥ����ɾ��ע�Ͳ��� 
				if ((pos = new_line.find('=')) == -1)
				{
					return "";  // û��=��
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
					{  // �еĵ�һ���ַ�����ע���ַ�
						continue;
					}
					end_pos = pos - 1;
				}
				new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // Ԥ����ɾ��ע�Ͳ��� 
				if ((pos = new_line.find('=')) == -1)
				{
					continue;  // û��=��
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


