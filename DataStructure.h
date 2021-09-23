#pragma once
#include"constDefiner.h"
#include<sstream>
struct inode
{
	long long createTime = 0;//文件创建时间：unix时间戳 8byte
	int mode_uid = 0;//文件属性以及文件所属者 4byte
	int fileSize = 0;//文件大小 4byte,在文件夹中作为首个可以mkdir的位置
	int file_id[10]{ 0 }; //10*4byte 包含320个文件夹或文件
	int singleIndex_id = 0;//一级文件索引
	int doubleIndex_id = 0;//二级文件索引
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

void TraversalDir(inode* inodep)//遍历目录
{
	//只要inode的数值为0，即代表可以被分配
	//删除时需要将父目录dir文件中的inode号设置为0,分配出去的file与inode也要被收回(从bitmap上
	for (int i = 0; i < 10; i++)
	{
		dirFile* p = (dirFile*)&file_head[inodep->file_id[i]];//将实际的物理块作为dir读取
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
		value += dec2hex(inodep->file_id[0]) + "\t";//文件索引分配，以第一个文件块作为物理地址
		value += std::to_string(inodep->mode_uid) + "\t";
		value += "--\t";
	}
	else
	{
		value += "file ";
		inode* inodep = (inode*)inode_head[0];
		value += dec2hex(inodep->file_id[0]) + "\t";//文件索引分配，以第一个文件块作为物理地址
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