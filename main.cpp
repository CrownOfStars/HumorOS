#include <iostream>
#include <bitset>
#include <cstdio>  
#include <fstream> 
#include <cmath>
#include <cstring>
#include <map>
#include <time.h>
#include "Python.h"
#include "disk_operator.h"


SuperBlock supBlock;

std::map<std::string, int(*)()> mFuncPtr;


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

//void addSubDir(int block_id,std::string fname,int dirInode)
//{
//	for (int i = 0; i < maxFileinDir; i++) {
//		if(!*(disk[block_id] + 32 * i))
//		{
//			strcpy(disk[block_id] + 32 * i,fname.c_str());
//			//int ����ֵд�뵽char��
//			break;
//		}
//	}
//}

int Info()
{
	std::cout << ("this is info command.\n");
	return 0;
}

int Cd()
{
	std::cout << ("this is cd command.\n");
	return 0;
}

int Dir()
{
	curDirInode;
	std::string dirInfo = "";
	bool endDir = false;
	for (int i = 0; i < 10; i++)
	{
		dirInfo = ParseDir(curDirInode->primaryIndex[i],endDir);
		if (endDir)
		{
			std::cout << dirInfo.c_str();
			return NULL;
		}
	}
	//TODO: Search in
	//curDirInode->singleIndex;
	//curDirInode->doubleIndex;
	std::cout << (dirInfo.c_str());
	return NULL;
}

bool CheckRepeatName(bool cover = false)
{
	return false;
	for (int i = 0; i < 10; i++)
	{
		dirFile d{ 0 };
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
				if (cover) {
					strcpy_s(d.aMap[u].Name, argv[0].c_str());
				}
				return false;
			}
		}
	}
	//more index
	return false;
}

int MkDir()
{
	if (CheckRepeatName(true))
	{
		std::cout << ("file already exists\n");
	}
	inode newInode{ 0 };
	time_t timep;
	time(&timep);
	newInode.createTime = timep;
	newInode.mode_uid;//TODO
	LinkDir(AddDir(argv[0],AssignInode(&newInode)));//TODO
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
	std::string cmd;
	while (std::cin >> cmd)
	{
		if (cmd == "inode")
		{
			std::cout << "inode file id>";
			int inode_file_id;
			std::cin >> inode_file_id;
			std::cout << ParseInodeFile(inode_file_id);
		}
		else if (cmd == "dir")
		{
			std::cout << "dir file id>";
			int dir_file_id;
			std::cin >> dir_file_id;
			std::cout << ParseDir(dir_file_id,true);
		}
		else if (cmd == "bitmap")
		{
			std::cout << "bitmap file id>";
			int bitmap_file_id;
			std::cin >> bitmap_file_id;
			//std::cout << ParseBitmap(bitmap_file_id);
		}
	}

	/*std::string s;
	cin >> s;
	if (s == "inodebitmap")
	{
		int idx;
		cin >> idx;
		std::cout << disk[idx];
	}
	else if(s == "filebitmap")
	{
		int idx;
		cin >> idx;
		std::cout << disk[idx];
	}*/
	for (int i = 0; i < 1024 * 100; i++)
	{
		if (disk[i][0])
		{
			std::cout << i << '\n';
		}
	}
	return NULL;
}

int Error()
{
	std::cout << (argv[0].c_str());
	return 0;
}

void InitRootPath()
{
	//homeInode ��inode���ĵ�һ��inode
	inode homeInode{ 0 };
	homeInode.createTime = 1492244880;
	//home dir ���ļ����ĵ�һ��dir file
	AddDir("home", AssignInode(&homeInode));
	memcpy_s(curDirInode, 64,&homeInode, 64);
}

void InitCmdMapping()
{
	mFuncPtr.insert(std::make_pair<std::string, int()>("info", Info));
	mFuncPtr.insert(std::make_pair<std::string, int()>("cd", Cd));
	mFuncPtr.insert(std::make_pair<std::string, int()>("dir", Dir));
	mFuncPtr.insert(std::make_pair<std::string, int()>("md", MkDir));
	mFuncPtr.insert(std::make_pair<std::string, int()>("rd", MkDir));
	mFuncPtr.insert(std::make_pair<std::string, int()>("newfile", NewFile));
	mFuncPtr.insert(std::make_pair<std::string, int()>("cat", Cat));
	mFuncPtr.insert(std::make_pair<std::string, int()>("copy", Copy));
	mFuncPtr.insert(std::make_pair<std::string, int()>("del", Delete));
	mFuncPtr.insert(std::make_pair<std::string, int()>("check", Check));
	mFuncPtr.insert(std::make_pair<std::string, int()>("error", Error));
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
		std::fstream fdisk("DISK", std::ios::binary | std::ios::out);
		memcpy_s(disk[0], 1024, (File*)&supBlock, 1024);
		for (int i = 1; i < blockNum; i++)
		{
			memset(disk[i], 0, 1024);
		}
		inode_head = &disk[1];
		fileBitmap_head = &disk[1025];
		inodeBitmap_head = &disk[1038];
		file_head = &disk[1040];
		memcpy_s(inodeBitmap_head,1, &abit_quick[0], 1);
		memcpy_s(fileBitmap_head, 1, &abit_quick[0], 1);
		InitRootPath();
		for (int i = 0; i < blockNum; i++)
		{
			fdisk.write(disk[i], 1024);
		}
		fdisk.close();
		curPath.push_back("home");
	}
	else
	{
		std::fstream fdisk("DISK", std::ios::binary | std::ios::in);
		fdisk.read(disk[0], 1024);
		memcpy_s(&supBlock, 1024, (SuperBlock*)disk[0], 1024);
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

void ShowCurPath()
{
	for (int u = 0; u < curPath.size(); u++)
	{
		std::cout << '/' << curPath[u];
	}
	std::cout << "/>";
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

	//����python�ű�

	pModule = PyImport_ImportModule("Pack");
	pFunc = PyObject_GetAttrString(pModule, "Func");
	std::string cmd;
	while (true)
	{
		ShowCurPath();
		pRet = PyEval_CallObject(pFunc, NULL);
		cmd = std::string(_PyUnicode_AsString(PyTuple_GetItem(pRet, 0)));
		pList = PyTuple_GetItem(pRet, 1);
		std::string str_item;//c���͵��б�Ԫ��
		for (std::vector <std::string>::iterator iter = argv.begin(); iter != argv.end();)
		{
			iter = argv.erase(iter);
		}
		for (int i = 0; i < PyObject_Size(pList); i++)
		{
			argv.push_back(std::string(_PyUnicode_AsString(PyList_GetItem(pList, i))));
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