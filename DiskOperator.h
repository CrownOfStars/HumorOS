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
	long long createTime = 0ll;//�ļ�����ʱ�䣺unixʱ��� 8byte
	int mode_uid = 0;//�ļ������Լ��ļ������� 4byte
	int fileSize = 0;//�ļ���С 4byte
	int file_id[10]{ 0 }; //10*4byte ����320���ļ��л��ļ�
	int singleIndex = 0;//һ���ļ�����
	int doubleIndex = 0;//�����ļ�����
	int file(int idx) {
		if (idx < 10)
		{
			return file_id[idx];
		}
		else// if (idx < 10 + 256)
		{
			throw("error ��");
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
			throw("��");
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
	char Name[28]{ 0 };//�ļ�/�ļ������Ƴ���<27���ֽ�
	int inodeId = 0;//i�������
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
		throw("error �ռ䲻��");
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
		throw("error �ռ䲻��");
	}
	/* λ������ʾ
	11100000
	10000000 == 1 -��next

	11100000
	01000000 == 1 -��next

	11100000
	00100000 == 1 -��next

	11100000
	00010000 == 0 -��ok
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

void RmInode(int* rmList, int rmSize)//ֻ��Ҫ��λͼ���Ƴ����ɣ�
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
		value += dec2hex(inodep->file_id[0]) + "\t";//�ļ��������䣬�Ե�һ���ļ�����Ϊ�����ַ
		value += std::to_string(inodep->mode_uid) + "\t";
		value += "--\t";
	}
	else
	{
		value += "file ";
		virtualDiskReader<inode>(inodep, inodeStart, 0, inode_id);
		value += dec2hex(inodep->file_id[0]) + "\t";//�ļ��������䣬�Ե�һ���ļ�����Ϊ�����ַ
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

//int* FirstFit(File* bmap, int mapSize, int assignSize)//��������
//{
//	int* assignFile_id = new int[assignSize];//��Ҫ������ļ�id���б�
//	int curAssigned = 0;//��ǰ�ѷ������
//	for (int i = 0; i < mapSize; i++)//������λͼ��
//	{
//		for (int u = 0; u < 1024; u++)
//		{
//			for (int t = 0; t < 8; t++)//��λ����
//			{
//				if (!((*bmap)[u] & (bitQuick[0] >> t)))//�����λΪ0
//				{
//					(*bmap)[u] |= (bitQuick[0] >> t);//�Ѹ�λ����Ϊ1
//					assignFile_id[curAssigned] = i * (1024 * 8) + u * 8 + t;//��¼��λ��λ��
//					curAssigned++;
//					if (curAssigned == assignSize)//��������������
//					{
//						return assignFile_id;
//					}
//				}
//			}
//		}
//	}
//	
//	throw("error ��Դ���㣬���ɷ���");
//	return nullptr;//��Դ���㣬���ɷ���
//}

/*
void RmFromBitMap(bool bmType, int* rmList, int rmSize)
{
	std::fstream f("DISK", std::ios::binary);
	char* p = new char();
	if (bmType)//�ļ�λͼ
	{
		for (int i = 0; i < rmSize; i++)
		{
			int byte_id = rmList[i] / 8;
			int offset = rmList[i] % 8;
			f.seekg(fileBmStart * 1024 + byte_id);
			f.read(p, 1);
			*p &= ~(bitQuick[0] >> offset);//���ø�λΪ0
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
			*p &= ~(bitQuick[0] >> offset);//���ø�λΪ0
			f.seekp(inodeBmStart * 1024 + byte_id);
			f.write(p, 1);
		}
	}
	f.close();
}
*/