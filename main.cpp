#include <iostream>
#include <bitset>
#include <cstdio>  
#include <fstream> 
#include <cmath>
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

int getInodeId(inode* cur)
{
	if (cur->fileSize < 1024 * 10)
	{
		return cur->file_id[cur->fileSize / 1024];
	}
	else if (cur->fileSize < 1024 * 10 + 1024 / 4 * 1024)
	{
		return SearchInIndex(cur->singleIndex_id);
	}
	else
	{
		return 0;//SearchInIndex();
	}
}

int Info()
{
	std::cout << ("this is info command.\n");
	return 0;
}

int Cd()
{
	if (strcmp("..", _PyUnicode_AsString(argv)) == 0)//�ص���һ��Ŀ¼
	{
		if (!pathDeque.empty())//���·�����в�Ϊ��
		{
			pathDeque.pop_back();
			if (!pathDeque.empty())//�ص���Ŀ¼��i���
			{
				int inode_id = pathDeque.back().second;//Ϊ�˴�����ÿ����Ż�����������
				diskPointer<inode*>(0, inode_id, curInode, inode_head);
			}
			else//����ص������
			{
				diskPointer<inode*>(0, 0, curInode, inode_head);//�ص���Ŀ¼��i���
			}
		}
		return 0;
	}
	else if (strcmp("/", _PyUnicode_AsString(argv)) == 0)//�ص���Ŀ¼
	{
		pathDeque.clear();
		diskPointer<inode*>(0, 0, curInode, inode_head);//�ص���Ŀ¼��i���
		return 0;
	}

	dirFile* pdirF = nullptr;
	for (int i = 0; i < 10; i++)
	{
		diskPointer<dirFile*>(curInode->file_id[i], 0, pdirF, file_head);
		for (int u = 0; u < 32; u++)
		{
			if (pdirF->aMap[u].Name[0] == '/')
			{
				if (strcmp(pdirF->aMap[u].Name + 1, _PyUnicode_AsString(argv)) == 0)
				{
					int inode_id = pdirF->aMap[u].inodeIndex; //Ϊ�˴�����ÿ����Ż�����������
					pathDeque.push_back(std::make_pair(_PyUnicode_AsString(argv), inode_id));
					diskPointer<inode*>(0, inode_id, curInode, inode_head);
					return u;
				}
			}
			else if (pdirF->aMap[u].Name[0])
			{
				continue;//this is a file name
			}
			else
			{
				std::cout << "no such dictionary" << std::endl;
				return false;
			}
		}
	}
	throw("�� error");
	return NULL;
}

int Dir()
{
	std::cout << "���� �����ַ ������ �ļ���С\t�ļ���\n";
	dirFile* pdirF = nullptr;
	for (int i = 0; i < 10; i++)
	{
		diskPointer<dirFile*>(curInode->file_id[i], 0, pdirF, file_head);
		for (int u = 0; u < 32; u++)
		{
			if (pdirF->aMap[u].Name && pdirF->aMap[u].inodeIndex != 0)
			{
				std::cout << inodeInfo(pdirF->aMap[u].Name[0], pdirF->aMap[u].inodeIndex);
				std::cout << pdirF->aMap[0].Name + 1 << std::endl;//�ļ�����,��ȡ�ļ���Ϣ
			}
			else if (!pdirF->aMap[u].Name)
			{
				return;//������δ�������ļ���������ֹ����
			}
		}
	}
	throw("error ��");
	//TODO: Search in
	//curInode->singleIndex_id;
	//curInode->doubleIndex;
	return NULL;
}

int MkDir()
{
	//TODO: ���ǵ����������������з��䣬���Բ���ִ�ж���ѭ��,��ָ�������ѭ��������ֹΪֹ
	//�����ļ��е�filesize�����ӵ�һ�����ݿ鿪ʼ���Ϊ0,���������ϱ�����˳�����+1
	if (curInode->fileSize < 10 * 32)
	{
		dirFile* pdirF = nullptr;
		for (int i = 0; i < 10; i++)
		{
			diskPointer<dirFile*>(curInode->file_id[i], 0, pdirF, file_head);
			for (int u = 0; u < 32; u++)
			{
				
			}
		}
		
		int nextFree = -1;
		for (int i = 0; i < 10; i++)
		{
			for (int u = 0; u < 32; u++)
			{
				if (pdirF->aMap[u].inodeIndex > 0)
				{
					if (pdirF->aMap[u].Name[0] == '/')
					{
						if (strcmp(pdirF->aMap[u].Name + 1, _PyUnicode_AsString(argv)) == 0)
						{
							std::cout << ("dir already exists\n");
							return -1;
						}
					}
				}
				else if(pdirF->aMap[u].inodeIndex == -1)
				{
					//ready to find next free pos
				}
				else
				{
					//TODO:��filesize��inode������Ϊ-1,�������ʱ��next filesize
					int disk_id = curInode->fileSize / 10;
					int offset = curInode->fileSize % 10;
					inodeMap* imap = nullptr;
					diskPointer<inodeMap*>(curInode->file_id[disk_id], offset, imap, file_head);
					strcpy_s(imap->Name, _PyUnicode_AsString(argv));
					//imap->inodeIndex = AssignInode()
					//end of dir
					//ready to 
					//ready to refresh next free pos
				}
			}
		}
	}
	else
	{
		throw("error ��");
	}
	//inodeMap* p = nullptr;
	//bool end = false;
	//for (int i = 0; i < 10; i++)
	//{
	//	dirFile d{ 0 };
	//	memcpy_s(&d, 1024, file_head[curInode->file_id[i]], 1024);
	//	for (int u = 0; u < 32; u++)
	//	{
	//		if (d.aMap[u].Name[0] == '/' and d.aMap[u].inodeIndex != 0)
	//		{
	//			if (strcmp(d.aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
	//			{
	//				std::cout << ("dir already exists\n");
	//				return NULL;
	//			}
	//		}
	//		else if (!p)
	//		{
	//			p = (inodeMap*)d.aMap[u].Name[0];
	//		}
	//		if (!d.aMap[u].Name[0])
	//		{
	//			inode newInode{ 0 };
	//			time_t timep;
	//			time(&timep);
	//			newInode.createTime = timep;
	//			newInode.mode_uid;//TODO
	//			AssignFile(&newInode);
	//			d.aMap[u].inodeIndex = AssignInode(&newInode);//TODO
	//			strcpy_s(d.aMap[u].Name, _PyUnicode_AsString(argv));
	//			memcpy_s(file_head[curInode->file_id[i]], 1024, &d, 1024);
	//			return false;
	//		}
	//	}
	//}
	//return NULL;
}

int RmDir()
{
	for (int i = 0; i < 10; i++)
	{
		dirFile d{ 0 };
		memcpy_s(&d, 1024, file_head[curInode->file_id[i]], 1024);
		for (int u = 0; u < 32; u++)
		{
			if (d.aMap[u].Name[0] == '/')
			{
				if (strcmp(d.aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
				{
					d.aMap[u].Name[0] = '\0';//rm dirinfo from parent dir
					//d.aMap[u].inodeIndex//rm inode info from inodebitmap
					//rm filebitmap
					//TODO: rmfile will only rm inodebitmap and filebitmap
					//TODO: dir command will judge if file exists by bitmap
					//helpful to distingwish between file removed and block unused
				}
			}
			else if (d.aMap[u].Name[0])
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
	for (int i = 0; i < 10; i++)
	{
		dirFile d{ 0 };
		memcpy_s(&d, 1024, file_head[curInode->file_id[i]], 1024);
		for (int u = 0; u < 32; u++)
		{
			if (d.aMap[u].Name[0])
			{
				if (strcmp(d.aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
				{
					std::cout << ("file already exists\n");
					return NULL;
				}
			}
			else
			{
				inode newInode{ 0 };
				time_t timep;
				time(&timep);
				newInode.createTime = timep;
				newInode.mode_uid;//TODO
				AssignFile(&newInode);
				d.aMap[u].inodeIndex = AssignInode(&newInode);//TODO
				strcpy_s(d.aMap[u].Name, _PyUnicode_AsString(argv));
				memcpy_s(file_head[curInode->file_id[i]], 1024, &d, 1024);
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
	for (int i = 0; i < 10; i++)
	{
		dirFile d{ 0 };
		memcpy_s(&d, 1024, file_head[curInode->file_id[i]], 1024);
		for (int u = 0; u < 32; u++)
		{
			if (d.aMap[u].Name[0])
			{
				if (strcmp(d.aMap[u].Name, _PyUnicode_AsString(argv)) == 0)
				{
					//new rm method
					return NULL;
				}
			}
			else
			{
				std::cout << "no such file" << std::endl;
				return false;
			}
		}
	}
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

void InitRootPath()
{
	//homeInode ��inode���ĵ�һ��inode
	inode homeInode{ 0 };
	homeInode.createTime = 1492244880;
	//home dir ���ļ����ĵ�һ��dir file
	AssignFile(&homeInode);
	AssignInode(&homeInode);
	curInode = (inode*)inode_head;
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
		inode_head = disk + 1;
		fileBitmap_head = disk + 1025;
		inodeBitmap_head = disk + 1038;
		file_head = disk + 1040;
		memcpy_s(inodeBitmap_head, 1, &abit_quick[0], 1);
		memcpy_s(fileBitmap_head, 1, &abit_quick[0], 1);
		InitRootPath();
		for (int i = 0; i < blockNum; i++)
		{
			fdisk.write(disk[i], 1024);
		}
		fdisk.close();
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
			mFuncPtr[_PyUnicode_AsString(PyTuple_GetItem(po, 0))]();
		}
		else if (PyObject_Size(po) == 2)
		{
			argv = PyTuple_GetItem(po, 1);
			mFuncPtr[_PyUnicode_AsString(PyTuple_GetItem(po, 0))]();
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

	//����python�ű�

	pModule = PyImport_ImportModule("Pack");
	pFunc = PyObject_GetAttrString(pModule, "Func");
	while (true)
	{
		ShowCurPath();
		ExecuteBat(PyEval_CallObject(pFunc, NULL));
	}
	Py_Finalize();
	delete[] disk;
	return 0;
}