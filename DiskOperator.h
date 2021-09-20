#pragma once
#include "bit_quick.h"
#include "DataStructure.h"

inode* curDirInode = new inode();

int* FirstFit(File* bmap,int mapSize, int assignSize)//索引分配
{
	int* assignFile_id = new int[assignSize];//将要分配的文件id的列表
	int curAssigned = 0;//当前已分配的项
	for (int i = 0; i < mapSize; i++)//遍历各位图块
	{
		for (int u = 0; u < 1024; u++)
		{
			for (int t = 0; t < 8; t++)//逐位查找
			{
				if (!((*bmap)[u] & (abit_quick[0] >> t)))//如果该位为0
				{
					(*bmap)[u] |= (abit_quick[0] >> t);//把该位设置为1
					assignFile_id[curAssigned] = i*(1024*8)+u*8+t;//记录该位的位置
					curAssigned++;
					if (curAssigned == assignSize)//满足条件，结束
					{
						return assignFile_id;
					}
				}
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
	throw("error 资源不足，不可分配");
	return nullptr;//资源不足，不可分配
}

void RmFromBitMap(File* bmap, int* rmList, int rmSize)
{
	for (int i = 0; i < rmSize; i++)
	{
		int byte_id = rmList[i] / 8;
		int offset = rmList[i] % 8;
		*((char*)bmap + byte_id) &= ~(abit_quick[0] >> offset);
	}
}

int FirstFit(File* bmap, BitmapOp bmo = BitmapOp::ReadOnly)
{
	for (int u = 0; u < 1024; u++)
	{
		for (int t = 0; t < 8; t++)
		{
			if (!((*bmap)[u] & (abit_quick[0] >> t)))
			{
				switch (bmo) {
				case BitmapOp::SetTrue: {
					(*bmap)[u] |= (abit_quick[0] >> t);
					break;
				}
				case BitmapOp::SetFalse: {
					(*bmap)[u] &=~(abit_quick[0] >> t);
				}
				case BitmapOp::ReadOnly: {
					break;
				}
				case BitmapOp::Reverse: {
					throw("error 鸽");
					break;
				}
				}
				return u*8+t;
			}
		}
	}
	
	throw("bitmap fit error");
	return -1;
}

int SearchInBitmap(File* region, int bitmapSize, BitmapOp bmo)
{
	int idx = 0;
	for (int i = 0; i < bitmapSize; i++)
	{
		idx = FirstFit(region+i, bmo);
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
	int inode_id = SearchInBitmap(inodeBitmap_head, inodeBitmapSize, BitmapOp::SetTrue);
	int offset = inode_id % 16;
	memcpy_s((inode_head+inode_id/16)[offset*64], 64, inodep, 64);
	return inode_id;
}

void RmInode(int inode_id)
{
	inode emp{ 0 };
	int offset = inode_id % 16;
	memcpy_s(inode_head[inode_id / 16] + offset * 64, 64, &emp, 64);
	//SetBitmap(inodeBitmap_head, inode_id, false);
}

void AssignFile(inode* inodep)
{
	int file_id = SearchInBitmap(fileBitmap_head, blockBitmapSize, BitmapOp::SetTrue);
	for (int i = 0;i < 10;i++)
	{
		if (!inodep->baseFile_id[i])
		{
			inodep->baseFile_id[i] = file_id;//
			return;
		}
	}
	throw("error 鸽");
	//inodep->baseFile_id;
}
