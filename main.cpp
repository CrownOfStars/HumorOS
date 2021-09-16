#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <bitset>
#include <cstdio>  
#include <fstream>  
#include <vector>
#include <cmath>
#include <string>  
#include <cstring>
#include <map>
#include<time.h>
#include"Python.h"
#include"bit_quick.h"

using namespace std;

typedef char Byte;

using File = char[1024];

constexpr int iNodeNum = 16 * 1024;
constexpr int blockNum = 100 * 1024;

File* disk = nullptr;
File* inode_head = nullptr;
File* fileBitmap_head = nullptr;
File* inodeBitmap_head = nullptr;
File* file_head = nullptr;

vector<string> argv;

string curPath = "/home/";

// 超级块位于第一个块 size:1k
constexpr int inodeSize = 1024;//i结点区大小为 16*1024*64/1024 1024k
constexpr int blockMapStart = 1025;//位图区起始点
constexpr int blockBitmapSize = 13;//文件块位图 100*1024/(1024*8) 12.5k ~ 13k
constexpr int inodeMapStart = 1038;
constexpr int inodeBitmapSize = 2;//i结点位图 16*1024/（1024*8） 2k
constexpr int fileStart = 1040;
constexpr int fileSize = 101360;//dir file region 100*1024 - 1040

struct inode
{
	long long createTime = 0;//文件创建时间：unix时间戳 8byte
	int mode_uid = 0;//文件属性以及文件所属者 4byte
	int fileSize = 0;//文件大小 4byte
	//文件索引指针
	int primaryIndex[10]{ 0 }; //10*4byte 包含320个文件夹或文件
	int singleIndex = 0;
	int doubleIndex = 0;
};//sum: 64 byte

struct inodeMap
{
	char Name[28]{ 0 };//文件/文件夹名称长度<27个字节
	int inodeIndex = 0;//i结点的序号
};//sum: 32 byte

struct dir
{
	inodeMap aMap[32];//一个目录包含1024/32=32个文件-i结点/文件夹-i结点映射
};//sum: 1k

struct BitMap
{
	unsigned bitUnit[256];//定义为unsigned数组，方便进行逻辑右移操作
};

struct SuperBlock
{
	int superBlockRegion[2]{ 0 };//8byte
	int fileMappingRegion[2]{ 0 };//8byte 文件名和文件inode的映射
	int iNodeRegion[2]{ 0 };//8byte
	int fileBlockRegion[2]{ 0 };//8byte
	int currentFreeInodePos = 0;//4byte
	Byte freeMem[1024 - 36]{ 0 };
};

SuperBlock supBlock;

map<string, int(*)()> mFuncPtr;

dir* curDir = nullptr;
inode* curDirInode = nullptr;

void SetBitmap(File* region,int bitmap_id, bool occupy) {
	BitMap b{ 0 };
	int disk_id = bitmap_id / (1024 * 8);
	int disk_offset = bitmap_id % (1024 * 8);
	memcpy_s(&b, 1024, region[disk_id], 1024);
	int i = disk_offset / 32;//unit的编号
	int offset = disk_offset % 32;
	b.bitUnit[i] = b.bitUnit[i] | (bit_quick[0] >> offset);
	memcpy_s(region[disk_id], 1024, &b, 1024);
	/*
	int i = pos / 32;
	int j = pos % 32;
	int value = 0;
	a[i] &= (~value ^ (1 << j));
	*/
}

int FirstFit(BitMap* bm,bool cover = false)
{
	for (int u = 0; u < 256; u++)
	{
		for (int t = 0; t < 32; t++)
		{
			if (!(bm->bitUnit[u] & (bit_quick[0] >> t)))
			{
				return t;
			}
		}
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
	throw("bitmap fit error");
	return -1;
}

int SearchInBitmap(File* region,int pos)
{
	BitMap bm{ 0 };
	memcpy_s(&bm, 1024, region[pos], 1024);
	int idx = FirstFit(&bm, true);
	if (idx >= 0)
	{
		return idx;
	}
	return -1;
}

int SetInode(File* region, inode* inodep)
{
	int pos = SearchInBitmap(inodeBitmap_head,0);
	SetBitmap(region,pos,true);
	int offset = pos % 16;
	memcpy_s(region[pos / 16] + offset * 64, 64, inodep, 64);
	return pos;
}

void RmInode(File* region,int inode_id)
{
	inode emp{ 0 };
	int offset = inode_id % 16;
	memcpy_s(disk[1 + inode_id / 16] + offset * 64, 64, &emp, 64);
	SetBitmap(region,inode_id, false);
}

string ParseDir(int dir_id)
{
	string value = "";
	dir tempDir{ 0 };
	memcpy_s(&tempDir, 1024, disk[dir_id], 1024);
	for (int i = 0; i < 32; i++)
	{
		if (tempDir.aMap[i].Name[0])
		{
			value += "\n";
			value += tempDir.aMap[i].Name;
			value += "\t";
			value += to_string(tempDir.aMap[i].inodeIndex);
		}
		else
		{
			break;
		}
	}
	return value;
}

int SearchInIndex(int inode_id)
{
	return 0;
}

int getInodeId(inode* cur)
{
	if (cur->fileSize < 1024 * 10)
	{
		return cur->primaryIndex[cur->fileSize / 1024];
	}
	else if (cur->fileSize < 1024 * 10 + 1024 / 4 * 1024)
	{
		return SearchInIndex(cur->singleIndex);
	}
	else
	{
		return 0;//SearchInIndex();
	}
}

//void addSubDir(int block_id,string fname,int dirInode)
//{
//	for (int i = 0; i < maxFileinDir; i++) {
//		if(!*(disk[block_id] + 32 * i))
//		{
//			strcpy(disk[block_id] + 32 * i,fname.c_str());
//			//int 的数值写入到char中
//			break;
//		}
//	}
//}

string ParseInodeFile(int inode_id)
{
	string value;
	inode inodetemp{ 0 };
	memcpy_s(&inodetemp, 1024, disk[inode_id], 1024);
	for (int i = 0; i < 16; i++)
	{
		if (!inodetemp.createTime)
		{
			value += to_string(inodetemp.createTime);
			value += "\t";
			value += to_string(inodetemp.mode_uid);
			value += "\n";
			for (int u = 0; u < 10; u++)
			{
				value += to_string(inodetemp.primaryIndex[u]);
				value += "\n";
			}
		}
	}
	return value;
}

string ParseBitmap(int bitmap_id)
{
	string value;
	BitMap bm{ 0 };
	memcpy_s(&bm, 1024, disk[bitmap_id], 1024);
	for (int i = 0; i < 32; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (bm.bitUnit[i] & (bit_quick[u] >> u))
			{
				value += "\n";
				value += to_string(i * 32 + u);
			}
		}
	}
	return value;
}

int Info()
{
	cout << ("this is info command.\n");
	return 0;
}

int Cd()
{
	cout << ("this is cd command.\n");
	return 0;
}

int Dir()
{
	curDirInode;
	string dirInfo = "";
	for (int i = 0; i < 10; i++)
	{
		dirInfo = ParseDir(curDirInode->primaryIndex[i]);
	}
	cout << (dirInfo.c_str());
	return NULL;
}

bool CheckRepeatName()
{
	return false;
	for (int i = 0; i < 10; i++)
	{
		dir d{ 0 };
		//ParseFileAsDir(disk[curDirInode->primaryIndex[i]], &d);
		for (int u = 0; u < 32; u++)
		{
			if (d.aMap[u].Name[0])
			{
				if (strcmp(d.aMap[u].Name, argv[0].c_str()) == 0)
				{
					return true;
				}
			}
			else
			{
				return false;
			}
		}
	}
	//more index
	return false;
}

int FirstFit()
{
	return 0;
	//search in bitmap
}

int MkDir()
{
	if (CheckRepeatName())
	{
		cout << ("file already exists\n");
	}
	//assign a inode from bitmap
	for (int i = 0; i < 32; i++)
	{
		if (!curDir->aMap[i].Name[0])
		{
			strcpy(curDir->aMap[i].Name, argv[0].c_str());
			curDir->aMap[i].inodeIndex = 0;//assign inode
		}
	}
	time_t timep;
	struct tm* p = nullptr;
	time(&timep);
	inode newInode;
	newInode.createTime = timep;
	newInode.mode_uid;
	//addSubDir(getInodeId(&curDirInode),argv[0],0);//TODO:
	//curDir
	return NULL;
}

int RmDir()
{
	return NULL;
}

int NewFile()
{
	return 0;
}

int Cat()
{
	return 0;
}

int Copy()
{
	return 0;
}

int Delete()
{
	return 0;
}

int Check()
{
	string cmd;
	while (cin >> cmd)
	{
		if (cmd == "inode")
		{
			cout << "inode file id>";
			int inode_file_id;
			cin >> inode_file_id;
			cout << ParseInodeFile(inode_file_id);
		}
		else if (cmd == "dir")
		{
			cout << "dir file id>";
			int dir_file_id;
			cin >> dir_file_id;
			cout << ParseDir(dir_file_id);
		}
		else if (cmd == "bitmap")
		{
			cout << "bitmap file id>";
			int bitmap_file_id;
			cin >> bitmap_file_id;
			cout << ParseBitmap(bitmap_file_id);
		}
	}

	/*string s;
	cin >> s;
	if (s == "inodebitmap")
	{
		int idx;
		cin >> idx;
		cout << disk[idx];
	}
	else if(s == "filebitmap")
	{
		int idx;
		cin >> idx;
		cout << disk[idx];
	}*/
	for (int i = 0; i < 1024 * 100; i++)
	{
		if (disk[i][0])
		{
			cout << i << '\n';
		}
	}
	return NULL;
}

int Error()
{
	cout << (argv[0].c_str());
	return 0;
}

bool SetDir(int file_id, string dirName, int inode_id)
{
	dir tempdir{ 0 };
	memcpy_s(&tempdir, 1024, file_head[file_id], 1024);
	for (int i = 0; i < 32; i++)
	{
		if (!tempdir.aMap[i].Name[0])
		{
			strcpy_s(tempdir.aMap[i].Name, dirName.c_str());
			tempdir.aMap[i].inodeIndex = inode_id;
			memcpy_s(file_head[file_id], 1024, &tempdir, 1024);
			return true;
		}
	}
	return false;
}

void InitRootPath()
{
	//homeInode 是inode区的第一个inode
	inode homeInode{ 0 };
	homeInode.createTime = 1492244880;
	//home dir 是文件区的第一个dir file
	SetDir(SetInode(inode_head, &homeInode), "home", 1);//ERROR
	//SetFileBitmap(0, 0, true);
}

void InitCmdMapping()
{
	mFuncPtr.insert(make_pair<string, int()>("info", Info));
	mFuncPtr.insert(make_pair<string, int()>("cd", Cd));
	mFuncPtr.insert(make_pair<string, int()>("dir", Dir));
	mFuncPtr.insert(make_pair<string, int()>("md", MkDir));
	mFuncPtr.insert(make_pair<string, int()>("rd", MkDir));
	mFuncPtr.insert(make_pair<string, int()>("newfile", NewFile));
	mFuncPtr.insert(make_pair<string, int()>("cat", Cat));
	mFuncPtr.insert(make_pair<string, int()>("copy", Copy));
	mFuncPtr.insert(make_pair<string, int()>("del", Delete));
	mFuncPtr.insert(make_pair<string, int()>("check", Check));
	mFuncPtr.insert(make_pair<string, int()>("error", Error));
}

void InitPython()
{
	Py_SetPythonHome(L"D:\\Anaconda3");
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");
}

void InitDisk()
{
	disk = new File[blockNum];
	if (true)//_access("DISK", 0))
	{
		fstream fdisk("DISK", ios::binary | ios::out);
		memcpy_s(disk[0], 1024, &supBlock, 1024);
		for (int i = 1; i < blockNum; i++)
		{
			memset(disk[i], 0, 1024);
		}
		inode_head = &disk[1];
		fileBitmap_head = &disk[1025];
		inodeBitmap_head = &disk[1038];
		file_head = &disk[1040];
		memcpy_s(inodeBitmap_head,4,&bit_quick[0],4);
		InitRootPath();
		for (int i = 0; i < blockNum; i++)
		{
			fdisk.write(disk[i], 1024);
		}
		fdisk.close();
	}
	else
	{
		fstream fdisk("DISK", ios::binary | ios::in);
		fdisk.read(disk[0], 1024);
		memcpy_s(&supBlock, 1024, disk[0], 1024);
		/*
		parseSuperBlock
		*/
		for (int i = 1; i < blockNum; i++)
		{
			fdisk.read(disk[i], 1024);
		}
		fdisk.close();
	}
}

/*
File fp{0};
void* p = &fp;
dir* dp = (dir*)p;
*/

int main()
{
	InitPython();
	InitCmdMapping();
	InitDisk();

	PyObject* pModule = NULL;
	PyObject* pFunc = NULL;
	PyObject* pRet = NULL;
	PyObject* pList = NULL;

	//加载python脚本

	pModule = PyImport_ImportModule("Pack");
	pFunc = PyObject_GetAttrString(pModule, "Func");
	string cmd;
	while (true)
	{
		cout << curPath << ">";
		pRet = PyEval_CallObject(pFunc, NULL);
		cmd = string(_PyUnicode_AsString(PyTuple_GetItem(pRet, 0)));
		pList = PyTuple_GetItem(pRet, 1);
		string str_item;//c类型的列表元素
		for (vector <string>::iterator iter = argv.begin(); iter != argv.end();)
		{
			iter = argv.erase(iter);
		}
		for (int i = 0; i < PyObject_Size(pList); i++)
		{
			argv.push_back(string(_PyUnicode_AsString(PyList_GetItem(pList, i))));
		}
		mFuncPtr[cmd]();
	}
	Py_Finalize();
	for (int i = 0; i < 102400; i++)
	{
		delete[] &disk[i];
	}
	return 0;
}