#pragma once
#include"constDefiner.h"
#include<sstream>
struct inode
{
	long long createTime = 0;//文件创建时间：unix时间戳 8byte
	int mode_uid = 0;//文件属性以及文件所属者 4byte
	int fileSize = 0;//文件大小 4byte
	//文件索引指针
	int baseFile_id[10]{ 0 }; //10*4byte 包含320个文件夹或文件
	int singleIndex_id = 0;
	int doubleIndex_id = 0;
};//sum: 64 byte

struct inodeMap
{
	char Name[28]{ 0 };//文件/文件夹名称长度<27个字节
	int inodeIndex = 0;//i结点的序号
};//sum: 32 byte

struct dirFile
{
	inodeMap aMap[32];//一个目录包含1024/32=32个文件-i结点/文件夹-i结点映射
};//sum: 1k

struct SuperBlock
{
	int superBlockRegion[2]{ 0 };//8byte
	int fileMappingRegion[2]{ 0 };//8byte 文件名和文件inode的映射
	int iNodeRegion[2]{ 0 };//8byte
	int fileBlockRegion[2]{ 0 };//8byte
	int currentFreeInodePos = 0;//4byte
	Byte freeMem[1024 - 36]{ 0 };
};


std::string dec2hex(int i)
{
	std::stringstream ioss;     //定义字符串流
	std::string s_temp;         //存放转化后字符
	ioss << std::hex << i;      //以十六制形式输出
	ioss >> s_temp;

	if (5 > s_temp.size())
	{
		std::string s_0(5 - s_temp.size(), '0');      //位数不够则补0
		s_temp = s_0 + s_temp;                            //合并
	}

	std::string s = s_temp.substr(s_temp.length() - 5, s_temp.length());    //取右width位
	return "0x"+s;
}

std::string ParseDir(int dir_id, bool* end)
{
	std::string value = "";
	dirFile tempDir{ 0 };
	memcpy_s(&tempDir, 1024, file_head[dir_id], 1024);

	for (int i = 0; i < 32; i++)
	{
		if (tempDir.aMap[i].Name[0] == '/')
		{
			value += "dir  ";
			value += dec2hex(tempDir.aMap[i].inodeIndex);
			value += "\t777\t";
			value += "----\t";
			value += tempDir.aMap[i].Name + 1;
			value += "\n";
		}
		else if (tempDir.aMap[i].Name[0])
		{
			value += "file ";
			value += dec2hex(tempDir.aMap[i].inodeIndex);
			value += "\t777\t";
			value += "----\t";
			value += tempDir.aMap[i].Name;
			value += "\n";
		}
		else
		{
			*end = true;
			break;
		}
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
				value += std::to_string(inodetemp.baseFile_id[u]);
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