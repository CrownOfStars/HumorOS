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
					(*bmap)[u] &= ~(abit_quick[0] >> t);
				}
				case BitmapOp::ReadOnly: {
					break;
				}
				case BitmapOp::Reverse: {
					throw("error 鸽");
					break;
				}
				}
				return u * 8 + t;
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

int SearchInBitmap(File* region, int bitmapSize, BitmapOp bmo)
{
	int idx = 0;
	for (int i = 0; i < bitmapSize; i++)
	{
		idx = FirstFit(region + i, bmo);
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
	memcpy_s((inode_head + inode_id / 16)[offset * 64], 64, inodep, 64);
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
	for (int i = 0; i < 10; i++)
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