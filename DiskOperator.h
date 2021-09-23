#pragma once
#include "DataStructure.h"
#include <fstream>

unsigned char abit_quick[8]{
	unsigned char(0b10000000),
	unsigned char(0b11000000),
	unsigned char(0b11100000),
	unsigned char(0b11110000),
	unsigned char(0b11111000),
	unsigned char(0b11111100),
	unsigned char(0b11111110),
	unsigned char(0b11111111)
};

inode* curInode = nullptr;

int* FirstFit(File* bmap,int mapSize, int assignSize)//��������
{
	int* assignFile_id = new int[assignSize];//��Ҫ������ļ�id���б�
	int curAssigned = 0;//��ǰ�ѷ������
	for (int i = 0; i < mapSize; i++)//������λͼ��
	{
		for (int u = 0; u < 1024; u++)
		{
			for (int t = 0; t < 8; t++)//��λ����
			{
				if (!((*bmap)[u] & (abit_quick[0] >> t)))//�����λΪ0
				{
					(*bmap)[u] |= (abit_quick[0] >> t);//�Ѹ�λ����Ϊ1
					assignFile_id[curAssigned] = i*(1024*8)+u*8+t;//��¼��λ��λ��
					curAssigned++;
					if (curAssigned == assignSize)//��������������
					{
						return assignFile_id;
					}
				}
			}
		}
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
	throw("error ��Դ���㣬���ɷ���");
	return nullptr;//��Դ���㣬���ɷ���
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
					throw("error ��");
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
	memcpy_s(pOffset(inode_head + inode_id / 16, offset * 64), 64, inodep, 64);
	return inode_id;
}

void RmInode(int inode_id)
{
	inode emp{ 0 };
	int offset = inode_id % 16;
	memcpy_s(pOffset(inode_head + inode_id / 16, offset * 64), 64, &emp, 64);
	//SetBitmap(inodeBitmap_head, inode_id, false);
}

void AssignFile(inode* inodep)
{
	int file_id = SearchInBitmap(fileBitmap_head, blockBitmapSize, BitmapOp::SetTrue);
	for (int i = 0;i < 10;i++)
	{
		if (!inodep->file_id[i])
		{
			inodep->file_id[i] = file_id;//
			return;
		}
	}
	throw("error ��");
	//inodep->file_id;
}

void createMigrateFile()
{
	std::fstream in("cache", std::ios::in);
	std::istreambuf_iterator<char>beg(in), end;
	std::string strdata(beg, end);
	in.close();
	//write strdata to virtual disk
}