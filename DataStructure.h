#pragma once
#include"constDefiner.h"
struct inode
{
	long long createTime = 0;//�ļ�����ʱ�䣺unixʱ��� 8byte
	int mode_uid = 0;//�ļ������Լ��ļ������� 4byte
	int fileSize = 0;//�ļ���С 4byte
	//�ļ�����ָ��
	int baseFile_id[10]{ 0 }; //10*4byte ����320���ļ��л��ļ�
	int singleIndex_id = 0;
	int doubleIndex_id = 0;
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

std::string ParseDir(int dir_id, bool* end)
{
	std::string value = "";
	dirFile tempDir{ 0 };
	memcpy_s(&tempDir, 1024, file_head[dir_id], 1024);

	for (int i = 0; i < 32; i++)
	{
		if (tempDir.aMap[i].Name[0])
		{
			value += tempDir.aMap[i].Name;
			value += "\t";
			value += std::to_string(tempDir.aMap[i].inodeIndex);
			//�����ַ(���?)\t������\t�ļ�����
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