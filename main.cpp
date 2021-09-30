#include <iostream>
#include <fstream> 
#include <cstring>
#include <map>
#include <time.h>
#include "Python.h"
#include "DiskOperator.h"

SuperBlock supBlock;
PyObject* argv = nullptr;

std::map<std::string, int(*)()> mFuncPtr;


int SearchInIndex(int inode_id)
{
	return 0;
}

int Info()
{
	std::cout << ("this is info command.\n");
	return 0;
}

int Cd()
{
	if (strcmp("..", _PyUnicode_AsString(argv)) == 0)//回到上一级目录
	{
		if (!pathDeque.empty())//如果路径队列不为空
		{
			pathDeque.pop_back();
			if (!pathDeque.empty())//回到父目录的i结点
			{
				int inode_id = pathDeque.back().second;//为了代码更好看，优化交给编译器
				virtualDiskReader<inode>(curInode, inodeStart, 0, inode_id);
			}
			else//如果回到根结点
			{
				virtualDiskReader<inode>(curInode, inodeStart, 0, 0);//回到根目录的i结点
			}
		}
		return 0;
	}
	else if (strcmp("/", _PyUnicode_AsString(argv)) == 0)//回到根目录
	{
		pathDeque.clear();
		virtualDiskReader<inode>(curInode, inodeStart, 0, 0);//回到根目录的i结点
		return 0;
	}
	dirFile* pdirF = curInode->getDir();
	for (int i = 0; i < curInode->dirSize(); i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (pdirF[i].aMap[u].Name[0] == '/')
			{
				if (strcmp(pdirF[i].aMap[u].Name + 1, _PyUnicode_AsString(argv)) == 0)
				{
					int inode_id = pdirF[i].aMap[u].inodeId; //为了代码更好看，优化交给编译器
					pathDeque.push_back(std::make_pair(_PyUnicode_AsString(argv), inode_id));
					virtualDiskReader<inode>(curInode, inodeStart, 0, inode_id);
					delete pdirF;
					return 0;
				}
			}
			else if (pdirF[i].aMap[u].Name[0])
			{
				continue;//this is a file name
			}
			else
			{
				std::cout << "no such dictionary" << std::endl;
				delete[] pdirF;
				return -1;
			}
		}
	}
	delete[] pdirF;
	return NULL;
}

int Dir()
{
	std::cout << "类型 物理地址 保护码 文件大小\t文件名\n";
	dirFile* pdirF = curInode->getDir();
	for (int i = 0; i < curInode->dirSize(); i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (pdirF->aMap[u].Name[0])
			{
				std::cout << inodeInfo(pdirF->aMap[u].Name[0], pdirF->aMap[u].inodeId);
				std::cout << pdirF->aMap[u].Name + 1 << std::endl;//文件存在,获取文件信息
			}
			else if (!pdirF->aMap[u].Name[0])
			{
				std::cout << "\n";
				return 0;//遍历到未创建过文件的区域，终止遍历
			}
		}
	}
	std::cout << "\n";
	return 0;
}

int MkDir()
{
	dirFile* dirF = curInode->getDir();
	inodeMap* imapp = new inodeMap();
	for (int i = 0; i < curInode->dirSize(); i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (strcmp(dirF[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
			{
				std::cout << "dir already exists\n";
				delete imapp;
				delete dirF;
				return -1;
			}
			else if (dirF[i].aMap[u].Name[0] == '-')//removed dir
			{
				if (!imapp)//把写指针定位到此处
				{
					imapp = dirF[i].aMap+u;
				}
			}
			else if (!dirF[i].aMap[u].Name[0])
			{
				imapp->inodeId = lastFreeInode;
				strcpy_s(imapp->Name, _PyUnicode_AsString(argv));//更新当前目录下的索引表
				AssignInode();//写入到位图中，更新i结点区,为刚刚建立的i结点分配空文件
				//AssignFile();
				curInode->addDir(imapp);
				delete imapp;
				delete[] dirF;
				return 0;
			}
		}
	}
	delete imapp;
	delete[] dirF;
	return 0;
}

int RmDir()
{
	dirFile* pdirF = new dirFile();
	for (int i = 0; i < curInode->fileSize; i++)
	{
		virtualDiskReader<dirFile>(pdirF, fileStart, curInode->file(i), 0);
		for (int u = 0; u < 32; u++)
		{
			if (pdirF->aMap[u].Name[0] == '/')
			{
				if (strcmp(pdirF->aMap[u].Name + 1, _PyUnicode_AsString(argv)) == 0)
				{
					pdirF->aMap[u].Name[0] = '\0';
					if (curInode->fileSize > i * 32 + u)
					{
						curInode->fileSize = i * 32 + u;
					}
					//rm dirinfo from parent dir
					//pdirF->aMap[u].inodeIndex//rm inode info from inodebitmap
					//rm filebitmap
					//TODO: rmfile will only rm inodebitmap and filebitmap
					//TODO: dir command will judge if file exists by bitmap
					//helpful to distingwish between file removed and block unused
				}
			}
			else if (pdirF->aMap[u].Name[0])
			{
				continue;
			}
			else
			{
				std::cout << ("no such dictionary\n");
				return NULL;
			}
		}
	}
	return NULL;
}

int NewFile()
{
	dirFile* pdirF = new dirFile();
	for (int i = 0; i < curInode->fileSize; i++)
	{
		virtualDiskReader<dirFile>(pdirF, fileStart, curInode->file(i), 0);
		for (int u = 0; u < 32; u++)
		{
			if (pdirF->aMap[u].Name[0])
			{
				if (strcmp(pdirF->aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
				{
					std::cout << ("file already exists\n");
					return NULL;
				}
			}
			else
			{
				inode newInode;
				time_t timep;
				time(&timep);
				newInode.createTime = timep;
				newInode.mode_uid;//TODO
				/*
				AssignFile(&newInode);
				pdirF->aMap[u].inodeIndex = AssignInode(&newInode);//TODO
				strcpy_s(pdirF->aMap[u].Name, _PyUnicode_AsString(argv));
				memcpy_s(file_head[curInode->file_id[i]], 1024, &d, 1024);
				*/
				return false;
			}
		}
	}
	return NULL;
}

int Cat()
{
	std::cout << "this is cat command\n";
	return 0;
}

int Copy()
{
	std::cout << "this is copy command\n";
	return 0;
}

int Delete()
{
	//for (int i = 0; i < 10; i++)
	//{
	//	dirFile d{ 0 };
	//	memcpy_s(&d, 1024, file_head[curInode->file_id[i]], 1024);
	//	for (int u = 0; u < 32; u++)
	//	{
	//		if (d.aMap[u].Name[0])
	//		{
	//			if (strcmp(d.aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
	//			{
	//				//new rm method
	//				return NULL;
	//			}
	//		}
	//		else
	//		{
	//			std::cout << "no such file" << std::endl;
	//			return false;
	//		}
	//	}
	//}
	return NULL;
}

int Check()
{
	std::cout << "this is a check command\n";
	return NULL;
}

int Error()
{
	std::cout << _PyUnicode_AsString(argv) << std::endl;
	return 0;
}

void InitCmdMapping()
{
	mFuncPtr.insert(std::make_pair<std::string, int()>("info", Info));
	mFuncPtr.insert(std::make_pair<std::string, int()>("cd", Cd));
	mFuncPtr.insert(std::make_pair<std::string, int()>("dir", Dir));
	mFuncPtr.insert(std::make_pair<std::string, int()>("md", MkDir));
	mFuncPtr.insert(std::make_pair<std::string, int()>("rd", RmDir));
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
	if (true)//_access("DISK", 0))
	{
		File* disk = new File[blockNum];
		std::ofstream fdisk("DISK", std::ios::binary);
		memcpy_s(disk[0], 1024, (File*)&supBlock, 1024);
		for (int i = 1; i < blockNum; i++)
		{
			memset(disk[i], 0, 1024);
		}
		//home dir 是文件区的第一个dir file
		inode homeInode;
		homeInode.createTime = 1492244880ll;
		homeInode.file_id[0] = 1;
		//homeInode 是inode区的第一个inode
		memcpy_s(disk + 1, 1024, &homeInode, sizeof(inode));
		memcpy_s(disk + fileBmStart, 1024, &bitQuick[1], sizeof(char));
		memcpy_s(disk + inodeBmStart, 1024, &bitQuick[1], sizeof(char));
		memcpy_s(curInode,sizeof(inode),&homeInode,sizeof(inode));
		fdisk.write((char*)disk, 1024*1024*100);
		fdisk.close();
		delete[] disk;
	}
	else
	{
		//std::fstream fdisk("DISK", std::ios::binary | std::ios::in);
		// 
		//fdisk.read(disk[0], 1024);
		//memcpy_s(&supBlock, 1024, (SuperBlock*)disk[0], 1024);
		///*
		//parseSuperBlock
		//*/
		//for (int i = 1; i < blockNum; i++)
		//{
		//	fdisk.read(disk[i], 1024);
		//}
		//fdisk.close();
	}
}

void ShowCurPath()
{
	std::cout << userName << "@Humor/";
	for (auto i = pathDeque.begin(); i != pathDeque.end(); i++)
	{
		std::cout << (*i).first << '/';
	}
	std::cout << ">";
}

void ExecuteBat(PyObject* batch)
{
	PyObject* po;
	for (int i = 0; i < PyObject_Size(batch); i++)
	{
		po = PyList_GetItem(batch, i);
		if (PyObject_Size(po) == 1)
		{
			if (mFuncPtr[_PyUnicode_AsString(PyTuple_GetItem(po, 0))]())
			{
				break;
			}
		}
		else if (PyObject_Size(po) == 2)
		{
			argv = PyTuple_GetItem(po, 1);
			if (mFuncPtr[_PyUnicode_AsString(PyTuple_GetItem(po, 0))]())
			{
				break;
			}
		}
	}
}

int main()
{
	InitPython();
	InitCmdMapping();
	InitDisk();

	PyObject* pModule = NULL;
	PyObject* pFunc = NULL;

	//加载python脚本

	pModule = PyImport_ImportModule("Pack");
	pFunc = PyObject_GetAttrString(pModule, "Func");
	while (true)
	{
		ShowCurPath();
		ExecuteBat(PyEval_CallObject(pFunc, NULL));
	}
	Py_Finalize();
	return 0;
}