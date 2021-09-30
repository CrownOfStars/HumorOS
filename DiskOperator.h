#pragma once
#include"constDefiner.h"
#include<sstream>

struct dirFile;
struct inodeMap;

template<typename T>
void virtualDiskReader(T* pointer, int filetype, int disk_id, int offset)
{
	char* tempCh = new char[sizeof(T)]{ 0 };
	std::ifstream f("DISK", std::ios::in | std::ios::binary);
	f.seekg((filetype + disk_id) * 1024 + offset * sizeof(T));
	f.read(tempCh, sizeof(T));
	memcpy_s(pointer, sizeof(T), tempCh, sizeof(T));
	f.close();
	delete[] tempCh;
}

template<typename T>
void virtualDiskWriter(T* pointer, int filetype, int disk_id, int offset)
{
	char* tempCh = new char[sizeof(T)]{ 0 };
	std::fstream f("DISK", std::ios::in | std::ios::out | std::ios::binary);
	f.seekp((filetype + disk_id) * 1024 + offset * sizeof(T));
	memcpy_s(tempCh, sizeof(T), pointer, sizeof(T));
	f.write(tempCh, sizeof(T));
	f.close();
	delete[] tempCh;
}

struct inode
{
	void init();
	long long createTime = 0ll;//文件创建时间：unix时间戳 8byte
	int mode_uid = 0;//文件属性以及文件所属者 4byte
	int fileSize = 0;//文件大小 4byte
	int file_id[10]{ 0 }; //10*4byte 包含320个文件夹或文件
	int singleIndex = 0;//一级文件索引
	int doubleIndex = 0;//二级文件索引
	int file(int idx) {
		if (idx < 10)
		{
			return file_id[idx];
		}
		else// if (idx < 10 + 256)
		{
			throw("error 鸽");
			return -1;
		}
	}
	int dirSize() {
		for (int i = 0; i < 10; i++)
		{
			if (file_id[i] == 0)
			{
				return i;
			}
		}
		if (singleIndex != 0)
		{
			throw("鸽");
			return 10;//TODO
		}
		return 10;
	}
	dirFile* getDir();
	void addDir(inodeMap* imapp);
	void clearDir();
};//sum: 64 byte

struct inodeMap
{
	char Name[28]{ 0 };//文件/文件夹名称长度<27个字节
	int inodeId = 0;//i结点的序号
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

void RemoveFromBitmap(bool bmType,int rm_id)
{
	char* p = new char();
	if (bmType)
	{
		virtualDiskReader<char>(p, fileBmStart, 0, rm_id / 8);
		*p &= ~(bitQuick[0] >> rm_id % 8);
		lastFreeFile = rm_id > lastFreeFile ? lastFreeFile : rm_id;
		virtualDiskWriter<char>(p, fileBmStart, 0, rm_id / 8);//rm from file bitmap
	}
	else
	{
		virtualDiskReader<char>(p, inodeBmStart, 0, rm_id / 8);
		*p &= ~(bitQuick[0] >> rm_id % 8);
		lastFreeInode = rm_id > lastFreeInode ? lastFreeInode : rm_id;
		virtualDiskWriter<char>(p, inodeBmStart, 0, rm_id / 8);//rm from inode bitmap
	}
	delete p;
	return;
}

int FillInBitmap(bool bmType)
{
	char* p = new char();
	if (bmType)
	{
		virtualDiskReader<char>(p, fileBmStart, 0, lastFreeFile/8);
		*p |= (bitQuick[0] >> (lastFreeFile % 8));
		virtualDiskWriter<char>(p, fileBmStart, 0, lastFreeFile / 8);
	}
	else
	{
		virtualDiskReader<char>(p, inodeBmStart, 0, lastFreeInode / 8);
		*p |= (bitQuick[0] >> (lastFreeInode % 8));
		virtualDiskWriter<char>(p, inodeBmStart, 0, lastFreeInode / 8);
	}
	delete p;
	return 0;
}

void SearchNextBit(bool bmType)
{
	char* p = new char();
	if (bmType)
	{
		for (int u = (lastFreeFile + 1) / 8; u < fileBmSize * 1024; u++)
		{
			virtualDiskReader<char>(p, fileBmStart, 0, u);
			for (int i = 0; i < 8; i++)
			{
				if (!(*p & (bitQuick[0] >> i)))
				{
					lastFreeFile = u * 8 + i;
					delete p;
					return;
				}
			}
		}
		delete p;
		throw("error 空间不足");
	}
	else
	{
		for (int u = (lastFreeInode+1)/8; u < inodeBmSize*1024; u++)
		{
			virtualDiskReader<char>(p, inodeBmStart, 0, u);
			for (int i = 0; i < 8; i++)
			{
				if (!(*p & (bitQuick[0] >> i)))
				{
					lastFreeInode = u * 8 + i;
					delete p;
					return;
				}
			}
		}
		delete p;
		throw("error 空间不足");
	}
	/* 位运算演示
	11100000
	10000000 == 1 -》next

	11100000
	01000000 == 1 -》next

	11100000
	00100000 == 1 -》next

	11100000
	00010000 == 0 -》ok
	*/
}

void AssignInode()
{
	inode* inodep = new inode{ 0 };
	inodep->init();
	int inode_id = lastFreeInode;
	inodep->file_id[0] = lastFreeFile;
	FillInBitmap(inodeBm);
	SearchNextBit(inodeBm);
	FillInBitmap(fileBm);
	SearchNextBit(fileBm);
	virtualDiskWriter<inode>(inodep, inodeStart, 0, inode_id);
	delete inodep;
	return;
}

void RmInode(int* rmList, int rmSize)//只需要在位图中移除即可？
{
	for (int i = 0;i < rmSize;i++)
	{
		rmList[i];//remove from bitmap
	}
}

int AssignFile()
{
	int file_id = lastFreeFile;
	FillInBitmap(true);
	SearchNextBit(true);
	return file_id;
}

void createMigrateFile()
{
	std::fstream in("cache", std::ios::in);
	std::istreambuf_iterator<char>beg(in), end;
	std::string strdata(beg, end);
	in.close();
	//write strdata to virtual disk
}

std::string inodeInfo(char fileType, int inode_id)
{
	std::string value;
	inode* inodep = new inode{ 0 };
	if (fileType == '/')
	{
		value += "dir  ";
		virtualDiskReader<inode>(inodep, inodeStart, 0, inode_id);
		value += dec2hex(inodep->file_id[0]) + "\t";//文件索引分配，以第一个文件块作为物理地址
		value += std::to_string(inodep->mode_uid) + "\t";
		value += "--\t";
	}
	else
	{
		value += "file ";
		virtualDiskReader<inode>(inodep, inodeStart, 0, inode_id);
		value += dec2hex(inodep->file_id[0]) + "\t";//文件索引分配，以第一个文件块作为物理地址
		value += std::to_string(inodep->mode_uid) + "\t";
		value += std::to_string(inodep->fileSize) + "\t";
	}
	delete inodep;
	return value;
}

void PrintBitmap(int bmid, int type = fileBmStart)
{
	File fi;
	virtualDiskReader<File>(&fi, type, bmid, 0);
	for (int i = 0; i < 1024; i++)
	{
		for (int u = 0; u < 8; u++)
		{
			if ((fi[i] & (bitQuick[0] >> u)))
			{
				std::cout << bmid * 1024 * 8 + i * 8 + u << std::endl;
			}
		}
	}
}

void* pOffset(void* p, int offset)
{
	return (int*)(((ll)(p)) + ll(offset));
}

template<typename T>
void diskPointer(int disk_id, int offset, T* pointer, File* p = disk)
{
	pointer = (T*)(p + disk_id) + offset;
}

void inode::init()
{
	time_t timep;
	time(&timep);
	createTime = timep;
	//assign inode and file
}

dirFile* inode::getDir() {

	dirFile* dirFp = new dirFile[dirSize()];

	for (int u = 0; u < dirSize(); u++)
	{
		virtualDiskReader<dirFile>(dirFp + u, fileStart, file_id[u], 0);
	}
	return dirFp;
}

void inode::addDir(inodeMap* imapp)
{
	dirFile* p = getDir();
	int dir_id = fileSize / 32;
	int offset = fileSize % 32;
	memcpy_s(&(p[dir_id].aMap[offset]), sizeof(inodeMap), imapp, sizeof(inodeMap));
	int dSize = dirSize();
	for (int u = dir_id; u < dirSize() * 32; u++)
	{
		if (p[u / 32].aMap[u % 32].Name[0] == '\0' or p[u / 32].aMap[u % 32].Name[0] == '-')
		{
			fileSize = u;
			if (fileSize == dSize * 32)
			{
				file_id[dSize] = AssignFile();
			}
			break;
		}
	}
	for (int u = 0; u < dSize; u++)
	{
		virtualDiskWriter<dirFile>(p + u, fileStart, file_id[u], 0);
	}
	delete p;
}

void inode::clearDir()
{
	dirFile* pdir = getDir();
	for (int i = 0; i < dirSize() * 32; i++)
	{
		if (pdir[i / 32].aMap[i % 32].Name[0] == '-')
		{
			continue;
		}
		else if (pdir[i / 32].aMap[i % 32].Name[0] == '/')
		{
			//->clearDir()TODO:
		}
		else if (pdir[i / 32].aMap[i % 32].Name[0])
		{
			//->deleteTODO:
		}
		else
		{
			return;
		}
	}
}

inode* curInode = new inode();

//int* FirstFit(File* bmap, int mapSize, int assignSize)//索引分配
//{
//	int* assignFile_id = new int[assignSize];//将要分配的文件id的列表
//	int curAssigned = 0;//当前已分配的项
//	for (int i = 0; i < mapSize; i++)//遍历各位图块
//	{
//		for (int u = 0; u < 1024; u++)
//		{
//			for (int t = 0; t < 8; t++)//逐位查找
//			{
//				if (!((*bmap)[u] & (bitQuick[0] >> t)))//如果该位为0
//				{
//					(*bmap)[u] |= (bitQuick[0] >> t);//把该位设置为1
//					assignFile_id[curAssigned] = i * (1024 * 8) + u * 8 + t;//记录该位的位置
//					curAssigned++;
//					if (curAssigned == assignSize)//满足条件，结束
//					{
//						return assignFile_id;
//					}
//				}
//			}
//		}
//	}
//	
//	throw("error 资源不足，不可分配");
//	return nullptr;//资源不足，不可分配
//}

/*
void RmFromBitMap(bool bmType, int* rmList, int rmSize)
{
	std::fstream f("DISK", std::ios::binary);
	char* p = new char();
	if (bmType)//文件位图
	{
		for (int i = 0; i < rmSize; i++)
		{
			int byte_id = rmList[i] / 8;
			int offset = rmList[i] % 8;
			f.seekg(fileBmStart * 1024 + byte_id);
			f.read(p, 1);
			*p &= ~(bitQuick[0] >> offset);//设置该位为0
			f.seekp(fileBmStart * 1024 + byte_id);
			f.write(p, 1);
		}
	}
	else
	{
		for (int i = 0; i < rmSize; i++)
		{
			int byte_id = rmList[i] / 8;
			int offset = rmList[i] % 8;
			f.seekg(inodeBmStart * 1024 + byte_id);
			f.read(p, 1);
			*p &= ~(bitQuick[0] >> offset);//设置该位为0
			f.seekp(inodeBmStart * 1024 + byte_id);
			f.write(p, 1);
		}
	}
	f.close();
}
*/