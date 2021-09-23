#pragma once
#include"constDefiner.h"
#include<sstream>
struct inode
{
	long long createTime = 0;//�ļ�����ʱ�䣺unixʱ��� 8byte
	int mode_uid = 0;//�ļ������Լ��ļ������� 4byte
	int fileSize = 0;//�ļ���С 4byte,���ļ�������Ϊ�׸�����mkdir��λ��
	int file_id[10]{ 0 }; //10*4byte ����320���ļ��л��ļ�
	int singleIndex_id = 0;//һ���ļ�����
	int doubleIndex_id = 0;//�����ļ�����
};//sum: 64 byte

struct inodeMap
{
	char Name[28]{ 0 };//�ļ�/�ļ������Ƴ���<27���ֽ�
	int inodeIndex = 0;//i�������
};//sum: 32 byte

struct dirFile
{
	inodeMap aMap[32];//һ��Ŀ¼����1024/32=32���ļ�-i���/�ļ���-i���ӳ��
};//sum: 1k

struct SuperBlock
{
	int superBlockRegion[2]{ 0 };//8byte
	int fileMappingRegion[2]{ 0 };//8byte �ļ������ļ�inode��ӳ��
	int iNodeRegion[2]{ 0 };//8byte
	int fileBlockRegion[2]{ 0 };//8byte
	int currentFreeInodePos = 0;//4byte
	Byte freeMem[1024 - 36]{ 0 };
};


std::string dec2hex(int i)
{
	std::stringstream ioss;     //�����ַ�����
	std::string s_temp;         //���ת�����ַ�
	ioss << std::hex << i;      //��ʮ������ʽ���
	ioss >> s_temp;

	if (5 > s_temp.size())
	{
		std::string s_0(5 - s_temp.size(), '0');      //λ��������0
		s_temp = s_0 + s_temp;                            //�ϲ�
	}

	std::string s = s_temp.substr(s_temp.length() - 5, s_temp.length());    //ȡ��widthλ
	return "0x"+s;
}

void TraversalDir(inode* inodep)//����Ŀ¼
{
	//ֻҪinode����ֵΪ0����������Ա�����
	//ɾ��ʱ��Ҫ����Ŀ¼dir�ļ��е�inode������Ϊ0,�����ȥ��file��inodeҲҪ���ջ�(��bitmap��
	for (int i = 0; i < 10; i++)
	{
		dirFile* p = (dirFile*)&file_head[inodep->file_id[i]];//��ʵ�ʵ��������Ϊdir��ȡ
		if (p->aMap[0].Name && p->aMap[0].inodeIndex != 0)
		{
			//this is a exist file
		}
		else if (p->aMap[0].Name && p->aMap[0].inodeIndex == 0)
		{
			//this is a removed file
		}
		else
		{
			//this block is never used
		}
	}
}

std::string inodeInfo(char fileType,int inode_id)
{
	std::string value;
	if (fileType == '/')
	{
		value += "dir  ";
		inode* inodep = (inode*)inode_head[0];
		value += dec2hex(inodep->file_id[0]) + "\t";//�ļ��������䣬�Ե�һ���ļ�����Ϊ�����ַ
		value += std::to_string(inodep->mode_uid) + "\t";
		value += "--\t";
	}
	else
	{
		value += "file ";
		inode* inodep = (inode*)inode_head[0];
		value += dec2hex(inodep->file_id[0]) + "\t";//�ļ��������䣬�Ե�һ���ļ�����Ϊ�����ַ
		value += std::to_string(inodep->mode_uid) + "\t";
		value += std::to_string(inodep->fileSize) + "\t";
	}
	return value;
}


std::string ParseInodeFile(int inode_id)
{
	std::string value;
	inode inodetemp{ 0 };
	memcpy_s(&inodetemp, 1024, disk[inode_id], 1024);
	for (int i = 0; i < 16; i++)
	{
		if (!inodetemp.createTime)
		{
			value += std::to_string(inodetemp.createTime);
			value += "\t";
			value += std::to_string(inodetemp.mode_uid);
			value += "\n";
			for (int u = 0; u < 10; u++)
			{
				value += std::to_string(inodetemp.file_id[u]);
				value += "\n";
			}
		}
	}
	return value;
}

void* pOffset(void* p,int offset)
{
	return (int*)(((ll)(p)) + ll(offset));
}

template<typename T>
void diskPointer(int disk_id, int offset,T pointer,File* p = disk)
{
	pointer = (T)(p + disk_id)+offset;
}

void getInodeByIndex(int inode_id,inode* p)
{
	p = (inode*)(inode_head)+inode_id;
}
//std::string ParseBitmap(int bitmap_id)
//{
//	std::string value;
//	for (int u = 0; u < 1024; u++)
//	{
//		if (fileBitmap_head[bitmap_id][u] & (abit_quick[0] >> u))
//		{
//			value += "\n";
//			value += std::to_string(u);
//		}
//	}
//	return value;
//}