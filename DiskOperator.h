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
	long long createTime = 0ll;//�ļ�����ʱ�䣺unixʱ��� 8byte
	int mode_uid = 0;//�ļ������Լ��ļ������� 4byte
	int fileSize = 0;//ʵ�ʿ��õ��ļ����� 4byte
	int file_id[10]{ 0 }; //10*4byte ����320���ļ��л��ļ�
	int singleIndex = 0;//һ���ļ�����
	int doubleIndex = 0;//�����ļ�����
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
			throw("ERROR ��");//TODO:
			return 266;
		}
		return 10;
	}
	dirFile* dir();
	char* getFile();
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
		//�ļ��������䣬�Ե�һ���ļ�����Ϊ�����ַ
		std::cout << std::setw(7) << inodep->mode_uid;
		std::cout << std::setw(9) << "--";
		std::cout << std::setw(30) << fileName + 1 << std::endl;
	}
	else
	{
		std::cout << std::setiosflags(std::ios::left) << std::setw(6) << "file";
		virtualDiskReader<inode>(inodep, inodeStart, inode_id);
		std::cout << std::setw(12) << dec2hex(inodep->file_id[0]);
		//�ļ��������䣬�Ե�һ���ļ�����Ϊ�����ַ
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

void runRm()//��Ծ����ļ�����ļ�λͼ���������һ��ִ��
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
				RemoveDFS(p, pdir, p->dirSize());//�ݹ��Ƴ����ļ���
				memcpy_s(curInode, sizeof(inode), p, sizeof(inode));//�ص����ļ���
				delete pdir;
				for (int i = 0; i < p->dirSize(); i++)
				{
					addRm(fileType, p->Id(i));//�������ļ��е��Ƴ�
				}
				delete p;
			}
			else if (pdirF[i].aMap[u].Name[0] == '-')
			{
				continue;
			}
			else if (pdirF[i].aMap[u].Name[0])
			{
				inode* p = new inode();///ֱ��ɾ���ļ�������ݹ����
				virtualDiskReader<inode>(p, inodeStart, pdirF[i].aMap[u].inodeId);
				addRm(inodeType, pdirF[i].aMap[u].inodeId);
				for (int i = 0; i < p->dirSize(); i++)
				{
					addRm(fileType, p->Id(i));//�������ļ����Ƴ�
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
	int fileMappingRegion[2]{ 0 };//8byte �ļ������ļ�inode��ӳ��
	int iNodeRegion[2]{ 0 };//i�������
	int inodeBitMapRegion[2]{ 0 };//i���λͼ��
	int fileBitMapRegion[2]{ 0 };//�ļ�λͼ��
	int fileBlockRegion[2]{ 0 };//�ļ�����
	int lastFreeFile = 0;//��ʾ��һ�������ļ�λͼ���ڵ�Byte
	int lastFreeInode = 0;//��ʾ��һ������inodeλͼ���ڵ�Byte

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
	if (Type)//�ļ�����
	{
		std::ifstream in("cache", std::ios::binary);
		std::istreambuf_iterator<char>beg(in), end;
		std::string strdata(beg, end);
		in.close();
		size_t m = strdata.size() / 1024;
		File f;
		char* src = new char[(m + 1) * 1024];
		fileSize = int(strdata.size());
		memset(src, 0, (m + 1) * 1024);//��ʼ��Ϊ��
		memcpy_s(src, (m + 1) * 1024, strdata.c_str(), strdata.size());
		//�淶Ϊ1024�ı������ֽ�
		int* fileId = FillFileBitmap(int(m + 1));//���ļ�λͼ�з���m+1����Ÿ��ļ�
		for (int i = 0; i < m + 1; i++)//����д�������
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
	else//�ļ���
	{
		//ֻ��һ��Ŀ¼�ļ�,ֱ�ӷ��伴��
		int* fileId = FillFileBitmap(1);
		file_id[0] = fileId[0];//����inode���ļ����
		File f;
		memset(f, 0, 1024);
		virtualDiskWriter<File>(&f, fileStart, fileId[0]);//��ʼ��Ϊ��Ŀ¼
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
