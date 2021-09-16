#pragma once
#include "bit_quick.h"
#include "DataStructure.h"

inode* curDirInode = new inode();

//void RemoveBitmap(File* region, int bitmap_id, bool occupy) {
//	int disk_id = bitmap_id / (1024 * 8);
//	int disk_offset = bitmap_id % (1024 * 8);
//	int i = disk_offset / 32;//unit的编号
//	int offset = disk_offset % 32;
//	(*region)[i] |= (abit_quick[0] >> offset);
//	/*
//	int i = pos / 32;
//	int j = pos % 32;
//	int value = 0;
//	a[i] &= (~value ^ (1 << j));
//	*/
//}

int FirstFit(File* bmap, bool cover = false)
{
	for (int u = 0; u < 1024; u++)
	{
		for (int t = 0; t < 8; t++)
		{
			if (!((*bmap)[u] & (abit_quick[0] >> t)))
			{
				if (cover) {
					(*bmap)[u] |= (abit_quick[0] >> t);
				}
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

int SearchInBitmap(File* region, int bitmapSize, bool occupy = false)
{
	for (int i = 0; i < bitmapSize; i++)
	{
		int idx = FirstFit(region+i, occupy);
		if (idx > 0)
		{
			return idx;
		}
	}
	throw("not found error");
	return -1;
}

int AssignInode(inode* inodep)
{
	int inode_id = SearchInBitmap(inodeBitmap_head, inodeBitmapSize, true);
	int offset = inode_id % 16;
	memcpy_s(inode_head[inode_id / 16] + offset * 64, 64, inodep, 64);
	return inode_id;
}

void RmInode(int inode_id)
{
	inode emp{ 0 };
	int offset = inode_id % 16;
	memcpy_s(inode_head[inode_id / 16] + offset * 64, 64, &emp, 64);
	//SetBitmap(inodeBitmap_head, inode_id, false);
}

int AddDir(std::string dirName, int inode_id)
{
	int file_id = SearchInBitmap(fileBitmap_head, 13, true);
	dirFile tempdir{ 0 };
	memcpy_s(&tempdir, 1024, (dirFile*)file_head[file_id], 1024);
	for (int i = 0; i < 32; i++)
	{
		if (!tempdir.aMap[i].Name[0])
		{
			strcpy_s(tempdir.aMap[i].Name, dirName.c_str());
			tempdir.aMap[i].inodeIndex = inode_id;
			memcpy_s(file_head[file_id], 1024, (File*)&tempdir, 1024);
			return file_id;
		}
	}
	return 0;
}

void LinkDir(int file_id)
{
	for (int i = 0; i < 10; i++)
	{
		if (!curDirInode->primaryIndex[i]) {
			curDirInode->primaryIndex[i] = file_id;
			return;
		}
	}
	throw("鸽 ERROR");
}