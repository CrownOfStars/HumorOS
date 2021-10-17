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
				//���inode����ϸ��Ϣ
				inode* inodep = new inode();
				virtualDiskReader<inode>(inodep, inodeStart, curDir[i].aMap[u].inodeId);
				std::cout << "����ʱ��" << inodep->createTime << std::endl;
				std::cout << "������" << inodep->mode_uid << std::endl;
				std::cout << "�ļ���С" << inodep->fileSize << std::endl;
				std::cout << "�ļ����:\n";
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
	if (strcmp("..", _PyUnicode_AsString(argv)) == 0)//�ص���һ��Ŀ¼
	{
		if (!pathDeque.empty())//���·�����в�Ϊ��
		{
			pathDeque.pop_back();
			if (!pathDeque.empty())//�ص���Ŀ¼��i���
			{
				int inode_id = pathDeque.back().second;
				virtualDiskReader<inode>(curInode, inodeStart, inode_id);
			}
			else//����ص������
			{
				virtualDiskReader<inode>(curInode, inodeStart, 0);//�ص���Ŀ¼��i���
			}
		}
		dirFile* p = curDir;
		curDir = curInode->dir();
		curDirSize = curInode->dirSize();
		delete[] p;//����curDir
		return 0;
	}
	else if (strcmp("/", _PyUnicode_AsString(argv)) == 0)//�ص���Ŀ¼
	{
		pathDeque.clear();
		virtualDiskReader<inode>(curInode, inodeStart, 0);//�ص���Ŀ¼��i���
		dirFile* p = curDir;
		curDir = curInode->dir();
		curDirSize = curInode->dirSize();
		delete[] p;//����curDir
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
					delete[] p;//����curDir
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
	std::cout << "����  �����ַ  ������ �ļ���С �ļ���" << std::endl;//�����ͷ

	for (int i = 0; i < curDirSize; i++)
	{
		for (int u = 0; u < 32; u++)
		{
			if (curDir[i].aMap[u].Name[0] == '-')//�Ѿ����Ƴ����ļ�(��)
			{
				continue;
			}
			else if (!curDir[i].aMap[u].Name[0])//�Ѿ�������β��׼��д��
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
			else if (curDir[i].aMap[u].Name[0] == '-')//���Ƴ����ļ���
			{
				if (!imapp)//��дָ�붨λ���˴�
				{
					imapp = curDir[i].aMap + u;
					pos = i;
				}
			}
			else if (!curDir[i].aMap[u].Name[0])//�Ѿ�������β��׼��д��
			{
				if (!imapp)
				{
					imapp = curDir[i].aMap + u;
					pos = i;
				}

				inode* inodep = new inode{ 0 };
				inodep->init(dir_Inode);

				strcpy_s(imapp->Name, _PyUnicode_AsString(argv));//���µ�ǰĿ¼�µ�������
				imapp->inodeId = FillInodeBitmap();//Ϊ��Ŀ¼����inode�����Լ�inodeλͼ��
				//ÿ���޸ĺ��ٰ�curDirͬ�����ļ���

				virtualDiskWriter<inode>(inodep, inodeStart, imapp->inodeId);
				//inodeд�뵽������
				virtualDiskWriter<dirFile>(&curDir[pos],fileStart, curInode->Id(pos));
				//ͬʱ���¸�Ŀ¼�ļ�
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
				addRm(inodeType, curDir[i].aMap[u].inodeId);//�Ƴ����ļ��е�inodeλͼ
				
				inode* p = new inode();
				virtualDiskReader<inode>(p, inodeStart, curDir[i].aMap[u].inodeId);
				
				dirFile* pdir = p->dir();
				RemoveDFS(p,pdir,p->dirSize());//�ݹ�ɾ�����ļ����µ���Ŀ¼
				
				for (int i = 0; i < p->dirSize(); i++)
				{//��Ҫɾ��p������ָ���file
					addRm(fileType, p->Id(i));
				}
				runRm();//ִ��ɾ������
				
				delete p;
				delete pdir;
				virtualDiskWriter<dirFile>(&curDir[i], fileStart, curInode->Id(i));
				//ͬʱ���¸�Ŀ¼�ļ�
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
				if (!imapp)//��дָ�붨λ���˴�
				{
					imapp = curDir[i].aMap + u;
					pos = i;
				}
			}
			else if (!curDir[i].aMap[u].Name[0])
			{
				if (!imapp)//��дָ�붨λ���˴�
				{
					imapp = curDir[i].aMap + u;
					pos = i;
				}
				inode* inodep = new inode{ 0 };///
				inodep->init(file_Inode);
				strcpy_s(imapp->Name, _PyUnicode_AsString(argv));
				imapp->inodeId = FillInodeBitmap();

				virtualDiskWriter<inode>(inodep, inodeStart, imapp->inodeId);//д�뵽������

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

int Copy()//�����ݸ��Ƶ�cache��
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
	if (true)//_access("DISK", 0))//����ļ�������
	{
		File* disk = new File[1024 * 100];
		std::ofstream fdisk("DISK", std::ios::binary);
		memcpy_s(disk[0], 1024, (File*)&supBlock, 1024);
		for (int i = 1; i < 1024 * 100; i++)
		{
			memset(disk[i], 0, 1024);
		}
		inode homeInode;//homeInode ��inode���ĵ�һ��inode
		homeInode.createTime = 1492244880ll;
		homeInode.file_id[0] = 1;
		memcpy_s(curInode, sizeof(inode), &homeInode, sizeof(inode));//��Ϊ��ǰ��inode
		dirFile* homedir = new dirFile();
		curDir = homedir;//��Ϊ��ǰ��dir
		memcpy_s(disk + 1, 1024, &homeInode, sizeof(inode));//д���ļ�
		memcpy_s(disk + fileBmStart, 1024, &bitQuick[1], sizeof(char));
		//homedirռ��1��,�ļ���Ϊ0��ʾ���ļ�������
		memcpy_s(disk + inodeBmStart, 1024, &bitQuick[0], sizeof(char));
		//homeinodeռ��0��

		fdisk.write((char*)disk, 1024 * 1024 * 100);//��¼�����ش���
		fdisk.close();
		delete[] disk;//��ճ�ʼ������
	}
	else
	{
		virtualDiskReader<SuperBlock>(&supBlock, 0, 0);//��ȡ������
		
		/*
		memcpy_s(curInode, sizeof(inode), &homeInode, sizeof(inode));//��Ϊ��ǰ��inode

		dirFile* homedir = new dirFile();
		curDir = homedir;//��Ϊ��ǰ��dir
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
	//��ʼ�������Ѿ�python����
	PyObject* pModule = NULL;
	PyObject* pFunc = NULL;

	pModule = PyImport_ImportModule("Pack");
	pFunc = PyObject_GetAttrString(pModule, "xFunc");
	//�������������
	while (true)
	{
		ShowCurPath();
		ExecuteBat(PyEval_CallObject(pFunc, NULL));
	}
	Py_Finalize();
	return 0;
}