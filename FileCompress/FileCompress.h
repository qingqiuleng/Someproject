#pragma once 
#include"HuffManTree.h"
#include<string>

struct CharInfo
{
	CharInfo(int count=0)
	:_count(count)
	{
	}

	bool operator<(const CharInfo info)
	{
		return _count < info._count;
	}

	bool operator>(const CharInfo info)
	{
		return _count>info._count;
	}

	bool operator!=(const CharInfo info)
	{
		return _count != info._count;
	}

	CharInfo operator+(const CharInfo Info)
	{
		return CharInfo(_count + Info._count);
	}

	char _ch;//�ַ�
	int _count;//�ַ����ֵĴ���
	string _code;//�ַ���Ӧ�ı���
};

class FileCompress
{
public:
	FileCompress()
	{
		for (int i = 0; i < 256; i++)
		{
			_info[i]._ch = i;
			_info[i]._count = 0;
		}
	}

public:
	void  Compress(const char* FileName)//ѹ��
	{ 
		FILE* fout = fopen(FileName, "rb");
		assert(fout);
		
		//ͳ���ַ����ֵĴ���
		int ch = fgetc(fout);
		printf("%c\n", ch);
		int count = 0;
		while (ch!= EOF)
		{
			_info[unsigned char(ch)]._count++;
			ch = fgetc(fout);
			count++;
		}

		//������������
		CharInfo invalid;
		HuffManTree<CharInfo> h(_info, 256, invalid);

		//���ɹ���������
		string code;
		_GetHuffManCode(h._GetRoot(), code);

		string CompressFileName = FileName;
		CompressFileName += ".compress";
		FILE* fin = fopen(CompressFileName.c_str(), "wb");
		assert(fin);
		fseek(fout, 0, SEEK_SET);//���ļ���ͷ

		ch =(unsigned char)fgetc(fout);

		char value = 0;
		int size = 0;
		while (ch != EOF)
		{
			string _ccode = _info[(unsigned char)ch]._code;
			for (int i = 0; i < _ccode.size(); ++i)
			{
				value <<= 1;
				if (_ccode[i] =='1')
				{
					value |=1;
				}
				size++;
				if (size == 8)
				{
					fputc(value, fin);
					value = 0;
					size = 0;
				}
				
			}
			ch = fgetc(fout);
		}
		//��λ
		if (size!=0)
		{
			value <<= ( 8- size);
			fputc(value, fin);
		}

		//д�����ļ�
		string configFileName = FileName;
		configFileName += ".config.txt";
		FILE* finConfig = fopen(configFileName.c_str(), "wb");
		assert(finConfig);
	
		string str;
		char buf[128];
		for (int i = 0; i < 256; i++)
		{
			if (_info[i]._count>0)
			{
				str += _info[i]._ch;
				str += ',';
				_itoa(_info[i]._count, buf, 10);
				str += buf;
				str += '\n';

				fputs(str.c_str(), finConfig);
				str.clear();
			}
		}
		
	

		fclose(fin);
		fclose(fout);
		fclose(finConfig);
	}

	void unCompress(const char* FileName)//��ѹ��
	{
		//�������ļ�
		string configFileNane = FileName;
		configFileNane += ".config.txt";
		FILE* foutConfig = fopen(configFileNane.c_str(), "rb");
		assert(foutConfig);
		int count = 0;
		string str;
		while (Read_a_Line(foutConfig,str))
		{
			if (str.empty())
			{
				str += '\n';
				count += 1;
				str.clear();
			}
			//else
			//{
			//	//_info[(unsigned char)str[0]] = atoi(str.substr(2).c_str());
			//	count += _info[(unsigned char)str[0]]._count;
			//	str.clear();
			//}	
		//	_info[((unsigned char)str[0])]._count = atoi(str.substr(2).c_str());
			//count += _info[(unsigned char)str[0]]._count;

			else
			{
				unsigned char ch = str[0];
				_info[ch]._count = atoi(str.substr(2).c_str());
				count += _info[ch]._count;
				str.clear();
			}
		
		}

		CharInfo invaild;
		HuffManTree<CharInfo> tree(_info, 256,invaild);

		string unCompressFileName = FileName;
		unCompressFileName += ".unCompress";//��ѹ���ļ�
		string CompressFileName = FileName;
		CompressFileName += ".compress";

		FILE* fout = fopen(CompressFileName.c_str(), "rb");
		assert(fout);
		FILE* fin = fopen(unCompressFileName.c_str(), "wb");
		assert(fin);

		HuffManTreeNode<CharInfo>* root = tree._GetRoot();
		HuffManTreeNode<CharInfo>* cur = root;
		int ch = fgetc(fout);
		int size =7;

		while (ch != EOF)
		{
			if (ch & (1 << size))
			{
				cur = cur->_right;
			}
			else
			{
				cur = cur->_left;
			}
			if (cur->_left==NULL&&cur->_right==NULL)
			{
				fputc(cur->_weight._ch, fin);
				cur = root;
				//count--;
				//if (count == 0)
				//	break;
			}
			size--;
			if (size<0)
			{
				ch=fgetc(fout);
				size = 7;
			}
		
		}

		fclose(fin);
		fclose(fout);
		fclose(foutConfig);
	}

protected:
	bool Read_a_Line(FILE*& fout,string& str)
	{
		int ch = fgetc(fout);
		if (ch == EOF)
			return false;
		
		while (ch != EOF&&ch!='\n')
		{
			str += ch;
			ch = fgetc(fout);
		}
		return true;
	}

	void _GetHuffManCode(const HuffManTreeNode<CharInfo>* root,string code)//���ɹ���������
	{
		if (root == NULL)
		{
			return;
		}
		if (root->_left == NULL&&root->_right == NULL)
		{
			_info[unsigned char((root->_weight)._ch)]._code = code;
			return;
		}
		if (root->_left)
			_GetHuffManCode(root->_left, code + '0');//��·Ϊ0
		if (root->_right)
			_GetHuffManCode(root->_right, code + '1');//��·Ϊ1
		
	}

private:
	CharInfo _info[256];
};