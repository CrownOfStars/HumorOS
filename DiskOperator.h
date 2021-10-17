#pragma once
#include"constDefiner.h"
#include<sstream>

struct dirFile;
struct inodeMap;

template<typename T>
void virtualDiskReader(T* pointer, int filetype, int offset)
{
	char* tempCh = new char[sizeof(T)]{ 0 };
	std::ifstream f("DISK", std::ios::in | std::ios::binary);
	f.seekg(filetype * 1024 + offset * sizeof(T));
	f.read(tempCh, sizeof(T));
	memcpy_s(pointer, sizeof(T), tempCh, sizeof(T));
	f.close();
	delete[] tempCh;
}

template<typename T>
void virtualDiskWriter(T* pointer, int filetype, int offset)
{
	char* tempCh = new char[sizeof(T)]{ 0 };
	std::fstream f("DISK", std::ios::in | std::ios::out | std::ios::binary);
	f.seekp(filetype * 1024 + offset * sizeof(T));
	memcpy_s(tempCh, sizeof(T), pointer, sizeof(T));
	f.write(tempCh, sizeof(T));
	f.close();
	delete[] tempCh;
}

struct inode
{
	void init(bool Type);
	long long createTime = 0ll;//文件创建时间：unix时间戳 8byte
	int mode_uid = 0;//文件属性以及文件所属者 4byte
	int fileSize = 0;//实际可用的文件数量 4byte
	int file_id[10]{ 0 }; //10*4byte 包含320个文件夹或文件
	int singleIndex = 0;//一级文件索引
	int doubleIndex = 0;//二级文件索引
	int Id(int idx) {
		if (idx < 10)
		{
			return file_id[idx];
		}
		else if (idx < 10 + 256)
		{
			File f;
			virtualDiskReader<File>(&f, fileStart, singleIndex);
			int* id = (int*)(f + idx - 10);
			return *id;
		}
		else
		{
			throw("ERROR");
			return -1;
		}
	}
	void addFile(int Id) {
		for (int i = 0; i < 10; i++)
		{
			if (file_id[i] == 0)
			{
				file_id[i] = Id;
				return;
			}
		}
		if (singleIndex != 0)
		{
			File f;
			virtualDiskReader<File>(&f, fileStart, singleIndex);
			for (int i = 0; i < 1024; i += 4)
			{
				int* id = (int*)((char*)f + i);
				if (*id == 0)
				{
					*id = Id;
					return;
				}
			}
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
			File f;
			virtualDiskReader<File>(&f, fileStart, singleIndex);
			for (int i = 0; i < 1024; i += 4)
			{
				int* id = (int*)((char*)f + i);
				if (*id == 0)
				{
					return 10 + i / 4;
				}
			}
			throw("ERROR 鸽");//TODO:
			return 266;
		}
		return 10;
	}
	dirFile* dir();
	char* getFile();
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

inode* curInode = new inode();

int curInodeOffset = 0;

dirFile* curDir = nullptr;

int curDirSize = 1;

void printInodeInfo(char* fileName, int inode_id)
{
	inode* inodep = new inode{ 0 };
	if (fileName[0] == '/')
	{
		std::cout << std::setiosflags(std::ios::left) << std::setw(6) << "dir";
		virtualDiskReader<inode>(inodep, inodeStart, inode_id);
		std::cout << std::setw(12) << dec2hex(inodep->file_id[0]);
		//文件索引分配，以第一个文件块作为物理地址
		std::cout << std::setw(7) << inodep->mode_uid;
		std::cout << std::setw(9) << "--";
		std::cout << std::setw(30) << fileName + 1 << std::endl;
	}
	else
	{
		std::cout << std::setiosflags(std::ios::left) << std::setw(6) << "file";
		virtualDiskReader<inode>(inodep, inodeStart, inode_id);
		std::cout << std::setw(12) << dec2hex(inodep->file_id[0]);
		//文件索引分配，以第一个文件块作为物理地址
		std::cout << std::setw(7) << inodep->mode_uid;
		std::cout << std::setw(9) << inodep->fileSize;
		std::cout << std::setw(30) << fileName << std::endl;
	}
	delete inodep;
	return;
}


void PrintDFS(int blankNum) {
	dirFile* pdirF = curInode->dir();
	for (int i = 0; i < curInode->dirSize(); i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (pdirF[i].aMap[u].Name[0] == '/')
			{
				inode* p = curInode;
				virtualDiskReader<inode>(curInode, inodeStart, pdirF[i].aMap[u].inodeId);
				std::cout << std::string(blankNum * 4, ' ');
				printInodeInfo(pdirF[i].aMap[u].Name, pdirF[i].aMap[u].inodeId);
				PrintDFS(blankNum + 1);
				curInode = p;
			}
			else if (pdirF[i].aMap[u].Name[0] == '-')
			{
				continue;
			}
			else if (pdirF[i].aMap[u].Name[0])
			{
				std::cout << std::string(blankNum * 4, ' ');
				printInodeInfo(pdirF[i].aMap[u].Name, pdirF[i].aMap[u].inodeId);
				return;
			}
			else
			{
				return;
			}
		}
	}
}

void addRm(bool Type, int n)
{
	if (Type)
	{
		if (rmFileNum < rmFileCap)
		{
			rmFileids[rmFileNum] = n;
		}
		else
		{
			int* temp = rmFileids;
			rmFileids = new int[rmFileCap + 10];
			memcpy_s(rmFileids, rmFileCap + 10, temp, rmFileCap);
			delete[] temp;
			rmFileCap += 10;
		}
		rmFileNum += 1;
		return;
	}
	else
	{
		if (rmInodeNum < rmInodeCap)
		{
			rmInodeids[rmInodeNum] = n;
		}
		else
		{
			int* temp = rmInodeids;
			rmInodeids = new int[rmInodeCap + 10];
			memcpy_s(rmInodeids, rmInodeCap + 10, temp, rmInodeCap);
			delete[] temp;
			rmInodeCap += 10;
		}
		rmInodeNum += 1;
		return;
	}
}

void runRm()//针对具体文件块和文件位图，放在最后一起执行
{
	char* bmap = new char();
	for (int i = 0; i < rmFileNum; i++)
	{
		virtualDiskReader<char>(bmap, fileBmStart, rmFileids[i] / 8);
		*bmap &= ~(bitQuick[0] >> (rmFileids[i] % 8));
		virtualDiskWriter<char>(bmap, fileBmStart, rmFileids[i] / 8);
	}
	int* p = rmFileids;
	rmFileids = new int[10];
	rmFileCap = 10;
	rmFileNum = 0;
	delete[] p;

	for (int i = 0; i < rmInodeNum; i++)
	{
		virtualDiskReader<char>(bmap, inodeBmStart, rmInodeids[i] / 8);
		*bmap &= ~(bitQuick[0] >> (rmInodeids[i] % 8));
		virtualDiskWriter<char>(bmap, inodeBmStart, rmInodeids[i] / 8);
	}
	int* pi = rmInodeids;
	rmInodeids = new int[10];
	rmInodeCap = 10;
	rmInodeNum = 0;
	delete[] pi;
	delete bmap;
}

void RemoveDFS(inode* inodep, dirFile* pdirF, int dirSize)
{
	for (int i = 0; i < dirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (pdirF[i].aMap[u].Name[0] == '/')
			{
				inode* p = new inode();///
				virtualDiskReader<inode>(p, inodeStart, pdirF[i].aMap[u].inodeId);
				dirFile* pdir = p->dir();

				addRm(inodeType, pdirF[i].aMap[u].inodeId);
				RemoveDFS(p, pdir, p->dirSize());//递归移除子文件夹
				memcpy_s(curInode, sizeof(inode), p, sizeof(inode));//回到父文件夹
				delete pdir;
				for (int i = 0; i < p->dirSize(); i++)
				{
					addRm(fileType, p->Id(i));//添加这个文件夹的移除
				}
				delete p;
			}
			else if (pdirF[i].aMap[u].Name[0] == '-')
			{
				continue;
			}
			else if (pdirF[i].aMap[u].Name[0])
			{
				inode* p = new inode();///直接删除文件，无需递归操作
				virtualDiskReader<inode>(p, inodeStart, pdirF[i].aMap[u].inodeId);
				addRm(inodeType, pdirF[i].aMap[u].inodeId);
				for (int i = 0; i < p->dirSize(); i++)
				{
					addRm(fileType, p->Id(i));//添加这个文件的移除
				}
				delete p;
				return;
			}
			else
			{
				return;
			}
		}
	}
}

struct SuperBlock
{
	int fileMappingRegion[2]{ 0 };//8byte 文件名和文件inode的映射
	int iNodeRegion[2]{ 0 };//i结点区域
	int inodeBitMapRegion[2]{ 0 };//i结点位图区
	int fileBitMapRegion[2]{ 0 };//文件位图区
	int fileBlockRegion[2]{ 0 };//文件区域
	int lastFreeFile = 0;//表示第一个空闲文件位图所在的Byte
	int lastFreeInode = 0;//表示第一个空闲inode位图所在的Byte

};

int FillInodeBitmap()
{
	char* p = new char();
	int file_id = 0;
	while (true)
	{
		virtualDiskReader<char>(p, inodeBmStart, lastFreeInode);
		for (int i = 0; i < 8; i++)
		{
			if (!(*p & (bitQuick[0] >> i)))
			{
				*p |= (bitQuick[0] >> i);
				virtualDiskWriter<char>(p, inodeBmStart, lastFreeInode);
				file_id = lastFreeInode * 8 + i;
				if (i == 7)
				{
					lastFreeInode += 1;
				}
				delete p;
				return file_id;
			}
		}
		lastFreeInode += 1;
	}
	throw("error");
	return -1;
}

int* FillFileBitmap(int fileSize)
{
	char* p = new char();
	int idx = 0;
	int* file_id = new int[fileSize];
	while (true)
	{
		virtualDiskReader<char>(p, fileBmStart, lastFreeFile);
		for (int i = 0; i < 8; i++)
		{
			if (!(*p & (bitQuick[0] >> i)))
			{
				*p |= (bitQuick[0] >> i);
				file_id[idx] = lastFreeFile * 8 + i;
				idx += 1;
			}
			if (fileSize == idx)
			{
				virtualDiskWriter<char>(p, fileBmStart, lastFreeFile);
				if (i == 7)
				{
					lastFreeFile += 1;
				}
				delete p;
				return file_id;
			}
		}
		virtualDiskWriter<char>(p, fileBmStart, lastFreeFile);
		lastFreeFile += 1;
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


void PrintBitmap(int bmid, int type = fileBmStart)
{
	File fi;
	virtualDiskReader<File>(&fi, type, bmid);
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

void inode::init(bool Type)
{
	if (Type)//文件类型
	{
		std::ifstream in("cache", std::ios::binary);
		std::istreambuf_iterator<char>beg(in), end;
		std::string strdata(beg, end);
		in.close();
		size_t m = strdata.size() / 1024;
		File f;
		char* src = new char[(m + 1) * 1024];
		fileSize = int(strdata.size());
		memset(src, 0, (m + 1) * 1024);//初始化为空
		memcpy_s(src, (m + 1) * 1024, strdata.c_str(), strdata.size());
		//规范为1024的倍数个字节
		int* fileId = FillFileBitmap(int(m + 1));//从文件位图中分配m+1个序号给文件
		for (int i = 0; i < m + 1; i++)//依次写入磁盘中
		{
			file_id[i] = fileId[i];
			memcpy_s(&f, 1024, src + (i * 1024), 1024);
			virtualDiskWriter<File>(&f, fileStart, fileId[i]);
		}

		delete[] src;
		delete[] fileId;
		time_t timep;
		time(&timep);
		createTime = timep;
	}
	else//文件夹
	{
		//只有一个目录文件,直接分配即可
		int* fileId = FillFileBitmap(1);
		file_id[0] = fileId[0];//更新inode的文件块号
		File f;
		memset(f, 0, 1024);
		virtualDiskWriter<File>(&f, fileStart, fileId[0]);//初始化为空目录
		delete[] fileId;
		time_t timep;
		time(&timep);
		createTime = timep;
	}
}

dirFile* inode::dir() {
	dirFile* dirFp = new dirFile[dirSize()];
	for (int u = 0; u < dirSize(); u++)
	{
		virtualDiskReader<dirFile>(dirFp + u, fileStart, file_id[u]);
	}
	return dirFp;
}

char* inode::getFile()
{
	File file;
	char* filep = new char[dirSize() * 1024];
	for (int u = 0; u < dirSize(); u++)
	{
		virtualDiskReader<File>(&file, fileStart, file_id[u]);
		memcpy_s(filep + u * 1024, 1024, file, 1024);
	}
	return filep;
}
