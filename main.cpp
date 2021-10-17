#include <iostream>
#include <iomanip>
#include <fstream> 
#include <cstring>
#include <map>
#include <time.h>
#include "Python.h"
#include "DiskOperator.h"

SuperBlock supBlock;
PyObject* argv = nullptr;

std::map<std::string, int(*)()> mFuncPtr;

int Info()
{
	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (strcmp(curDir[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
			{
				//输出inode的详细信息
				inode* inodep = new inode();
				virtualDiskReader<inode>(inodep, inodeStart, curDir[i].aMap[u].inodeId);
				std::cout << "创建时间" << inodep->createTime << std::endl;
				std::cout << "保护码" << inodep->mode_uid << std::endl;
				std::cout << "文件大小" << inodep->fileSize << std::endl;
				std::cout << "文件块号:\n";
				for (int i = 0; i < inodep->dirSize(); i++) {
					std::cout << inodep->Id(i) << std::endl;
				}
				delete inodep;
				return 0;
			}
			else if (!curDir[i].aMap[u].Name[0])
			{
				std::cout << "no such file" << std::endl;
				return 0;
			}
		}
	}
	return -1;
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
				int inode_id = pathDeque.back().second;
				virtualDiskReader<inode>(curInode, inodeStart, inode_id);
			}
			else//如果回到根结点
			{
				virtualDiskReader<inode>(curInode, inodeStart, 0);//回到根目录的i结点
			}
		}
		dirFile* p = curDir;
		curDir = curInode->dir();
		curDirSize = curInode->dirSize();
		delete[] p;//更新curDir
		return 0;
	}
	else if (strcmp("/", _PyUnicode_AsString(argv)) == 0)//回到根目录
	{
		pathDeque.clear();
		virtualDiskReader<inode>(curInode, inodeStart, 0);//回到根目录的i结点
		dirFile* p = curDir;
		curDir = curInode->dir();
		curDirSize = curInode->dirSize();
		delete[] p;//更新curDir
		return 0;
	}
	else
	{
		for (int i = 0; i < curDirSize; i++)
		{
			for (int u = 0; u < 32; u++)
			{
				if (strcmp(curDir[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
				{
					int inode_id = curDir[i].aMap[u].inodeId;
					pathDeque.push_back(std::make_pair(curDir[i].aMap[u].Name+1, inode_id));
					virtualDiskReader<inode>(curInode, inodeStart, inode_id);
					dirFile* p = curDir;
					curDir = curInode->dir();
					curDirSize = curInode->dirSize();
					delete[] p;//更新curDir
					return 0;
				}
				else
				{
					std::cout << "no such dictionary" << std::endl;
					return -1;
				}
			}
		}
	}
	return -1;
}

int Dir()
{
	std::cout << "类型  物理地址  保护码 文件大小 文件名" << std::endl;//输出表头

	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (curDir[i].aMap[u].Name[0] == '-')//已经被移除的文件(夹)
			{
				continue;
			}
			else if (!curDir[i].aMap[u].Name[0])//已经遍历到尾部准备写入
			{
				return 0;
			}
			else
			{
				printInodeInfo(curDir[i].aMap[u].Name, curDir[i].aMap[u].inodeId);
			}
		}
	}
	return -1;
}

int Dir_r()
{
	PrintDFS(0);
	return 0;
}

int MkDir()
{
	inodeMap* imapp = nullptr;
	int pos = 0;
	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (strcmp(curDir[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
			{
				std::cout << "dir already exists" << std::endl;
				return 0;
			}
			else if (curDir[i].aMap[u].Name[0] == '-')//被移除的文件夹
			{
				if (!imapp)//把写指针定位到此处
				{
					imapp = curDir[i].aMap + u;
					pos = i;
				}
			}
			else if (!curDir[i].aMap[u].Name[0])//已经遍历到尾部准备写入
			{
				if (!imapp)
				{
					imapp = curDir[i].aMap + u;
					pos = i;
				}

				inode* inodep = new inode{ 0 };
				inodep->init(dir_Inode);

				strcpy_s(imapp->Name, _PyUnicode_AsString(argv));//更新当前目录下的索引表
				imapp->inodeId = FillInodeBitmap();//为该目录分配inode区域以及inode位图号
				//每次修改后再把curDir同步到文件中

				virtualDiskWriter<inode>(inodep, inodeStart, imapp->inodeId);
				//inode写入到磁盘中
				virtualDiskWriter<dirFile>(&curDir[pos],fileStart, curInode->Id(pos));
				//同时更新父目录文件
				delete inodep;
				return 0;
			}
		}
	}
	int fileId = *FillFileBitmap(1);
	curInode->addFile(fileId);
	virtualDiskWriter<inode>(curInode, inodeStart, curInodeOffset);
	curDirSize += 1;
	dirFile* dirp = new dirFile[curDirSize];
	memcpy_s(dirp, size_t(curDirSize) * 1024, curDir, size_t(curDirSize-1) * 1024);
	delete[] curDir;
	curDir = dirp;
	dirFile* newdir = new dirFile();
	strcpy_s(newdir->aMap[0].Name, _PyUnicode_AsString(argv));
	inode* inodep = new inode{ 0 };
	inodep->init(dir_Inode);
	newdir->aMap[0].inodeId = FillInodeBitmap();
	memcpy_s(dirp + size_t(curDirSize - 1), 1024, newdir, 1024);
	virtualDiskWriter<inode>(inodep, inodeStart, newdir->aMap[0].inodeId);
	virtualDiskReader<dirFile>(newdir,fileStart, fileId);
	delete newdir;
	return 0;
}

int RmDir()
{
	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (strcmp(curDir[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
			{
				curDir[i].aMap[u].Name[0] = '-';
				addRm(inodeType, curDir[i].aMap[u].inodeId);//移除该文件夹的inode位图
				
				inode* p = new inode();
				virtualDiskReader<inode>(p, inodeStart, curDir[i].aMap[u].inodeId);
				
				dirFile* pdir = p->dir();
				RemoveDFS(p,pdir,p->dirSize());//递归删除该文件夹下的子目录
				
				for (int i = 0; i < p->dirSize(); i++)
				{//需要删除p本身所指向的file
					addRm(fileType, p->Id(i));
				}
				runRm();//执行删除操作
				
				delete p;
				delete pdir;
				virtualDiskWriter<dirFile>(&curDir[i], fileStart, curInode->Id(i));
				//同时更新父目录文件
				return 0;
			}
			else if (curDir[i].aMap[u].Name[0] == '-')
			{
				continue;
			}
			else if (!curDir[i].aMap[u].Name[0])
			{
				std::cout << "no such dictionary" << std::endl;
				return 0;
			}
		}
	}
	return -1;
}

int NewFile()
{
	inodeMap* imapp = nullptr;
	int pos = 0;
	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (strcmp(curDir[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
			{
				std::cout << "file already exists" << std::endl;
				return 0;
			}
			else if (curDir[i].aMap[u].Name[0] == '-')//removed dir
			{
				if (!imapp)//把写指针定位到此处
				{
					imapp = curDir[i].aMap + u;
					pos = i;
				}
			}
			else if (!curDir[i].aMap[u].Name[0])
			{
				if (!imapp)//把写指针定位到此处
				{
					imapp = curDir[i].aMap + u;
					pos = i;
				}
				inode* inodep = new inode{ 0 };///
				inodep->init(file_Inode);
				strcpy_s(imapp->Name, _PyUnicode_AsString(argv));
				imapp->inodeId = FillInodeBitmap();

				virtualDiskWriter<inode>(inodep, inodeStart, imapp->inodeId);//写入到磁盘中

				virtualDiskWriter<dirFile>(&curDir[pos], fileStart, curInode->Id(pos));
				delete inodep;
				return 0;
			}
		}
	}
	return -1;
}

int Cat()
{
	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			
			if (strcmp(curDir[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
			{
				inode* inodep = new inode();
				virtualDiskReader<inode>(inodep, inodeStart, curDir[i].aMap[u].inodeId);
				char* fileContent = inodep->getFile();
				std::cout << fileContent << std::endl;
				delete[] fileContent;
				return 0;
			}
			else if (!curDir[i].aMap[u].Name[0])
			{
				std::cout << "no such file" << std::endl;
				return 0;
			}
		}
	}
	return -1;
}

int Copy()//把数据复制到cache中
{
	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{

			if (strcmp(curDir[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
			{
				inode* inodep = new inode();
				virtualDiskReader<inode>(inodep, inodeStart, curDir[i].aMap[u].inodeId);
				char* fileContent = inodep->getFile();
				std::ofstream fw("cache", std::ios::binary);
				fw.write(fileContent,size_t(inodep->dirSize())*1024);
				fw.close();
				delete[] fileContent;
				return 0;
			}
			else if (!curDir[i].aMap[u].Name[0])
			{
				std::cout << "no such file" << std::endl;
				return -1;
			}
		}
	}
	return -1;
}

int Delete()
{
	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (strcmp(curDir[i].aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
			{
				curDir[i].aMap[u].Name[0] = '-';
				addRm(inodeType, curDir[i].aMap[u].inodeId);
				inode* inodep = new inode();
				virtualDiskReader<inode>(inodep, inodeStart, curDir[i].aMap[u].inodeId);
				for (int i = 0; i < inodep->dirSize(); i++)
				{
					addRm(fileType, inodep->Id(i));
				}
				runRm();
				delete inodep;
				return 0;
			}
			else if (!curDir[i].aMap[u].Name[0])
			{
				std::cout << "no such file" << std::endl;
				return 0;
			}
		}
	}
	return -1;
}

int Check()
{
	PyEval_CallObject(PyObject_GetAttrString(PyImport_ImportModule("View"), "Func"), NULL);
	return 0;
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
	mFuncPtr.insert(std::make_pair<std::string, int()>("dir-r", Dir_r));
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
	if (true)//_access("DISK", 0))//如果文件不存在
	{
		File* disk = new File[1024 * 100];
		std::ofstream fdisk("DISK", std::ios::binary);
		memcpy_s(disk[0], 1024, (File*)&supBlock, 1024);
		for (int i = 1; i < 1024 * 100; i++)
		{
			memset(disk[i], 0, 1024);
		}
		inode homeInode;//homeInode 是inode区的第一个inode
		homeInode.createTime = 1492244880ll;
		homeInode.file_id[0] = 1;
		memcpy_s(curInode, sizeof(inode), &homeInode, sizeof(inode));//设为当前的inode
		dirFile* homedir = new dirFile();
		curDir = homedir;//设为当前的dir
		memcpy_s(disk + 1, 1024, &homeInode, sizeof(inode));//写入文件
		memcpy_s(disk + fileBmStart, 1024, &bitQuick[1], sizeof(char));
		//homedir占有1号,文件号为0表示该文件不存在
		memcpy_s(disk + inodeBmStart, 1024, &bitQuick[0], sizeof(char));
		//homeinode占有0号

		fdisk.write((char*)disk, 1024 * 1024 * 100);//记录到本地磁盘
		fdisk.close();
		delete[] disk;//清空初始化数据
	}
	else
	{
		virtualDiskReader<SuperBlock>(&supBlock, 0, 0);//读取超级块
		
		/*
		memcpy_s(curInode, sizeof(inode), &homeInode, sizeof(inode));//设为当前的inode

		dirFile* homedir = new dirFile();
		curDir = homedir;//设为当前的dir
		*/
		//fileStart = supBlock.fileBlockRegion[0];
		//inodeStart = supBlock.iNodeRegion[0];
		//TODO:
		//parseSuperBlock
		//*/
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
	//初始化磁盘已经python环境
	PyObject* pModule = NULL;
	PyObject* pFunc = NULL;

	pModule = PyImport_ImportModule("Pack");
	pFunc = PyObject_GetAttrString(pModule, "xFunc");
	//加载命令解释器
	while (true)
	{
		ShowCurPath();
		ExecuteBat(PyEval_CallObject(pFunc, NULL));
	}
	Py_Finalize();
	return 0;
}